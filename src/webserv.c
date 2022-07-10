/**
 * webserv.c - A web server that feeds files from the current directory
 * 
 * USAGE
 *   webserv
 */

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "netlib.h"
#include "utillib.h"

#define DEFAULT_PORT 8080
#define DEBUG_RES "<h1 style=\"font-family: cursive;\">Welcome to McDonalds, how may I help you?</h1>\n"

void inthandler(int);

int main(int argc, char** argv) {
	//Variables
	int sock_fd;
	int port = DEFAULT_PORT;
	int data_sock_fd;
	http_req_t* request_data;
	http_res_t* response_data;
	char* data_path;
	char* mime_type;
	int data_fd;

	//Handle errors gracefully
	signal(SIGINT, inthandler);
	signal(SIGSEGV, inthandler);

	//Change port if first argument is a port
	if(argc > 1) {
		port = atoi(argv[1]);
		if(port == 0) {
			fprintf(stderr, "%s: Invalid port %s\n", argv[0], argv[1]);
			exit(1);
		}
	}

	//Create socket
	printf("Initalizing socket on port %i...\n", port);
	if((sock_fd = init_socket(port)) == -1)
		exit(1);

	//Proces data
	while(1) {
		//Accept data
		if((data_sock_fd = accept(sock_fd, NULL, NULL)) == -1) {
			perror("accept");
			continue;
		}
		printf("### REQUEST ###\n");

		//Parse data
		if((request_data = read_req(data_sock_fd)) == NULL) {
			fprintf(stderr, "error processing request headers\n");
			continue;
		}
		printf(" Method: %s\n", request_data->method);
		printf("    URI: %s\n", request_data->uri);
		printf("Version: %hhi.%hhi\n", request_data->major_ver, request_data->minor_ver);

		//Process requests
		if(!strcmp(request_data->method, "GET")) {
			printf("PROCESSING GET\n");

			//Allocate space to turn URI into local path
			data_path = calloc(strlen(request_data->uri) + 2, sizeof(char));
			if(data_path == NULL) {
				INTERNAL_ERR("Error allocating space for path.\n")
			}
			strcat(data_path, ".");
			strcat(data_path, request_data->uri);

			printf("Local path to file: %s\n", data_path);

			//Check if file exists and can be accessed
			if((access(data_path, F_OK)) != 0) {
				NOT_FOUND_ERR("Could not find file.\n")
			//Check if file exists and can be read
			} else if((access(data_path, R_OK)) != 0) {
				FORBIDDEN_ERR("Cannot read file.\n")
			}

			//Prepare response data
			response_data = create_res(200, "OK");

			//Let's abuse the file command
			mime_type = get_mime_type(data_path);
			if(mime_type == NULL) {
				INTERNAL_ERR("Error getting MIME type.\n")
			}

			//Set MIME type accordingly
			response_data->headers = create_header("Content-Type", mime_type);

			//Open file and send to client
			if((data_fd = open(data_path, O_RDONLY)) == -1) {
				NOT_FOUND_ERR("Could not open file.\n")
			}
			send_res_fd(data_sock_fd, response_data, data_fd);
		}

		//Complete connection
		close(data_sock_fd);
	}

	//End code
	return 0;
}
void inthandler(int sig) {
	exit(1);
}