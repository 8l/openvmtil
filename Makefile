
# major cleanup (deletions) in 0.737.042 - 20130717

SOURCES = core/compiler/machineCode.c core/compiler/compile.c core/compiler/memory.c\
	core/compiler/combinators.c core/compiler/math.c core/compiler/cpu.c \
	core/compiler/stack.c core/compiler/sequence.c core/compiler/logic.c core/dataObjectRun.c\
	core/compiler/conditionals.c core/compiler/blocks.c core/interpreter.c core/compile.c\
	core/compiler/optimize.c core/compiler/bits.c core/compiler/udis.c core/compiler/arrays.c \
	core/io.c core/compiler/_debug.c core/symbol.c core/repl.c core/syntax.c core/dataObjectNew.c\
        core/cfrtil.c core/parse.c core/memSpace.c core/init.c core/system.c core/charSet.c\
	core/dllist.c core/interpret.c core/lexer.c core/cstack.c core/classes.c core/debugOutput.c\
	core/namespace.c core/history.c core/readline.c core/dataStack.c core/context.c\
	core/_system.c core/word.c core/readTable.c core/bigNum.c core/readinline.c core/array.c\
	core/compiler.c core/dllnodes.c core/finder.c core/tabCompletion.c core/colors.c\
	core/string.c core/dobject.c core/openVmTil.c core/object.c core/property.c core/lists.c\
	core/linux.c core/exception.c core/types.c core/lambdaCalculus.c core/locals.c core/debug.c\
	primitives/strings.c primitives/bits.c primitives/maths.c primitives/logics.c\
	primitives/ios.c primitives/parsers.c primitives/interpreter.c primitives/namespaces.c primitives/systems.c\
	primitives/stack.c primitives/compiler.c primitives/words.c  primitives/file.c\
	primitives/debugger.c primitives/memory.c primitives/primitives.c primitives/contexts.c\
	primitives/disassembler.c primitives/syntax.c
	#primitives/eval1.c
	#primitives/eval2.c
	#primitives/eval.c
	#primitives/boot-eval.c 
	#primitives/sl5.c 
	#primitives/bootForth.c primitives/maru.c primitives/tiny.c 
	#primitives/retro-sockets.c primitives/joy.c primitives/maru.c 
	#primitives/boot-eval.c primitives/lisps.c 

INCLUDES = includes/machineCode.h includes/defines.h includes/types.h \
	includes/cfrtil.h includes/macros.h \
	includes/machineCodeMacros.h includes/stacks.h #includes/gc.h

OBJECTS = $(SOURCES:%.c=%.o) 
#CC = g++
#CC = gcc -std=gnu99 #-std=c99
#CC = clang
CC = gcc
OUT = cfrtil-gdb

default : debug

debug : cfrtil-gdb
run : cfrtil
all: cfrtil cfrtil-gdb


CFLAGS_CORE = -m32 -march=core2 -finline-functions # -O3 -fomit-frame-pointer
CFLAGS = $(CFLAGS_CORE) -Wall 
LIBS = -ludis86 -lgmp -lrt -lc -ldl -lm 
#LIBS = -ludis86 -lgmp -lrt -lc -ldl -lm #-lffi -lgc

