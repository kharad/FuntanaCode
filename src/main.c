/** MAIN FUNCTION FOR THE FUNTANA-100
 * This function:
 * -# Creates two timers 
 *		- Thread 1 is for pressure data acquisition
 *		- Thread 2 is for the controller and aircraft state data acquisition
 * -# Starts the two threads
 * -# Waits for the two threads to join
 *
 * The frequency used by these two threads can be set directly in the code.
 * The total time of the program to run is also directed set in the code.
 */

#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h> 
#include <unistd.h>	

#include "../include/press.h"

/** Global variables that control the threads */
int 			*cid1, *cid2; 	       /**< Pointers to the timers */
long int 		max_num1, max_num2;    /**< Max number of iterations of the program (aprox) */
struct timeval start_time;
 
/** Prototype of the function to create the timer */
int timer_init(int* id, double num);

/** Prototype of the thread functions */
void* thread_pdaq(void *);
void* thread_acdaq(void *);

/** Main function */
int main(int argc,char** argv){
	
	double	st1 = 200; 	     /**< Frequency of thread 1 (Hz) - ADS aquisition */
	double 	st2 = 100;  	     /**< Frequency of thread 2 (Hz) - All other functions */
	
	int  ttime = atoi(argv[1]);  /**< Total execution time (in mins) of the code - user defined */
	pthread_t  pid1, pid2; 	     /**< Thread pointers */
	


	gettimeofday(&start_time, NULL);


	/** Calculate the total number of iterations of the program */
	max_num1 = ttime*60*((int)st1);
	max_num2 = ttime*60*((int)st2);

	/** Allocate memory to the timer ids */
	cid1 = (int *) malloc(sizeof(int)); 
	cid2 = (int *) malloc(sizeof(int));	

	/** Initialize the timers */
	timer_init(cid1,st1);
   	timer_init(cid2,st2);

	/** Create the two threads */
	pthread_create(&pid1,NULL,thread_pdaq,NULL);	
	pthread_create(&pid2,NULL,thread_acdaq,NULL);	

	/** Join the two threads */
	pthread_join(pid1,NULL);
	pthread_join(pid2,NULL);

	return 0;	
}	
