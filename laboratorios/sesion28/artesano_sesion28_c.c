/* artesano_sesion28_c.c - Fase 1: El Artesano (sin IA)
 * Plegado de constantes e identidades.
 *
 * Implementa lo que declara api_sesion28.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion28.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int hay(const char *s) {
    return s != NULL && s[0] != '\0';
}

/* Entero completo: '-' opcional y solo digitos, nada de espacios, '+', 0x ni texto sobrante.
 * Ademas tiene que caber en un int, que es el tamano del destino. */
static int entero(const char *s, long long *v) {
    const char *p = s;
    int neg = (*p == '-');
    if (neg) p++;
    if (*p == '\0') return 0;                        /* agregado en la auditoria: "-" solo no es un numero */
    if (strlen(p) > 10) return 0;                    /* un int tiene a lo mas 10 digitos */
    long long x = 0;
    for (; *p; p++) {
        if (*p < '0' || *p > '9') return 0;
        x = x * 10 + (*p - '0');
    }
    x = neg ? -x : x;
    if (x < INT_MIN || x > INT_MAX) return 0;
    *v = x;
    return 1;
}

/* Calcula en long long y solo pliega si el resultado cabe en un int.
 * Asi INT_MAX + 1 o INT_MIN / -1 se quedan como estaban, igual que en ejecucion. */
static int calcular(char o, long long x, long long y, long long *r) {
    switch (o) {
    case '+': *r = x + y; break;
    case '-': *r = x - y; break;
    case '*': *r = x * y; break;
    case '/':
        if (y == 0) return 0;                        /* entre cero no se pliega */
        *r = x / y;
        break;
    default: return 0;
    }
    return *r >= INT_MIN && *r <= INT_MAX;
}

void plegar(char *dst, int cap, const char *res, const char *op, const char *a, const char *b) {
    if (dst == NULL || cap <= 0) return;
    dst[0] = '\0';
    if (!hay(res) || !hay(op) || !hay(a) || !hay(b)) return;

    long long x = 0, y = 0, r = 0;
    int ca = entero(a, &x), cb = entero(b, &y);
    char o = (strlen(op) == 1) ? op[0] : '?';
    int w;

    /* 1) dos literales: un solo literal */
    if (ca && cb && calcular(o, x, y, &r)) {
        w = snprintf(dst, (size_t)cap, "%s = %lld", res, r);
    } else {
        /* 2) identidades: el operando que sobrevive, o 0 */
        const char *queda = NULL;
        if (o == '+' && cb && y == 0) queda = a;                     /* x + 0 */
        else if (o == '+' && ca && x == 0) queda = b;                /* 0 + x */
        else if (o == '-' && cb && y == 0) queda = a;                /* x - 0 (0 - x no) */
        else if ((o == '*' || o == '/') && cb && y == 1) queda = a;  /* x * 1, x / 1 */
        else if (o == '*' && ca && x == 1) queda = b;                /* 1 * x */
        else if (o == '*' && ((cb && y == 0) || (ca && x == 0))) queda = "0";
        /* 0 / x no se toca: si x vale 0 en ejecucion, el programa original falla */

        w = queda ? snprintf(dst, (size_t)cap, "%s = %s", res, queda)
                  : snprintf(dst, (size_t)cap, "%s = %s %s %s", res, a, op, b);   /* 3) la original */
    }
    if (w < 0 || w >= cap) dst[0] = '\0';            /* cortado no: mejor vacio */
}
