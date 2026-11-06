/* artesano_sesion01_c.c - Fase 1: El Artesano (sin IA)
 * Inversion de buffer in-place.
 *
 * Implementa lo que declara api_sesion01.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion01.h"
#include <stdio.h>
#include <string.h>

/* Un indice al inicio, otro al final y un char temporal para el intercambio. */
void invertir(char *s) {
    if (s == NULL) return;              /* NULL: no hay buffer que tocar */

    size_t n = strlen(s);
    if (n < 2) return;                  /* "" y "a" ya estan invertidas; ademas evita n - 1 con n = 0 */

    size_t i = 0;
    size_t j = n - 1;                   /* ultimo caracter visible, no el '\0' */
    while (i < j) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}
