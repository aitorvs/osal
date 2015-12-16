
# Massive Memory Unit Software for RASTA -- make definitions
#
# Copyright (C) 2009  Cesar Rodriguez <cesar.rodriguez@srg.aut.uah.es>
# Space Research group, Universidad de Alcala (Spain)
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

COPY          = cp -f
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p
MKPROM		  = /opt/mkprom2/mkprom2
MK_OPTS		  = -rmw -$(CONFIG_ARCH) -baud 38400 -ramsize 4096 -romsize 2048 -freq 80 -ccprefix $(PCROSS)
DLIB_OSAL	  = $(CONFIG_OSAL_INSTALL_ROOT)/libosal
#RTEMS_PATH	  = /opt/rtems-4.10-tmtc/bin/sparc-rtems
#RTEMS_PATH	  = /opt/rtems-4.6/bin/sparc-rtems
#RTEMS_PATH	  = /opt/rtems-4.8-tmtc/bin/sparc-rtems
RTEMS_PATH	  = /opt/rtems-4.8-tmtc
RTEMS_LINKCMDS= $(RTEMS_PATH)/sparc-rtems/$(CONFIG_ARCH)/lib/linkcmds

# traditional variables
R:=$(shell pwd)
CFLAGS:=-Wall -g -Wshadow -Wstrict-prototypes -Wredundant-decls
GMONFLAGS:=
CPPFLAGS:=-I include/ 
LDFLAGS:= -Wl,-Map,linkmap

ifeq ($(CONFIG_LINUX), y)
GMON:=$(wildcard $R/src/os/linux/gmon.c)
endif
ifeq ($(CONFIG_RTEMS), y)
GMON:=$(wildcard $R/src/os/rtems/gmon.c)
endif

# RTEMS networking tasks allocate a lot of memory. The heap size must be
# increased at least to 256Kbytes
#ifeq ($(CONFIG_OS_ENABLE_NETWORKING), y)
LDFLAGS+= -Wl,--defsym -Wl,HeapSize=0x80000
#endif

ifeq ($(CONFIG_DEBUG), y)
LIB:=libosal_$(CONFIG_OS)_$(CONFIG_PLATFORM)$(CONFIG_ARCH)_dbg.a
LOSAL:=osal_$(CONFIG_OS)_$(CONFIG_PLATFORM)$(CONFIG_ARCH)_dbg
else
LIB:=libosal_$(CONFIG_OS)_$(CONFIG_PLATFORM)$(CONFIG_ARCH).a
LOSAL:=osal_$(CONFIG_OS)_$(CONFIG_PLATFORM)$(CONFIG_ARCH)
endif


ifeq ($(CONFIG_LINUX), y)

ifeq ($(CONFIG_PC), y)
LDFLAGS+= -lpthread -lrt -lm
endif

endif

ifeq ($(CONFIG_RTEMS), y)

ifeq ($(CONFIG_RASTA), y)
LDFLAGS+= -qnolinkcmds -T scripts/linkcmds.$(CONFIG_ARCH)
CPPFLAGS+= -I/opt/libbackport/include
LDFLAGS+= -L/opt/libbackport/lib -lbackport_$(CONFIG_ARCH)-4.10-4.8
endif

ifeq ($(CONFIG_SPARC), y)

ifeq ($(CONFIG_LEON2), y)
CPPFLAGS+= -DRTEMS -qleon2
CFLAGS+= -DLEON2
#CPPFLAGS+= -DRTEMS -isystem $(RTEMS_PATH)/sparc-rtems/leon2/lib/include

endif ## end of LEON2

ifeq ($(CONFIG_LEON3), y)
CPPFLAGS+= -DRTEMS #-isystem $(RTEMS_PATH)/sparc-rtems/leon3/lib/include
CFLAGS+= -DLEON3
endif ## end of LEON3

CFLAGS+= -msoft-float
CFLAGS+=-O3
endif ## end of SPARC
endif ## end of RTEMS

GMONFLAGS+=$(CFLAGS)
ifeq ($(CONFIG_OS_PROFIL_ENABLE), y)
CFLAGS+= -pg
LDFLAGS += -Wl,--wrap,Clock_isr
endif


ifeq ($(CONFIG_OS_MALLOC_DEBUG_LIB), y)
#LDFLAGS+= -Wl,--wrap,malloc -Wl,--wrap,free
endif

