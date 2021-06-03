CC=gcc
MUXHOME=../..
CFLAGS=-g -I$(MUXHOME) -w -I./mech
OBJ=mbankinterface.o mbank.o vmacctlist.o acct.o vmadslist.o vmminelist.o vmcomm.o vmroomlist.o vmcreate.o vmdamrep.o vmeng.o vmflags.o vmhelm.o vminterface.o vmmanlist.o vmmisc.o vmmissle.o vmmlist.o vmnav.o vmpower.o vmsecurity.o vmsensors.o vmspace.o vmspaceflags.o vmspacelist.o vmsystems.o vmupdate.o vmutils.o vmweaponq.o vmwlist.o atr.o bank.o combat.o combatcmd.o comm.o econ.o econinterface.o factory.o glue.o lists.o relays.o security.o skills.o string2.o tech.o vmrelaylist.o xcode.o gen.o elevator.o sbrep.o orglist.o vmads.o check.o comp.o conv.o debug.o map.o mech.advanced.o mech.build.o mech.contacts.o mech.los.o mech.maps.o mech.move.o mech.notify.o mech.physical.o mech.restrict.o mech.scan.o mech.startup.o mech.status.o mech.tic.o mech.update.o mech.utils.o mechrep.o mech.combat.o

SRC=../mbankinterface.c ../mbank.c vmacctlist.c acct.c vmadslist.c vmminelist.c vmcomm.c vmroomlist.c vmcreate.c vmdamrep.c vmeng.c vmflags.c vmhelm.c vminterface.c vmmanlist.c vmmisc.c vmmissle.c vmmlist.c vmnav.c vmpower.c vmsecurity.c vmsensors.c vmspace.c vmspaceflags.c vmspacelist.c vmsystems.c vmupdate.c vmutils.c vmweaponq.c vmwlist.c ../atr.c ../bank.c ../combat.c ../combatcmd.c ../comm.c ../econ.c ../econinterface.c ../factory.c ../glue.c ../lists.c ../relays.c ../security.c ../skills.c ../string2.c ../tech.c ../vmrelaylist.c ../xcode.c ../gen.c ../elevator.c sbrep.c orglist.c vmads.c mech/check.c mech/comp.c mech/conv.c mech/debug.c mech/map.c mech/mech.advanced.c mech/mech.build.c mech/mech.combat.c mech/mech.contacts.c mech/mech.los.c mech/mech.maps.c mech/mech.move.c mech/mech.notify.c mech/mech.physical.c mech/mech.restrict.c mech/mech.scan.c mech/mech.startup.c mech/mech.status.c mech/mech.tic.c mech/mech.update.c mech/mech.utils.c mech/mechrep.c

all: libspace.a

atr.o: ../atr.c
	$(CC) $(CFLAGS) -c ../atr.c 
bank.o: ../bank.c 
	$(CC) $(CFLAGS) -c ../bank.c 
mbank.o: ../mbank.c
	$(CC) $(CFLAGS) -c ../mbank.c
mbankinterface.o: ../mbankinterface.c
	$(CC) $(CFLAGS) -c ../mbankinterface.c
combat.o: ../combat.c
	$(CC) $(CFLAGS) -c ../combat.c 
combatcmd.o: ../combatcmd.c
	$(CC) $(CFLAGS) -c ../combatcmd.c 
comm.o: ../comm.c
	$(CC) $(CFLAGS) -c ../comm.c 
econ.o: ../econ.c
	$(CC) $(CFLAGS) -c ../econ.c 
econinterface.o: ../econinterface.c
	$(CC) $(CFLAGS) -c ../econinterface.c 
factory.o: ../factory.c
	$(CC) $(CFLAGS) -c ../factory.c 
glue.o: ../glue.c
	$(CC) $(CFLAGS) -c ../glue.c 
lists.o: ../lists.c
	$(CC) $(CFLAGS) -c ../lists.c 
relays.o: ../relays.c
	$(CC) $(CFLAGS) -c ../relays.c 
security.o: ../security.c
	$(CC) $(CFLAGS) -c ../security.c 
skills.o: ../skills.c
	$(CC) $(CFLAGS) -c ../skills.c 
string2.o: ../string2.c
	$(CC) $(CFLAGS) -c ../string2.c 
tech.o: ../tech.c
	$(CC) $(CFLAGS) -c ../tech.c 
vmrelaylist.o: ../vmrelaylist.c
	$(CC) $(CFLAGS) -c ../vmrelaylist.c 
xcode.o: ../xcode.c
	$(CC) $(CFLAGS) -c ../xcode.c 
gen.o: ../gen.c
	$(CC) $(CFLAGS) -c ../gen.c 
elevator.o: ../elevator.c
	$(CC) $(CFLAGS) -c ../elevator.c

