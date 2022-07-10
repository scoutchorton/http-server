/**
 * netlib.h - HTTP and networking library header for web server
 */

#include <limits.h>
#include <stdint.h>

#ifndef NETLIB_H
#define NETLIB_H 1

/**
 * HTTP Requests
 */

//HTTP codes
#define HTTP_OK 200
#define HTTP_NOT_FOUND 404
#define HTTP_IM_A_TEAPOT 418

//Implements headers for requests and responses
typedef struct _http_header {
	char* name;
	char* value;
	struct _http_header* next;
} http_header_t;

//Implements a request line:
//https://tools.ietf.org/html/rfc2068#section-5.1
typedef struct _http_req {
	char method[8]; //Large enough to store "OPTIONS\0"
	char uri[PATH_MAX]; //Since the URI is a path, let's roll with that
	                    //The HTTP RFC doesn't give a maximum length
						//https://tools.ietf.org/html/rfc2068#section-3.2
	uint8_t major_ver;
	uint8_t minor_ver;
} http_req_t;

//Implements a status line:
//https://tools.ietf.org/html/rfc2068#section-6.1
typedef struct _http_res {
	uint8_t major_ver;
	uint8_t minor_ver;
	int status_code;
	char* reason;
	http_header_t* headers;
} http_res_t;

http_req_t* read_req(int);
http_res_t* create_res(int, char*);
http_header_t* create_header(char*, char*);
int send_res_fd(int, http_res_t*, int);
int send_res_header(int fd, http_res_t* res);
int send_res(int, http_res_t*, uint8_t*, int);

/**
 * Misc
 */

int init_socket(int);

#endif // NETLIB_H