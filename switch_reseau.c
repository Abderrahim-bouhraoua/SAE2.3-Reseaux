#include "switch_reseau.h"

#include <stdio.h>

bridge_id creer_bridge_id(unsigned int priorite, adresse_mac mac)
{
    bridge_id id;
    id.priorite = priorite;
    id.mac = mac;
    return id;
}

int comparer_bridge_id(bridge_id a, bridge_id b)
{
    if (a.priorite < b.priorite) {
        return -1;
    }
    if (a.priorite > b.priorite) {
        return 1;
    }

    return comparer_mac(a.mac, b.mac);
}

void afficher_bridge_id(bridge_id id)
{
    printf("%u/", id.priorite);
    afficher_mac(id.mac);
}

const char *nom_etat_port(etat_port etat)
{
    switch (etat) {
        case PORT_UNKNOWN:
            return "UNKNOWN";
        case PORT_ROOT:
            return "ROOT";
        case PORT_DESIGNATED:
            return "DESIGNATED";
        case PORT_BLOCKED:
            return "BLOCKED";
        default:
            return "INVALID";
    }
}

switch_reseau creer_switch(adresse_mac mac, size_t nb_ports, unsigned int priorite)
{
    switch_reseau sw;

    sw.mac = mac;
    sw.nb_ports_max = nb_ports;
    sw.nb_ports_actifs = 0;
    sw.priorite = priorite;
    sw.nb_entrees = 0;

    sw.bridge = creer_bridge_id(priorite, mac);
    sw.root_id = sw.bridge;      // Au depart, chaque switch pense etre le root.
    sw.cout_vers_root = 0;
    sw.port_racine = (size_t)-1;

    for (size_t i = 0; i < NB_PORTS_SWITCH_MAX; i++) {
        sw.ports[i].voisin_idx = (size_t)-1;
        sw.ports[i].cout_lien = 0;
        sw.ports[i].etat = PORT_UNKNOWN;
        sw.ports[i].buffer_in.debut = NULL;
        sw.ports[i].buffer_in.fin = NULL;
        sw.ports[i].buffer_out.debut = NULL;
        sw.ports[i].buffer_out.fin = NULL;
    }

    return sw;
}

void afficher_table_commutation(switch_reseau sw)
{
    printf("  Table de commutation (%zu entree(s)):\n", sw.nb_entrees);

    if (sw.nb_entrees == 0) {
        printf("    (vide)\n");
        return;
    }

    printf("    %-20s | %s\n", "MAC", "Port");
    printf("    ---------------------+------\n");

    for (size_t i = 0; i < sw.nb_entrees; i++) {
        printf("    ");
        afficher_mac(sw.table[i].mac);
        printf(" | %zu\n", sw.table[i].port);
    }
}

void afficher_ports_stp(switch_reseau sw)
{
    printf("  Etat STP:\n");
    printf("    Bridge ID : ");
    afficher_bridge_id(sw.bridge);
    printf("\n    Root ID   : ");
    afficher_bridge_id(sw.root_id);
    printf("\n    Cout root : %u", sw.cout_vers_root);

    if (sw.port_racine != (size_t)-1) {
        printf(" via port %zu", sw.port_racine);
    }
    printf("\n");

    if (sw.nb_ports_actifs == 0) {
        printf("    Aucun port actif\n");
        return;
    }

    printf("    %-5s | %-11s | %-7s | %s\n", "Port", "Etat", "Cout", "Voisin");
    printf("    ------+-------------+---------+--------\n");

    for (size_t p = 0; p < sw.nb_ports_actifs; p++) {
        printf("    %-5zu | %-11s | %-7u | equipement %zu\n",
               p,
               nom_etat_port(sw.ports[p].etat),
               sw.ports[p].cout_lien,
               sw.ports[p].voisin_idx);
    }
}

void afficher_switch(switch_reseau sw)
{
    printf("Switch | MAC: ");
    afficher_mac(sw.mac);

    printf(" | ports (actifs/max): %zu/%zu | priorite STP: %u\n",
           sw.nb_ports_actifs,
           sw.nb_ports_max,
           sw.priorite);

    afficher_ports_stp(sw);
    afficher_table_commutation(sw);
}
