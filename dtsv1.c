/* Title: CIS 415 Project 3
 * 
 * Description: DTS Echos the request it receives.
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
#define USAGE "./dtsv1"

static void* dtsv1() {
	/* Echo back received request along with a status byte;
	 * the first byte of the response is 1 for success
	 * and 0 for failure.*/
	BXPEndpoint sender;
	char *query = (char *)malloc(BUFSIZ);
	char *resp = (char *)malloc(BUFSIZ);
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
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		sprintf(resp, "1%s", query);
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		bxp_response(bxps, &sender, resp, strlen(resp) + 1);
	}
	free(query);
	free(resp);
	return EXIT_SUCCESS;
}

int main(UNUSED int argc, UNUSED char *argv[]) {
	/*Create and initialize thread.*/
	pthread_t dtsv1_thread;
	if(pthread_create(&dtsv1_thread, NULL, dtsv1, NULL)) {
		fprintf(stderr, "Error creating thread.\n");
		exit(EXIT_FAILURE);
	}
	pthread_join(dtsv1_thread, NULL);
	return EXIT_SUCCESS;
}
