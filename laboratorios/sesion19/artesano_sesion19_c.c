/* artesano_sesion19_c.c - Fase 1: El Artesano (sin IA)
 * Offsets de miembros y tamaño de struct.
 *
 * Implementa lo que declara api_sesion19.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion19.h"
#include <stdio.h>
#include <string.h>

/* Ancho de cada tipo en x86-64; -1 si no existe. */
static int ancho_de(const char *tipo) {
    if (tipo == NULL) return -1;
    if (strcmp(tipo, "char") == 0)    return 1;
    if (strcmp(tipo, "short") == 0)   return 2;
    if (strcmp(tipo, "int") == 0)     return 4;
    if (strcmp(tipo, "float") == 0)   return 4;
    if (strcmp(tipo, "double") == 0)  return 8;
    if (strcmp(tipo, "puntero") == 0) return 8;
    return -1;
}

/* Redondea hacia arriba al siguiente multiplo de ancho. */
static int alinear(int cursor, int ancho) {
    return (cursor + ancho - 1) / ancho * ancho;
}

/* Agregado en la auditoria: si algun campo tiene un tipo desconocido,
 * todo el struct es invalido, aunque el campo buscado este antes. */
static int tipos_validos(const Campo *campos, int total) {
    for (int i = 0; i < total; i++) {
        if (ancho_de(campos[i].tipo) < 0) return 0;
    }
    return 1;
}

/* Acomoda los campos en orden: cada uno se alinea a su ancho y despues
 * avanza el cursor. Cuando llega al campo pedido, regresa su offset. */
int offset_campo(const Campo *campos, int total, const char *nombre) {
    if (campos == NULL || nombre == NULL || total < 0) return -1;
    if (!tipos_validos(campos, total)) return -1;

    int cursor = 0;
    for (int i = 0; i < total; i++) {
        int ancho = ancho_de(campos[i].tipo);
        cursor = alinear(cursor, ancho);
        if (campos[i].nombre != NULL && strcmp(campos[i].nombre, nombre) == 0) {
            return cursor;
        }
        cursor += ancho;
    }
    return -1;                                  /* el campo no existe */
}

/* Mismo recorrido, pero al final redondea al ancho del campo mas grande:
 * asi, en un arreglo de estos structs, cada elemento queda alineado. */
int tamano_struct(const Campo *campos, int total) {
    if (campos == NULL || total < 0) return -1;
    if (total == 0) return 0;                   /* struct sin campos */
    if (!tipos_validos(campos, total)) return -1;

    int cursor = 0;
    int mayor = 1;
    for (int i = 0; i < total; i++) {
        int ancho = ancho_de(campos[i].tipo);
        cursor = alinear(cursor, ancho) + ancho;
        if (ancho > mayor) mayor = ancho;
    }
    return alinear(cursor, mayor);              /* padding final */
}
