#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
// MIDG-II Headers
#include "midg/mMIDG2.h"
#include <termios.h>  //wasnt included before cfsetspeed() needs this 

///////////////////////////////////////////////////////////////////////////////////////

// ACState data structure
// Maintained by the Autopilot thread.
//
#ifndef ACSTRUCT
#define ACSTRUCT
struct ACState
{
  double time;  // May need to split this into IMU vs. GPS time if not 6DOF estimate

  double yaw;   // XYZ Euler angles:  degrees
  double pitch;
  double roll;
  double yaw_rate;  // Body axis frame:  deg/sec
  double pitch_rate;
  double roll_rate;

  double x;  // Inertial frame (latitude) (deg)
  double y;  // Inertial frame (longitude) (deg)
  double z;  // Altitude (positive down) (meters; MSL=0)
  double xdot;  // Inertial frame (positive North):  m/sec
  double ydot;  // Inertial frame (positive East):  m/sec
  double zdot;  // Climb rate (positive down) (m/sec)
  
  double q[4];  // Quaternion vector (qw, qx, qy, qz)

  double wind_vx;  // Inertial-x wind velocity (m/sec)
  double wind_vy;  // Inertial-y wind velocity (m/sec)
  double wind_vz;  // Inertial-z (vertical) wind velocity (m/sec)
};

// GPS Data Structure 
// Maintained by INS_comm thread
// (Note:  Typically will populate only 
// one position & one velocity format option)
//
struct GPS_Data
{
  double time;  // GPS time
  unsigned char pos_coordsys;  // GPS_LLA, GPS_ECEF, GPS_ENU
  unsigned char vel_coordsys;  // GPS_ENU, GPS_dXdYdZ
  unsigned char status;        // 0=no fix; 1=dead reckoning; 2=2D fix; 3=3D fix; 4=GPS+dead reckoning
  unsigned char corrector;     // DGPS, WAAS, EGNOS, OmniSTAR, RTCM, SkyFix

  // Position
  double latitude;  // Format 1:  LLA
  double longitude;
  double altitude;

  double ecef_x; // Format 2:  XYZ - ECEF
  double ecef_y;
  double ecef_z;

  double enu_east; // Format 3:  XYZ - ENU
  double enu_north;
  double enu_up;

  // Velocity
  double enu_veast;  // Format 1:  ENU (East, North, Up)
  double enu_vnorth;
  double enu_vup;

  double ecef_vx;  // Format 2:  ECEF
  double ecef_vy;
  double ecef_vz;

  // GPS Health (specifics TBD)
  double health;

  // GPS Almanac (double type placeholder; Almanac struct TBD)
  double almanac;
};

// Inertial Measurement Unit (IMU) Data Structure 
// Maintained by INS_comm thread (when monitored)
//
struct IMU_Data
{
  double mag_x;  // Unit vector (body axis frame) to magnetic North
  double mag_y;
  double mag_z;

  double accel_x;  // Acceleration vector (body axis frame)
  double accel_y;  // in g's:  (Nominal magnitude:  1-g down) 
  double accel_z;

  double gyro_x;   // Rates (m/sec):  roll rate
  double gyro_y;   // pitch rate
  double gyro_z;   // yaw rate
};
#endif

///////////////////////////////////////////////////////////////////////////////////////
int INS_comm_init();

void INS_comm_loop();
void read_IMU_DATA();
void read_NAV_SENSOR();
void read_NAV_PV();
void read_GPS_PV();
int openSerial( char* name, speed_t speed );

/////////////////////
#define GPS_DEFAULT 0x00  // Default position/velocity format (LLA, ENU)
#define GPS_LLA 0x01
#define GPS_ECEF 0x02
#define GPS_ENU 0x03
