#include "reseau.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void enlever_retour_ligne(char *texte)
{
    texte[strcspn(texte, "\r\n")] = '\0';
}

static bool lire_equipement(char *ligne, equipement *e)
{
    enlever_retour_ligne(ligne);

    char *type_txt = strtok(ligne, ";");
    if (type_txt == NULL) {
        return false;
    }

    int type = atoi(type_txt);

    if (type == TYPE_SWITCH) {
        char *mac_txt = strtok(NULL, ";");
        char *ports_txt = strtok(NULL, ";");
        char *priorite_txt = strtok(NULL, ";");

        if (mac_txt == NULL || ports_txt == NULL || priorite_txt == NULL) {
            return false;
        }

        adresse_mac mac;
        if (!lire_mac(mac_txt, &mac)) {
            return false;
        }

        e->type = TYPE_SWITCH;
        e->donnees.sw = creer_switch(
            mac,
            (size_t)atoi(ports_txt),
            (unsigned int)atoi(priorite_txt)
        );

        return true;
    }

    if (type == TYPE_STATION) {
        char *mac_txt = strtok(NULL, ";");
        char *ip_txt = strtok(NULL, ";");

        if (mac_txt == NULL || ip_txt == NULL) {
            return false;
        }

        adresse_mac mac;
        adresse_ip ip;

        if (!lire_mac(mac_txt, &mac)) {
            return false;
        }
        if (!lire_ip(ip_txt, &ip)) {
            return false;
        }

        e->type = TYPE_STATION;
        e->donnees.st = creer_station(mac, ip);
        return true;
    }

    return false;
}

static bool ajouter_port_switch(switch_reseau *sw, size_t voisin_idx, unsigned int cout)
{
    if (sw->nb_ports_actifs >= sw->nb_ports_max || sw->nb_ports_actifs >= NB_PORTS_SWITCH_MAX) {
        return false;
    }

    sw->ports[sw->nb_ports_actifs].voisin_idx = voisin_idx;
    sw->ports[sw->nb_ports_actifs].cout_lien = cout;
    sw->ports[sw->nb_ports_actifs].etat = PORT_UNKNOWN;
    sw->nb_ports_actifs++;
    return true;
}

static bool connecter_equipements(reseau *r, size_t id1, size_t id2, unsigned int cout)
{
    if (id1 >= r->nb_equipements || id2 >= r->nb_equipements) {
        return false;
    }

    equipement *eq1 = &r->equipements[id1];
    equipement *eq2 = &r->equipements[id2];

    if (eq1->type == TYPE_STATION) {
        eq1->donnees.st.port_unique.voisin_idx = id2;
        eq1->donnees.st.port_unique.cout_lien = cout;
    } else {
        if (!ajouter_port_switch(&eq1->donnees.sw, id2, cout)) {
            printf("Attention: impossible d'ajouter un port au switch %zu\n", id1);
            return false;
        }
    }

    if (eq2->type == TYPE_STATION) {
        eq2->donnees.st.port_unique.voisin_idx = id1;
        eq2->donnees.st.port_unique.cout_lien = cout;
    } else {
        if (!ajouter_port_switch(&eq2->donnees.sw, id1, cout)) {
            printf("Attention: impossible d'ajouter un port au switch %zu\n", id2);
            return false;
        }
    }

    return true;
}

bool charger_reseau(const char *nom_fichier, reseau *r)
{
    FILE *f = fopen(nom_fichier, "r");
    if (f == NULL) {
        printf("Erreur: impossible d'ouvrir le fichier %s\n", nom_fichier);
        return false;
    }

    char ligne[256];

    if (fgets(ligne, sizeof(ligne), f) == NULL) {
        fclose(f);
        return false;
    }

    if (sscanf(ligne, "%zu %zu", &r->nb_equipements, &r->nb_liens) != 2) {
        fclose(f);
        return false;
    }

    r->equipements = calloc(r->nb_equipements, sizeof(equipement));
    r->liens = calloc(r->nb_liens, sizeof(lien));

    if (r->equipements == NULL || r->liens == NULL) {
        fclose(f);
        liberer_reseau(r);
        return false;
    }

    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (fgets(ligne, sizeof(ligne), f) == NULL) {
            fclose(f);
            liberer_reseau(r);
            return false;
        }

        if (!lire_equipement(ligne, &r->equipements[i])) {
            printf("Erreur lecture equipement ligne %zu\n", i + 2);
            fclose(f);
            liberer_reseau(r);
            return false;
        }
    }

    for (size_t i = 0; i < r->nb_liens; i++) {
        if (fgets(ligne, sizeof(ligne), f) == NULL) {
            fclose(f);
            liberer_reseau(r);
            return false;
        }

        if (sscanf(ligne, "%zu;%zu;%u",
                   &r->liens[i].eq1,
                   &r->liens[i].eq2,
                   &r->liens[i].cout) != 3) {
            printf("Erreur lecture lien ligne %zu\n", i + 2 + r->nb_equipements);
            fclose(f);
            liberer_reseau(r);
            return false;
        }

        if (!connecter_equipements(r, r->liens[i].eq1, r->liens[i].eq2, r->liens[i].cout)) {
            printf("Attention: lien %zu non connecte correctement\n", i);
        }
    }

    fclose(f);
    return true;
}

