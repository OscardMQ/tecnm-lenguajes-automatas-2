/* artesano_sesion23_c.c - Fase 1: El Artesano (sin IA)
 * Etiquetas y saltos incondicionales.
 *
 * Implementa lo que declara api_sesion23.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion23.h"
#include <stdio.h>
#include <string.h>

/* Escribe LN con el valor actual del contador y despues lo incrementa:
 * la primera etiqueta es L0. Es la misma idea que nuevo_temporal (sesion 21). */
void nueva_etiqueta(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) return;
    if (contador == NULL) { dst[0] = '\0'; return; }

    int n = snprintf(dst, (size_t)cap, "L%d", *contador);
    if (n < 0 || n >= cap) {        /* cortada ya no es unica: no gasto el numero */
        dst[0] = '\0';
        return;
    }
    (*contador)++;
}

/* es_salto distinto de 0: "GOTO Lx". es_salto en 0: la marca "Lx:". */
void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto) {
    if (dst == NULL || cap <= 0) return;
    if (etiqueta == NULL) { dst[0] = '\0'; return; }

    int n = es_salto ? snprintf(dst, (size_t)cap, "GOTO %s", etiqueta)
                     : snprintf(dst, (size_t)cap, "%s:", etiqueta);
    if (n < 0 || n >= cap) dst[0] = '\0';   /* un GOTO cortado saltaria a otra etiqueta */
}
