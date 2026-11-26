/* artesano_sesion25_c.c - Fase 1: El Artesano (sin IA)
 * Esqueleto TAC de while y for.
 *
 * Implementa lo que declara api_sesion25.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion25.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Escribe una linea en dst; regresa 0 si no cupo completa (una linea cortada no sirve). */
static int linea(char dst[64], const char *formato, ...) {
    va_list args;
    va_start(args, formato);
    int k = vsnprintf(dst, 64, formato, args);
    va_end(args);
    return k >= 0 && k < 64;
}

/* Copia las k lineas preparadas al final de lineas, solo si caben todas. */
static int agregar(char lineas[][64], int max, int *n, char tmp[][64], int k) {
    if (*n < 0) return 0;                           /* agregado en la auditoria: n negativo */
    if (*n > max - k) return 0;                     /* o todo o nada, como en la sesion 24 */
    for (int i = 0; i < k; i++) memcpy(lineas[*n + i], tmp[i], 64);
    *n += k;
    return 1;
}

/* L_inicio:  /  IFFALSE cond GOTO L_fin  /  (cuerpo)  /  GOTO L_inicio  /  L_fin:
 * La marca va antes de la condicion para que se vuelva a evaluar en cada vuelta. */
void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL) return;

    int ini = *etiquetas, fin = *etiquetas + 1;
    char tmp[4][64];
    if (!linea(tmp[0], "L%d:", ini) || !linea(tmp[1], "IFFALSE %s GOTO L%d", cond, fin) ||
        !linea(tmp[2], "GOTO L%d", ini) || !linea(tmp[3], "L%d:", fin)) return;
    if (agregar(lineas, max, n, tmp, 4)) *etiquetas += 2;   /* las etiquetas solo se gastan si se usan */
}

/* for (inicio; cond; post) se desazucara como: inicio; while (cond) { cuerpo; post; } */
void emitir_for(char lineas[][64], int max, int *n, const char *inicio, const char *cond, const char *post, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL) return;

    int ini = *etiquetas, fin = *etiquetas + 1, k = 0;
    char tmp[6][64];
    if (inicio != NULL && !linea(tmp[k++], "%s", inicio)) return;   /* antes de la marca: corre una vez */
    if (!linea(tmp[k++], "L%d:", ini)) return;
    if (!linea(tmp[k++], "IFFALSE %s GOTO L%d", cond, fin)) return;
    if (post != NULL && !linea(tmp[k++], "%s", post)) return;       /* justo antes del GOTO de regreso */
    if (!linea(tmp[k++], "GOTO L%d", ini)) return;
    if (!linea(tmp[k++], "L%d:", fin)) return;
    if (agregar(lineas, max, n, tmp, k)) *etiquetas += 2;
}
