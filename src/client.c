/******************************************************************************/
/*			Application: FTP												  */
/******************************************************************************/
/*									        								  */
/*			 programme  CLIENT				               					  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs : ELIAS EL YANDOUZI & AMAD SALMON	| June 2020				  */
/*									      									  */
/******************************************************************************/	

#include <stdio.h>
#include <stdlib.h>
#include <curses.h> 		/* Primitives de gestion d'ecran */
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "fon.h"   		/* primitives de la boite a outils */

#define SERVICE_DEFAUT "1111"
#define SERVEUR_DEFAUT "127.0.0.1"

#define SIZE 1480
#define DELIMITORS "\n\r\t\f\v" /* Les delimiteurs usuelles */

void client_appli (char *serveur, char *service);


/*****************************************************************************/
/*--------------- programme client -----------------------*/

int main(int argc, char *argv[])
{

	char *serveur= SERVEUR_DEFAUT; /* serveur par defaut */
	char *service= SERVICE_DEFAUT; /* numero de service par defaut (no de port) */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch(argc)
	{
 	case 1 :		/* arguments par defaut */
		  printf("serveur par defaut: %s\n",serveur);
		  printf("service par defaut: %s\n",service);
		  break;
  	case 2 :		/* serveur renseigne  */
		  serveur=argv[1];
		  printf("service par defaut: %s\n",service);
		  break;
  	case 3 :		/* serveur, service renseignes */
		  serveur=argv[1];
		  service=argv[2];
		  break;
    default:
		  printf("Usage:client serveur(nom ou @IP)  service (nom ou port) \n");
		  exit(1);
	}

	/* serveur est le nom (ou l'adresse IP) auquel le client va acceder */
	/* service le numero de port sur le serveur correspondant au  */
	/* service desire par le client */
	
	client_appli(serveur, service);
}


void trim(char **s){
	int flag = 0;

	strtok(*s, DELIMITORS);
	for(int i = 0; i < SIZE; i++){	
		if(flag){
			(*s)[i] = 0;
			continue;
		}

		if((*s)[i] == ' '){
			(*s)[i] = '\0';
			flag = 1;
		}
	}
}


int get_command(char* s){
	trim(&s);
	if (strcmp("ls", s) == 0)
		return 1;
	else if (strcmp("put", s) == 0)
		return 2;
	else if (strcmp("get", s) == 0)
		return 3;
	else if (strcmp("close", s) == 0){
		return 4;
	}
	
	return -1;
}

char* get_fname(char* s){
	int i = 0;
	while(s[i] != ' ')
		i++;
	
	/* Nous sommes maintenant devant le supposé nom de fichier */
	int size_res = 10;
	char* res = malloc(size_res*sizeof(char));
	int j = 0;
	while(i < SIZE && s[i] != '\n'){
		if(j == size_res){
			size_res *= 2;
			res = realloc(res, size_res*sizeof(char));
		}

		res[j] = s[i];
		j++, i++;
	}
	res[j] = '\0';

	return res;
}

void build_file(int c_sock){
	char* buffer = malloc(SIZE*sizeof(char));

	h_reads(c_sock, buffer, 1);
	int len_name = buffer[0];

	h_reads(c_sock, buffer, len_name);
	char* fname = malloc(len_name * sizeof(char));
	for(int i = 0; i < len_name; i++)
		fname[i] = buffer[i+1];

	FILE* f = fopen(fname, "w");

	if(f == NULL){
		printf("PANIC! Couldn't create a new file");
		return;
	}

	// A verifier : Conversion de char* vers long
	memset(buffer, 0, SIZE);
	char * sfile = malloc(8*sizeof(char));
	h_reads(c_sock, sfile, 8);
	u_long len_file = atol(sfile);

	printf("LEN_FILE = %lu\n", len_file);
	
	// Nombre de bytes lu en tout
	u_long bytes_read = 0;
	int read;

	while(bytes_read < len_file){
		read = h_reads(c_sock, buffer, SIZE);
		bytes_read += read;
		
		fwrite(buffer, 1, read, f);
		memset(buffer, 0, SIZE);
	}

	fclose(f);
	free(buffer);
	free(fname);

	return;
}

void send_file(int c_sock, char* fname, int len_name){
	char* buffer = malloc(SIZE*sizeof(char));
	FILE* f = fopen(fname, "r");

	if (f == NULL){
		printf("PANIC! File not found or cannot be opened\n");
		return;
	}

	/* Permet de connaître la taille du fichier */
	fseek(f, 0L, SEEK_END);
	long idx_end = ftell(f);
	fseek(f, 0L, SEEK_SET);

	char* fsize = malloc(20*sizeof(char)); // 2^64 en base 10 fait au plus 20 digits de long

	int l_fsize = sprintf(fsize, "%lu", idx_end); //moyen tordu de convertir un long en chaîne de charactères

	h_writes(c_sock, fsize, l_fsize);

	long bytes_sent = 0;
	long sent = 0;

	while(bytes_sent < idx_end){
		fgets(buffer, SIZE, f);

		sent = h_writes(c_sock, buffer, SIZE);

		if (sent != SIZE && bytes_sent + sent != idx_end) {
			for(int i = sent; i < SIZE; i++){
				buffer[i - sent] = buffer[i];
			}
		}

		bytes_sent += sent;

		int left = SIZE - sent;

		while (left){
			sent = h_writes(c_sock, buffer, left);
			bytes_sent += sent;
			for(int i = sent ; i < left; i++)
				buffer[i - sent] = buffer[i];
			left -= sent;
		}

	}

	free(buffer);
	fclose(f);
	return;
}


/*****************************************************************************/
void client_appli (char *serveur,char *service)

/* procedure correspondant au traitement du client de votre application */

{
	struct sockaddr_in *clientAddress, *serverAddress;
	int c_sock = h_socket(AF_INET, SOCK_STREAM);
	adr_socket("2222", "127.0.0.1", SOCK_STREAM, &clientAddress);

	adr_socket(SERVICE_DEFAUT, SERVEUR_DEFAUT, SOCK_STREAM, &serverAddress);

	h_bind(c_sock, clientAddress);

	h_connect(c_sock, serverAddress);


	char* buffer; 
	int command = -1;
	char * fname;
	int len_fname = 0;

	while(1){
		buffer = malloc(SIZE*sizeof(char));
		fgets(buffer, SIZE, stdin);
		command = get_command(buffer);
		switch (command){
			case 1: // ls
				h_writes(c_sock, "1", 1);
				break;
			case 2: // get
				h_writes(c_sock, "2", 1);
				fname = get_fname(buffer);
				len_fname = strlen(fname);
				char sfname[1]; 
				sfname[0] = (char)len_fname; 
				/* Permet d'envoyer un entier precisant la longueur de la chaine, utile pour le décodage */
				h_writes (c_sock, sfname, 1);
				h_writes (c_sock, fname, len_fname);
				build_file(c_sock);
				break;
			case 3: // put 
				/* Même chose que lorsque le serveur recoit un get (ie: découper le fichier et l'envoyer morceau par morceau) */
				h_writes(c_sock, "3", 1);
				fname = get_fname(buffer);
				len_fname = strlen(fname);
				send_file(c_sock, fname, len_fname); 
				break;
			case 4: // close
				h_close(c_sock);
				break;
			default:
				printf("PANIC! - Unkwonw command\n");
		}
		free(buffer);
	}

	printf("Connection closed \n");
	return;
 }

/*****************************************************************************/

