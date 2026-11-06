/* ia_sesion06_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion06.h. No puede traer main().
 */
#include "api_sesion06.h"

#define NUM_TIPOS 4

/* Matriz estatica [destino][origen]: 1 = asignacion implicita permitida.
 * Fila y columna 0 (TIPO_ERROR) son siempre 0. */
static const int COMPATIBLE[NUM_TIPOS][NUM_TIPOS] = {
    /* origen:   ERROR CHAR INT FLOAT */
    /* ERROR */ { 0,    0,   0,  0 },
    /* CHAR  */ { 0,    1,   0,  0 },
    /* INT   */ { 0,    1,   1,  0 },
    /* FLOAT */ { 0,    1,   1,  1 },
};

static const char *const NOMBRES[NUM_TIPOS] = { "error", "char", "int", "float" };

static int en_rango(Tipo t) {
    return (int)t >= 0 && (int)t < NUM_TIPOS;
}

/* O(1): una consulta a la matriz. */
int compatible(Tipo destino, Tipo origen) {
    if (!en_rango(destino) || !en_rango(origen)) {
        return 0;
    }
    return COMPATIBLE[destino][origen];
}

/* O(1). */
const char *nombre_tipo(Tipo t) {
    if (!en_rango(t)) {
        return "desconocido";
    }
    return NOMBRES[t];
}
