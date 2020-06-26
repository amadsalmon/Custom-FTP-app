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

#define PUBLIC_FOLDER_PATH "./public"


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
	
	c_sock = h_accept(s_sock, clientAddress);
	
	while(true){
		handle_request(c_sock);
	}
}

void handle_request(int c_sock){
	char *buffer = malloc(SIZE*sizeof(char));
	h_reads(c_sock, buffer, 1);
	int command = buffer[0] - '0';
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
	char* fname = malloc(len_name + 3 * sizeof(char));
	for(int i = 0; i < len_name; i++)
		fname[i] = buffer[i];

	fname[len_name] = '_';
	fname[len_name+1] = '1';
	fname[len_name+2] = '\0';

	FILE* f = fopen(fname, "w");

	if(f == NULL){
		printf("PANIC! Couldn't create a new file");
		return;
	}


	memset(buffer, 0, SIZE);
	h_reads(c_sock, buffer, 1);
	char * sfile = malloc(buffer[0]*sizeof(char));
	h_reads(c_sock, sfile, buffer[0] - '0');
	u_long len_file = atol(sfile);
	
	// Nombre de bytes lu en tout
	u_long bytes_read = 0;
	int read;
	int to_read = SIZE;


	while(bytes_read < len_file){
		if(len_file - bytes_read < SIZE)
			to_read = len_file - bytes_read;
	
		read = h_reads(c_sock, buffer, to_read);
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
	char l_name[1];
	l_name[0] = len_name;

	h_writes(c_sock, l_name, 1);
	h_writes(c_sock, fname, len_name);
	/* Permet de connaître la taille du fichier */
	fseek(f, 0L, SEEK_END);
	long idx_end = ftell(f);

	fseek(f, 0L, SEEK_SET);

	char* fsize = malloc(20*sizeof(char)); // 2^64 en base 10 fait au plus 20 digits de long
	char* n_digits = malloc(sizeof(char));
	int l_fsize = sprintf(fsize, "%lu", idx_end); //moyen tordu de convertir un long en chaîne de charactères
	
	sprintf(n_digits, "%d", l_fsize);
	h_writes(c_sock, n_digits ,1);
	h_writes(c_sock, fsize, l_fsize);

	long bytes_sent = 0;
	long sent = 0;
	long to_send = SIZE;

	while(bytes_sent < idx_end){
		if (idx_end - bytes_sent < SIZE)
			to_send = idx_end - bytes_sent + 1;

		fgets(buffer, to_send, f);
		sent = h_writes(c_sock, buffer, to_send);
		bytes_sent += sent;
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
	char space[1];
	space[0] = ' ';

	char* ls_string = malloc(255 * sizeof(char)); // limite de la taille 255 car 1 octet
	int ls_len = 0;
	char len[1];

	DIR *d = opendir(PUBLIC_FOLDER_PATH);

	if (d == NULL)
	{
		printf("Could not open current directory." ); 
		return;
	}

	while ((dir = readdir(d)) != NULL){
		if(dir->d_name[0] != '.'){
			strcat(ls_string, dir->d_name);
			strcat(ls_string, " ");
			ls_len += dir->d_namlen + 1;
		}
	} 
	len[0] = ls_len;
	h_writes(c_sock, len, 1);
	h_writes(c_sock, ls_string, ls_len);	
	closedir(d);
	return;
}

/*
Pour le serveur :

Besoin d'utiliser les threads dans le main


*/