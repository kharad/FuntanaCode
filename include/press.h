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
