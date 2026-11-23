/* ia_sesion12_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion12.h. No puede traer main().
 */
#include "api_sesion12.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *nombre;
    int ancho;
} TipoBasico;

/* Modelo de datos LP64 (x86-64 Linux). */
static const TipoBasico TIPOS[] = {
    { "char", 1 }, { "short", 2 }, { "int", 4 },
    { "float", 4 }, { "double", 8 }, { "puntero", 8 },
};

/* O(T), T = 6 tipos. -1 si el tipo es NULL o desconocido. */
int ancho_de_tipo(const char *tipo) {
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

/* O(1). Como el ancho es potencia de 2, alinear es redondear con una mascara:
 * offset = (cursor + ancho - 1) & ~(ancho - 1). Devuelve el cursor siguiente,
 * o -1 si el registro es NULL, el cursor es negativo, el tipo no existe o el
 * resultado no cabe en int. */
int asignar_offset(Simbolo *simbolo, int cursor) {
    if (simbolo == NULL || cursor < 0) {
        return -1;
    }

    int ancho = ancho_de_tipo(simbolo->tipo);
    if (ancho < 0) {
        return -1;
    }
    if (cursor > INT_MAX - 2 * ancho) {
        return -1;
    }

    int offset = (cursor + ancho - 1) & ~(ancho - 1);
    simbolo->offset = offset;
    return offset + ancho;
}
