/* artesano_sesion06_c.c - Fase 1: El Artesano (sin IA)
 * Tabla de compatibilidad de tipos.
 *
 * Implementa lo que declara api_sesion06.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion06.h"
#include <stdio.h>
#include <string.h>

/* Solo CHAR, INT y FLOAT son tipos reales. ERROR o un valor fuera del enum no. */
static int es_valido(Tipo t) {
    return t >= TIPO_CHAR && t <= TIPO_FLOAT;
}

/* La jerarquia CHAR < INT < FLOAT ya esta en los numeros del enum:
 * se puede asignar si el destino es igual o mas ancho que el origen. */
int compatible(Tipo destino, Tipo origen) {
    if (!es_valido(destino) || !es_valido(origen)) return 0;
    return destino >= origen;
}

const char *nombre_tipo(Tipo t) {
    switch (t) {
        case TIPO_ERROR: return "error";
        case TIPO_CHAR:  return "char";
        case TIPO_INT:   return "int";
        case TIPO_FLOAT: return "float";
    }
    return "desconocido";           /* cualquier valor que no este en el enum */
}
