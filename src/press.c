#include "../include/press.h"
#include "../include/SSC.h" 

// Global Variables

/** ADS Mutex creation and initialization */
pthread_mutex_t adsmutex = PTHREAD_MUTEX_INITIALIZER;	


/** Variables for AD aquisition */
DSCSAMPLE 	*ATHENA_samples, 	*MM32_samples;
DSCB 		ATHENA_dscb, 		MM32_dscb;
DSCADSCAN 	ATHENA_dscadscan, 	MM32_dscadscan;


/** Variables with the pressure data */
signed short *ATHENA_buffer;  	// Buffered data from Athena; SWORD in dscud.h
signed short *MM32_buffer;  	// Buffered data from DMM32; SWORD in dscud.h
float ATHENA_volts[NUM_FILTER][ATHENA_NUMCHANNELS];  // Athena dataset in volts (float to support ATI function calls)
double MM32_volts[NUM_FILTER][MM32_NUMCHANNELS];     // DMM32 dataset in volts (double is the C default)

float ATHENA_pres[ATHENA_NUMCHANNELS];  // Athena dataset in volts (float to support ATI function calls)
double MM32_pres[MM32_NUMCHANNELS];     // DMM32 dataset in volts (double is the C default)
extern struct Servo_Commands ssc_servos; // Actual servo command data returned from SSC

double F[3];
double M[3];

/** Function to initialize the pressure aquisition */
void press_aq_init(){

	/** Diamond Defined Structures for AD Scan */
	BYTE 		result; 

	/** Start Diamond aquisition */
  	DiamondInit(&ATHENA_samples, &ATHENA_dscb, &ATHENA_dscadscan,
              	&MM32_samples,   &MM32_dscb,   &MM32_dscadscan);

  	/** Assign memory to the buffers */
 	ATHENA_buffer = (signed short *) malloc(1*sizeof(signed short)*ATHENA_NUMCHANNELS);
  	MM32_buffer = (signed short *) malloc(1*sizeof(signed short)*MM32_NUMCHANNELS);

	/** Do one initial scan	that is discarded - usually have noise  */
  	if( (result = dscADScan( ATHENA_dscb, &ATHENA_dscadscan, ATHENA_samples) )!= DE_NONE)  
    	fprintf(stderr, "ATHENA ERROR:  %s\n", dscGetErrorString(result) );   
  	if( (result = dscADScan( MM32_dscb, &MM32_dscadscan, MM32_samples) )!= DE_NONE)  
    	fprintf(stderr, "DMM32 ERROR:  %s\n", dscGetErrorString(result) );   
}

/** Function that does the pressure aquisition 
 * Fills Athena_volts and MM32_volts structure */
void press_aq_loop(){

  	int i; 		/**< Loop counters */ 
	static int k ;

	/** Diamond Defined Structures for AD Scan */
	BYTE 		result; 

	/** Data aquisition */
   	if( (result = dscADScan( ATHENA_dscb, &ATHENA_dscadscan, ATHENA_samples) )!= DE_NONE)  {
   		fprintf(stderr, "ATHENA ERROR:  %s\n", dscGetErrorString(result) );
   	}
  	if( (result = dscADScan( MM32_dscb, &MM32_dscadscan, MM32_samples) )!= DE_NONE)  {
   		fprintf(stderr, "DMM32 ERROR:  %s\n", dscGetErrorString(result) );
  	}

   	/** Store new data to buffers */
   	memcpy(ATHENA_buffer, ATHENA_dscadscan.sample_values, ATHENA_NUMCHANNELS*sizeof(signed short));
   	memcpy(MM32_buffer, MM32_dscadscan.sample_values, MM32_NUMCHANNELS*sizeof(signed short));

   	/** Lock the ADS mutex */
	pthread_mutex_lock(&adsmutex);

   	/** Convert raw Athena dataset to volts and write to structure */
   	for (i=0; i<ATHENA_NUMCHANNELS; i++) {
   		ATHENA_volts[k][i] = (float) (ATHENA_buffer[i]/32768.0) * 10.0;  // -5 to 5V range
   	}

   	/** Convert raw DMM32 dataset to volts and write to file */
   	for (i=0; i<MM32_NUMCHANNELS; i++) { // Converting all A/D channel readings to volts then writing
      		MM32_volts[k][i] = (double) (((short) MM32_buffer[i] + 32768)/65536.0 * 5.0);  // 0 to 5V range
   	}

	//printf("Acquiring data:\n ");
	//for(i = 5; i < 10; i++){
  //		printf("%f\t",MM32_volts[k][i]);
	//}
	//printf("\n");

   	/** Unlock the ADS mutex */
	pthread_mutex_unlock(&adsmutex);

	/** Update the pointer to the head */
	k++;
	if (k >= NUM_FILTER)
		k = 0;
}


void press_cal_loop(){

	int i, j; /**< Loop counter variables */

	// Downsamplig and converting from volts to Pa
	pthread_mutex_lock(&adsmutex);		
	for(i = 0; i < ATHENA_NUMCHANNELS; i++){
			ATHENA_pres[i] = 0;
			for(j = 0; j < NUM_FILTER; j++)
				ATHENA_pres[i] += ATHENA_volts[j][i];
			ATHENA_pres[i] = ATHENA_pres[i]/NUM_FILTER;
	}

//	for(i = 0; i < ATHENA_NUMCHANNELS; i++){
//	  printf("%f\t",ATHENA_pres[i]);
//	}
//	printf("\n");

	for(i = 0; i < MM32_NUMCHANNELS; i++){
			MM32_pres[i] = 0;
			for(j = 0; j < NUM_FILTER; j++)
				MM32_pres[i] += MM32_volts[j][i];
			MM32_pres[i] = MM32_pres[i]/NUM_FILTER;
	}

	//printf("Filtering: \n");
	//printf("Values Stored: \n");	
	//for(j=0; j < NUM_FILTER; j++){ 
	//	for(i = 5; i < 10; i++)
	//		printf("%f\t",MM32_volts[j][i]);
	//	printf("\n");
	//}
	//printf("\n");

	pthread_mutex_unlock(&adsmutex);		

	//printf("Filtered Data: \n");	
	//for(i = 5; i < 10; i++){
	//  		printf("%f\t",MM32_pres[i]);
	//}
	//printf("\n");

	// Convert from volts to pressure in Pa
	for(i = 0; i < ATHENA_NUMCHANNELS; i++)
		ATHENA_pres[i] = (ATHENA_pres[i]*0.515-1.316)*249;

	for(i = 0; i < ATHENA_NUMCHANNELS; i++)
		MM32_pres[i] = (MM32_pres[i]*0.515-1.316)*249;

	// Calculate the forces of horizontal stabilizer and elevator
	double anchorpointX=314.21;
	double anchorpointY=337.95;
	double tailLE_to_cg=876; 
	double tailLE_to_centreline=20;
	double hingeline = 202.34;

	double hs_lengths[9] = {62.23,62.23,75.97,51.88,51.17,115.36,24.45,78.14,78.92};
	double hs_widths[9] = {114.26,114.26,114.26,107.33,107.33,107.33,79.76,79.76,79.76};
	double hs_centerX[9] = {255.15,255.15,256.65,142.94,142.94,142.94,47.53,47.53,46.03};
	double hs_centerY[9] = {296.22,233.46,162.75,280.71,228.46,143.06,277.33,223.09,143.06};
	double pforce[9], Fp[9][3], Mp[9][3];
	double def_angle;
	double hingedis;

	double vs_lengths[6] = {78.12,88.52,133.72,120.61,133.42,152.11};
	double vs_widths[6] = {105.02,105.02,123.69,75.74,82.96,91.01};
	double vs_centerX[6] = {-204.21,-291.91,-168.13,-295.30,-303.49,-312.83};
	double vs_centerY[6] = {299.81,299.81,182.65,206.84,126.26,34.81};
	
	for (i=0;i<3;i++){
		F[i] = 0;
		M[i] = 0;
	}

	// Left Horizontal Stabilizer
	for(i=0;i<9;i++){
		pforce[i] = hs_widths[i]*hs_lengths[i]*0.001*0.001*MM32_pres[i+10];
		if((i== 2)||(i==5)||(i==7)|(i==8))
			def_angle = (0.1056*ssc_servos.elevator - 164.9611)/57.2958;
		else
			def_angle = 0;
		Fp[i][0] = + pforce[i] * -sin(def_angle);
		Fp[i][1] = 0;
	    	Fp[i][2] = - pforce[i] * cos(def_angle);

    		hingedis = (hingeline-hs_centerY[i])*cos(def_angle); 

    		Mp[i][0] = -Fp[i][2]*(anchorpointX - hs_centerX[i]);
    		Mp[i][1] = Fp[i][2]*(anchorpointY - hingeline + hingedis + tailLE_to_cg);
       		Mp[i][1] = Mp[i][1]+Fp[i][0]*(hingeline- hs_centerY[i])*sin(def_angle);
		Mp[i][2] = Fp[i][0]*(anchorpointX - hs_centerX[i]);
	}

	for (j=0;j<3;j++){
		for (i=0;i<9;i++){
			F[j] += Fp[i][j];
			M[j] += Mp[i][j]/1000.0;
		}
	}
	
    //printf("Fx = %f \t Fy = %f \t Fz = %f\n",F[0],F[1],F[2]);
	//printf("Mx = %f \t My = %f \t Mz = %f\n",M[0],M[1],M[2]);

	// Right Horizontal Stabilizer
	for(i=0;i<9;i++){
		pforce[i] = hs_widths[i]*hs_lengths[i]*0.001*0.001*MM32_pres[i+10+10];
		if((i== 2)||(i==5)||(i==7)|(i==8))
			def_angle = (0.1056*ssc_servos.elevator - 164.9611)/57.2958;
		else
			def_angle = 0;
		Fp[i][0] = + pforce[i] * -sin(def_angle);
		Fp[i][1] = 0;
	    	Fp[i][2] = - pforce[i] * cos(def_angle);

	    	hingedis = (hingeline-hs_centerY[i])*cos(def_angle); 

	    	Mp[i][0] = Fp[i][2]*(anchorpointX - hs_centerX[i]);
	    	Mp[i][1] = Fp[i][2]*(anchorpointY - hingeline + hingedis + tailLE_to_cg);
	       	Mp[i][1] = Mp[i][1]+Fp[i][0]*(hingeline- hs_centerY[i])*sin(def_angle);
		Mp[i][2] = -Fp[i][0]*(anchorpointX - hs_centerX[i]);
	}

	for (j=0;j<3;j++){
		for (i=0;i<9;i++){
			F[j] += Fp[i][j];
			M[j] += Mp[i][j]/1000.0;
		}
	}
	
 	//printf("Fx = %f \t Fy = %f \t Fz = %f\n",F[0],F[1],F[2]);
	//printf("Mx = %f \t My = %f \t Mz = %f\n",M[0],M[1],M[2]);

	// Vertical Stabilizer
	anchorpointX=-68.43; 
	anchorpointY=133.38; 
	hingeline = -234.99;

    for(i=0;i<6;i++){
        pforce[i] = vs_widths[i]*vs_lengths[i]*0.001*0.001*MM32_pres[i];
        if(i!=2)
            def_angle = (0.0882*ssc_servos.rudder - 133.7118)/57.2958;
        else
            def_angle = 0;
        Fp[i][0] = pforce[i] * sin(def_angle);
        Fp[i][1] = pforce[i] * cos(def_angle);
        Fp[i][2] = 0;

        hingedis= (hingeline-vs_centerX[i])*cos(def_angle);

        Mp[i][0] = Fp[i][1]*(anchorpointY - vs_centerY[i]);
        Mp[i][1] = -Fp[i][0]*(anchorpointY - vs_centerY[i]);
        Mp[i][2] = Fp[i][1]*(anchorpointX - hingeline + hingedis + tailLE_to_cg);
		Mp[i][2] += Fp[i][0]*sin(def_angle)*(hingeline-vs_centerX[i]);
    }

    for (j=0;j<3;j++){
        for (i=0;i<6;i++){
            F[j] += Fp[i][j];
            M[j] += Mp[i][j]/1000.0;
        }
    }

	//printf("\nFx = %f \t Fy = %f \t Fz = %f\n",F[0],F[1],F[2]);
//	printf("Mx = %f \t My = %f \t Mz = %f\n\n",M[0],M[1],M[2]);

}

