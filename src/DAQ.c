////////////////////////
//
// DAQ.c
//
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

#include "../include/DAQ.h"
#include "../include/press.h"

struct 	timeval daq_start_time; // DAq init time
FILE 	*fp_acstate, *fp_servos, *fp_ssc_servos, *fp_press;
int 	numexecs = 0;
int 	daq_datapoints = 1;
extern struct Servo_Commands ssc_servos; // Actual servo command data returned from SSC
extern struct Servo_Commands servos; // Actual servo command data returned from SSC
extern struct ACState acstate;  	// Aircraft state
extern double MM32_pres[MM32_NUMCHANNELS];     // DMM32 dataset in volts (double is the C default)

//////////////////////
// DAQ_init()
//
int DAQ_init()
{
  char s[100], month[4];
  int day, year, hour, minute;
  FILE *fpdate; // Read startup date/time stamp information from 
                // ../../i2c/startup_datetime
  FILE *ADS;
  gettimeofday(&daq_start_time,NULL);  // Initialize start time
  struct tm * timeinfo;
  time_t nowtime;
  timeinfo = localtime ( &nowtime );
  char title [100];

  sprintf(s,"./data/ACState.dat");
  if ((fp_acstate = fopen(s,"w"))==NULL) {
    fprintf(stderr,"Error opening acstate data file.\n");
    fflush(stdout);
    return -1;
  }

  sprintf(s,"./data/Servo_Commands.dat");
  if ((fp_ssc_servos = fopen(s,"w"))==NULL) {
    fprintf(stderr,"Error opening servos data file.\n");
    return -1;
  }

  sprintf(s,"./data/AP_Commands.dat");
  if ((fp_servos = fopen(s,"w"))==NULL) {
    fprintf(stderr,"Error opening servos data file.\n");
    return -1;
  }
  
  sprintf(s,"./data/Press.dat");
  if ((fp_press = fopen(s,"w"))==NULL) {
    fprintf(stderr,"Error opening servos data file.\n");
    return -1;
  }

  return 0;  // Nominal execution
}

///////////////////////////////
//
// Data acquisition loop executed by DAQ timer
//
void DAQ_loop()
{    
  static unsigned char count_40=0, count_10=0;
  struct timeval cur_time;
  double time;
  char s[200];
  int i;
   
  // Compute daq_run_time for current DAQ loop
  gettimeofday(&cur_time, NULL);
  time = (cur_time.tv_sec - daq_start_time.tv_sec) +
    1.0E-6*((double) (cur_time.tv_usec - daq_start_time.tv_usec));


  fprintf(fp_acstate, // daq_run_time + 6 data values 
	  "%.3lf\t%.3lf\t%.3lf\t%.3lf\t%.3lf\t%.3lf\t%.3lf\t",
	  time, 
	  acstate.yaw, acstate.pitch, acstate.roll,
	  acstate.yaw_rate, acstate.pitch_rate, acstate.roll_rate
	  );
  fprintf(fp_acstate,
	  "%.5lf\t%.5lf\t%.5lf\t%.5lf\t",
	  acstate.q[0], acstate.q[1], acstate.q[2], acstate.q[3]
	  );
  fprintf(fp_acstate, // 6 data values 
	  "%.7lf\t%.7lf\t%.3lf\t%.3lf\t%.3lf\t%.3lf\t",
	  acstate.x, acstate.y, acstate.z,
	  acstate.xdot, acstate.ydot, acstate.zdot
	  );
  fprintf(fp_acstate, // 3 data values 
	  "%.3lf\t%.3lf\t%.3lf\n",
	  acstate.wind_vx, acstate.wind_vy, acstate.wind_vz
	  );

  fprintf(fp_ssc_servos,
	    "%.3lf\t%hd\t%2hd\t%2hd\t%2hd\t%2hd\t%2hd\n",
	    time,
	    ssc_servos.throttle, ssc_servos.left_aileron, ssc_servos.elevator,
	    ssc_servos.rudder, ssc_servos.flap, ssc_servos.right_aileron 
	    );

  fprintf(fp_servos,
	    "%.3lf\t%d\t%hd\t%2hd\t%2hd\t%2hd\t%2hd\t%2hd\n",
	    time,ssc_servos.command_source,
	    servos.throttle, servos.left_aileron, servos.elevator,
	    servos.rudder, servos.flap, servos.right_aileron 
	    );
	
	fprintf(fp_press,"%.3lf\t",time);    
	for(i = 0; i < MM32_NUMCHANNELS; i++){
	  fprintf(fp_press,"%f\t",MM32_pres[i]);
	}
	fprintf(fp_press,"\n");

  if (daq_datapoints%4500 == 0){
    	fclose(fp_acstate);
    	fclose(fp_ssc_servos);
	    fclose(fp_servos);
	    fclose(fp_press);
    	numexecs++;
       	sprintf(s,"./data/ACState%d.dat", numexecs);
    	fp_acstate = fopen(s,"w");
    	sprintf(s,"./data/Servo_Commands%d.dat",numexecs);
    	fp_ssc_servos = fopen(s,"w");
  	  sprintf(s,"./data/AP_Commands%d.dat",numexecs);
    	fp_servos = fopen(s,"w");
    	sprintf(s,"./data/Press%d.dat",numexecs);
    	fp_press = fopen(s,"w");
  	}
  	else if (daq_datapoints%50 == 0){
    	fflush(fp_acstate);
	    fflush(fp_ssc_servos);
	    fflush(fp_press);
	    fflush(fp_servos);
  	}
  
	daq_datapoints++;  // Increment number of stored datapoints
  
	return;
}

void DAQ_deinit() 
{
	fclose(fp_acstate);
	fclose(fp_servos);
	fclose(fp_ssc_servos);
}

