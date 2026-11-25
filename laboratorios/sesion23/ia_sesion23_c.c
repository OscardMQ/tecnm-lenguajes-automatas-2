/* ia_sesion23_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion23.h. No puede traer main().
 */
#include "api_sesion23.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Escribe "L<n>" con n = *contador y despues incrementa el contador.
 * Si la etiqueta no cabe completa, deja dst vacio y no consume el numero:
 * una etiqueta recortada podria repetirse. */
void nueva_etiqueta(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    dst[0] = '\0';
    if (contador == NULL || *contador < 0) {
        return;
    }

    int escritos = snprintf(dst, (size_t)cap, "L%d", *contador);
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
        return;
    }
    (*contador)++;
}

/* O(L), L = largo de la etiqueta. es_salto != 0 escribe "GOTO <etiqueta>";
 * es_salto == 0 escribe la marca "<etiqueta>:". Etiqueta NULL: cadena vacia. */
void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    if (etiqueta == NULL) {
        dst[0] = '\0';
        return;
    }
    if (es_salto) {
        snprintf(dst, (size_t)cap, "GOTO %s", etiqueta);
    } else {
        snprintf(dst, (size_t)cap, "%s:", etiqueta);
    }
}
