#include "station.h"

#include <stdio.h>

station creer_station(adresse_mac mac, adresse_ip ip)
{
    station st;

    st.mac = mac;
    st.ip = ip;
    
    st.port_unique.voisin_idx = (size_t)-1;
    st.port_unique.cout_lien = 0;
    st.port_unique.buffer_in.debut = NULL;
    st.port_unique.buffer_in.fin = NULL;
    st.port_unique.buffer_out.debut = NULL;
    st.port_unique.buffer_out.fin = NULL;

    return st;
}

void afficher_station(station st)
{
    printf("Station | MAC: ");
    afficher_mac(st.mac);

    printf(" | IP: ");
    afficher_ip(st.ip);

    printf("\n");
}