# object file targets
# Common code
SRCS:=
SRCS+=$(wildcard $R/src/common/*c)
SRCS+=$(wildcard $R/src/glue/*c)

# OSAL code
ifeq ($(CONFIG_LINUX), y)
SRCS+=$(wildcard $R/src/os/linux/*c)
SRCS+=$(wildcard $R/src/arch/x86/common/*c)
SRCS+=$(wildcard $R/src/arch/x86/hal/*c)
SRCS+=$(wildcard $R/src/arch/x86/linux/bsp/*c)
#SRCS:=$(filter-out $(addprefix $R/src/os/linux/, ostimer.c), $(SRCS))
#SRCS:=$(filter-out $(addprefix $R/src/os/linux/, osqueue.c), $(SRCS))

SRCS:=$(filter-out $(addprefix $R/src/os/linux/, gmon.c), $(SRCS))
endif

ifeq ($(CONFIG_RTEMS), y)
SRCS+=$(wildcard $R/src/os/rtems/*c)
SRCS:=$(filter-out $(addprefix $R/src/os/rtems/, gmon.c), $(SRCS))
#SRCS:=$(filter-out $(addprefix $R/src/os/rtems/, osqueue.c), $(SRCS))

ifeq ($(CONFIG_LEON2), y)
SRCS+=$(wildcard $R/src/arch/sparc/leon2/rtems/bsp/*c)
endif

ifeq ($(CONFIG_LEON3), y)
SRCS+=$(wildcard $R/src/arch/sparc/leon3/rtems/bsp/*c)
endif

SRCS+=$(wildcard $R/src/arch/sparc/common/rtems/*c)
SRCS:=$(filter-out $(addprefix $R/src/os/rtems/, oshandlers.c), $(SRCS))
SRCS:=$(filter-out $(addprefix $R/src/arch/sparc/common/rtems/, rtems-network.c rtems4.10-network.c), $(SRCS))

endif

OBJS:=$(patsubst %.c,%.o,$(SRCS))
OGMON:=$(patsubst %.c,%.o,$(GMON))

##	------ TESTS ----------
MSRCS:=

################################################
############### CORE examples	################
################################################
ifeq ($(CONFIG_TEST_CORE_ENABLE), y)
MSRCS+=$R/samples/core/timer_linux.c
MSRCS+=$R/samples/core/timer_tod.c
MSRCS+=$R/samples/core/timer_simple.c
MSRCS+=$R/samples/core/time_gettime.c
MSRCS+=$R/samples/core/time_getticks.c
MSRCS+=$R/samples/core/task_sporadic.c
MSRCS+=$R/samples/core/task_monotonic.c
MSRCS+=$R/samples/core/task_yield.c
MSRCS+=$R/samples/core/task_suspend.c
MSRCS+=$R/samples/core/errno.c
MSRCS+=$R/samples/core/queue.c
MSRCS+=$R/samples/core/sem_counting.c
MSRCS+=$R/samples/core/sem_flush.c
MSRCS+=$R/samples/core/clockbug.c
MSRCS+=$R/samples/core/mem_mgr.c
MSRCS+=$R/samples/core/mem_pool.c
MSRCS+=$R/samples/core/deadman.c

##	If the memory is compiled under OSAL enable the test too
ifeq ($(CONFIG_OS_MEMMGR_ENABLE), y)
endif

endif

#MSRCS+=$R/samples/demo/demo.c

################################################
############### FPSS examples	################
################################################

################################################
############### MISC examples	################
################################################

ifeq ($(CONFIG_TEST_MISC_ENABLE), y)
#MSRCS+=$R/samples/misc/dhry_1.c
#MSRCS+=$R/samples/misc/whetstone.c
MSRCS+=$R/samples/misc/whets.c
MSRCS+=$R/samples/misc/quicksort.c
MSRCS+=$R/samples/misc/bubble.c
MSRCS+=$R/samples/misc/hanoi.c
MSRCS+=$R/samples/misc/matmul.c
MSRCS+=$R/samples/misc/perm.c
MSRCS+=$R/samples/misc/qsort.c
MSRCS+=$R/samples/misc/queen.c
MSRCS+=$R/samples/misc/sieve.c
ifeq ($(CONFIG_RASTA), y)
#MSRCS+=$R/samples/rasta-spw/echos.c
#MSRCS+=$R/samples/rasta-spw/echoc.c
#MSRCS+=$R/samples/rasta-spw/bwc.c
#MSRCS+=$R/samples/rasta-spw/bws.c
endif
ifeq ($(CONFIG_OS_ENABLE_NETWORKING), y)
MSRCS+=$R/samples/misc/soclient.c
endif

endif

MOBJS:=$(patsubst %.c,%.o,$(MSRCS))

# dependency files
DEPS:=$(patsubst %.o,%.d,$(OBJS)) $(patsubst %.o,%.d,$(MOBJS))

# compilation targets
TARGETS:=$(patsubst %.o,%,$(MOBJS))

# define the toolchain
CROSS:="not-defined-"
ifeq ($(CONFIG_RTEMS), y)
PCROSS:=$(RTEMS_PATH)/bin/sparc-rtems
CROSS:=$(PCROSS)-
endif
ifeq ($(CONFIG_LINUX), y)
CROSS:=
endif

AS:=$(CROSS)as
LD:=$(CROSS)ld
CC:=$(CROSS)gcc
CXX:=$(CROSS)g++
CPP:=$(CROSS)cpp
AR:=$(CROSS)ar
NM:=$(CROSS)nm
OBJCPY:=$(CROSS)objcopy
OBJDUMP:=$(CROSS)objdump
STRIP:=$(CROSS)strip

%.d : %.c
	@echo " DEP $<"
	@set -e; $(CC) -MM -MT $*.o $(CPPFLAGS) $< | \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@;

.c.o:
	@echo " CC  $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.out : %.cml
	@echo " CNF $<"
	@./tools/cml2/cmlcompile.py -o $@ $<

