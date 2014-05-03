############################
#
# Makefile for Funtana project
# 
#
#############################

IDIR      := include
SDIR      := src
ODIR      := obj

CC        = g++
LDFLAGS   = -lm -ldscud5

_DEPS     = DAQ.h SSC.h INS.h press.h \
	    midg/mBin.h midg/mMIDG2.h midg/mQueue.h

_AC_SRC   = main.c INS.c DAQ.c utils.c SSC.c \
	    thread_pdaq.c thread_acdaq.c press.c timer_init.c

_MIDG_SRC = midg/mBin.c midg/mQueue.c midg/mMIDG2.c

_AC_OBJ   = $(patsubst %.c,%.o,$(_AC_SRC))

DEPS      = $(addprefix $(IDIR)/,$(_DEPS))

AC_SRC    = $(addprefix $(SDIR)/,$(_AC_SRC)) 
MIDG_SRC  = $(addprefix $(IDIR)/,$(_MIDG_SRC)) 

AC_OBJ    = $(addprefix $(ODIR)/,$(_AC_OBJ))
MIDG_OBJ  = $(patsubst %.c,%.o,$(MIDG_SRC))

EXEC      = FUNTANA

$(EXEC): $(AC_OBJ) $(MIDG_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $<

$(IDIR)/midg/%.o: $(IDIR)/midg/%.c $(DEPS)
	$(CC) -c -o $@ $<

clean:
	rm obj/*.o *~
