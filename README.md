# TP N°5 - Programmation d’une application client/serveur

## Elias El Yandouzi & Amad Salmon - Réseaux - INFO3 - Juin 2020

## Application FTP

Une application FTP consiste à transferer des fichiers via le réseau. Usuellement les ports 20 et 21 sont utilisés, respectivement pour la donnée et les commandes.

Les commandes seront ici aux nombres de 3 :
- __ls__ : afficher la liste des fichiers disponibles sur le serveur

- __get__ : permet de récuperer un fichier depuis le serveur vers le client

- __put__ : permet de déposer un fichier sur le serveur depuis le client.

### Cahier des charges

Nous utiliserons le protocole de transport TCP car nous souhaitons un transfert de données fiable, sans perte, quitte à ce qu'il soit un peu plus lent. 

Dans un premier temps, nous chercherons à rendre quelque chose de fonctionnel et donc nous irons au plus simple. C'est pour cette raison que proposerons dans un premier temps un server itératif.

![Schéma du fonctionnement](https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_72/rzab6/rxab6505.gif)

Par la suite (et très rapidement) nous utiliserons un serveur parallèle. (Disponible pour la version finale)
