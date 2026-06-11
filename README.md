# SAE2.3 Réseaux - Simulation Ethernet & STP

## Objectif du projet

Ce programme est une simulation en C d'un réseau local Ethernet. Il permet de charger une topologie depuis un fichier texte, de créer des stations, des switchs et des liens, puis de simuler la circulation de trames Ethernet dans ce réseau.

Le programme montre aussi une version simplifiée du protocole STP : les switchs échangent des BPDU, élisent un root bridge, choisissent des ports root/designated et bloquent certains ports pour éviter les boucles.

## Ce que le programme fait

1. Lit un fichier de configuration.
2. Crée les équipements du réseau : stations et switchs.
3. Connecte les équipements avec des liens pondérés.
4. Affiche le réseau chargé.
5. Lance plusieurs ticks STP pour échanger des BPDU.
6. Affiche le résultat STP : root bridge, root port, designated ports, blocked ports.
7. Injecte une trame Ethernet de test entre deux stations.
8. Simule l'apprentissage MAC, le forwarding et le flooding.
9. Affiche les tables de commutation finales.
10. Affiche une trame en mode utilisateur et en hexadécimal.

## Compilation

```bash
make
```

Ou directement :

```bash
gcc -Wall -Wextra -std=c11 main.c adresse.c station.c switch_reseau.c reseau.c trame.c -o sae23
```

## Exécution

Démo avec le réseau STP cyclique :

```bash
./sae23 configs/config_stp_cycle.txt 6
```

Démo simple avec un seul switch :

```bash
./sae23 configs/config_simple.txt 3
```

Démo pour montrer que la priorité STP est utilisée :

```bash
./sae23 configs/config_priority.txt 6
```

Le deuxième argument est le nombre de ticks STP avant l'envoi de la trame DATA.

## Format du fichier de configuration

Première ligne :

```txt
nombre_equipements nombre_liens
```

Ligne d'une station :

```txt
1;MAC;IP
```

Ligne d'un switch :

```txt
2;MAC;nombre_ports;priorite_STP
```

Ligne d'un lien :

```txt
equipement1;equipement2;cout
```

Exemple :

```txt
3 2
2;01:45:23:a6:f7:01;8;1024
1;00:15:5d:db:40:61;176.173.199.138
1;00:15:5d:da:56:c3;176.173.199.155
0;1;4
0;2;4
```

## Rôle des fichiers

| Fichier | Rôle |
|---|---|
| `adresse.h/.c` | Représentation, lecture et affichage des adresses MAC et IPv4. |
| `station.h/.c` | Structure d'une station avec MAC, IP et port unique. |
| `switch_reseau.h/.c` | Structure d'un switch, priorité STP, ports, table de commutation et affichage STP. |
| `trame.h/.c` | Structure d'une trame Ethernet simulée, BPDU, affichage utilisateur/hexadécimal, files FIFO. |
| `reseau.h/.c` | Chargement du réseau, connexion des équipements, simulation tick par tick. |
| `main.c` | Démonstration : chargement, STP, injection de trame DATA, affichage final. |

## Points importants pour la soutenance

### Adresse MAC / IP

Une adresse MAC est stockée dans 6 octets (`uint8_t octets[6]`) car une MAC fait 6 octets. Une adresse IPv4 est stockée dans 4 octets car IPv4 fait 4 octets.

### Switch et table de commutation

Quand un switch reçoit une trame DATA, il apprend l'adresse MAC source sur le port d'entrée. Ensuite :

- si la destination est connue, il envoie la trame vers le bon port ;
- si la destination est inconnue, il fait du flooding vers tous les autres ports non bloqués.

### STP

Chaque switch possède un Bridge ID composé de :

```txt
priorité + adresse MAC
```

Le root bridge est le switch avec le meilleur Bridge ID : priorité la plus faible, puis MAC la plus faible en cas d'égalité.

Les BPDU transportent :

- le root bridge connu ;
- le coût pour atteindre le root ;
- le bridge ID du switch émetteur ;
- le port émetteur.

### Ports STP

- `ROOT` : port utilisé par un switch pour rejoindre le root bridge ;
- `DESIGNATED` : port ouvert à la transmission ;
- `BLOCKED` : port bloqué pour casser les cycles ;
- `UNKNOWN` : état initial avant les premiers échanges BPDU.

## Limites honnêtes

Cette implémentation reste une simulation pédagogique. Elle ne prétend pas être un vrai STP industriel complet. Les timers STP réels, les états Listening/Learning/Forwarding et le vrai CRC Ethernet ne sont pas implémentés. Le champ FCS affiché est un checksum simplifié pour visualiser la présence du champ dans la trame.

## Phrase courte pour présenter le projet

> Notre programme simule un réseau local Ethernet. Il charge une topologie depuis un fichier, crée les stations et switchs, simule les trames Ethernet, fait apprendre les adresses MAC aux switchs, puis utilise une version simplifiée de STP avec des BPDU pour choisir un root bridge et bloquer certains ports afin d'éviter les boucles.
