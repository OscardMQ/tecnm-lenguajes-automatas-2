/* ia_sesion10_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion10.h. No puede traer main().
 */
#include "api_sesion10.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>

/* Gramatica (descendente recursivo, LL(1)):
 *   expr     := termino  { ("+" | "-") termino }
 *   termino  := primaria { ("*" | "/") primaria }
 *   primaria := NUMERO | "(" expr ")"
 * error: 0 = bien, 1 = division entre cero, 2 = sintaxis invalida.
 * Solo se conserva el primer error. */
typedef struct {
    const char *s;
    int error;
} Parser;

enum { ERR_NINGUNO = 0, ERR_DIV_CERO = 1, ERR_SINTAXIS = 2 };

static void saltar(Parser *p) {
    while (*p->s == ' ') {
        p->s++;
    }
}

static void fallar(Parser *p, int codigo) {
    if (p->error == ERR_NINGUNO) {
        p->error = codigo;
    }
}

/* Devuelve 1 y consume c si es el siguiente caracter no blanco. */
static int aceptar(Parser *p, char c) {
    saltar(p);
    if (*p->s == c) {
        p->s++;
        return 1;
    }
    return 0;
}

long expr(Parser *p);

/* O(d), d = digitos. Un literal que no cabe en long se trata como sintaxis invalida. */
long primaria(Parser *p) {
    if (p->error != ERR_NINGUNO) {
        return 0;
    }
    if (aceptar(p, '(')) {
        long v = expr(p);
        if (!aceptar(p, ')')) {
            fallar(p, ERR_SINTAXIS);
            return 0;
        }
        return v;
    }

    saltar(p);
    if (!isdigit((unsigned char)*p->s)) {
        fallar(p, ERR_SINTAXIS);
        return 0;
    }

    long v = 0;
    while (isdigit((unsigned char)*p->s)) {
        int d = *p->s - '0';
        if (v > (LONG_MAX - d) / 10) {
            fallar(p, ERR_SINTAXIS);
            return 0;
        }
        v = v * 10 + d;
        p->s++;
    }
    return v;
}

long termino(Parser *p) {
    long v = primaria(p);
    for (;;) {
        if (p->error != ERR_NINGUNO) {
            return 0;
        }
        if (aceptar(p, '*')) {
            v *= primaria(p);
        } else if (aceptar(p, '/')) {
            long d = primaria(p);
            if (p->error != ERR_NINGUNO) {
                return 0;
            }
            if (d == 0) {
                fallar(p, ERR_DIV_CERO);
                return 0;
            }
            v /= d;
        } else {
            return v;
        }
    }
}

long expr(Parser *p) {
    long v = termino(p);
    for (;;) {
        if (p->error != ERR_NINGUNO) {
            return 0;
        }
        if (aceptar(p, '+')) {
            v += termino(p);
        } else if (aceptar(p, '-')) {
            v -= termino(p);
        } else {
            return v;
        }
    }
}

/* O(n), n = longitud de la cadena: cada caracter se consume una sola vez. */
long evaluar_expresion(const char *fuente, int *error) {
    static int sin_error;
    if (error == NULL) {
        error = &sin_error;
    }
    *error = ERR_NINGUNO;
    if (fuente == NULL) {
        *error = ERR_SINTAXIS;
        return 0;
    }

    Parser p = { fuente, ERR_NINGUNO };
    long v = expr(&p);
    saltar(&p);
    if (p.error == ERR_NINGUNO && *p.s != '\0') {
        p.error = ERR_SINTAXIS;
    }
    *error = p.error;
    return (p.error != ERR_NINGUNO) ? 0 : v;
}
