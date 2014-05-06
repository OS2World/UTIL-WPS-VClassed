
default:
	@echo VClassed's Makefile 
	@echo -------------------
	@echo make all          : Creates zip archive with all what's needed for distribution
	@echo make vclassed.exe : Creates main program
	@echo make install.exe  : Creates install program
	@echo make pack         : Creates distribution packages
	@echo make dist         : Same as 'make all'
	@echo make distsrc      : Creates source package

all: zip

procs.obj: procs.c
	gcc -Zomf -c procs.c

vclassed.exe: res/VClassed.RES procs.obj vclassed.h
	gcc -Zomf -Zsys vclassed.c procs.obj res/VClassed.RES vclassed.def

install.res: res/install.rc res/install.ico
	@echo Building Install's resources
	rc -r res/install.rc install.res

install.exe: install.c install.h install.res
	gcc -Zomf -Zsys install.c install.RES install.def

vclassed.pak: install.exe vclassed.exe doc/readme.txt doc/vclassed16.txt doc/COPYING res/vclassed.ico
	@echo Building VClassed pack
	@echo vclassed.exe > pack.txt
	@echo doc\readme.txt >> pack.txt
	@echo doc\vclassed16.txt >> pack.txt
	@echo doc\COPYING >> pack.txt
	@echo res\vclassed.ico >> pack.txt
	@pack pack.txt vclassed.pak /l
	@rm pack.txt

dist: vclassed.pak install.exe 
	@echo Making distribution archive
	@zip -j VClassed16 install.exe vclassed.pak doc\readme.txt

distsrc:
	@echo Making distribution archive
	zip VClassed16src doc/*
	zip VClassed16src res/*
	zip VClassed16src install.c install.h install.def vclassed.c procs.c vclassed.h vclassed.def Makefile

clean:
	@echo Cleaning up
	@rm *.obj *.exe install.res *.pak *.zip pack.txt