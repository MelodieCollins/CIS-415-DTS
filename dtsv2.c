/* Title: CIS 415 Project 3
 *
 * Description: DTS parses the requests and sets the satus byte.
 *
 * Author: Melodie Collins
 * DuckID: mcolli11
 * This is my own work.
 */

#include "BXP/bxp.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <valgrind/valgrind.h>

#define UNUSED __attribute__((unused))

#define PORT 19999
#define SERVICE "DTS"
#define USAGE "./dtsv2"

static void* dtsv2() {
	BXPEndpoint sender;
	char *query = (char *)malloc(BUFSIZ);
	char *resp = (char *)malloc(BUFSIZ);
	
	char *q_copy = (char *)malloc(BUFSIZ);
	
	char *cmd, *rest;
	unsigned len;
	BXPService bxps;
	char *service;
	unsigned short port;

	service = SERVICE;
	port = PORT;
	assert(bxp_init(port, 1));
	bxps = bxp_offer(service);
	if(bxps == NULL) {
		fprintf(stderr, "Failure offering Echo service.\n");
		free(query);
		free(resp);
		exit(EXIT_FAILURE);
	}
	VALGRIND_MONITOR_COMMAND("leak_check summary");
	while((len = bxp_query(bxps, &sender, query, BUFSIZ)) > 0) {
		/* Check that the first word of each request 
		 * is one of the three legal strings:
		 * OneShot, Repeat, Cancel
		 *
		 * Check length of args is correct
		 * Cancel: 1 arg
		 * Repeat and  OneShot: 6 args
		 * 
		 * if both first word and arg length are true:
		 * sprintf(resp, "1%s", query)
		 * else: sprintf(resp, "0")*/
		
		// Split/parse query string
		query[len] = '\0';
		// Make a copy of query
		strcpy(q_copy, query);
		cmd = q_copy;
		// Get first word
		rest = strtok(cmd, "|");

		int count = 0;
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		if(strcmp(rest, "OneShot") == 0) {
			while(rest != NULL) {
				// Increase space count
				count++;
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 7) { // Account for space at end
				sprintf(resp, "1%s", query);
			}
			else {
				sprintf(resp, "0");
			}
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		}else if(strcmp(rest, "Repeat") == 0) {
			while(rest != NULL){
				// Increase space count
				count++;
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 7){
				sprintf(resp, "1%s", query);
			}
			else {
				sprintf(resp, "0");
			}
		VALGRIND_MONITOR_COMMAND("leak_check summary");	
		}else if(strcmp(rest, "Cancel") == 0) {
			while(rest != NULL){
				// Increase space count
				count++;
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 2){
				sprintf(resp, "1%s", query);
			}
			else {
				sprintf(resp, "0");
			}

		VALGRIND_MONITOR_COMMAND("leak_check summary");
		}else {
			sprintf(resp, "0");
		}
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		bxp_response(bxps, &sender, resp, strlen(resp) + 1);
	}
	free(query);
	free(resp);
	return EXIT_SUCCESS;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
	/*Create and initialize thread.*/
	pthread_t dtsv2_thread;
	if(pthread_create(&dtsv2_thread, NULL, dtsv2, NULL)) {
		fprintf(stderr, "Error creating thread.\n");
		exit(EXIT_FAILURE);
	}
	pthread_join(dtsv2_thread, NULL);
	return EXIT_SUCCESS;
}
