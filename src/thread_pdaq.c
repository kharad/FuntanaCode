////////////////////////
//
//
// Copyright (C) 2010, Ella Atkins, Marc Lecerf
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
// 02110-1301, USA.
////////////////////////////


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
