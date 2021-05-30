COLFILES = README Makefile robot.manual run-colin.csh \
	   colin.c robot.c explore.c players.c reply.c rand.c \
	   colin.h robot.h vars.h mn6cvt.c

all: colin julia mn6cvt

colin: colin.o robot.o explore.o players.o reply.o rand.o
	cc -g -o colin colin.o robot.o explore.o players.o reply.o rand.o

colin.o: colin.c colin.h robot.h vars.h
	cc -g -c colin.c

julia: julia.o robot.o explore.o players.o reply.o rand.o
	cc -g -o julia julia.o robot.o explore.o players.o reply.o rand.o

julia.o: julia.c julia.h robot.h vars.h
	cc -g -c julia.c

gloria: gloria.o robot.o explore.o players.o reply.o rand.o
	cc -g -o gloria gloria.o robot.o explore.o players.o reply.o rand.o

gloria.o: gloria.c gloria.h robot.h vars.h
	cc -g -c gloria.c

robot.o: robot.c robot.h
	cc -g -c robot.c

explore.o: explore.c robot.h vars.h
	cc -g -c explore.c

players.o: players.c robot.h vars.h
	cc -g -c players.c

reply.o: reply.c robot.h vars.h
	cc -g -c reply.c

rand.o: rand.c
	cc -g -c rand.c

mn6cvt: mn6cvt.c robot.h
	cc -g -o mn6cvt mn6cvt.c

dist:
	packmail -odist -t'Robot Package' $(COLFILES)

tar:
	tar cvf - $(COLFILES) | compress > robot.tar.Z
	ls -l robot.tar.Z

clean:
	rm -f *.o core *.CKP *.BAK