vmminelist.o: vmminelist.c
	$(CC) $(CFLAGS) -c vmminelist.c
vmadslist.o: vmadslist.c vmadslist.h header.h
	$(CC) $(CFLAGS) -c vmadslist.c


check.o: mech/check.c
	$(CC) $(CFLAGS) -c mech/check.c 

comp.o: mech/comp.o
	$(CC) $(CFLAGS) -c mech/comp.c 

conv.o: mech/conv.o
	$(CC) $(CFLAGS) -c mech/conv.c 

debug.o: mech/debug.o
	$(CC) $(CFLAGS) -c mech/debug.c 

map.o: mech/map.o
	$(CC) $(CFLAGS) -c mech/map.c 

mech.advanced.o: mech/mech.advanced.o
	$(CC) $(CFLAGS) -c mech/mech.advanced.c 

mech.build.o: mech/mech.build.o
	$(CC) $(CFLAGS) -c mech/mech.build.c 

mech.combat.o: mech/mech.combat.o
	$(CC) $(CFLAGS) -c mech/mech.combat.c 

mech.contacts.o: mech/mech.contacts.o
	$(CC) $(CFLAGS) -c mech/mech.contacts.c 

mech.los.o: mech/mech.los.o
	$(CC) $(CFLAGS) -c mech/mech.los.c 

mech.maps.o: mech/mech.maps.o
	$(CC) $(CFLAGS) -c mech/mech.maps.c 

mech.move.o: mech/mech.move.o
	$(CC) $(CFLAGS) -c mech/mech.move.c 

mech.notify.o: mech/mech.notify.o
	$(CC) $(CFLAGS) -c mech/mech.notify.c 

mech.physical.o: mech/mech.physical.o
	$(CC) $(CFLAGS) -c mech/mech.physical.c 

mech.restrict.o: mech/mech.restrict.o
	$(CC) $(CFLAGS) -c mech/mech.restrict.c 

mech.scan.o: mech/mech.scan.o
	$(CC) $(CFLAGS) -c mech/mech.scan.c 

mech.startup.o: mech/mech.startup.o
	$(CC) $(CFLAGS) -c mech/mech.startup.c 

mech.status.o: mech/mech.status.o
	$(CC) $(CFLAGS) -c mech/mech.status.c 

mech.tic.o: mech/mech.tic.o
	$(CC) $(CFLAGS) -c mech/mech.tic.c 

mech.update.o: mech/mech.update.o
	$(CC) $(CFLAGS) -c mech/mech.update.c 

mech.utils.o: mech/mech.utils.o
	$(CC) $(CFLAGS) -c mech/mech.utils.c 

mechrep.o: mech/mechrep.o
	$(CC) $(CFLAGS) -c mech/mechrep.c



libspace.a: $(OBJ)
	rm -f libvmspace.a
	ar -qc libvmspace.a $(OBJ)
	ranlib libvmspace.a
clean: 
	-rm -f libvmspace.a *.o 

dist: $(SRC)
	rm -f ~/vmspace.tgz
	(cd /export/home/mike/mux/src;tar -cf /export/home/mike/mux/vmspace.tar valcode valcode-real;gzip /export/home/mike/mux/vmspace.tar)
out: $(SRC)
rcsdist: 
	rm -f ~/vmspace.tgz
	(cd /export/home/mike/mux/src;tar -cf /export/home/mike/mux/vmspacercs.tar valcode/RCS valcode/vmspace/RCS;gzip /export/home/mike/mux/vmspacercs.tar)
##These definitions are needed for RCS
vmacctlist.o: vmacctlist.c
vmacctlist.c: header.h vmacctlist.h 
header.h: vmconfig.h vmspace.h mike.h vmspaceflags.h extern.h vmmissle.h vmpower.h ../vmrelaylist.h ../cmds.h ../lists.h
vmspace.h: vmcomm.h vmtemp.h vmroomlist.h vmspacelist.h vmmanlist.h vmmlist.h vmhelm.h vmnav.h vmeng.h vmdefs.h vmwlist.h vmweaponstuff.h vmsystems.h vmminelist.h vmsensors.h vmadslist.h proto.h
vmflags.c: vmflags.h
vmhelm.c: vmweaptypes.h
vmsecurity.c: vmsecurity.h
../glue.c: ../glue.h vminterface.h ../relays.h ../elevator.h
orglist.c: orglist.h
../mbankinterface.c: ../mbankinterface.h ../mbank.h
../atr.c: ../header.h
../header.h: ../gen.h
../combat.c: ../autocombat.h
../comm.c: ../comm.h proto.h
../econ.c: ../econ.h
../skills.c: ../skills.h
../bank.c: ../econ.h
