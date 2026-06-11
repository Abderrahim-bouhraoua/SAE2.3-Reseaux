#include "trame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t calculer_fcs_simple(const trame *t)
{
    // Ce n'est pas le vrai CRC-32 Ethernet. On l'utilise seulement pour avoir
    // un champ FCS visible dans la simulation et dans l'affichage hexadecimale.
    uint32_t somme = 0;

    for (int i = 0; i < 6; i++) {
        somme += t->destination.octets[i];
        somme += t->source.octets[i];
    }

    somme += (t->ether_type >> 8) & 0xFF;
    somme += t->ether_type & 0xFF;

    if (t->type == TYPE_TRAME_DATA) {
        for (size_t i = 0; i < t->taille_donnees; i++) {
            somme += (unsigned char)t->donnees.payload[i];
        }
    } else {
        somme += t->donnees.info_stp.root_id.priorite;
        somme += t->donnees.info_stp.root_path_cost;
        somme += t->donnees.info_stp.bridge_id.priorite;
        somme += t->donnees.info_stp.port_id;
        for (int i = 0; i < 6; i++) {
            somme += t->donnees.info_stp.root_id.mac.octets[i];
            somme += t->donnees.info_stp.bridge_id.mac.octets[i];
        }
    }

    return somme;
}

static void initialiser_champs_communs(trame *t)
{
    for (int i = 0; i < 7; i++) {
        t->preambule[i] = 0xAA;
    }

    t->sfd = 0xAB;
    t->taille_donnees = 0;
    t->taille_bourrage = 0;
    t->fcs = 0;

    for (size_t i = 0; i < TAILLE_PADDING_MIN; i++) {
        t->bourrage[i] = 0x00;
    }
}

void initialiser_trame_data(trame *t, adresse_mac source, adresse_mac destination, const char *message)
{
    if (t == NULL) {
        return;
    }

    initialiser_champs_communs(t);
    t->source = source;
    t->destination = destination;
    t->type = TYPE_TRAME_DATA;
    t->ether_type = ETHER_TYPE_DATA_SIMULATION;

    if (message == NULL) {
        message = "";
    }

    strncpy(t->donnees.payload, message, TAILLE_PAYLOAD_MAX);
    t->donnees.payload[TAILLE_PAYLOAD_MAX] = '\0';
    t->taille_donnees = strlen(t->donnees.payload);

    if (t->taille_donnees < TAILLE_PADDING_MIN) {
        t->taille_bourrage = TAILLE_PADDING_MIN - t->taille_donnees;
    }

    t->fcs = calculer_fcs_simple(t);
}

void initialiser_trame_bpdu(trame *t, adresse_mac source, bpdu info)
{
    if (t == NULL) {
        return;
    }

    initialiser_champs_communs(t);
    t->source = source;
    t->type = TYPE_TRAME_BPDU;
    t->ether_type = ETHER_TYPE_BPDU_SIMULATION;
    t->donnees.info_stp = info;
    t->taille_donnees = sizeof(bpdu);

    for (int i = 0; i < 6; i++) {
        t->destination.octets[i] = 0xFF; // broadcast de simulation
    }

    if (t->taille_donnees < TAILLE_PADDING_MIN) {
        t->taille_bourrage = TAILLE_PADDING_MIN - t->taille_donnees;
    }

    t->fcs = calculer_fcs_simple(t);
}

static void afficher_octets_hex(const uint8_t *octets, size_t taille, size_t *colonne)
{
    for (size_t i = 0; i < taille; i++) {
        printf("%02x ", octets[i]);
        (*colonne)++;
        if (*colonne == 16) {
            printf("\n");
            *colonne = 0;
        }
    }
}

