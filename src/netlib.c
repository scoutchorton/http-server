/**
 * netlib.c - HTTP and networking library for web server
 */

#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "netlib.h"

/**
 * create_res - Create a HTTP response
 * 
 * USAGE
 *   http_req_t read_req(int fd)
 * 
 * ARGUMENTS
 *   fd - File descriptor to read from
 * 
 * RETURN VALUE
 *   Pointer to request data or NULL on error
 */
http_req_t* read_req(int fd) {
	http_req_t* req;
	FILE* sock_stream;
	char ignore_buf[BUFSIZ];
	char* tmp;
	char c;

	//Allocate space for request
	if((req = malloc(sizeof(http_req_t))) == NULL)
		return NULL;

	//Convert file descriptor to file stream
	if((sock_stream = fdopen(dup(fd), "r")) == NULL)
		return NULL;
	
	//Read data to request data
	//https://tools.ietf.org/html/rfc2068#section-5.1
	fscanf(sock_stream, "%7s ", req->method);
	tmp = req->uri;
	while((c = fgetc(sock_stream)) != EOF && c != ' ') {
		*tmp = c;
		tmp++;
	}
	*tmp = '\0';
	fscanf(sock_stream, "HTTP/%hhi.%hhi\r\n", \
	       &req->major_ver, &req->minor_ver);

	//Ignore headers for now
	while(fgets(ignore_buf, BUFSIZ, sock_stream) != NULL &&
	      strcmp("\r\n", ignore_buf) != 0);

	//Close file stream
	fclose(sock_stream);

	return req;
}

/**
 * create_res - Create a HTTP response
 * 
 * USAGE
 *   http_res_t create_res(int status, char* reason)
 * 
 * ARGUMENTS
 *   status - HTTP status
 *   reason - Text to explain the status
 * 
 * RETURN VALUE
 *   A pointer to repsonse data or NULL on error
 */
http_res_t* create_res(int status, char* reason) {
	http_res_t* res;

	//Attempt to allocate space for response
	if((res = malloc(sizeof(http_res_t))) == NULL)
		return NULL;

	//Set up data
	res->major_ver = 1;
	res->minor_ver = 1;
	res->status_code = status;
	res->reason = reason;
	res->headers = NULL;

	return res;
}

/**
 * create_header - Create a HTTP response
 * 
 * USAGE
 *   http_header_t* create_header(char* name, char* value)
 * 
 * ARGUMENTS
 *   name - Name of the header
 *   value - Data to go with header
 * 
 * RETURN VALUE
 *   A pointer to a header struct or NULL on error
 */
http_header_t* create_header(char* name, char* value) {
	http_header_t* header;

	//Attempt to allocate space for response
	if((header = malloc(sizeof(http_header_t))) == NULL)
		return NULL;

	//Set up data
	header->name = name;
	header->value = value;
	header->next = NULL;

	return header;
}

/**
 * send_res_fd - Send a HTTP response from a file descriptor
 * 
 * USAGE
 *   int send_res_fd(int fd, http_res_t* res, int fd)
 * 
 * ARGUMEMTS
 *   fd - File descriptor to send data on
 *   res - Response status data
 *   src_fd - File descriptor to read data from
 * 
 * RETURN VALUE
 *   0 on success, -1 on error
 */
int send_res_fd(int fd, http_res_t* res, int src_fd) {
	FILE* sock_stream;
	FILE* src_stream;
	int c;

	//Argument checks
	if(src_fd < 0)
		return -1;

	//Convert file descriptor to file stream
	if((sock_stream = fdopen(dup(fd), "w")) == NULL)
		return -1;
	if((src_stream = fdopen(dup(src_fd), "r")) == NULL)
		return -1;

	//Send header
	if(send_res_header(fd, res) == -1)
		return -1;

	//Read data
	while((c = fgetc(src_stream)) != EOF)
		fputc(c, sock_stream);

	//Close files
	fclose(sock_stream);
	fclose(src_stream);
	
	return 0;
}

/**
 * send_res_header - Send a HTTP response header before sending data
 * 
 * USAGE
 *   int send_res_header(int fd, http_res_t* res)
 * 
 * ARGUMEMTS
 *   fd - File descriptor to send data on
 *   res - Response status data
 * 
 * RETURN VALUE
 *   0 on success, -1 on error
 */
int send_res_header(int fd, http_res_t* res) {
	FILE* sock_stream;
	http_header_t* cur_header;

	//Convert file descriptor to file stream
	if((sock_stream = fdopen(dup(fd), "w")) == NULL)
		return -1;

	//Write status line
	fprintf(sock_stream, "HTTP/%i.%i %i %s\r\n", \
	        res->major_ver, res->minor_ver, res->status_code, res->reason);
	for(cur_header = res->headers; cur_header != NULL;
	    cur_header = cur_header->next)
		fprintf(sock_stream, "%s: %s\r\n", cur_header->name, cur_header->value);
	fprintf(sock_stream, "\r\n");
	
	//Close file stream
	fclose(sock_stream);

	return 0;
}

/**
 * send_res - Send a HTTP response
 * 
 * USAGE
 *   int send_res(http_res_t* header, uint8_t* data, int len)
 * 
 * ARGUMEMTS
 *   fd - File descriptor to send data on
 *   res - Response status data
 *   data - Raw data to send
 *   len - Length of data to send
 * 
 * RETURN VALUE
 *   0 on success, -1 on error
 */
int send_res(int fd, http_res_t* res, uint8_t* data, int len) {
	//Argument checks
	if(fd < 0)
		return -1;
	else if(data == NULL)
		return -1;
	else if(len < 1)
		return -1;

	//Send header
	if(send_res_header(fd, res) == -1)
		return -1;

	//Send data
	if(write(fd, data, len) < len)
		return -1;

	return 0;
}

/**
 * init_socket - Initalize socket for listening on a port
 * 
 * USAGE
 *   int init_socket(int port)
 * 
 * ARGUMENTS
 *   port - Port number to listen on
 * 
 * RETURN VALUE
 *   File descriptor of new socket, or -1 on error
 */
int init_socket(int port) {
	int sock_fd;
	struct sockaddr_in sock_addr;
	struct hostent* host;

	//Request socket
	if((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	//Obtain host information
	host = gethostbyname("0.0.0.0");

	//Setup socket address
	bzero(&sock_addr, sizeof(struct sockaddr_in));
	bcopy(host->h_addr, &sock_addr.sin_addr, host->h_length);
	sock_addr.sin_port = htons(port);
	sock_addr.sin_family = AF_INET;

	//Bind socket with address
	if(bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) != 0) {
		close(sock_fd);
		perror("bind");
		return -1;
	}
	
	//Start listening for information on socket
	if(listen(sock_fd, 1) != 0) {
		close(sock_fd);
		perror("listen");
		return -1;
	}

	return sock_fd;
}