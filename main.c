#include "reseau.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    const char *fichier = "configs/config2.txt";
    int nb_ticks_stp = 6;
    int nb_ticks_data = 8;

    if (argc >= 2) {
        fichier = argv[1];
    }
    if (argc >= 3) {
        nb_ticks_stp = atoi(argv[2]);
        if (nb_ticks_stp < 0) {
            nb_ticks_stp = 0;
        }
    }

    reseau r = {0};

    printf("SAE2.3 Reseaux - Simulation Ethernet + STP\n");
    printf("Fichier charge : %s\n", fichier);

    if (!charger_reseau(fichier, &r)) {
        printf("Erreur chargement reseau.\n");
        return 1;
    }

    afficher_reseau(&r);

    printf("\n[DEMO STP] Lancement de %d tick(s) pour echanger des BPDU.\n", nb_ticks_stp);
    for (int tick = 0; tick < nb_ticks_stp; tick++) {
        printf("\n--------------- TICK STP %d ---------------\n", tick + 1);
        reseau_simuler_tick(&r);
    }

    afficher_etat_stp(&r);

    size_t station_source;
    size_t station_destination;
    if (reseau_trouver_deux_stations(&r, &station_source, &station_destination)) {
        if (reseau_injecter_trame_station_vers_station(&r,
                                                       station_source,
                                                       station_destination,
                                                       "Hello Network!")) {
            printf("\n[DEMO ETHERNET] Circulation de la trame DATA pendant %d tick(s).\n", nb_ticks_data);
            for (int tick = 0; tick < nb_ticks_data; tick++) {
                printf("\n-------------- TICK DATA %d ---------------\n", tick + 1);
                reseau_simuler_tick(&r);
            }
        }
    } else {
        printf("\nPas assez de stations pour injecter une trame DATA de demo.\n");
    }

    afficher_tables_commutation(&r);
    afficher_etat_stp(&r);

    printf("\nFin de la demonstration.\n");
    liberer_reseau(&r);
    return 0;
}
