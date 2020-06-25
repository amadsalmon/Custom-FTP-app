OBJ1 = fon.o client.o 
OBJ2 = fon.o serveur.o 
OBJ3 = fon.o testsocket.o 
OPTIONS	=
# Adaptation a Darwin / MacOS X avec fink
# Du fait de l'absence de libtermcap on se fait pas mal
# cracher dessus mais ca marche...
ifeq ($(shell uname), Darwin)
#LFLAGS	+= -L/opt/local/lib
CFLAGS	+= -I /opt/local/include
endif
#Changer si necessaire le chemin d'acces aux librairies

# Adaptation a Linux
ifeq ($(shell uname), Linux)
OPTIONS	+= -ltermcap
endif

# Adaptation a FreeBSD
# Attention : il faut utiliser gmake...
ifeq ($(shell uname),FreeBSD)
OPTIONS	+= -ltermcap
endif

# Adaptation a Solaris

ifeq ($(shell uname),SunOS)
OPTIONS	+= -ltermcap  -lsocket -lnsl
CFLAGS	+= -I..
endif

INC_DIR=../include/
CFLAGS += -Wall -I$(INC_DIR)

EXEC = ${OBJ1} client ${OBJ2} serveur ${OBJ3} testsocket
all: ${EXEC} 	


fon.o :  $(INC_DIR)fon.h fon.c
	gcc -g -DDEBUG $(CFLAGS) -c fon.c

client.o : $(INC_DIR)fon.h client.c 
	gcc -g  $(CFLAGS) -c  client.c	

serveur.o : $(INC_DIR)fon.h	serveur.c 
	gcc -g $(CFLAGS) -c  serveur.c	

testsocket.o : $(INC_DIR)fon.h	testsocket.c 
	gcc -g $(CFLAGS) -c  testsocket.c	

client : ${OBJ1}	
	gcc $(LFLAGS) ${OBJ1} -o client -lcurses   $(OPTIONS)

serveur : ${OBJ2}	
	gcc $(LFLAGS) ${OBJ2} -o serveur -lcurses   $(OPTIONS)

testsocket : ${OBJ3}	
	gcc $(LFLAGS) ${OBJ3} -o testsocket -lcurses   $(OPTIONS)



clean : 
	rm -f ${EXEC} core

