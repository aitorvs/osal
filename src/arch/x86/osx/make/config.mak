#-*-Makefile-*-
###############################################################################
## config.mak - This is the configuration file for the OSAL.
##
## Modifications:
##
###############################################################################
## Begin configuration options **
## Debug & optimization options
DEBUG=-g -O0
DEBUG_LEVEL=3
#DEBUG=-g -O2

WARNINGS=-Wall

#################################################################################
## RTEMS target defintions
##

CAT=cat
TR=tr
RM=rm
CP=cp
CHMOD=chmod

COMPILER=gcc
CPPOMPILER=g++
ASSEMBLER=as
LINKER=ld
AR=ar
NM=nm
SIZE=size
OBJCOPY=objcopy
OBJDUMP=objdump

#################################################################################
## Relevant directories
##
## Note BUILDBASE must be set up right before getting here....
##
SRCDIR=$(BUILDBASE)/common
INCDIR=$(BUILDBASE)/include
HWINCDIR=$(BUILDBASE)/arch/$(HWARCH)/$(CPU)/inc
OBJDIR=obj/$(HWARCH)/$(OS)

#################################################################################
## Compiler suite config options
##

TARGETDEFS=-D_OSX_
## Used to be -D$(OSTARGET)

## x86 = _EL, PPC & MongooseV = _EB  -I$(BSP_INCDIR) 

COPTS=

# Note: This include directory should not be available to tasks!
#        this would bypass the osal..
INCS+=

ENDIAN=-D_EL -DENDIAN=_EL -DSOFTWARE_LITTLE_BIT_ORDER 

ASOPTS=

# everybody gets the math lib
LIBS+=-lc
# LIBS+=-lgcc
LIBS+=-lm
LIBS+=-lstdc++
LIBS+=-lpthread

# -I$(BSP_INCDIR)

INCS+=-I$(INCDIR) -I. -I$(HWINCDIR) $(LOCALINCS)

LIBDIR=

# general gcc options that apply to compiling and dependency generation
COPT_D=$(COPTS) $(ENDIAN) $(TARGETDEFS) -DMS_HOST -D_REENTRANT $(INCS) $(LOCALCOPTS) -D$(HWARCH) -D _EMBED_ -DOS_DEBUG_LEVEL=$(DEBUG_LEVEL) -DFLIGHT_PACK

# additional options only used for compiling that generates objects
COPT_T=$(DEBUG) $(COPT_D) $(WARNINGS) -c

DEPENDFILE=depend.mak
TBLDEPENDFILE=tbldepend.mak

# LDFLAGS = -L $(LIBDIR) -N -M
LDFLAGS = 


## End configuration options ##