//
// Function to initialize both the Diamond Systems Athena and MM-32 boards
// Integrated to avoid multiple calls to dscInit
//
// Written:  Ella M. Atkins
// Last Modified:  Nov. 2010
// 
// Relies on Diamond Systems Corporation Universal Driver (DSCUD) 
// software (www.diamondsystems.com)

void DiamondInit(DSCSAMPLE **ATHENA_samples, DSCB *ATHENA_dscb, DSCADSCAN *ATHENA_dscadscan,
		 DSCSAMPLE **MM32_samples, DSCB *MM32_dscb, DSCADSCAN *MM32_dscadscan)
{
  DSCCB 	 ATHENA_dsccb, MM32_dsccb; // structure containing board settings
  DSCADSETTINGS  ATHENA_dscadsettings, MM32_dscadsettings; // structure containing A/D conversion settings	
  BYTE result;
  ERRPARAMS errorParams;
  
  /***** Initialize DSCUD library (call only once for both boards) *****/
  
  if( dscInit( DSC_VERSION ) != DE_NONE )  {
    dscGetLastError(&errorParams);
    fprintf( stderr, "DSCUD ERROR:  %s\n", errorParams.errstring );
    return;
  } 

  /*********************************************/
  /********* Initialize Athena board ***********/
  ATHENA_dsccb.boardtype = DSC_ATHENA;
  ATHENA_dsccb.base_address = ATHENA_BASEADD;
  ATHENA_dsccb.int_level = 5;
  ATHENA_dsccb.dma_level = 3;
  ATHENA_dsccb.clock_freq = 10000000L;
  if(dscInitBoard(DSC_ATHENA, &ATHENA_dsccb, ATHENA_dscb)!= DE_NONE)  {
    dscGetLastError(&errorParams);
    fprintf(stderr, "ATHENA:  %s\n", errorParams.errstring);
    return;
  }

  ATHENA_dscadsettings.range = RANGE_10;  // -5 to 5 V A/D range
  ATHENA_dscadsettings.polarity = BIPOLAR;
  ATHENA_dscadsettings.gain = GAIN_1;
  ATHENA_dscadsettings.load_cal = (BYTE)TRUE;
  ATHENA_dscadsettings.current_channel = 0;
  ATHENA_dscadsettings.load_cal = 0;
  if( ( result = dscADSetSettings( *ATHENA_dscb, &ATHENA_dscadsettings ) ) != DE_NONE ) {
    dscGetLastError(&errorParams);
    fprintf(stderr, "ATHENA:  %s\n", errorParams.errstring);
    return;
  }
  ATHENA_dscadscan->low_channel 	= 0;
  ATHENA_dscadscan->high_channel        = ATHENA_HIGH_CH;
  ATHENA_dscadscan->gain		= ATHENA_dscadsettings.gain;
  *ATHENA_samples = (DSCSAMPLE*) 
    malloc(sizeof(DSCSAMPLE)*(ATHENA_dscadscan->high_channel - ATHENA_dscadscan->low_channel+1));

  /*********************************************/
  /***** Initialize Diamond-MM-32-AT board *****/  
  MM32_dsccb.boardtype = DSC_DMM32AT;
  MM32_dsccb.base_address = MM32_BASEADD;
  MM32_dsccb.int_level = 5;
  MM32_dsccb.dma_level = 3;
  MM32_dsccb.clock_freq = 10000000L;
  if(dscInitBoard(DSC_DMM32, &MM32_dsccb, MM32_dscb)!= DE_NONE)	{
    dscGetLastError(&errorParams);
    fprintf(stderr, "DMM32:  %s\n", errorParams.errstring);
    return;
  }
  MM32_dscadsettings.range = RANGE_10;  // Range, gain set for 0-5V
  MM32_dscadsettings.polarity = UNIPOLAR;
  MM32_dscadsettings.gain = GAIN_2;
  MM32_dscadsettings.load_cal = (BYTE)TRUE;	
  MM32_dscadsettings.current_channel = 0;
  MM32_dscadsettings.load_cal = 0;
  
  if( ( result = dscADSetSettings( *MM32_dscb, &MM32_dscadsettings ) ) != DE_NONE )  {
    dscGetLastError(&errorParams);
    fprintf(stderr, "DMM32:  %s\n", errorParams.errstring);
    return;
  }
  MM32_dscadscan->low_channel  = 0;
  MM32_dscadscan->high_channel = DMM_HIGH_CH;
  MM32_dscadscan->gain	  = MM32_dscadsettings.gain;
  *MM32_samples = (DSCSAMPLE*) 
    malloc(sizeof(DSCSAMPLE)*(MM32_dscadscan->high_channel - MM32_dscadscan->low_channel+1));

  return;
}