void afficher_trame(const trame *t)
{
    if (t == NULL) {
        return;
    }

    printf("\n=== TRAME ETHERNET ===\n");
    printf("Destination : ");
    afficher_mac(t->destination);
    printf("\nSource      : ");
    afficher_mac(t->source);
    printf("\nEtherType   : 0x%04x", t->ether_type);

    if (t->type == TYPE_TRAME_DATA) {
        printf(" (DATA)\n");
        printf("Donnees     : \"%s\"\n", t->donnees.payload);
        printf("Taille data : %zu octet(s), bourrage : %zu octet(s)\n",
               t->taille_donnees,
               t->taille_bourrage);
    } else {
        const bpdu *b = &t->donnees.info_stp;
        printf(" (BPDU/STP)\n");
        printf("Root ID     : priorite %u / ", b->root_id.priorite);
        afficher_mac(b->root_id.mac);
        printf("\nCout root   : %u\n", b->root_path_cost);
        printf("Bridge ID   : priorite %u / ", b->bridge_id.priorite);
        afficher_mac(b->bridge_id.mac);
        printf("\nPort ID     : %u\n", b->port_id);
    }

    printf("FCS simulé  : 0x%08x\n", t->fcs);
}

void afficher_trame_hex(const trame *t)
{
    if (t == NULL) {
        return;
    }

    printf("\n--- TRAME EN HEXADECIMAL ---\n");
    size_t colonne = 0;

    afficher_octets_hex(t->preambule, 7, &colonne);
    afficher_octets_hex(&t->sfd, 1, &colonne);
    afficher_octets_hex(t->destination.octets, 6, &colonne);
    afficher_octets_hex(t->source.octets, 6, &colonne);

    uint8_t type_octets[2] = {
        (uint8_t)((t->ether_type >> 8) & 0xFF),
        (uint8_t)(t->ether_type & 0xFF)
    };
    afficher_octets_hex(type_octets, 2, &colonne);

    if (t->type == TYPE_TRAME_DATA) {
        afficher_octets_hex((const uint8_t *)t->donnees.payload, t->taille_donnees, &colonne);
    } else {
        const bpdu *b = &t->donnees.info_stp;
        uint8_t tmp[24];
        size_t k = 0;

        tmp[k++] = (uint8_t)((b->root_id.priorite >> 8) & 0xFF);
        tmp[k++] = (uint8_t)(b->root_id.priorite & 0xFF);
        memcpy(&tmp[k], b->root_id.mac.octets, 6); k += 6;
        tmp[k++] = (uint8_t)((b->root_path_cost >> 24) & 0xFF);
        tmp[k++] = (uint8_t)((b->root_path_cost >> 16) & 0xFF);
        tmp[k++] = (uint8_t)((b->root_path_cost >> 8) & 0xFF);
        tmp[k++] = (uint8_t)(b->root_path_cost & 0xFF);
        tmp[k++] = (uint8_t)((b->bridge_id.priorite >> 8) & 0xFF);
        tmp[k++] = (uint8_t)(b->bridge_id.priorite & 0xFF);
        memcpy(&tmp[k], b->bridge_id.mac.octets, 6); k += 6;
        tmp[k++] = (uint8_t)((b->port_id >> 8) & 0xFF);
        tmp[k++] = (uint8_t)(b->port_id & 0xFF);
        afficher_octets_hex(tmp, k, &colonne);
    }

    afficher_octets_hex(t->bourrage, t->taille_bourrage, &colonne);

    uint8_t fcs_octets[4] = {
        (uint8_t)((t->fcs >> 24) & 0xFF),
        (uint8_t)((t->fcs >> 16) & 0xFF),
        (uint8_t)((t->fcs >> 8) & 0xFF),
        (uint8_t)(t->fcs & 0xFF)
    };
    afficher_octets_hex(fcs_octets, 4, &colonne);

    if (colonne != 0) {
        printf("\n");
    }
}

void enfiler_trame(file_trames *f, trame t)
{
    noeud_trame *n = malloc(sizeof(noeud_trame));
    if (n == NULL) {
        return;
    }

    n->t = t;
    n->suivant = NULL;

    if (f->fin == NULL) {
        f->debut = n;
        f->fin = n;
    } else {
        f->fin->suivant = n;
        f->fin = n;
    }
}

bool defiler_trame(file_trames *f, trame *t)
{
    if (f->debut == NULL) {
        return false;
    }

    noeud_trame *n = f->debut;
    *t = n->t;

    f->debut = n->suivant;
    if (f->debut == NULL) {
        f->fin = NULL;
    }

    free(n);
    return true;
}

void vider_file_trames(file_trames *f)
{
    trame t;
    while (defiler_trame(f, &t)) {
        // nothing else to do
    }
}
