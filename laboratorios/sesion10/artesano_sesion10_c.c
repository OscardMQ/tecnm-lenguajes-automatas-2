/* artesano_sesion10_c.c - Fase 1: El Artesano (sin IA)
 * Mini-evaluador de expresiones (Hito 1).
 *
 * Implementa lo que declara api_sesion10.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion10.h"
#include <stdio.h>
#include <string.h>

#include <ctype.h>
#include <limits.h>

/* El analizador ya viene armado: completa las tres funciones marcadas. */
typedef struct { const char *s; int error; } Parser;

static void saltar(Parser *p) {
    while (*p->s == ' ') p->s++;
}

long expr(Parser *p);

/* primaria := numero | "(" expr ")" */
long primaria(Parser *p) {
    saltar(p);
    if (p->error != 0) return 0;

    if (*p->s == '(') {
        p->s++;                                 /* consume '(' */
        long v = expr(p);
        saltar(p);
        if (p->error != 0) return 0;
        if (*p->s != ')') {                     /* falta cerrar el parentesis */
            p->error = 2;
            return 0;
        }
        p->s++;                                 /* consume ')' */
        return v;
    }

    if (isdigit((unsigned char)*p->s)) {
        long v = 0;
        while (isdigit((unsigned char)*p->s)) { /* arma el numero digito por digito */
            int d = *p->s - '0';
            if (v > (LONG_MAX - d) / 10) {      /* agregado en la auditoria: el literal no cabe en long */
                p->error = 2;
                return 0;
            }
            v = v * 10 + d;
            p->s++;
        }
        return v;
    }

    p->error = 2;                               /* ni numero ni '(': sintaxis invalida */
    return 0;
}

/* termino := primaria { ("*" | "/") primaria } */
long termino(Parser *p) {
    long v = primaria(p);
    while (p->error == 0) {
        saltar(p);
        char op = *p->s;
        if (op != '*' && op != '/') break;
        p->s++;
        long d = primaria(p);
        if (p->error != 0) break;
        if (op == '*') {
            v = v * d;
        } else {
            if (d == 0) {                       /* revisar ANTES de dividir */
                p->error = 1;
                break;
            }
            v = v / d;
        }
    }
    return v;
}

/* expr := termino { ("+" | "-") termino } */
long expr(Parser *p) {
    long v = termino(p);
    while (p->error == 0) {
        saltar(p);
        char op = *p->s;
        if (op != '+' && op != '-') break;
        p->s++;
        long d = termino(p);
        if (p->error != 0) break;
        v = (op == '+') ? v + d : v - d;        /* el ciclo deja la asociatividad a la izquierda */
    }
    return v;
}

long evaluar_expresion(const char *fuente, int *error) {
    static int sin_error;
    if (error == NULL) error = &sin_error;
    *error = 0;
    if (fuente == NULL) { *error = 2; return 0; }
    Parser p = { fuente, 0 };
    long v = expr(&p);
    saltar(&p);
    if (p.error == 0 && *p.s != '\0') p.error = 2;
    *error = p.error;
    return p.error != 0 ? 0 : v;
}
