/* ia_sesion21_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion21.h. No puede traer main().
 */
#include "api_sesion21.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Escribe "_tN" con N = *contador y luego incrementa el contador.
 * Si el nombre no cabe completo en dst, deja dst vacio y NO consume el
 * numero: un temporal recortado ("_t1" en lugar de "_t12") dejaria de
 * ser unico. */
void nuevo_temporal(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    dst[0] = '\0';
    if (contador == NULL || *contador < 0) {
        return;
    }

    int escritos = snprintf(dst, (size_t)cap, "_t%d", *contador);
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
        return;
    }
    (*contador)++;
}

static const char *o_guion(const char *s) {
    return (s != NULL) ? s : "_";
}

/* O(L), L = largo del cuadruplo. Formato "(op, a1, a2, res)"; un campo
 * NULL se escribe como "_". Respeta cap y siempre termina en '\0'. */
void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1, const char *a2, const char *res) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    int escritos = snprintf(dst, (size_t)cap, "(%s, %s, %s, %s)",
                            o_guion(op), o_guion(a1), o_guion(a2), o_guion(res));
    if (escritos < 0) {
        dst[0] = '\0';
    }
}
