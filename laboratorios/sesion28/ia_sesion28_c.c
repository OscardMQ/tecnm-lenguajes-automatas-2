/* ia_sesion28_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion28.h. No puede traer main().
 */
#include "api_sesion28.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Devuelve 1 si s es un literal entero completo y guarda su valor. */
static int es_entero(const char *s, long *valor) {
    if (s == NULL || *s == '\0') return 0;
    char *fin = NULL;
    errno = 0;
    long v = strtol(s, &fin, 10);
    if (errno == ERANGE || fin == s || *fin != '\0') return 0;  /* texto sobrante o fuera de rango */
    *valor = v;
    return 1;
}

/* Escribe en dst sin truncar: si no cabe completo, deja la cadena vacia. */
static void escribir(char *dst, int cap, int escritos) {
    if (escritos < 0 || escritos >= cap) dst[0] = '\0';
}

void plegar(char *dst, int cap, const char *res, const char *op, const char *a, const char *b) {
    if (dst == NULL || cap <= 0) return;
    dst[0] = '\0';
    if (res == NULL || op == NULL || a == NULL || b == NULL) return;
    if (*res == '\0' || *op == '\0' || *a == '\0' || *b == '\0') return;

    long va = 0, vb = 0;
    int a_const = es_entero(a, &va);
    int b_const = es_entero(b, &vb);
    char o = (op[1] == '\0') ? op[0] : '\0';

    /* Plegado de constantes: los dos operandos son literales. */
    if (a_const && b_const && o != '\0') {
        long r = 0;
        int plegable = 1;
        switch (o) {
            case '+': r = va + vb; break;
            case '-': r = va - vb; break;
            case '*': r = va * vb; break;
            case '/':
                if (vb == 0) plegable = 0;   /* division entre cero: no se pliega */
                else r = va / vb;
                break;
            default: plegable = 0; break;
        }
        if (plegable) {
            escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %ld", res, r));
            return;
        }
    }

    /* Identidades algebraicas. */
    if (o == '+' && b_const && vb == 0) {               /* x + 0 -> x */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s", res, a));
        return;
    }
    if (o == '*' && b_const && vb == 1) {               /* x * 1 -> x */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s", res, a));
        return;
    }
    if (o == '*' && b_const && vb == 0) {               /* x * 0 -> 0 */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = 0", res));
        return;
    }

    /* Sin simplificacion posible: la instruccion original. */
    escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s %s %s", res, a, op, b));
}
