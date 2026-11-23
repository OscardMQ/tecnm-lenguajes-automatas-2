/* artesano_sesion21_c.c - Fase 1: El Artesano (sin IA)
 * Cuádruplos y temporales únicos.
 *
 * Implementa lo que declara api_sesion21.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion21.h"
#include <stdio.h>
#include <string.h>

/* Escribe _tN con el valor actual del contador y despues lo incrementa:
 * la primera llamada con contador 0 da _t0. */
void nuevo_temporal(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) return;
    if (contador == NULL) { dst[0] = '\0'; return; }

    int n = snprintf(dst, (size_t)cap, "_t%d", *contador);
    if (n < 0 || n >= cap) {        /* agregado en la auditoria: cortado ya no es unico */
        dst[0] = '\0';
        return;                     /* no se gasta el numero */
    }
    (*contador)++;
}

/* Formato (op, a1, a2, res); cualquier campo NULL se escribe como _. */
void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1, const char *a2, const char *res) {
    if (dst == NULL || cap <= 0) return;
    snprintf(dst, (size_t)cap, "(%s, %s, %s, %s)",
             op  ? op  : "_",
             a1  ? a1  : "_",
             a2  ? a2  : "_",
             res ? res : "_");
}
