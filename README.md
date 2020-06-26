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

__La règle d'or :__
Peu importe ce que va faire le client avec notre application, s'il communique avec le serveur des données vont être envoyées et vont devoir être décodée. Il faut donc savoir comment s'y prendre pour comprendre ce que demande le client.
Le client peut formuler ces quatre demandes : __ls, get, put et close__. Seul __get__ et __put__ néccessite un nom de fichier en plus.
L'utilisateur côté client va donc entrer dans le terminal les commandes suivantes : 
	-  $ > <commandes> <nom_de_fichier>

Nous avons donc attribué pour les commandes un entier, ainsi lorsque client envoie une commande, le serveur recevra un entier entre 1 et 4.

La règle d'or ensuite est de préciser à chaque fois la longueur de la string que nous allons envoyer. De ce fait le serveur pourra lire que ce qui est nécessaire sans soucis.

Prenons un exemple : $ > get toto
Les différents segmetns contiendront donc :
- 2 pour get
- 4 pour la longueur de la chaine du nom du fichier à venir
- toto pour le nom du fichier

On a donc un modèle de référence et on sait que chaque requête suivra cela.

En somme, la solution proposé est semblable à ce que nous avons pu faire durant POO : nous indiquons la taille chacun de nos éléments fournis ainsi que de leur nom puis on parse les n chars indiqués.

__La transimission des fichiers :__

Plusieurs points sont à traités ici, quel est le nom du fichier ? quelle est sa taille ?

Voici une solution possible (probablement pas optimale mais qui a le mérite de fonctionner) :
1. À ce stade là, on a déjà le nom du fichier ainsi que la taille de cette fameuse string.
2. On ouvre le fichier, on se place à la fin (grâce à fseek) pour connaître l'indice du dernier élément et donc la taille du fichier.
3. On envoie donc la taille du nom du fichier n sur 1 octet, la string sur n octets.
4. On transmet la taille du fichier au client pour qu'il puisse savoir combien d'octets il doit attendre. Pour cela on donne sur un octet le nombre de digit que contient la taille du fichier (20 digits maximum pour 2^64 octets). Puis on converti la taille, long de base, en char* qu'on pourra envoyer grâce à h_writes.
5. On compte les octets envoyés et on boucle tant que l'on a pas atteint la taille du fichier

Du côté du client pour un get ou du serveur pour un put :
1. On a tous les ingrédients pour reconstruire le fichier (nom, longeur de la string nom, taille du fichier et des données)
2. On lit jusqu'à arriver à la bonne taille du fichier

__Version concurrente :__

La version itératif sert en premier lieu à debug, les threads rendent la chose impossible à faire. 

On pouvait gérer un client à la fois, ce n'était pas tip-top.

Pour passer à la version concurrente nous avons du utiliser la fonction fork qui recopie un processus à l'identique. Si nous sommes dans un processus fils, on ferme la socket passive, on traite la/les requêtes et on coupe le thread avec un exit lorsque la socket est fermée. Si on est dans le processus père on ferme la socket active puisque on s'occupe déjà d'elle dans le processus fils.

__Démonstration :__

L'objectif était de pouvoir notre application réellement et ne pas se bloquer à du localhost. Nous avons donc configuré notre box afin de pouvoir faire fonctionner le tout et s'échanger des fichiers de Nice à Grenoble.

Dans l'odre voici les manipulations effectués : 
1. Attribution d'un adresse IP statique.
2. Redirection de port vers la machine (si une connection se fait sur le routeur, sur le port 5555 dans notre cas, il la redirige sur la machine sur le port 1111).
3. Nous avons desactivé le pare-feu temporairement au lieu de d'ouvrir les ports
4. Et c'est tout !



