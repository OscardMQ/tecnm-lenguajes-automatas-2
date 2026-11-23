/* ia_sesion19_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion19.h. No puede traer main().
 */
#include "api_sesion19.h"
#include <stddef.h>
#include <string.h>

/* Ancho = alineacion en x86-64 (LP64) para los tipos escalares. */
static int ancho_tipo(const char *tipo) {
    static const struct { const char *nombre; int ancho; } TIPOS[] = {
        { "char", 1 }, { "short", 2 }, { "int", 4 },
        { "float", 4 }, { "double", 8 }, { "puntero", 8 },
    };
    if (tipo == NULL) {
        return -1;
    }
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++) {
        if (strcmp(tipo, TIPOS[i].nombre) == 0) {
            return TIPOS[i].ancho;
        }
    }
    return -1;
}

/* O(n). Recorre la disposicion completa del struct. Si 'objetivo' no es
 * NULL, guarda en *offset_objetivo el offset de ese campo (o deja -1).
 * Devuelve el tamano total con padding final, o -1 si algun tipo es
 * desconocido. */
static int disponer(const Campo *campos, int total, const char *objetivo, int *offset_objetivo) {
    int cursor = 0;
    int alineacion_max = 1;

    for (int i = 0; i < total; i++) {
        int ancho = ancho_tipo(campos[i].tipo);
        if (ancho < 0) {
            return -1;
        }
        cursor = (cursor + ancho - 1) / ancho * ancho;
        if (objetivo != NULL && *offset_objetivo < 0 &&
            campos[i].nombre != NULL && strcmp(campos[i].nombre, objetivo) == 0) {
            *offset_objetivo = cursor;
        }
        cursor += ancho;
        if (ancho > alineacion_max) {
            alineacion_max = ancho;
        }
    }
    return (cursor + alineacion_max - 1) / alineacion_max * alineacion_max;
}

/* O(n). -1 si la lista o el nombre son NULL, si un tipo es desconocido
 * o si el campo no existe. */
int offset_campo(const Campo *campos, int total, const char *nombre) {
    if (campos == NULL || nombre == NULL || total < 0) {
        return -1;
    }
    int offset = -1;
    if (disponer(campos, total, nombre, &offset) < 0) {
        return -1;
    }
    return offset;
}

/* O(n). 0 para un struct sin campos; -1 si la lista es NULL o un tipo es
 * desconocido. */
int tamano_struct(const Campo *campos, int total) {
    if (campos == NULL || total < 0) {
        return -1;
    }
    if (total == 0) {
        return 0;
    }
    return disponer(campos, total, NULL, NULL);
}
