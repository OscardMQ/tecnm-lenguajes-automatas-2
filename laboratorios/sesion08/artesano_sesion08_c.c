/* artesano_sesion08_c.c - Fase 1: El Artesano (sin IA)
 * Reglas de promocion y perdida.
 *
 * Implementa lo que declara api_sesion08.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion08.h"
#include <stdio.h>
#include <string.h>

/* Solo CHAR, INT y FLOAT son tipos reales; ERROR o un valor raro no. */
static int es_valido(Tipo t) {
    return t >= TIPO_CHAR && t <= TIPO_FLOAT;
}

/* El tipo comun es el mas ancho. Si alguno ya es ERROR, el resultado
 * tambien lo es, para que el error no se pierda en la expresion. */
Tipo promover(Tipo a, Tipo b) {
    if (!es_valido(a) || !es_valido(b)) return TIPO_ERROR;
    return (a > b) ? a : b;
}

/* Hay perdida cuando el destino es mas angosto que el origen (FLOAT -> INT).
 * Con ERROR no hay conversion que medir: ese error ya se reporta aparte. */
int hay_perdida(Tipo destino, Tipo origen) {
    if (!es_valido(destino) || !es_valido(origen)) return 0;
    return destino < origen;
}

const char *nombre_tipo(Tipo t) {
    switch (t) {
        case TIPO_ERROR: return "error";
        case TIPO_CHAR:  return "char";
        case TIPO_INT:   return "int";
        case TIPO_FLOAT: return "float";
    }
    return "desconocido";
}
