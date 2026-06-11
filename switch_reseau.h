#ifndef SWITCH_RESEAU_H
#define SWITCH_RESEAU_H

#include "adresse.h"
#include "trame.h"

#include <stddef.h>

#define TAILLE_TABLE_COMMUTATION 128
#define NB_PORTS_SWITCH_MAX 32

typedef enum {
    PORT_UNKNOWN,
    PORT_ROOT,
    PORT_DESIGNATED,
    PORT_BLOCKED
} etat_port;

typedef struct {
    size_t voisin_idx;
    unsigned int cout_lien;
    etat_port etat;
    file_trames buffer_in;
    file_trames buffer_out;
} port_switch;

typedef struct {
    adresse_mac mac;
    size_t port;
} entree_commutation;

typedef struct {
    adresse_mac mac;
    size_t nb_ports_max;
    size_t nb_ports_actifs;
    unsigned int priorite;

    // STP state: Bridge ID = priorite + MAC.
    bridge_id bridge;
    bridge_id root_id;
    unsigned int cout_vers_root;
    size_t port_racine;

    port_switch ports[NB_PORTS_SWITCH_MAX];
    entree_commutation table[TAILLE_TABLE_COMMUTATION];
    size_t nb_entrees;
} switch_reseau;

bridge_id creer_bridge_id(unsigned int priorite, adresse_mac mac);
int comparer_bridge_id(bridge_id a, bridge_id b);
void afficher_bridge_id(bridge_id id);
const char *nom_etat_port(etat_port etat);

switch_reseau creer_switch(adresse_mac mac, size_t nb_ports, unsigned int priorite);
void afficher_switch(switch_reseau sw);
void afficher_table_commutation(switch_reseau sw);
void afficher_ports_stp(switch_reseau sw);

#endif
