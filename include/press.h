#include <stdio.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_FILTER  2

/// Diamond System library and variables
#include <dscud.h> // Diamond Systems header file

#define ATHENA_BASEADD 0x280  // Hardware base address (set by jumpers)
#define MM32_BASEADD 0x300    // Hardware base address (set by jumpers)
#define ATHENA_HIGH_CH 15 // Highest A/D channel to scan on the Athena board (starts with 0)
#define DMM_HIGH_CH 31  // Highest A/D channel to scan on the DMM32 board (starts with 0)
#define ATHENA_NUMCHANNELS (ATHENA_HIGH_CH + 1)
#define MM32_NUMCHANNELS (DMM_HIGH_CH + 1)


/** Function prototypes */
void DiamondInit(	DSCSAMPLE **ATHENA_samples, DSCB *ATHENA_dscb, DSCADSCAN *ATHENA_dscadscan,
        			DSCSAMPLE **MM32_samples, DSCB *MM32_dscb, DSCADSCAN *MM32_dscadscan);
void press_aq_init();
void press_aq_loop(); 
void press_cal_loop();
