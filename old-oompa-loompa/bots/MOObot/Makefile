COLFILES = README Makefile robot.manual run-colin.csh \
	   colin.c robot.c explore.c players.c reply1.c reply2.c \
	   rand.c words.c colin.h robot.h vars.h codeword.c \
	   status.muf pose.muf notify.muf score.muf

all: colin

codeword: codeword.c rand.o
	cc -DTESTING -g -o codeword codeword.c rand.o
	@rm -f codeword.o

colin: colin.o robot.o explore.o players.o reply1.o reply2.o rand.o \
	codeword.o words.o
	cc -g -o colin colin.o robot.o explore.o players.o \
	reply1.o reply2.o codeword.o rand.o words.o

colin.o: colin.c colin.h robot.h vars.h
	cc -g -c -D__STRICT_BSD__ colin.c

hash.o: hash.c hash.h
	cc -g -c -D__STRICT_BSD__ hash.c

reply1.o: reply1.c robot.h vars.h
	cc -g -c -D__STRICT_BSD__ reply1.c

reply2.o: reply2.c robot.h vars.h
	cc -g -c -D__STRICT_BSD__ reply2.c

robot.o: robot.c robot.h
	cc -g -c -D__STRICT_BSD__ robot.c

explore.o: explore.c robot.h vars.h
	cc -g -c -D__STRICT_BSD__ explore.c

players.o: players.c robot.h vars.h
	cc -g -c -D__STRICT_BSD__ players.c

words.o: words.c robot.h
	cc -g -c -D__STRICT_BSD__ words.c

rand.o: rand.c
	cc -g -c -D__STRICT_BSD__ rand.c

codeword.o: codeword.c
	cc -g -c -D__STRICT_BSD__ codeword.c

dist:
	packmail -odist -t'Robot Package' $(COLFILES)

tar:
	tar cvf - $(COLFILES) | compress > robot.tar.Z
	ls -l robot.tar.Z

clean:
	rm -f *.o core *.CKP *.BAK

