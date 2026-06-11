#ifndef TRAME_H
#define TRAME_H

#include "adresse.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TAILLE_PAYLOAD_MAX 1500
#define TAILLE_PADDING_MIN 46
#define ETHER_TYPE_DATA_SIMULATION 0x0800
#define ETHER_TYPE_BPDU_SIMULATION 0x4242

typedef enum {
    TYPE_TRAME_DATA,
    TYPE_TRAME_BPDU
} type_trame;

typedef struct {
    unsigned int priorite;
    adresse_mac mac;
} bridge_id;

typedef struct {
    bridge_id root_id;              // meilleur switch root connu
    unsigned int root_path_cost;    // cout pour aller vers le root
    bridge_id bridge_id;            // switch qui envoie la BPDU
    unsigned int port_id;           // port d'envoi
} bpdu;

typedef struct {
    // Champs proches du format Ethernet reel, utiles surtout pour l'affichage.
    uint8_t preambule[7];
    uint8_t sfd;
    adresse_mac destination;
    adresse_mac source;
    uint16_t ether_type;

    // Type interne de simulation.
    type_trame type;

    size_t taille_donnees;
    union {
        char payload[TAILLE_PAYLOAD_MAX + 1];
        bpdu info_stp;
    } donnees;

    uint8_t bourrage[TAILLE_PADDING_MIN];
    size_t taille_bourrage;
    uint32_t fcs; // checksum simplifie pour affichage, pas un vrai CRC Ethernet.
} trame;

// Simple FIFO for simulation buffers
typedef struct noeud_trame {
    trame t;
    struct noeud_trame *suivant;
} noeud_trame;

typedef struct {
    noeud_trame *debut;
    noeud_trame *fin;
} file_trames;

void initialiser_trame_data(trame *t, adresse_mac source, adresse_mac destination, const char *message);
void initialiser_trame_bpdu(trame *t, adresse_mac source, bpdu info);
void afficher_trame(const trame *t);
void afficher_trame_hex(const trame *t);

void enfiler_trame(file_trames *f, trame t);
bool defiler_trame(file_trames *f, trame *t);
void vider_file_trames(file_trames *f);

#endif
