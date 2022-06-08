/* Title: CIS 415 Project 3 
 * 
 * Description: DTS processes the command arguments, stores them in appropriate 
 * data structures, and processes the timer request.
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
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <valgrind/valgrind.h>
#include "ADTs/heapprioqueue.h"
#include "ADTs/queue.h"
#include "ADTs/hashcskmap.h"

#define UNUSED __attribute__((unused))
#define USECS (10 * 1000)
#define PORT 19999
#define SERVICE "DTS"
#define USAGE "./dtsv3"

const PrioQueue *q = NULL;
const CSKMap *m = NULL;
pthread_mutex_t b_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t map_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long svid = 0L;

typedef struct entry{
	struct timeval *time; // time->tv_sec and time->tv_usec
	long msec;
	char* cmd;
	long repeats;
	long unsigned svid;
	long clid;
	char* host;
	char* service;
	unsigned int port;
} Entry;

int entry_cmp(void* time1, void* time2){ // priority function, smallest has priority
	struct timeval *t1 = (struct timeval *)time1;
	struct timeval *t2 = (struct timeval *)time2;
	if(t1->tv_sec > t2->tv_sec){
		return -1; // reverse, want smaller num first
	}
	if(t1->tv_sec < t2->tv_sec){
		return 1; // in order
	}
	if(t1->tv_usec > t2->tv_usec){
		return -1; // reverse, want smaller num first
	}
	if(t1->tv_usec < t2->tv_usec){
		return 1; // in order
	}
	return 0; // they are the same
}

static void* dtsv3() {
	BXPEndpoint sender;
	char *query = (char *)malloc(BUFSIZ);
	char *resp = (char *)malloc(BUFSIZ);	
	char *q_copy = (char *)malloc(BUFSIZ);
	char *cmd, *rest, *command;
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
		// Split/parse query string
		query[len] = '\0';
		// Make a copy of query
		strcpy(q_copy, query);
		cmd = q_copy;
		// Get first word
		rest = strtok(cmd, "|");
		// save copy of command
		command = rest;

		int count = 0;
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		if(strcmp(rest, "OneShot") == 0) {
			// malloc entry struct
			Entry *e = (Entry*)malloc(sizeof(Entry));
			e->cmd = command;
			e->msec = 0L;
			e->repeats = 0L;
			while(rest != NULL) {
				count++;
				if(count == 2){
					e->clid = atoi(rest);
				}
				if(count == 3){
					e->time->tv_sec = atoi(rest);	
				}
				if(count == 4){
					e->time->tv_usec = atoi(rest);
				}
				if(count == 5){
					e->host = rest;
				}
				if(count == 6){
					e->service = rest;
				}
				if(count == 7){
					e->port = (unsigned int)atoi(rest);
				}
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 7) { // Account for space at end
				// increase svid
				e->svid = ++svid;
				// lock
				int locked = pthread_mutex_lock(&b_mutex);
				if(locked == 0){ // successfuly locked
					// put on queue
					bool inserted = q->insert(q, e->time, (void *)e);
					VALGRIND_MONITOR_COMMAND("leak_check summary");
					// if put on queue successfully
					if(inserted == true){
						sprintf(resp, "1%08lu", e->svid);
					} else{
						sprintf(resp, "0");
					}
					// unlock
					pthread_mutex_unlock(&b_mutex);
				}
			}
			else {
				sprintf(resp, "0");
			}
		VALGRIND_MONITOR_COMMAND("leak_check summary");
		}else if(strcmp(rest, "Repeat") == 0) {
			long clid;
			long msec;
			long repeats;
			char* host;
			char* serv;
			unsigned int port;
			while(rest != NULL){
				count++;
				if(count == 2){
					clid = atoi(rest);
				}
				if(count == 3){
					msec = atoi(rest);
				}
				if(count == 4){
					repeats = atoi(rest);
				}
				if(count == 5){
					host = rest;
				}
				if(count == 6){
					serv = rest;
				}
				if(count == 7){
					port = (unsigned int)atoi(rest);
				}
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 7){
				// malloc entry struct
				Entry *e = (Entry*)malloc(sizeof(Entry));
				e->cmd = command;
				e->clid = clid;
				// look up current time
				struct timeval now;
				gettimeofday(&now, NULL);
				// set sec
				e->time->tv_sec = now.tv_sec;
				// set usec
				e->time->tv_usec = now.tv_usec;
				e->msec = msec;
				e->repeats = repeats;
				e->host = host;
				e->service = serv;
				e->port = port;

				// increase svid
				e->svid = ++svid;
				// lock
				int locked = pthread_mutex_lock(&b_mutex);
				if(locked == 0){ // successfuly locked
					// put on queue
					bool inserted = q->insert(q, e->time, (void *)e);
					VALGRIND_MONITOR_COMMAND("leak_check summary");	
					// if put on queue successfully
					if(inserted == true){
						sprintf(resp, "1%08lu", e->svid);
					} else{
						sprintf(resp, "0");
					}
					// unlock
					pthread_mutex_unlock(&b_mutex);
				}
			}
			else {
				sprintf(resp, "0");
			}
		VALGRIND_MONITOR_COMMAND("leak_check summary");	
		}else if(strcmp(rest, "Cancel") == 0) {
			char svid_str [20];
			while(rest != NULL){
				count++;
				if(count == 2){
					strcpy(svid_str, rest);	
				}
				// Move to next arg
				rest = strtok(NULL, "|");
			}
			if(count == 2){
				// lock map
				pthread_mutex_lock(&map_mutex);
				m->put(m, svid_str, NULL);
				// unlock map
				pthread_mutex_unlock(&map_mutex);
				
				sprintf(resp, "%08lu", (unsigned long)atoi(svid_str));
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

bool isReady(struct entry *e){
	struct timeval now;
	gettimeofday(&now, NULL);
	if(e->time->tv_sec > now.tv_sec){
		return false;
	}
	if(e->time->tv_sec < now.tv_sec){
		return true;
	}
	if(e->time->tv_usec > now.tv_usec){
		return false;
	}
	if(e->time->tv_usec < now.tv_usec){
		return true;
	}
	return true;
}

void *timer() {
	while(1) {
			// while(peek->in past)
			while(1){
				struct entry *e;
				void* priority;
				pthread_mutex_lock(&b_mutex);
				// peak
				bool success = q->min(q, (void **)&priority, (void **)&e);
				if(!success || !isReady(e)){ //q is empty
					// unlock
					pthread_mutex_unlock(&b_mutex);
					break;
				}
				// remove struct
				q->removeMin(q, (void **)&priority, (void **)&e);
				// unlock
				pthread_mutex_unlock(&b_mutex);
				
				// lock map
				int map_locked = pthread_mutex_lock(&map_mutex);
				if(map_locked == 0){
					//convert svid to str
					char buf [20];
					sprintf(buf, "%ld", e->svid);
					if(m->containsKey(m, buf)){ // if svid in map
				 		// remove from map
						m->remove(m, buf);
				 		// unlock map
						pthread_mutex_unlock(&map_mutex);
						break; // skip this command (its cancelled)
					}
					// unlock map
					pthread_mutex_unlock(&map_mutex);
				}
				
				printf("Event fired: %lu|%s|%s|%u\n", e->clid, e->host, e->service, e->port);
				
				//e->repeats--;
				if(e->repeats > 0){
					e->repeats--;
					// compute new time
					struct timeval now;
					gettimeofday(&now, NULL);
					// add msec to now
					double time_in_msec = (now.tv_sec) * 1000 +(now.tv_usec) / 1000.0;
					time_in_msec += e->msec; //milliseconds
					// set e->sec
					e->time->tv_sec = (long)time_in_msec / 1000;
					// set e->usec
					e->time->tv_usec = (long)(time_in_msec - (e->time->tv_sec * 1000) * 1000);
					// push struct back on
					pthread_mutex_lock(&b_mutex);
					q->insert(q, e->time, (void *)e);
					// unlock
					pthread_mutex_unlock(&b_mutex);
				}
				VALGRIND_MONITOR_COMMAND("leak_check summary");	
			}
			// sleep
			usleep(USECS);
		
	}
}

int main(UNUSED int argc, UNUSED char *argv[]) {
	q = HeapPrioQueue(entry_cmp, doNothing, doNothing);
	m = HashCSKMap(100, 0.8, doNothing);
	
	/*Create and initialize threads.*/
	pthread_t dtsv3_thread; // read, lock, put stuff on queue, unlock
	if(pthread_create(&dtsv3_thread, NULL, dtsv3, NULL) != 0) {
		fprintf(stderr, "Error creating thread.\n");
		exit(EXIT_FAILURE);
	}
	
	pthread_t timer_thread;
	if(pthread_create(&timer_thread, NULL, timer, NULL) != 0) {
		fprintf(stderr, "Can't start timer thread.\n");
		exit(EXIT_FAILURE);
	}
	pthread_join(dtsv3_thread, NULL);
	pthread_join(timer_thread, NULL);
	return EXIT_SUCCESS;
}