void afficher_reseau(const reseau *r)
{
    printf("\n================ RESEAU LOCAL ================\n");
    printf("Nombre equipements : %zu\n", r->nb_equipements);
    printf("Nombre liens       : %zu\n", r->nb_liens);

    printf("\n---------------- EQUIPEMENTS -----------------\n");
    for (size_t i = 0; i < r->nb_equipements; i++) {
        printf("\n[%zu] ", i);
        if (r->equipements[i].type == TYPE_SWITCH) {
            afficher_switch(r->equipements[i].donnees.sw);
        } else {
            afficher_station(r->equipements[i].donnees.st);
        }
    }

    printf("\n------------------- LIENS --------------------\n");
    for (size_t i = 0; i < r->nb_liens; i++) {
        printf("%zu <-> %zu | cout: %u\n",
               r->liens[i].eq1,
               r->liens[i].eq2,
               r->liens[i].cout);
    }
    printf("==============================================\n");
}

void afficher_etat_stp(const reseau *r)
{
    printf("\n================ ETAT STP ====================\n");
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->equipements[i].type == TYPE_SWITCH) {
            printf("\nSwitch [%zu] ", i);
            afficher_mac(r->equipements[i].donnees.sw.mac);
            printf("\n");
            afficher_ports_stp(r->equipements[i].donnees.sw);
        }
    }
    printf("==============================================\n");
}

void afficher_tables_commutation(const reseau *r)
{
    printf("\n=========== TABLES DE COMMUTATION ============\n");
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->equipements[i].type == TYPE_SWITCH) {
            printf("\nSwitch [%zu] ", i);
            afficher_mac(r->equipements[i].donnees.sw.mac);
            printf("\n");
            afficher_table_commutation(r->equipements[i].donnees.sw);
        }
    }
    printf("==============================================\n");
}

void liberer_reseau(reseau *r)
{
    if (r == NULL) {
        return;
    }

    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->equipements[i].type == TYPE_STATION) {
            vider_file_trames(&r->equipements[i].donnees.st.port_unique.buffer_in);
            vider_file_trames(&r->equipements[i].donnees.st.port_unique.buffer_out);
        } else if (r->equipements[i].type == TYPE_SWITCH) {
            switch_reseau *sw = &r->equipements[i].donnees.sw;
            for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
                vider_file_trames(&sw->ports[p].buffer_in);
                vider_file_trames(&sw->ports[p].buffer_out);
            }
        }
    }

    free(r->equipements);
    free(r->liens);

    r->equipements = NULL;
    r->liens = NULL;
    r->nb_equipements = 0;
    r->nb_liens = 0;
}

static void deplace_trames_vers_voisin(reseau *r, size_t eq_source_idx, file_trames *buffer_out, size_t voisin_idx)
{
    trame t;
    while (defiler_trame(buffer_out, &t)) {
        if (voisin_idx == (size_t)-1 || voisin_idx >= r->nb_equipements) {
            continue;
        }

        equipement *voisin = &r->equipements[voisin_idx];

        if (voisin->type == TYPE_STATION) {
            enfiler_trame(&voisin->donnees.st.port_unique.buffer_in, t);
        } else if (voisin->type == TYPE_SWITCH) {
            switch_reseau *sw = &voisin->donnees.sw;
            for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
                if (sw->ports[p].voisin_idx == eq_source_idx) {
                    enfiler_trame(&sw->ports[p].buffer_in, t);
                    break;
                }
            }
        }
    }
}

static void apprendre_mac(switch_reseau *sw, adresse_mac mac, size_t port)
{
    for (size_t entry = 0; entry < sw->nb_entrees; entry++) {
        if (comparer_mac(sw->table[entry].mac, mac) == 0) {
            sw->table[entry].port = port;
            return;
        }
    }

    if (sw->nb_entrees < TAILLE_TABLE_COMMUTATION) {
        sw->table[sw->nb_entrees].mac = mac;
        sw->table[sw->nb_entrees].port = port;
        sw->nb_entrees++;

        printf("[LEARNING] Switch ");
        afficher_mac(sw->mac);
        printf(" apprend ");
        afficher_mac(mac);
        printf(" sur le port %zu\n", port);
    }
}

