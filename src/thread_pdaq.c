#include "../include/press.h"

extern int *cid1;
extern long int max_num1;
extern struct timeval start_time;

void * thread_pdaq(void *){

	struct _pulse pulse;
	int 		counter;
	
	double time;
  	struct timeval cur_time;
 
	press_aq_init();

	for(counter = 0; counter < max_num1; counter++){
		MsgReceive(*cid1,&pulse,sizeof(pulse),NULL);

	  	// Compute daq_run_time for current DAQ loop
  		gettimeofday(&cur_time, NULL);
  		time = (cur_time.tv_sec - start_time.tv_sec) +
    			1.0E-6*((double) (cur_time.tv_usec - start_time.tv_usec));
//		printf("Thread 1 Running! - T = %f\n",time);
		press_aq_loop();
	}
}	
