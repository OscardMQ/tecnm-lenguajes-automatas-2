/* artesano_sesion12_c.c - Fase 1: El Artesano (sin IA)
 * Registro de símbolo y offsets alineados.
 *
 * Implementa lo que declara api_sesion12.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion12.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* Tamanos de x86-64: char 1, short 2, int 4, float 4, double 8, puntero 8. */
int ancho_de_tipo(const char *tipo) {
    if (tipo == NULL) return -1;
    if (strcmp(tipo, "char") == 0)    return 1;
    if (strcmp(tipo, "short") == 0)   return 2;
    if (strcmp(tipo, "int") == 0)     return 4;
    if (strcmp(tipo, "float") == 0)   return 4;
    if (strcmp(tipo, "double") == 0)  return 8;
    if (strcmp(tipo, "puntero") == 0) return 8;
    return -1;                                  /* tipo que no existe */
}

/* Redondea el cursor hacia arriba al siguiente multiplo del ancho,
 * guarda ese offset en el registro y regresa donde empieza la siguiente. */
int asignar_offset(Simbolo *simbolo, int cursor) {
    if (simbolo == NULL || cursor < 0) return -1;

    int ancho = ancho_de_tipo(simbolo->tipo);
    if (ancho < 0) return -1;
    if (cursor > INT_MAX - 2 * ancho + 1) return -1;    /* agregado en la auditoria: offset + ancho no cabe en int */

    int offset = (cursor + ancho - 1) / ancho * ancho;  /* alineacion natural */
    simbolo->offset = offset;
    return offset + ancho;
}
