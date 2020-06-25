#include <stdio.h>
#include <curses.h> 		/* Primitives de gestion d'ecran */
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "fon.h"   		/* primitives de la boite a outils */

int main(int argc, char const *argv[])
{
    /*  Définition du serveur  */
    char serveur_IP[] = "127.0.0.1";
    char serveur_port[] = "23670";
    struct sockaddr_in *p_adr_socket_s;
    adr_socket(serveur_port, serveur_IP, SOCK_STREAM, &p_adr_socket_s);

    /*  Définition du client  */
    char client_IP[] = "127.0.0.1";
    char client_port[] = "28000";
    int client_socket = h_socket(AF_INET,SOCK_STREAM);
    struct sockaddr_in *p_adr_socket_c;
    adr_socket(client_port, client_IP, SOCK_STREAM, &p_adr_socket_c);
    h_bind(client_socket, p_adr_socket_c);


    /*  Définition du buffer  */
    int taille_buffer = 8; // taille en octets
    // 1 char ASCII = 1 octet donc "Bonjour\O"=8 octets
    char buffer[] = "Bonjour";

    h_connect(client_socket, p_adr_socket_s);
    
    h_writes(client_socket, buffer, taille_buffer);

    return 0;
}
