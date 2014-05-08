////////////////////////
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


#include "../include/INS.h"

///////////////////////////////////////////////////////////////////////////////////////
struct ACState acstate;  	// Aircraft state
struct ACState acstate_ref; // Reference (autopilot) acstate 
struct GPS_Data gpsdata; 	// GPS data
struct IMU_Data imudata; 	// IMU data
int SSC_portID;
int INS_portID;  // INS serial port file descriptor
mQueueRH hrMIDGrx;  // MIDG-II data handling variables
mQueueWH hwMIDGrx;
mtMIDG2State *MIDG2;
double sea_level_altitude;


///////////////////////////////
// INS_comm_init()
//
int INS_comm_init()
{
  //
  // Open serial port; check for error
  // Hard-coded to Gumstix STUART (/dev/ttyS2) at 115200 bps
  //
	// COM3 Port on Athena II /dev/ser3
 	INS_portID = openSerial("/dev/ser3",B115200);

  if (INS_portID < 0) 
    return -1;

  //
  // Create queues to be used by midg2 parser, and setup the parser
  //
  mQueueCreate(&hrMIDGrx, &hwMIDGrx, 1024);
  MIDG2=mMIDG2Setup(hrMIDGrx);

  return 0;  // Nominal execution
}

///////////////////////////////
// INS_comm_loop
//
// Reads/processes up to 100 characters from 
// the INS serial queue each time function is 
// invoked by the timer.  Currently don't use
// "updated" flags in MMIDG2 data structures --
// could do this in the future to separate 
// serial port reading from data processing.
// 
// Note:  Queue management code adapted from example
// supplied by Microbotics.  Efficiency improvements
// possible if speed is an issue.
//
void INS_comm_loop()
{
  unsigned char ID, bufchar;
  short int i;
  double mag;

  for (i=0;i<512;i++) {  // Up from 256
    if (read(INS_portID, &bufchar, 1) > 0) {
      mQueueWriteChar(hwMIDGrx,bufchar);
      while(ID = mMIDG2Parse(MIDG2)) { // Process all data stored in queue

	if (ID == 1) {   // STATUS
	  // Error checks, etc.; add this code later
	  // printf("STATUS...\n");

	} else if (ID == 2) {  // IMU_DATA (unfiltered)
	  read_IMU_DATA();
	  
	} else if (ID == 10) { // NAV_SENSOR: Filtered attitude, rate, accel data
	  read_NAV_SENSOR();
	
	} else if (ID == 12) { // NAV_PV:  Filtered position and velocity data
	  read_NAV_PV();

	} else if (ID == 20) { // GPS_PV:  Raw GPS position and velocity data
	  read_GPS_PV();
	}
      }
    } else {
      break;  // No more data available
    }
  }
  return;
} 

///////////////////////////////
// read_IMU_DATA
//
void read_IMU_DATA()
{
  double mag;

  // Currently not sent from MIDG-II due to bandwidth considerations.
  //  This message should be added (through Win-based MIDG-II config program) 
  //  to debug MIDG-II sensor system.

  // MIDG2->IMU_DATA.Time, MIDG2->IMU_DATA.Flags not processed

  mag = sqrt((double) (MIDG2->IMU_DATA.mxyz[0]*MIDG2->IMU_DATA.mxyz[0] +
		       MIDG2->IMU_DATA.mxyz[1]*MIDG2->IMU_DATA.mxyz[1] +
		       MIDG2->IMU_DATA.mxyz[2]*MIDG2->IMU_DATA.mxyz[2]));

  imudata.mag_x = MIDG2->IMU_DATA.mxyz[0]/mag;
  imudata.mag_y = MIDG2->IMU_DATA.mxyz[1]/mag;
  imudata.mag_z = MIDG2->IMU_DATA.mxyz[2]/mag;

  imudata.gyro_x = MIDG2->IMU_DATA.pqr[0]/100.0; // MIDG-II: 0.01 deg/s
  imudata.gyro_y = MIDG2->IMU_DATA.pqr[1]/100.0;
  imudata.gyro_z = MIDG2->IMU_DATA.pqr[2]/100.0;

  // Reconcile this data with that from NAV_SENSOR if both sent by MIDG-II
  imudata.accel_x = MIDG2->IMU_DATA.axyz[0]/1000.0; // MIDG-II:  milli-g's
  imudata.accel_y = MIDG2->IMU_DATA.axyz[1]/1000.0;
  imudata.accel_z = MIDG2->IMU_DATA.axyz[2]/1000.0;

  return;
}

