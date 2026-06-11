#include "adresse.h"

#include <stdio.h>

bool lire_mac(const char *texte, adresse_mac *mac)
{
    unsigned int o[6];

    if (sscanf(texte, "%x:%x:%x:%x:%x:%x",
               &o[0], &o[1], &o[2], &o[3], &o[4], &o[5]) != 6) {
        return false;
    }

    for (int i = 0; i < 6; i++) {
        if (o[i] > 255) {
            return false;
        }

        mac->octets[i] = (uint8_t)o[i];
    }

    return true;
}

bool lire_ip(const char *texte, adresse_ip *ip)
{
    unsigned int o[4];

    if (sscanf(texte, "%u.%u.%u.%u",
               &o[0], &o[1], &o[2], &o[3]) != 4) {
        return false;
    }

    for (int i = 0; i < 4; i++) {
        if (o[i] > 255) {
            return false;
        }

        ip->octets[i] = (uint8_t)o[i];
    }

    return true;
}

void afficher_mac(adresse_mac mac)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac.octets[0],
           mac.octets[1],
           mac.octets[2],
           mac.octets[3],
           mac.octets[4],
           mac.octets[5]);
}

void afficher_ip(adresse_ip ip)
{
    printf("%u.%u.%u.%u",
           ip.octets[0],
           ip.octets[1],
           ip.octets[2],
           ip.octets[3]);
}

int comparer_mac(adresse_mac a, adresse_mac b)
{
    for (int i = 0; i < 6; i++) {
        if (a.octets[i] < b.octets[i]) {
            return -1;
        }

        if (a.octets[i] > b.octets[i]) {
            return 1;
        }
    }

    return 0;
}