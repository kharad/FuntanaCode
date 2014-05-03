#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>

#include "SSC.h"
#include "INS.h"


int DAQ_init();
void DAQ_loop();
void DAQ_deinit();
