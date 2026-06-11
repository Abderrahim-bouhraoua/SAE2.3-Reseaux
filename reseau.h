#ifndef RESEAU_H
#define RESEAU_H

#include "station.h"
#include "switch_reseau.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TYPE_STATION = 1,
    TYPE_SWITCH = 2
} type_equipement;

typedef struct {
    type_equipement type;

    union {
        station st;
        switch_reseau sw;
    } donnees;
} equipement;

typedef struct {
    size_t eq1;
    size_t eq2;
    unsigned int cout;
} lien;

typedef struct {
    size_t nb_equipements;
    size_t nb_liens;

    equipement *equipements;
    lien *liens;
} reseau;

bool charger_reseau(const char *nom_fichier, reseau *r);
void afficher_reseau(const reseau *r);
void afficher_etat_stp(const reseau *r);
void afficher_tables_commutation(const reseau *r);
void liberer_reseau(reseau *r);

// Simulation
void reseau_simuler_tick(reseau *r);
bool reseau_injecter_trame_station_vers_station(reseau *r, size_t station_source_idx, size_t station_destination_idx, const char *message);
bool reseau_trouver_deux_stations(const reseau *r, size_t *source_idx, size_t *destination_idx);

#endif
