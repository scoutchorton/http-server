/**
 * utillib.c - Extra utilities library for web server
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utillib.h"

/**
 * get_mime_type - Get the MIME type of a file
 * 
 * USAGE
 *   char* get_mime_type(char* path)
 * 
 * ARGUMENTS
 *   path - Path to file to get the MIME type of
 * 
 * RETURN VALUE
 *   A string with the MIME type, or NULL on error
 */
char* get_mime_type(char* path) {
	char* mime_type;
	char* cmd;
	FILE* file_proc;

	//The type and subtype can be 127 characters long, plus the slash and null
	//byte, resulting in a 256 long string
	//https://tools.ietf.org/html/rfc4288#section-4.2
	//Allocate space for the MIME type string
	if((mime_type = calloc(256, sizeof(char))) == NULL)
		return NULL;

	//Check if file exists
	if(access(path, R_OK) != 0)
		return NULL;

	//Allocate space for string and get a final command
	cmd = calloc(strlen(FILE_CMD) + strlen(path) + 1, sizeof(char));
	if(cmd == NULL)
		return NULL;
	strcat(cmd, FILE_CMD);
	strcat(cmd, path);

	//Attempt to open the file process
	if((file_proc = popen(cmd, "r")) == NULL)
		return NULL;
	
	//Read data from file process
	fread(mime_type, 256, sizeof(char), file_proc);

	//Close process after reading
	pclose(file_proc);

	return mime_type;
}