_clean : 
	-rm core/*.o primitives/*.o core/compiler/*.o cfrtil cfrtil-gdb
	#-cd $(joy) && make -f make.gc clean

clean : 
	make _clean
	touch includes/defines.h
	make includes/prototypes.h

includes/prototypes.h : $(INCLUDES)
	cp includes/_proto.h includes/prototypes.h
	cproto -o proto.h $(SOURCES)
	mv proto.h includes/prototypes.h
	make _clean

cfrtil : CFLAGS = $(CFLAGS_CORE)
cfrtil : includes/prototypes.h $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o cfrtil $(LIBS)
	strip cfrtil
	cp cfrtil bin/
	
cfrtil-gdb : CFLAGS = $(CFLAGS_CORE) -ggdb 
cfrtil-gdb : includes/prototypes.h $(OBJECTS) 
	$(CC) $(CFLAGS) $(OBJECTS) -o cfrtil-gdb $(LIBS)
	strip -o cfrtil cfrtil-gdb
	cp cfrtil bin/

cfrtilo : $(OBJECTS)
	$(CC) $(CFLAGS) core/compiler/*.o core/*.o primitives/*.o -o $(OUT) $(LIBS)
	strip $(OUT)

_cfrtil_O1 : CFLAGS = $(CFLAGS_CORE) -O1
_cfrtil_O1 : OUT = cfrtilo1
_cfrtil_O1 : cfrtilo

_cfrtil_O2 : CFLAGS = $(CFLAGS_CORE) -O2
_cfrtil_O2 : OUT = cfrtilo2
_cfrtil_O2 : cfrtilo

_cfrtil_O3 : CFLAGS = $(CFLAGS_CORE) -O3
_cfrtil_O3 : OUT = cfrtilo3
_cfrtil_O3 : cfrtilo

_cfrtilo :  includes/prototypes.h $(OBJECTS)
	$(CC) $(CFLAGS) core/compiler/*.o core/*.o primitives/*.o -o $(OUT) $(LIBS)

primitives/maths.o : primitives/maths.c
	$(CC) $(CFLAGS) -O3 -c primitives/maths.c -o primitives/maths.o
	
#core/readline.o : core/readline.o
#	$(CC) $(CFLAGS) -O3 -c core/readline.c -o core/readline.o
	
primitives/sl5.o : primitives/sl5.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/sl5.c -o primitives/sl5.o
	
primitives/boot-eval.o : primitives/boot-eval.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/boot-eval.c -o primitives/boot-eval.o
	
primitives/eval.o : primitives/eval.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval.c -o primitives/eval.o
	
primitives/eval1.o : primitives/eval1.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval1.c -o primitives/eval1.o
	
primitives/eval2.o : primitives/eval2.c
	$(CC) $(CFLAGS_CORE) -ggdb -w -fpermissive -c primitives/eval2.c -o primitives/eval2.o
	
primitives/wcs.o : primitives/wcs.c
	$(CC) $(CFLAGS_CORE) -w -fpermissive -c primitives/wcs.c -o primitives/wcs.o
	
primitives/libgc.o : primitives/libgc.c
	$(CC) $(CFLAGS_CORE) -w -fpermissive -c primitives/libgc.c -o primitives/libgc.o
	
primitives/gc.o : primitives/gc.c
	$(CC) $(CFLAGS) -w -fpermissive -c primitives/gc.c -o primitives/gc.o
	
tags :
	ctags -R --c++-types=+px --excmd=pattern --exclude=Makefile --exclude=. /home/dennisj/projects/workspace/cfrtil

proto:
	touch includes/defines.h
	make includes/prototypes.h

optimize1 : _clean _cfrtil_O1

optimize2 : _clean _cfrtil_O2

optimize3 : _clean _cfrtil_O3

optimize :
	make optimize1
	make optimize2
	make optimize3
	make clean
	make
	-rm bin/cfrtilo*
	mv cfrtil cfrtilo* bin

cleanCfrtil : clean cfrtil

cleanCfrtil-gdb : clean cfrtil-gdb

cleaned :
	make _cfrtil
	make cfrtil-gdb

editorClean :
	rm *.*~ core/*.*~ core/compiler/*.*~ primitives/*.*~ includes/*.*~

realClean : _clean editorClean
	rm cfrtil cfrtil-gdb

udis :
	wget http://prdownloads.sourceforge.net/udis86/udis86-1.7.2.tar.gz
	tar -xvf udis86-1.7.2.tar.gz 
	cd udis86-1.7.2 && \
	./configure CFLAGS=-m32 --enable-shared && \
	make && \
	sudo make install && \
	sudo ldconfig
	
gmp : 
	wget https://ftp.gnu.org/gnu/gmp/gmp-6.0.0a.tar.bz2
	tar -xjvf gmp-6.0.0a.tar.bz2
	cd gmp-6.0.0 && \
	./configure CFLAGS=-m32 ABI=32 --enable-shared && \
	make && \
	sudo make install && \
	sudo ldconfig
	
cproto :
	wget https://launchpad.net/ubuntu/+archive/primary/+files/cproto_4.7l.orig.tar.gz
	tar -xvf cproto_4.7l.orig.tar.gz
	cd cproto-4.7l && \
	./configure CFLAGS=-m32 && \
	make && \
	sudo make install
	
# suffix for zipfiles
suffix = *.[cfht^~]*
sfx = *.[a-np-wz]*
zip : 
	-mv nbproject ../backup
	zip  cfrtil.zip */$(sfx) */*/$(sfx) *.htm* Makefile m r script* netbeansReset *.cft* retroImage $(sfx)			    
	-mv ../backup/nbproject .

_xz : 
	-rm -rf ~/backup/cfrtil.bak
	-mv ~/backup/cfrtil ~/backup/cfrtil.bak
	-mv .git ..
	-cp -r ~/openvmtil-code ~/backup/
	-mv ../.git .
	-mv ~/backup/openvmtil-code ~/backup/cfrtil
	-cp cfrtil bin
	-rm cfrtil
	tar -c --xz --exclude=nbproject --exclude=.git --exclude=*.png --exclude=cfrtil-gdb  --exclude=*.o -f cfrtil.tar.xz * *.* .init.cft
	-cp bin/cfrtil cfrtil

xz : 
	-make default
	make _xz

_all : realClean install
	make xz

_install :
	sudo cp bin/cfrtil /usr/local/bin/cfrtil
	-sudo mkdir /usr/local/lib/cfrTil
	-sudo cp ./.init.cft namespaces
	-sudo cp ./.init.cft /usr/local/lib/cfrTil
	-sudo cp -r namespaces /usr/local/lib/cfrTil
	-sudo cp lib/lib*.* /usr/local/lib
#	-sudo chown dennisj /usr/local/bin/cfrtil
	-sudo ldconfig

_install_1 :
	sudo cp bin/cfrtil /usr/local/bin/cfrtil
#	-sudo mkdir /usr/local/lib/cfrTil
	-sudo cp ./.init.cft namespaces
	-sudo cp ./.init.cft /usr/local/lib/cfrTil
	-sudo cp -r namespaces /usr/local/lib/cfrTil
#	-sudo cp lib/lib*.* /usr/local/lib
#	-sudo chown dennisj /usr/local/bin/cfrtil
#	-sudo ldconfig

install0 :
	make _clean
	make 
	make _install
	ls -l ./cfrtil
	
install :
	make
	make _install
	ls -l bin/cfrtil
	
install_1 :
	make
	make _install_1
	ls -l /usr/local/bin/cfrtil
	
run :
	cfrtil

runLocal :
	./cfrtil

#	as -adhls=vm.list -o stack.o vm.s
# DO NOT DELETE
