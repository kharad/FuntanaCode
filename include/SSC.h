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



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>
#include <termios.h>  //wasnt included before cfsetspeed() needs this 
#include "midg/mMIDG2.h"
#include <termios.h>  //wasnt included before cfsetspeed() needs this 


//////////////////////////
// Servo Commands Struct
//
// Maintaining thread is a function of control mode:
// Autopilot control:  set by Autopilot thread
// Pilot control:  set by Robostix_comm thread
// Ground station control:  set by GS_comm thread
//
// Data members structured to support slaved or independent
// (left-right) servo commands.  Data type is currently
// set to (unsigned short) to mimic format in which data is
// output to servos and communicated (for efficiency).  
//

#ifndef SERVCOM
#define SERVCOM
struct Servo_Commands{
  unsigned short throttle;
  unsigned short left_throttle;
  unsigned short right_throttle;

  unsigned short aileron;
  unsigned short left_aileron;
  unsigned short right_aileron;

  unsigned short elevator;
  unsigned short left_elevator;
  unsigned short right_elevator;

  unsigned short rudder;
  unsigned short left_rudder;
  unsigned short right_rudder;

  unsigned short flap;  
  unsigned short left_flap;
  unsigned short right_flap;

  unsigned short nose_gear;
  unsigned short brake; 

  unsigned short pan;
  unsigned short tilt;
 
  unsigned short payload;  // TBD
  unsigned short aux_payload;

  unsigned short select_switch;
  unsigned short command_source;
  unsigned short op_mode;
};
#endif

// Global functions
int SSC_comm_init();
void SSC_comm_loop();
int SSC_readSerial();
int SSC_writeSerial();
void SSC_msgProcess(unsigned char msgtype, unsigned char *msg, unsigned char length);
int openSerial( char* name, speed_t speed );