static bool trouver_port_destination(const switch_reseau *sw, adresse_mac destination, size_t *port)
{
    for (size_t entry = 0; entry < sw->nb_entrees; entry++) {
        if (comparer_mac(sw->table[entry].mac, destination) == 0) {
            *port = sw->table[entry].port;
            return true;
        }
    }

    return false;
}

static void traiter_trame_data(switch_reseau *sw, size_t port_entree, trame t)
{
    if (sw->ports[port_entree].etat == PORT_BLOCKED) {
        printf("[STP] Switch ");
        afficher_mac(sw->mac);
        printf(" ignore une trame DATA sur le port bloque %zu\n", port_entree);
        return;
    }

    apprendre_mac(sw, t.source, port_entree);

    size_t port_sortie;
    if (trouver_port_destination(sw, t.destination, &port_sortie)) {
        if (port_sortie != port_entree && port_sortie < sw->nb_ports_actifs && sw->ports[port_sortie].etat != PORT_BLOCKED) {
            printf("[FORWARD] Switch ");
            afficher_mac(sw->mac);
            printf(" envoie la trame vers le port %zu\n", port_sortie);
            enfiler_trame(&sw->ports[port_sortie].buffer_out, t);
        }
        return;
    }

    printf("[FLOOD] Switch ");
    afficher_mac(sw->mac);
    printf(" ne connait pas la destination, diffusion sauf port %zu\n", port_entree);

    for (size_t port_sortie = 0; port_sortie < sw->nb_ports_actifs; port_sortie++) {
        if (port_sortie != port_entree && sw->ports[port_sortie].etat != PORT_BLOCKED) {
            enfiler_trame(&sw->ports[port_sortie].buffer_out, t);
        }
    }
}

static int comparer_infos_designated(unsigned int cout_a, bridge_id bridge_a,
                                      unsigned int cout_b, bridge_id bridge_b)
{
    if (cout_a < cout_b) {
        return -1;
    }
    if (cout_a > cout_b) {
        return 1;
    }
    return comparer_bridge_id(bridge_a, bridge_b);
}

static void mettre_port_designated_si_possible(switch_reseau *sw, size_t p)
{
    if (sw->ports[p].etat != PORT_ROOT && sw->ports[p].etat != PORT_BLOCKED) {
        sw->ports[p].etat = PORT_DESIGNATED;
    }
}

static void traiter_bpdu(switch_reseau *sw, size_t port_entree, bpdu b)
{
    unsigned int cout_via_port = b.root_path_cost + sw->ports[port_entree].cout_lien;
    bool maj_root = false;

    int cmp_root = comparer_bridge_id(b.root_id, sw->root_id);
    if (cmp_root < 0) {
        maj_root = true;
    } else if (cmp_root == 0) {
        if (sw->port_racine == (size_t)-1 && comparer_bridge_id(sw->bridge, sw->root_id) != 0) {
            maj_root = true;
        } else if (cout_via_port < sw->cout_vers_root) {
            maj_root = true;
        } else if (cout_via_port == sw->cout_vers_root && port_entree < sw->port_racine) {
            maj_root = true;
        }
    }

    if (maj_root) {
        sw->root_id = b.root_id;
        sw->cout_vers_root = cout_via_port;
        sw->port_racine = port_entree;

        for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
            sw->ports[p].etat = (p == port_entree) ? PORT_ROOT : PORT_DESIGNATED;
        }

        printf("[STP] Switch ");
        afficher_mac(sw->mac);
        printf(" choisit root ");
        afficher_bridge_id(sw->root_id);
        printf(" cout %u via port %zu\n", sw->cout_vers_root, port_entree);
        return;
    }

    if (cmp_root == 0 && comparer_bridge_id(sw->bridge, sw->root_id) != 0 && port_entree != sw->port_racine) {
        int cmp_segment = comparer_infos_designated(
            b.root_path_cost,
            b.bridge_id,
            sw->cout_vers_root,
            sw->bridge
        );

        if (cmp_segment < 0) {
            if (sw->ports[port_entree].etat != PORT_BLOCKED) {
                sw->ports[port_entree].etat = PORT_BLOCKED;
                printf("[STP] Switch ");
                afficher_mac(sw->mac);
                printf(" bloque le port %zu (voisin meilleur sur ce lien)\n", port_entree);
            }
        } else {
            mettre_port_designated_si_possible(sw, port_entree);
        }
    } else if (comparer_bridge_id(sw->bridge, sw->root_id) == 0) {
        sw->ports[port_entree].etat = PORT_DESIGNATED;
    }
}

