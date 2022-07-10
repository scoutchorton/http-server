/**
 * utillib.h - Extra utilities library header for web server
 */

#ifndef UTILLIB_H
#define UTILLIB_H

#define FILE_CMD "file -ib "

char* get_mime_type(char*);

#define INTERNAL_ERR(msg) \
	fprintf(stderr, msg); \
	fprintf(stderr, "Responding with 500 status code...\n"); \
	response_data = create_res(500, "Internal Server Error"); \
	send_res_header(data_sock_fd, response_data); \
	close(data_sock_fd); \
	continue;
#define NOT_FOUND_ERR(msg) \
	fprintf(stderr, msg); \
	fprintf(stderr, "Responding with 404 status code...\n"); \
	response_data = create_res(404, "Not Found"); \
	send_res_header(data_sock_fd, response_data); \
	close(data_sock_fd); \
	continue;
#define FORBIDDEN_ERR(msg) \
	fprintf(stderr, msg); \
	fprintf(stderr, "Responding with 403 status code...\n"); \
	response_data = create_res(403, "Forbidden"); \
	send_res_header(data_sock_fd, response_data); \
	close(data_sock_fd); \
	continue;

#endif // UTILLIB_H