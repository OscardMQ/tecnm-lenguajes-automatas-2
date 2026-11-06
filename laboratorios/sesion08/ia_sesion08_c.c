/* ia_sesion08_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion08.h. No puede traer main().
 */
#include "api_sesion08.h"

/* Rango de cada tipo en la jerarquia CHAR < INT < FLOAT.
 * -1 marca TIPO_ERROR o cualquier valor fuera del enum. */
static int rango(Tipo t) {
    switch (t) {
    case TIPO_CHAR:
        return 1;
    case TIPO_INT:
        return 2;
    case TIPO_FLOAT:
        return 3;
    case TIPO_ERROR:
    default:
        return -1;
    }
}

/* O(1). Devuelve el tipo de mayor rango; TIPO_ERROR si alguno es invalido. */
Tipo promover(Tipo a, Tipo b) {
    if (rango(a) < 0 || rango(b) < 0) {
        return TIPO_ERROR;
    }
    return (rango(a) >= rango(b)) ? a : b;
}

/* O(1). 1 si convertir origen -> destino estrecha el tipo. Con un tipo
 * invalido devuelve 0: el error ya lo reporta el verificador de tipos. */
int hay_perdida(Tipo destino, Tipo origen) {
    if (rango(destino) < 0 || rango(origen) < 0) {
        return 0;
    }
    return rango(destino) < rango(origen);
}

/* O(1). */
const char *nombre_tipo(Tipo t) {
    switch (t) {
    case TIPO_ERROR:
        return "error";
    case TIPO_CHAR:
        return "char";
    case TIPO_INT:
        return "int";
    case TIPO_FLOAT:
        return "float";
    default:
        return "desconocido";
    }
}
