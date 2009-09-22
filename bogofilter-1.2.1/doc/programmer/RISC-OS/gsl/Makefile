# Makefile for the RISC OS version of gsl


# Define which compiler to use:

CC = cc
#CC = gcc


#########################################
# DO NOT EDIT ANYTHING BELOW THIS LINE! #
#########################################

ifeq (${CC},cc)
CC = cc
LD = link
AR = libfile
DEPEND = -depend !Depend
CC_FLAGS = -Wdp -throwback -Otime -I^,@,Unix: -JUnix: -D__riscos__ -DHAVE_CONFIG_H
UNIXLIB = Unix:o.UnixLib
else
ifeq (${CC},gcc)
CC = gcc
LD = gcc
AR = ar
CC_FLAGS = -mthrowback -O3 -I^ -I@ -D__riscos__ -DHAVE_CONFIG_H
else
# No other compiler supported!
endif
endif

CDF_OBJS = cdf.o.chisq \
	cdf.o.gamma

ERR_OBJS = err.o.error \
	err.o.stream

SPECFUNC_OBJS = specfunc.o.elementary \
	specfunc.o.erfc \
	specfunc.o.exp \
	specfunc.o.expint \
	specfunc.o.gamma_inc \
	specfunc.o.log \
	specfunc.o.psi \
	specfunc.o.trig \
	specfunc.o.zeta

SYS_OBJS = sys.o.coerce \
	sys.o.fdiv \
	sys.o.infnan \
	sys.o.pow_int

.INIT:
	@cdir cdf.o
	@cdir err.o
	@cdir specfunc.o
	@cdir sys.o

## Rule Patterns ##

.SUFFIXES:	.c .o

.c.o:
	$(CC) $(CC_FLAGS) $(DEPEND) -c -o $@ $<

# Static dependencies:

libgsl: $(CDF_OBJS) $(ERR_OBJS) $(SPECFUNC_OBJS) $(SYS_OBJS)
	$(AR) $(AR_FLAGS) -c -o $@ $(CDF_OBJS) $(ERR_OBJS) $(SPECFUNC_OBJS) $(SYS_OBJS)

clean:
	-ifthere libgsl then wipe libgsl ~CFR~V
	-ifthere cdf.o.* then wipe cdf.o.* ~CFR~V
	-ifthere err.o.* then wipe err.o.* ~CFR~V
	-ifthere specfunc.o.* then wipe specfunc.o.* ~CFR~V
	-ifthere sys.o.* then wipe sys.o.* ~CFR~V

# Dynamic dependencies:
