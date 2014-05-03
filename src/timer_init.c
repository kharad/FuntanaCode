/** Initialize Timer Function 
 *	Create a timer that give a pulse with a specified sampling rate
 *  Inputs:
 *  	- chan_id - pointer to the channel ID
 *		- sampling_rate
 *  Based in: flapper_timer_init
*/	

#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h> 	
#include <time.h> 			/**< define structure timer_t 	*/
#include <sys/siginfo.h>	/**< define struct sigevent 	*/
#include <sys/neutrino.h> 	

int timer_init(int *chan_id, double sampling_rate)
{
  	timer_t             	timerid;    /**< timer ID for timer 		*/
  	struct sigevent     	event;      /**< event to deliver			*/
  	struct itimerspec   	timer;      /**< the timer data structure	*/
  	int 					connect_id; /**< connection from timer back to this program */

   /** Open channel for the timer	*/
	if ((*chan_id = ChannelCreate (0)) == -1) { 						
    	fprintf(stderr, "Error creating channel.\n");
    	return -1;
  	}

   /** Connect to the channel created */
 	if ((connect_id = ConnectAttach (0, 0, *chan_id, 0, 0)) == -1) {
    	fprintf (stderr, "Error connecting the channel.\n");
    	return -1;
  	}

   /** Create the event that will be delivered. In this case is a pulse
	* SIGEV_PULSE_PRIO_INHERIT creates a pulse with relative time.
	*/
  	SIGEV_PULSE_INIT (&event, connect_id, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

   /**  Create the timer 	*/
  	if (timer_create (CLOCK_REALTIME, &event, &timerid) == -1) {
    	fprintf (stderr, "Error creating timer.\n");
    	return -1;
  	}

   /** Set the timer parameters */
  	timer.it_value.tv_sec = 1;  	/**< Initial delay in seconds 	*/
  	timer.it_value.tv_nsec = 0; 	/**< Initial delay nanoseconds 	*/
  	timer.it_interval.tv_sec = 0; 	/**< Interval between pulses (seconds) */
  	timer.it_interval.tv_nsec = (long int) (1.0E9 / sampling_rate); /**< Interval between pulses (nanosecond) */

   /** Start the timer with the parameters above */
	timer_settime (timerid, 0, &timer, NULL); // Start the timer

   /** Finishes the function without errors */
  	return 0;
}
