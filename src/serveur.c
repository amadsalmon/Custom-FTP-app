/******************************************************************************/
/*			Application: FTP												  */
/******************************************************************************/
/*									        								  */
/*			 programme  SERVEUR				               					  */
/*									      									  */
/******************************************************************************/
/*									      									  */
/*		Auteurs : ELIAS EL YANDOUZI & AMAD SALMON	| June 2020				  */
/*									      									  */
/******************************************************************************/	

#include<stdio.h>
#include<stdlib.h>
#include <curses.h>

#include<sys/signal.h>
#include<sys/wait.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <string.h>

#include "fon.h"     		/* Primitives de la boite a outils */

#include <dirent.h>

#define SERVICE_DEFAUT "1111"
#define SERVEUR_DEFAUT "127.0.0.1"

#define PUBLIC_FOLDER_PATH "../public"


#define SIZE 1480
#define DELIMITORS "\n\r\t\f\v" /* Les delimiteurs usuelles */



void serveur_appli (char *service);   /* programme serveur */
void handle_request(int c_sock);
void build_file(int c_sock);
void send_file(int c_sock, char* fname, int len_name);
void ls(int c_sock);

/******************************************************************************/	
/*---------------- programme serveur ------------------------------*/

int main(int argc,char *argv[])
{

	char *service= SERVICE_DEFAUT; /* numero de service par defaut */


	/* Permet de passer un nombre de parametre variable a l'executable */
	switch (argc)
 	{
   	case 1:
		  printf("defaut service = %s\n", service);
		  		  break;
 	case 2:
		  service=argv[1];
            break;

   	default :
		  printf("Usage:serveur service (nom ou port) \n");
		  exit(1);
 	}

	/* service est le service (ou numero de port) auquel sera affecte
	ce serveur*/
	
	serveur_appli(service);
}


/******************************************************************************/	
void serveur_appli(char *service)

/* Procedure correspondant au traitemnt du serveur de votre application */

{
	struct sockaddr_in *serverAddress, *clientAddress = malloc(sizeof(struct sockaddr_in));
	int s_sock = h_socket(AF_INET, SOCK_STREAM);
	adr_socket(SERVICE_DEFAUT, SERVEUR_DEFAUT, SOCK_STREAM, &serverAddress);
	h_bind(s_sock, serverAddress);
	h_listen(s_sock, 5);

	int c_sock;


	
	while(true){
		c_sock = h_accept(s_sock, clientAddress);

		if(c_sock != 0)
			handle_request(c_sock);
		else
			continue;
		c_sock = 0;
	}

}

void handle_request(int c_sock){
	char *buffer = malloc(SIZE*sizeof(char));
	h_reads(c_sock, buffer, 1);
	int command = buffer[0];
	switch(command){
		case 1: //ls 
			ls(c_sock);
			break;
		case 2: //get - here means send the file to the client
			h_reads(c_sock, buffer, 1);
			int len_name = buffer[0];
			h_reads(c_sock, buffer, len_name);
			char* fname = malloc(len_name*sizeof(char));
			for(int i = 0; i < len_name; i++){
				fname[i] = buffer[i];
			}
			send_file(c_sock, fname, len_name);
			free(fname);
			break; 
		case 3://put - here means recieve a file
			build_file(c_sock);
			break;
		default:
			printf("PANIC! - What do you want ?\n");
	}
	free(buffer);
}
	
void build_file(int c_sock){
	char* buffer = malloc(SIZE*sizeof(char));

	h_reads(c_sock, buffer, 1);
	int len_name = buffer[0];

	h_reads(c_sock, buffer, len_name);
	char* name_file = malloc(len_name * sizeof(char));
	for(int i = 0; i < len_name; i++)
		name_file[i] = buffer[i+1];

	FILE* f = fopen(name_file, "w");

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

/**
 * Fonction appelée par le serveur lorsque celui-ci recevra de la part du client une commande "ls". Liste alors tous les fichiers contenus dans le répertoire courant.
 * */
void ls(int c_sock)
{
	struct dirent *dir;  // Pointer for directory entry 
	DIR *d = opendir(PUBLIC_FOLDER_PATH);

	if (d == NULL)
	{
		printf("Could not open current directory." ); 
		return;
	}

	while ((dir = readdir(d)) != NULL) 
		h_writes(c_sock, dir->d_name, dir->d_namlen);
	
	closedir(d);
	return;
}

