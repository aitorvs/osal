
# Massive Memory Unit Software for RASTA -- main makefile
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

-include .config
include defs.mk

.PHONY: fake all menuconfig m g clean distclean

all: $(LIB) $(TARGETS)

profile : $(GMON)
	@echo " CC  $(OGMON)"
	$(CC) $(GMONFLAGS) $(CPPFLAGS) -c -o $(OGMON) $(GMON)

fake :
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(CC)

$(TARGETS) : % : %.o $(OBJS)
	@echo " LD  $@"
	@$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $@.o $(LIB) -lm $(LDFLAGS)
ifeq ($(CONFIG_RTEMS), y)
	@-$(MKPROM) $(MK_OPTS) $@ -o $@.prom > /dev/null
endif

$(LIB) : profile $(OBJS)
	@echo " AR  $@"
	@$(AR) cr $(LIB) $(OBJS) $(OGMON)

tsim :
	xterm -geometry 100x15 -e tsim-leon -port 1234 -gdb &

grmon :
	xterm -geometry 100x15 -e /opt/grmon/linux/grmon -abaoud 115200 -uart /dev/ttyS3 -dsu -leon2 -ramws 2 -romws 10 -nosram  -stack 0x40ffff00 -gdb &
#xterm -geometry 100x15 -e /opt/grmon/linux/grmon -abaoud 115200 -uart /dev/ttyS3 -dsu -leon2 -ramws 2 -romws 10  -gdb &
g :
	stty -F /dev/ttyS2 38400 cs8 -parenb -cstopb -crtscts
	xterm -geometry 100x32 -e cat /dev/ttyS2 | tee /tmp/rtems-tmtc.log&
minicom :
	xterm -geometry 200x32 -e minicom ttyS2 &
gdb :
	/opt/rtems-4.6/bin/sparc-rtems-gdb ./samples/fpss/psas_thruput

menuconfig .config : rules.out
	@echo " CNF $<"
	@./tools/cml2/cmlconfigure.py -c -i .config -o .config
	@./tools/cml2/configtrans.py -h include/public/osal_config.h .config
	@rm -f $(DEPS)

include/public/osal_config.h : .config
	@echo " CNF $<"
	@./tools/cml2/configtrans.py -h include/public/osal_config.h .config

clean :
	@rm -f $(TARGETS) $(MOBJS) $(OBJS) $(LIB) $(OGMON)
	@-rm $(shell find . -name *.prom)
	@echo Cleaning done.

distclean : clean
	@rm -f rules.out .config $R/include/public/osal_config.h
	@rm -f $(DEPS)
	@echo Mr. Proper done.

install_target :
	@$(CHK_DIR_EXISTS) $(CONFIG_OSAL_INSTALL_ROOT)/libosal/ || $(MKDIR) $(CONFIG_OSAL_INSTALL_ROOT)/libosal/ 
	$(INSTALL_FILE) $(LIB) $(CONFIG_OSAL_INSTALL_ROOT)/libosal/

install_headerfiles :
	$(INSTALL_DIR) ./include/ $(CONFIG_OSAL_INSTALL_ROOT)/

install : all install_target install_headerfiles

-include $(DEPS)

