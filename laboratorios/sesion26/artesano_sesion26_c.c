/* artesano_sesion26_c.c - Fase 1: El Artesano (sin IA)
 * Llamadas, parámetros y retorno.
 *
 * Implementa lo que declara api_sesion26.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion26.h"
#include <stdio.h>
#include <string.h>

/* agregado en la auditoria: "" cuenta igual que NULL (si no, sale "PARAM " o "CALL , 1") */
static int hay(const char *s) {
    return s != NULL && s[0] != '\0';
}

/* PARAM a1, PARAM a2 (solo los que hay) y despues el CALL con la cuenta real.
 * Regresa cuantas lineas agrego; 0 si no agrego nada. */
int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion, const char *a1, const char *a2, const char *resultado) {
    if (lineas == NULL || n == NULL || !hay(funcion)) return 0;

    const char *args[2] = { a1, a2 };
    char tmp[3][64];
    int k = 0;
    for (int i = 0; i < 2; i++) {
        if (!hay(args[i])) continue;
        int w = snprintf(tmp[k++], 64, "PARAM %s", args[i]);
        if (w < 0 || w >= 64) return 0;
    }

    int params = k;                                  /* el CALL lleva los PARAM que de verdad salieron */
    int c = hay(resultado) ? snprintf(tmp[k++], 64, "%s = CALL %s, %d", resultado, funcion, params)
                           : snprintf(tmp[k++], 64, "CALL %s, %d", funcion, params);
    if (c < 0 || c >= 64) return 0;

    if (*n < 0 || *n > max - k) return 0;            /* lo de la sesion 25: n tambien puede venir mal */
    for (int i = 0; i < k; i++) memcpy(lineas[*n + i], tmp[i], 64);
    *n += k;
    return k;
}

/* RETURN v, o solo RETURN si no hay valor. Si no cabe completo, cadena vacia. */
void emitir_return(char *dst, int cap, const char *valor) {
    if (dst == NULL || cap <= 0) return;

    int w = hay(valor) ? snprintf(dst, (size_t)cap, "RETURN %s", valor)
                       : snprintf(dst, (size_t)cap, "RETURN");
    if (w < 0 || w >= cap) dst[0] = '\0';            /* RETURN _t10 cortado seria RETURN _t1 */
}
