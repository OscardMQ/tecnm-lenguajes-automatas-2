/* ia_sesion27_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion27.h. No puede traer main().
 */
#include "api_sesion27.h"
#include <stdio.h>
#include <string.h>

#define ANCHO_LINEA 64
#define MAX_LINEAS_ACCESO 4

/* Un texto es valido solo si no es NULL ni la cadena vacia. */
static int texto_valido(const char *s) {
    return s != NULL && *s != '\0';
}

/* Copia las lineas preparadas al destino solo si todas estan completas
 * y hay espacio para todas (todo o nada). Devuelve cuantas copio. */
static int confirmar(char lineas[][64], int max, char buffer[][ANCHO_LINEA],
                     const int *escritos, int cantidad) {
    if (max < cantidad) return 0;
    for (int k = 0; k < cantidad; k++) {
        if (escritos[k] < 0 || escritos[k] >= ANCHO_LINEA) return 0;  /* linea truncada */
    }
    for (int k = 0; k < cantidad; k++) {
        memcpy(lineas[k], buffer[k], ANCHO_LINEA);
    }
    return cantidad;
}

int emitir_acceso(char lineas[][64], int max, const char *base, const char *indice,
                  int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL) return 0;
    if (!texto_valido(base) || !texto_valido(indice) || ancho <= 0) return 0;

    char buffer[MAX_LINEAS_ACCESO][ANCHO_LINEA];
    int escritos[2];
    int t = *temporales;

    /* Offset: el indice siempre se multiplica por el ancho del elemento. */
    escritos[0] = snprintf(buffer[0], ANCHO_LINEA, "_t%d = %s * %d", t, indice, ancho);
    /* Lectura indexada desde la base con el offset calculado. */
    escritos[1] = snprintf(buffer[1], ANCHO_LINEA, "_t%d = %s[_t%d]", t + 1, base, t);

    int copiadas = confirmar(lineas, max, buffer, escritos, 2);
    if (copiadas > 0) *temporales = t + 2;
    return copiadas;
}

int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i,
                     const char *j, int filas, int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL) return 0;
    if (!texto_valido(base) || !texto_valido(i) || !texto_valido(j)) return 0;
    if (filas <= 0 || ancho <= 0) return 0;

    char buffer[MAX_LINEAS_ACCESO][ANCHO_LINEA];
    int escritos[4];
    int t = *temporales;

    /* Aplanado en orden de filas: i * filas + j. */
    escritos[0] = snprintf(buffer[0], ANCHO_LINEA, "_t%d = %s * %d", t, i, filas);
    escritos[1] = snprintf(buffer[1], ANCHO_LINEA, "_t%d = _t%d + %s", t + 1, t, j);
    /* Escalado por el ancho del elemento y lectura indexada. */
    escritos[2] = snprintf(buffer[2], ANCHO_LINEA, "_t%d = _t%d * %d", t + 2, t + 1, ancho);
    escritos[3] = snprintf(buffer[3], ANCHO_LINEA, "_t%d = %s[_t%d]", t + 3, base, t + 2);

    int copiadas = confirmar(lineas, max, buffer, escritos, 4);
    if (copiadas > 0) *temporales = t + 4;
    return copiadas;
}
