#include "../include/press.h"
#include "../include/INS.h"
#include "../include/SSC.h"
#include "../include/DAQ.h"

/** Global variables  required in this function */
extern int *cid2;
extern long int max_num2;
extern struct Servo_Commands ssc_servos; // Actual servo command data returned from SSC
extern struct timeval start_time;


void * thread_acdaq(void *){

  struct _pulse pulse;
  int 		counter;

  /** Initialize INS */
  INS_comm_init();
  
  /** Initialize SSC */
  SSC_comm_init();
  
  /** Initialize files for DAQ */
  DAQ_init();
  
  for(counter = 0; counter < max_num2; counter++){
	  
	  MsgReceive(*cid2,&pulse,sizeof(pulse),NULL);	

	  /** Get INS data */
	  INS_comm_loop();
	  
	  /** Get and send SSC data */
	  SSC_comm_loop();
	  
	  /** process pressure sensor data*/
	  press_cal_loop();
	  
	  /** Select controller based on command source*/
	  if (ssc_servos.command_source == 1) {
		
	  }
	  else{

	  }

	  /** Write data to file */
	  DAQ_loop();	
		
		
	}	
}	
