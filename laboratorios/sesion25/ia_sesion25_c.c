/* ia_sesion25_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion25.h. No puede traer main().
 */
#include "api_sesion25.h"
#include <stdio.h>
#include <string.h>

#define ANCHO 64

/* Una linea solo sirve si snprintf la escribio completa. */
static int cabe(int escritos) {
    return escritos >= 0 && escritos < ANCHO;
}

/* La posicion de escritura debe estar dentro del arreglo (0 <= *n <= max)
 * y deben quedar al menos k lineas libres. */
static int hay_espacio(int max, const int *n, int k) {
    return *n >= 0 && max >= 0 && *n <= max && k <= max - *n;
}

/* O(L), L = largo de la condicion. Emite:
 *   L_inicio:
 *   IFFALSE <cond> GOTO L_fin
 *   GOTO L_inicio
 *   L_fin:
 * Todo o nada: si falta espacio o una linea no cabe, no escribe ni consume etiquetas. */
void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL || *etiquetas < 0) {
        return;
    }
    if (!hay_espacio(max, n, 4)) {
        return;
    }

    int inicio = *etiquetas;
    int fin = *etiquetas + 1;
    char tmp[4][ANCHO];
    if (!cabe(snprintf(tmp[0], ANCHO, "L%d:", inicio)) ||
        !cabe(snprintf(tmp[1], ANCHO, "IFFALSE %s GOTO L%d", cond, fin)) ||
        !cabe(snprintf(tmp[2], ANCHO, "GOTO L%d", inicio)) ||
        !cabe(snprintf(tmp[3], ANCHO, "L%d:", fin))) {
        return;
    }

    memcpy(lineas + *n, tmp, sizeof tmp);
    *n += 4;
    *etiquetas += 2;
}

/* O(L). Desazucara for (inicio; cond; post) en:
 *   <inicio>                     (se omite si es NULL)
 *   L_inicio:
 *   IFFALSE <cond> GOTO L_fin
 *   <post>                       (se omite si es NULL; va antes del GOTO de regreso)
 *   GOTO L_inicio
 *   L_fin:
 * Misma regla de todo o nada que emitir_while. */
void emitir_for(char lineas[][64], int max, int *n, const char *inicio, const char *cond, const char *post, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL || *etiquetas < 0) {
        return;
    }
    int total = 4 + (inicio != NULL) + (post != NULL);
    if (!hay_espacio(max, n, total)) {
        return;
    }

    int l_inicio = *etiquetas;
    int l_fin = *etiquetas + 1;
    char tmp[6][ANCHO];
    int k = 0;

    if (inicio != NULL && !cabe(snprintf(tmp[k++], ANCHO, "%s", inicio))) {
        return;
    }
    if (!cabe(snprintf(tmp[k++], ANCHO, "L%d:", l_inicio)) ||
        !cabe(snprintf(tmp[k++], ANCHO, "IFFALSE %s GOTO L%d", cond, l_fin))) {
        return;
    }
    if (post != NULL && !cabe(snprintf(tmp[k++], ANCHO, "%s", post))) {
        return;
    }
    if (!cabe(snprintf(tmp[k++], ANCHO, "GOTO L%d", l_inicio)) ||
        !cabe(snprintf(tmp[k++], ANCHO, "L%d:", l_fin))) {
        return;
    }

    memcpy(lineas + *n, tmp, (size_t)k * ANCHO);
    *n += k;
    *etiquetas += 2;
}
