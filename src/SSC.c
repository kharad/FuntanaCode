#include "../include/SSC.h"

// Global variables
extern int SSC_portID;
struct Servo_Commands servos; // Autopilot-computed servo commands
struct Servo_Commands ssc_servos; // Actual servo command data returned from SSC
struct Servo_Commands ssc_servos_input; // Actual servo command data returned from SSC


///////////////////////////////////////////////////////////////////////
// SSC_comm_init()
//
int SSC_comm_init()
{
  //
  // Open serial port; check for error
  // Hard-coded to Gumstix STUART (/dev/ttyS2) at 115200 bps
  //
  // COM4 Port on Athena II /dev/ser4
	SSC_portID = openSerial("/dev/ser4",B115200);
	printf("SSC Port Open \n");

  	if (SSC_portID < 0) return -1;

  	return 0;  // Nominal execution
}

///////////////////////////////////////////////////////////////////////
void SSC_comm_loop()
{
 	
	if ((ssc_servos.command_source == 1) || (ssc_servos.command_source == 2))
		SSC_writeSerial();
	SSC_readSerial();
 
	return;
}

///////////////////////////////////////////////////////////////////////
int SSC_readSerial()
{
  unsigned char j, buffchar;  
  static unsigned char state=0,  msgtype, msg[50], ck0, ck1,i, length;

  for (j=0;j<200;j++)
    { 
      if (read(SSC_portID, &buffchar, 1) > 0)
	{  
	  if (state == 0) 
	    { 
	      i=0; // payload counter
	   
	      if ( buffchar == 0x81) // Start Byte Received
		{ 
		  j = 0; //Reset j to accomodate all of the new message 
		  state = 1;
		}
	    }
      
	  else if (state == 1) 
	    {
	      if (buffchar == 0xA1)
		{
		  state = 2;  // Second start byte received
		}
	      else  state = 0;  // Bad message
	    }
     
	  else if (state == 2)  // ID byte received
	    {
	      if ((buffchar==10) || (buffchar==12) || (buffchar==13) || (buffchar==14)) //Valid ID  
		{ 
		  msgtype = buffchar;  
		  ck0     = buffchar;
		  ck1     = buffchar;
		  state = 3;
		}
	      else state = 0;  //Invalid ID return to state 0;
	    } 
	  
	  else if (state == 3) // Message payload length
	    { 
	      if (buffchar < 38  && buffchar > 0) 
		{
		  length = buffchar; 		  
		  ck0   += buffchar;
		  ck1   += ck0;
		  state  = 4;
		}
	      else state = 0;
	    }
	  
	  else if (state == 4) // Read data bytes into msg[] array 
	    {  	      
	      msg[i++] = buffchar;
	      ck0     += buffchar;
	      ck1     += ck0;
	      if (i == length) state = 5; // Done reading data
	    } 
	  
	  else if (state == 5)  // Read & check the first checksum byte
	    {
	      if (buffchar != ck0) state = 0;
	      else state = 6;
	    } 
	  
	  else if (state == 6) // Read & check the second checksum byte
	    { 
	      if (buffchar == ck1)  // Valid message -- process it
		{ 	    
		  SSC_msgProcess(msgtype, msg, length);		  
		}//end if
	     	      
	      state = 0;

      return 0; //End Loop return from function if  complete message is received 
	      
	    }//end elseif state== 6
		 
	}///end if read > 0   
      
    } // end for j loop

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////
//
// Function to write the servo states to the Microbotics SSC.
// Assumes SSC is attached to Robostix UART0, and
// that the Gumstix reads the UART register through I2C.
//
int SSC_writeSerial()
{
	int status;
  	unsigned char buffer[25], *p_end;
  	unsigned char length;

  	// Write servo commands (msgID = 20)  NOTE MUST have at least 8 message outputs

  	p_end = mBinPacketOpen(buffer, 20);  // Create header
	p_end = mBinPutUShort(p_end, servos.right_aileron);
  	p_end = mBinPutUShort(p_end, servos.elevator); // left elevator
  	p_end = mBinPutUShort(p_end, servos.throttle);
  	p_end = mBinPutUShort(p_end, servos.rudder);
  	p_end = mBinPutUShort(p_end, servos.left_aileron);
  	p_end = mBinPutUShort(p_end, servos.elevator); // right elevator
  	p_end = mBinPutUShort(p_end, 1000);  // Placeholder variables
  	p_end = mBinPutUShort(p_end, 1000); // Placeholder  
  	length = (unsigned char) mBinPacketClose(buffer, p_end);

	status = write(SSC_portID,buffer,length); 

	if(status ==-1)
		{ printf("ssc write failed");
	}

 	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////
void SSC_msgProcess(unsigned char msgtype, unsigned char *msg, unsigned char length)
{
  // Process Message 12:  Pulse Outputs Message
  //
  if (msgtype == 12) 
{
    if (length < 12) return;  // Minimum of 6 output channels (12 data bytes)

    ssc_servos.right_aileron	= (msg[0]<<8) + msg[1];
    ssc_servos.elevator  	= (msg[2]<<8) + msg[3];  //left elevator
    ssc_servos.throttle 	= (msg[4]<<8) + msg[5];
    ssc_servos.rudder 		= (msg[6]<<8) + msg[7];
    ssc_servos.left_aileron  	= (msg[8]<<8) + msg[9];
    ssc_servos.right_elevator   = (msg[10]<<8) + msg[11];
    ssc_servos.select_switch	= (msg[12]<<8) + msg[13];

   if (length >= 16) {
      ssc_servos.pan = (msg[12]<<8) + msg[13];
      ssc_servos.tilt = (msg[14]<<8) + msg[15];
    }

}

if (msgtype == 10) 
{
  if (length < 2) return;

  if ( (msg[1] & 0x06) == 0x02)
 {  
   ssc_servos.command_source = 1;
  }
  if ( (msg[1] & 0x06) == 0x06) 
  {  
    ssc_servos.command_source = 3;
  }

  if ( (msg[1] & 0x06) == 0x04) 
  {  
    ssc_servos.command_source = 2;
  }

} // End if (msgtype == 10) 

// Process Pin Inputs Message
if (msgtype == 13)
{

  if (length < 12) return;
    ssc_servos_input.right_aileron  = (msg[2]<<8) + msg[3];  // pint 2 input
    ssc_servos_input.elevator  	    = (msg[4]<<8) + msg[5];  // pin 3 input
    ssc_servos_input.throttle 	    = (msg[6]<<8) + msg[7];  // pin 4
    ssc_servos_input.rudder 	    = (msg[8]<<8) + msg[9];  // pin 5
    ssc_servos_input.left_aileron   = (msg[10]<<8) + msg[11]; // pin6
    ssc_servos_input.right_elevator = (msg[12]<<8) + msg[13]; // pin7
    ssc_servos_input.select_switch  = (msg[14]<<8) + msg[15]; //pin8

}

  return;
} // End SSC_Process
