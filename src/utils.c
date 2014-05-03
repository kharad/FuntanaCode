////////////////////////////
//
// utils.c 
//
// Utility functions for the Aerospace 
// Engineering Dept. Gumstix-based FMS, 
// adapted to the FlyingFish (ff) project.
//
// Contents:
// -- init_timer:  Initialize thread timer
// -- deinit_timer:  Deinitialize thread timer
// -- openSerial:  Open serial port (user-specified name and speed)
//
// Data files generated for the following structs:
// Thread_Stat, ACState, GPS_Data, ADS_Data, RPM_Data, IMU_Data, 
// Power_Data, Servo_Commands, Autopilot_Stat, Waypoint_Setup, 
// Control_Gain_Setup
//
// Copyright (C) 2007, Ella Atkins, Andy Klesh
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

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

#include <termios.h>  //wasnt included before cfsetspeed() needs this 
#include <sys/modem.h>

int openSerial( char* name, speed_t speed )
 {
  struct termios portOptions;
  int status,portHandle;
  int BAUD, DATABITS, PARITY, PARITYON, STOPBITS;
 
  printf("running modem_open() \n");
  fflush(stdout);
 
  portHandle = modem_open(name, speed);
  
  fcntl(portHandle, F_SETFL, O_NDELAY); // Do not delay before returning from read
  
  BAUD = B115200; // Baudrate
  DATABITS = CS8; // Number of Data bits
  STOPBITS = 0; // Number of stop bits 0 for None
  PARITYON = 0; // Parity 0 for None
  PARITY = 0;
  
  /* Not setting CRTSCTS which turns on hardware flow control */
  
  portOptions.c_cflag = BAUD | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;
  
  /*  set for non-canonical (raw processing, no echo, etc.) */
  portOptions.c_iflag = IGNPAR; /* ignore parity check */
  portOptions.c_oflag = 0; /* raw output */
  portOptions.c_lflag = 0; /* raw input  */
  
  cfsetospeed(&portOptions, BAUD);  /* redundant with the cflag setting, above */
  cfsetispeed(&portOptions, BAUD);
  
  status = tcsetattr(portHandle, TCSANOW, &portOptions);
  
  if (status != 0){printf("* TCSETATTR failed with status = %d \n ",status);}
  else{printf("COM attributes set\n");}

  return portHandle;
 }