static void envoyer_bpdus_switch(switch_reseau *sw)
{
    for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
        if (sw->ports[p].etat == PORT_BLOCKED) {
            continue;
        }

        if (sw->ports[p].etat == PORT_UNKNOWN) {
            sw->ports[p].etat = PORT_DESIGNATED;
        }

        bpdu b;
        b.root_id = sw->root_id;
        b.root_path_cost = sw->cout_vers_root;
        b.bridge_id = sw->bridge;
        b.port_id = (unsigned int)p;

        trame out_bpdu;
        initialiser_trame_bpdu(&out_bpdu, sw->mac, b);
        enfiler_trame(&sw->ports[p].buffer_out, out_bpdu);
    }
}

void reseau_simuler_tick(reseau *r)
{
    // Phase 1: chaque equipement traite les trames deja recues.
    for (size_t i = 0; i < r->nb_equipements; i++) {
        equipement *eq = &r->equipements[i];

        if (eq->type == TYPE_STATION) {
            station *st = &eq->donnees.st;
            trame t;
            while (defiler_trame(&st->port_unique.buffer_in, &t)) {
                if (t.type == TYPE_TRAME_DATA && comparer_mac(t.destination, st->mac) == 0) {
                    printf("[RECEPTION] Station ");
                    afficher_mac(st->mac);
                    printf(" recoit une trame de ");
                    afficher_mac(t.source);
                    printf(" : \"%s\"\n", t.donnees.payload);
                }
            }
        } else if (eq->type == TYPE_SWITCH) {
            switch_reseau *sw = &eq->donnees.sw;

            for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
                trame t;
                while (defiler_trame(&sw->ports[p].buffer_in, &t)) {
                    if (t.type == TYPE_TRAME_DATA) {
                        traiter_trame_data(sw, p, t);
                    } else if (t.type == TYPE_TRAME_BPDU) {
                        traiter_bpdu(sw, p, t.donnees.info_stp);
                    }
                }
            }
        }
    }

    // Phase 2: les switchs annoncent leur information STP par BPDU.
    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->equipements[i].type == TYPE_SWITCH) {
            envoyer_bpdus_switch(&r->equipements[i].donnees.sw);
        }
    }

    // Phase 3: les trames sortantes voyagent sur les liens vers les voisins.
    for (size_t i = 0; i < r->nb_equipements; i++) {
        equipement *eq = &r->equipements[i];

        if (eq->type == TYPE_STATION) {
            station *st = &eq->donnees.st;
            deplace_trames_vers_voisin(r, i, &st->port_unique.buffer_out, st->port_unique.voisin_idx);
        } else if (eq->type == TYPE_SWITCH) {
            switch_reseau *sw = &eq->donnees.sw;
            for (size_t p = 0; p < sw->nb_ports_actifs; p++) {
                deplace_trames_vers_voisin(r, i, &sw->ports[p].buffer_out, sw->ports[p].voisin_idx);
            }
        }
    }
}

bool reseau_trouver_deux_stations(const reseau *r, size_t *source_idx, size_t *destination_idx)
{
    bool trouve_source = false;

    for (size_t i = 0; i < r->nb_equipements; i++) {
        if (r->equipements[i].type == TYPE_STATION) {
            if (!trouve_source) {
                *source_idx = i;
                trouve_source = true;
            } else {
                *destination_idx = i;
                return true;
            }
        }
    }

    return false;
}

bool reseau_injecter_trame_station_vers_station(reseau *r, size_t station_source_idx, size_t station_destination_idx, const char *message)
{
    if (station_source_idx >= r->nb_equipements || station_destination_idx >= r->nb_equipements) {
        return false;
    }
    if (r->equipements[station_source_idx].type != TYPE_STATION ||
        r->equipements[station_destination_idx].type != TYPE_STATION) {
        return false;
    }

    station *source = &r->equipements[station_source_idx].donnees.st;
    station *destination = &r->equipements[station_destination_idx].donnees.st;

    trame t;
    initialiser_trame_data(&t, source->mac, destination->mac, message);

    printf("\n[DEMO] Injection d'une trame DATA de station %zu vers station %zu\n",
           station_source_idx,
           station_destination_idx);
    afficher_trame(&t);
    afficher_trame_hex(&t);

    enfiler_trame(&source->port_unique.buffer_out, t);
    return true;
}
