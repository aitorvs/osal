# -*-Makefile-*-
###############################################################################
##
## Filename: stdtask.mak
##
## Purpose:  ST5 shared makefile definitions
##
## Modifications:
##
## 05/10/02  A.Ferrer, Code 582, Makefile cleanup.  Removed maketasklist rule.
##           Renamed PROCS to DIRS.  Added comments.
##
##############################################################################
##
## Basic dependency rule

# Rule to make the specified TARGET
# The final output in any directory is the target object
LOPT=-r       #-r option needed to force link to not try to resolve dependencies

$(TARGET): $(OBJS)
	$(LINKER) $(LOPT) $(OBJS) -o $(TARGET)

###############################################################################
##  ASSEMBLY CODE RULE
##
.s.o:
	$(ASSEMBLER) $(ASOPTS) -o $*.o $*.S

###############################################################################
##  "C" COMPILER RULE
##
.c.o:
	$(COMPILER) $(COPT_T) $*.c

###############################################################################
##  "C++" COMPILER RULE
##
.cpp.o:
#	$(COMPILER) $(COPT_T) $*.cpp
	$(CPPOMPILER) $(COPT_T) $*.cpp

##############################################################################
##
clean ::
	find . -name "*.o" | (while read fn; do rm "$$fn"; done)
	find . -name "*.lis" | (while read fn; do rm "$$fn"; done)
##############################################################################
##
##
depend dep:
	$(COMPILER) -M $(COPT_D) $(SOURCES) > $(TMPDIR)/$(DEPENDFILE)
	$(CAT) $(TMPDIR)/$(DEPENDFILE) | $(TR) -d \\r > $(DEPENDFILE)

tbldepend:
	$(COMPILER) -M $(COPT_D) $(SOURCES) > $(TMPDIR)/$(TBLDEPENDFILE)
	$(CAT) $(TMPDIR)/$(TBLDEPENDFILE) | $(TR) -d \\r > $(TBLDEPENDFILE)


# eof
