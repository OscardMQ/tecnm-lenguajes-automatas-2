/* artesano_sesion27_c.c - Fase 1: El Artesano (sin IA)
 * Desplazamientos para arreglos 1D y 2D.
 *
 * Implementa lo que declara api_sesion27.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion27.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* "" cuenta igual que NULL (lo de la sesion 26) */
static int hay(const char *s) {
    return s != NULL && s[0] != '\0';
}

/* Todo o nada: copia las k lineas solo si todas cupieron completas y hay lugar.
 * Cada linea crea un temporal nuevo, asi que el contador avanza k. */
static int volcar(char lineas[][64], int max, char tmp[][64], const int *w, int k, int *temporales) {
    for (int i = 0; i < k; i++)
        if (w[i] < 0 || w[i] >= 64) return 0;
    if (max < k) return 0;
    for (int i = 0; i < k; i++) memcpy(lineas[i], tmp[i], 64);
    *temporales += k;
    return k;
}

/* _tK = indice * ancho   y   _tK+1 = base[_tK]. Regresa cuantas lineas escribio. */
int emitir_acceso(char lineas[][64], int max, const char *base, const char *indice, int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL || !hay(base) || !hay(indice)) return 0;
    if (ancho <= 0) return 0;                        /* agregado en la auditoria: con ancho 0 todo i lee A[0] */
    int t = *temporales;
    if (t < 0 || t > INT_MAX - 2) return 0;          /* ni _t-1 ni un contador que se desborde */

    char tmp[2][64];
    int w[2];
    w[0] = snprintf(tmp[0], 64, "_t%d = %s * %d", t, indice, ancho);     /* el offset siempre lleva el ancho */
    w[1] = snprintf(tmp[1], 64, "_t%d = %s[_t%d]", t + 1, base, t);
    return volcar(lineas, max, tmp, w, 2, temporales);
}

/* Aplanado: i * filas + j, despues por el ancho y la lectura.
 * filas es lo largo de cada fila (en A[N][M] es la M). */
int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i, const char *j, int filas, int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL || !hay(base) || !hay(i) || !hay(j)) return 0;
    if (filas <= 0 || ancho <= 0) return 0;          /* agregado en la auditoria */
    int t = *temporales;
    if (t < 0 || t > INT_MAX - 4) return 0;

    char tmp[4][64];
    int w[4];
    w[0] = snprintf(tmp[0], 64, "_t%d = %s * %d", t, i, filas);
    w[1] = snprintf(tmp[1], 64, "_t%d = _t%d + %s", t + 1, t, j);
    w[2] = snprintf(tmp[2], 64, "_t%d = _t%d * %d", t + 2, t + 1, ancho);
    w[3] = snprintf(tmp[3], 64, "_t%d = %s[_t%d]", t + 3, base, t + 2);
    return volcar(lineas, max, tmp, w, 4, temporales);
}
