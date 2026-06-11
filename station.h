#ifndef STATION_H
#define STATION_H

#include "adresse.h"
#include "trame.h"

typedef struct {
    size_t voisin_idx;      // Index of the equipment connected to this port
    unsigned int cout_lien;
    file_trames buffer_in;
    file_trames buffer_out;
} port;

typedef struct {
    adresse_mac mac;
    adresse_ip ip;
    port port_unique;       // A station has only one port
} station;

station creer_station(adresse_mac mac, adresse_ip ip);
void afficher_station(station st);

#endif