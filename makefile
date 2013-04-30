#
# Makefile of atalanta
#
CFLAGS = -O -g
GDATA =	define.h global.h macro.h parameter.h stdafx.h
CC=cc
#
# for atpg
#
SRCS =	error.cpp hash.cpp help.cpp read_cct.cpp structure.cpp io.cpp pio.cpp \
	stem.cpp define_fault_list.cpp testability.cpp fan.cpp \
	learn.cpp ppsfp.cpp truth.cpp lsim.cpp fsim.cpp random.cpp print.cpp sim.cpp atalanta.cpp
HEADERS = error.h hash.h help.h read_cct.h structure.h io.h pio.h \
	stem.h define_fault_list.h testability.h fan.h \
	learn.h ppsfp.h truthtable.h lsim.h fsim.h random.h print.h sim.h atalanta.h
OBJS =	error.o hash.o help.o read_cct.o structure.o io.o pio.o \
	stem.o define_fault_list.o testability.o fan.o \
	learn.o ppsfp.o truth.o lsim.o fsim.o random.o print.o sim.o atalanta.o




#SRCS =	error.cpp hash.cpp help.cpp read_cct.cpp structure.cpp io.cpp pio.cpp \
#	stem.cpp define_fault_list.cpp testability.cpp fan.cpp help_fsim.cpp \
#	learn.cpp ppsfp.cpp truth.cpp lsim.cpp fsim.cpp random.cpp print.cpp sim.cpp main_fsim.cpp
#OBJS =	error.o hash.o help.o read_cct.o structure.o io.o pio.o \
#	stem.o define_fault_list.o testability.o fan.o help_fsim.o \
#	learn.o ppsfp.o truth.o lsim.o fsim.o random.o print.o sim.o main_fsim.o

#
# for fault simulator
#
OBJSF =	error.o hash.o help_fsim.o read_cct.o structure.o io.o pio.o \
	define_fault_list.o ppsfp.o random.o print.o main_fsim.o

atalanta:	$(OBJS) $(GDATA) $(HEADERS)
	g++ $(CFLAGS) $(OBJS) -o atalanta

fsim:	$(OBJSF) $(GDATA) $(HEADERS)
	g++ $(CFLAGS) $(OBJSF) -o fsim

atalanta.o:	atalanta.cpp $(GDATA) $(HEADERS)
	g++ $(CFLAGS) -c atalanta.cpp

main_fsim.o: main_fsim.cpp $(GDATA) $(HEADERS)
	g++ $(CFLAGS) -c main_fsim.cpp

io.o:	io.cpp io.h $(GDATA)
	g++ $(CFLAGS) -c io.cpp

print.o: print.cpp print.h $(GDATA) pio.h
	g++ $(CFLAGS) -c print.cpp

pio.o: pio.cpp pio.h $(GDATA)
	g++ $(CFLAGS) -c pio.cpp

stem.o:	stem.cpp stem.h $(GDATA) error.h
	g++ $(CFLAGS) -c stem.cpp

define_fault_list.o: define_fault_list.cpp define_fault_list.h $(GDATA) error.h hash.h
	g++ $(CFLAGS) -c define_fault_list.cpp

testability.o: testability.cpp testability.h $(GDATA)
	g++ $(CFLAGS) -c testability.cpp

fan.o:	fan.cpp fan.h $(GDATA) learn.h sim.h
	g++ $(CFLAGS) -c fan.cpp

learn.o: learn.cpp learn.h $(GDATA) error.h
	g++ $(CFLAGS) -c learn.cpp

ppsfp.o: ppsfp.cpp ppsfp.h $(GDATA)
	g++ $(CFLAGS) -c ppsfp.cpp

truth.o: truth.cpp $(GDATA)
	g++ $(CFLAGS) -c truth.cpp

lsim.o:	lsim.cpp lsim.h $(GDATA)
	g++ $(CFLAGS) -c lsim.cpp

fsim.o:	fsim.cpp fsim.h $(GDATA) error.h
	g++ $(CFLAGS) -c fsim.cpp

random.o:	random.cpp random.h
	g++ $(CFLAGS) -c random.cpp

hash.o:	hash.cpp hash.h $(GDATA) error.h
	g++ $(CFLAGS) -c hash.cpp

error.o: error.cpp error.h $(GDATA)
	g++ $(CFLAGS) -c error.cpp

help.o:	help.cpp help.h $(GDATA)
	g++ $(CFLAGS) -c help.cpp

help_fsim.o:	help_fsim.cpp help_fsim.h $(GDATA)
	g++ $(CFLAGS) -c help_fsim.cpp

read_cct.o:	read_cct.cpp read_cct.h $(GDATA) error.h hash.h
	g++ $(CFLAGS) -c read_cct.cpp

structure.o:	structure.cpp structure.h $(GDATA) error.h hash.h
	g++ $(CFLAGS) -c structure.cpp

sim.o:	sim.cpp sim.h $(GDATA) random.h ppsfp.h pio.h lsim.h fsim.h fan.h define_fault_list.h io.h
	g++ $(CFLAGS) -c sim.cpp

lint:
	lint $(SRCS)

clear:
	rm -f *.o

