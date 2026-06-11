#ifndef ADRESSE_H
#define ADRESSE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t octets[6];
} adresse_mac;

typedef struct {
    uint8_t octets[4];
} adresse_ip;

bool lire_mac(const char *texte, adresse_mac *mac);
bool lire_ip(const char *texte, adresse_ip *ip);

void afficher_mac(adresse_mac mac);
void afficher_ip(adresse_ip ip);

int comparer_mac(adresse_mac a, adresse_mac b);

#endif