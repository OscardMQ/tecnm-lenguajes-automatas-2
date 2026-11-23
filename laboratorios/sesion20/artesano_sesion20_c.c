/* artesano_sesion20_c.c - Fase 1: El Artesano (sin IA)
 * Resolución de nombres y visibilidad.
 *
 * Implementa lo que declara api_sesion20.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion20.h"
#include <stdio.h>
#include <string.h>

/* 1 si el ambito 'a' (solo ese, sin subir) declara 'nombre'. */
static int declara(const Ambito *a, const char *nombre) {
    for (int i = 0; i < a->total; i++) {
        if (a->simbolos[i] != NULL && strcmp(a->simbolos[i], nombre) == 0) return 1;
    }
    return 0;
}

/* Sube por padre contando niveles; regresa en el primer ambito que tiene
 * el nombre, asi el mas interno siempre gana (shadowing). */
int profundidad_de(const Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) return -2;
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre, nivel++) {
        if (declara(a, nombre)) return nivel;
    }
    return -1;
}

/* Cuenta un nombre solo la primera vez que se ve desde el ambito actual:
 * se salta si ya estaba en un ambito mas interno o antes en el mismo. */
int contar_visibles(const Ambito *actual) {
    if (actual == NULL) return -2;              /* el contrato no lo dice: uso el mismo codigo que profundidad_de */
    int total = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        for (int i = 0; i < a->total; i++) {
            const char *s = a->simbolos[i];
            if (s == NULL) continue;
            int repetido = 0;
            for (const Ambito *b = actual; b != a && !repetido; b = b->padre) {
                repetido = declara(b, s);        /* ya lo tapaba uno mas interno */
            }
            for (int j = 0; j < i && !repetido; j++) {
                repetido = (a->simbolos[j] != NULL && strcmp(a->simbolos[j], s) == 0);
            }
            if (!repetido) total++;
        }
    }
    return total;
}
