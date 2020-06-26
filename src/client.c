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

#define PUBLIC_FOLDER_PATH "../public/"

#define SIZE 1480
#define DELIMITORS "\n\r\t\f\v" /* Les delimiteurs usuelles */

#define ls "1"
#define get "2"
#define put "3"
#define close "4"

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


void trim(char *s){
	int flag = 0;

	strtok(s, DELIMITORS);
	for(int i = 0; i < SIZE; i++){	
		if(flag){
			s[i] = 0;
			continue;
		}

		if(s[i] == ' '){
			s[i] = '\0';
			flag = 1;
		}
	}
}


int get_command(char* s){

	if (strncmp("ls", s, 2) == 0)
		return 1;
	else if (strncmp("get", s, 3) == 0)
		return 2;
	else if (strncmp("put", s, 3) == 0)
		return 3;
	else if (strncmp("close", s, 4) == 0)
		return 4;
	

	return -1;
}

char* get_fname(char* s){
	int i = 1;
	while(s[i - 1] != ' ')
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

/**
 * Fonction appelée à la réponse du serveur après que le client ait émis une commande "get". 
 * Permet de récupérer le fichier demandé que le serveur envoie octet par octet sur la socket.
 * */
void build_file(int c_sock){
	char* buffer = malloc(SIZE*sizeof(char));

	h_reads(c_sock, buffer, 1);
	int len_name = buffer[0];

	h_reads(c_sock, buffer, len_name);
	
	int i = len_name;
	for(; i > 0; i--){
		if(buffer[i-1] == '/')
			break;
	}

	int trimmed_len = len_name - i;
	int j = i;
	char* trimmed_name = malloc(trimmed_len+1*sizeof(char));
	for(; i <= len_name; i++){
		trimmed_name[i - j] = buffer[i];
	}


	char* name = malloc((trimmed_len + 3 + 10)* sizeof(char));
	strcat(name, PUBLIC_FOLDER_PATH);
	char* fname = malloc(trimmed_len + 3 * sizeof(char));
	for(int i = 0; i < trimmed_len; i++)
		fname[i] = trimmed_name[i];
	
	fname[trimmed_len] = '_';
	fname[trimmed_len+1] = '1';
	fname[trimmed_len+2] = '\0';
	strcat(name, fname);

	FILE* f = fopen(name, "w");

	if(f == NULL){
		printf("PANIC! Couldn't create a new file");
		return;
	}

	// A verifier : Conversion de char* vers long
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

/**
 * Fonction appelée lorsque le client émettra une commande "put". 
 * Permet d'envoyer le fichier souhaité octet par octet sur la socket.
 * */
void send_file(int c_sock, char* fname, int len_name){
	char* buffer = malloc(SIZE*sizeof(char));
	char* name = malloc((len_name + 10)* sizeof(char));
	strcat(name, PUBLIC_FOLDER_PATH);
	strcat(name, fname);
	FILE* f = fopen(name, "r");

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

	while(!feof(f) && bytes_sent < idx_end){
		if (idx_end - bytes_sent < SIZE)
			to_send = idx_end - bytes_sent + 1;

		fgets(buffer, to_send, f);
		sent = h_writes(c_sock, buffer, strlen(buffer));
		bytes_sent += sent;
	}

	free(buffer);
	fclose(f);
	return;
}


/*****************************************************************************/
void client_appli (char *serveur,char *service)

/* procedure correspondant au traitement du client de votre application */

{
	struct sockaddr_in *serverAddress;
	int c_sock = h_socket(AF_INET, SOCK_STREAM);
	adr_socket(SERVICE_DEFAUT, SERVEUR_DEFAUT, SOCK_STREAM, &serverAddress);

	h_connect(c_sock, serverAddress);


	char* buffer; 
	int command = -1, flag = 1;
	char * fname;
	int len_fname = 0;


	while(flag){
		printf("$> ");
		buffer = malloc(SIZE*sizeof(char));
		fgets(buffer, SIZE, stdin);
		command = get_command(buffer);
		switch (command){
			case 1: // ls
				h_writes(c_sock, ls, 1);
				h_reads(c_sock, buffer, 1);
				h_reads(c_sock, buffer, buffer[0]);
				printf("%s\n", buffer);
				break;
			case 2: // get
				h_writes(c_sock, get, 1);
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
				h_writes(c_sock, put, 1);
				fname = get_fname(buffer);
				len_fname = strlen(fname);
				send_file(c_sock, fname, len_fname); 
				break;
			case 4: // close
				h_writes(c_sock, close, 1);
				h_close(c_sock);
				flag = 0;
				break;
			default:
				printf("PANIC! - Unknown command.\n");
		}
		free(buffer);
	}

	printf("Connection closed... Bye !\n");
	return;
 }

/*****************************************************************************/