///////////////////////////////
// read_NAV_SENSOR
//
// Note:  Verify GPS time is updated with each reading.
//
void read_NAV_SENSOR()
{
  acstate.time = MIDG2->NAV_SENSOR.Time/1000.0;  // MIDG-II:  GPS time in msec
  acstate.yaw = MIDG2->NAV_SENSOR.ypr[0]/100.0;  // MIDG-II:  0.01 deg
  acstate.pitch = MIDG2->NAV_SENSOR.ypr[1]/100.0;
  acstate.roll = MIDG2->NAV_SENSOR.ypr[2]/100.0;

/*	printf("yaw : %f\t", acstate.yaw);	
	printf("pitch : %f\t", acstate.pitch);
	printf("roll : %f\t\n", acstate.roll);*/
	
  acstate.roll_rate = MIDG2->NAV_SENSOR.pqr[0]/100.0; // MIDG-II:  0.01 deg/s  
  acstate.pitch_rate = MIDG2->NAV_SENSOR.pqr[1]/100.0;
  acstate.yaw_rate = MIDG2->NAV_SENSOR.pqr[2]/100.0;
  
  // Store filtered acceleration data in IMU_Data structure 
  // Reconcile with raw IMU_DATA message if IMU_DATA is downloaded later
  imudata.accel_x = MIDG2->NAV_SENSOR.axyz[0]/1000.0;  // MIDG-II:  milli-g's
  imudata.accel_y = MIDG2->NAV_SENSOR.axyz[1]/1000.0;
  imudata.accel_z = MIDG2->NAV_SENSOR.axyz[2]/1000.0;

  // Capture quaternion data
  acstate.q[0] = MIDG2->NAV_SENSOR.Q[0] / pow(2.0,30.0);
  acstate.q[1] = MIDG2->NAV_SENSOR.Q[1] / pow(2.0,30.0);
  acstate.q[2] = MIDG2->NAV_SENSOR.Q[2] / pow(2.0,30.0);
  acstate.q[3] = MIDG2->NAV_SENSOR.Q[3] / pow(2.0,30.0);
/*	printf("q1 : %f\t", acstate.q[0]);
	printf("q2 : %f\t", acstate.q[1]);
	printf("q3 : %f\t", acstate.q[2]);
	printf("q4 : %f\t\n", acstate.q[3]);

	printf("p : %f\t", acstate.roll_rate);
	printf("q : %f\t", acstate.pitch_rate);
	printf("r : %f\t\n", acstate.yaw_rate);*/
 return;
}

///////////////////////////////
// read_NAV_PV
//
// In this version, assume position will be in
// LLA format and velocity will be in ENU format.
// The INS is (or can be) configured to provide these
// formats by default.  Should later add checking of LLA/ENU
// format and status error status flags.
//
void read_NAV_PV()
{
  //pthread_mutex_lock(&acstate_mutex);  // Lock acstate

  acstate.time = MIDG2->NAV_PV.Time/1000.0;  // GPS time in msec

  // Note MIDG2 Pos[1]=latitude; Pos[0]=longitude
  acstate.x = MIDG2->NAV_PV.Pos[1]/1.0E7; 
  acstate.y = MIDG2->NAV_PV.Pos[0]/1.0E7; 
  acstate.z = -MIDG2->NAV_PV.Pos[2]/100.0; // MIDG2: + up; our FMS: + down
  // Set sea level altitude if flagged to be reset
  if ((sea_level_altitude-10000.0) < 0.00001) sea_level_altitude = acstate.z;

  acstate.xdot = MIDG2->NAV_PV.Vel[0]/100.0; // MIDG2:  cm/sec
  acstate.ydot = MIDG2->NAV_PV.Vel[1]/100.0;
  acstate.zdot = -MIDG2->NAV_PV.Vel[2]/100.0;

  //pthread_mutex_unlock(&acstate_mutex);  // Unlock acstate

  return;
}

///////////////////////////////
// read_GPS_PV
//
void read_GPS_PV()
{
  unsigned char uc;

  //pthread_mutex_lock(&gpsdata_mutex);  // Lock gpsdata

  gpsdata.time = MIDG2->GPS_PV.Time/1000.0;  // MIDG-II:  GPS time in msec  
  gpsdata.status = (unsigned char) ((MIDG2->GPS_PV.Details & 0x0700)>>8);
  
  // Process position data
  uc = (unsigned char) ((MIDG2->GPS_PV.Details & 0x000C)>>2); 
  if (!uc)          gpsdata.pos_coordsys = GPS_ECEF;
  else if (uc == 1) gpsdata.pos_coordsys = GPS_ENU;
  else              gpsdata.pos_coordsys = GPS_LLA; 
    
  if (gpsdata.pos_coordsys == GPS_LLA) { 
    gpsdata.longitude = MIDG2->GPS_PV.Pos[0]/1.0E7;
    gpsdata.latitude = MIDG2->GPS_PV.Pos[1]/1.0E7;
    gpsdata.altitude = MIDG2->GPS_PV.Pos[2]/100.0;
  } else if (gpsdata.pos_coordsys == GPS_ECEF) {  // Need ECEF calib if used.
    gpsdata.ecef_x = MIDG2->GPS_PV.Pos[0];
    gpsdata.ecef_y = MIDG2->GPS_PV.Pos[1];
    gpsdata.ecef_z = MIDG2->GPS_PV.Pos[2];
  } else if (gpsdata.pos_coordsys == GPS_ENU) {  // Need ENU calib if used.
    gpsdata.enu_east = MIDG2->GPS_PV.Pos[0];
    gpsdata.enu_north = MIDG2->GPS_PV.Pos[1];
    gpsdata.enu_up = MIDG2->GPS_PV.Pos[2];
  }
  
  // Process velocity data
  uc = (unsigned char) (MIDG2->GPS_PV.Details & 0x0002); 
  if (!uc) gpsdata.vel_coordsys = GPS_ECEF;
  else     gpsdata.vel_coordsys = GPS_ENU;

  if (gpsdata.vel_coordsys == GPS_ENU) {
    gpsdata.enu_veast = MIDG2->GPS_PV.Vel[0]/100.0;  // MIDG 2:  cm/sec
    gpsdata.enu_vnorth = MIDG2->GPS_PV.Vel[1]/100.0;
    gpsdata.enu_vup = MIDG2->GPS_PV.Vel[2]/100.0;
  } else if (gpsdata.vel_coordsys == GPS_ECEF) {
    gpsdata.ecef_vx = MIDG2->GPS_PV.Vel[0]/100.0;
    gpsdata.ecef_vy = MIDG2->GPS_PV.Vel[1]/100.0;
    gpsdata.ecef_vz = MIDG2->GPS_PV.Vel[2]/100.0;
  }

  return;
}
