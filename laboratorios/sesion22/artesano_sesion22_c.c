/* artesano_sesion22_c.c - Fase 1: El Artesano (sin IA)
 * Traductor de AST a TAC con precedencia.
 *
 * Implementa lo que declara api_sesion22.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion22.h"
#include <stdio.h>
#include <string.h>

/* Los temporales ya estan resueltos: usalos tal cual. */
typedef struct {
    char nombre[64][16];
    int usados;
} Pool;

const char *nuevo_temporal(Pool *p, int *contador) {
    if (p->usados >= 64) return "_t?";
    snprintf(p->nombre[p->usados], sizeof p->nombre[0], "_t%d", (*contador)++);
    return p->nombre[p->usados++];
}

/* Postorden: primero los dos hijos y despues el operador.
 * Regresa el nombre donde quedo el resultado del subarbol (la hoja o el
 * temporal), o NULL si ya no se pudo emitir. */
static const char *recorrer(const Nodo *n, char lineas[][64], int max, int *emitidas,
                            Pool *p, int *contador) {
    if (n == NULL) return NULL;
    if (n->tipo != 'B') return n->valor;        /* una hoja no emite nada */

    const char *izq = recorrer(n->izq, lineas, max, emitidas, p, contador);
    const char *der = recorrer(n->der, lineas, max, emitidas, p, contador);
    if (izq == NULL || der == NULL || *emitidas >= max) return NULL;

    const char *t = nuevo_temporal(p, contador);
    if (strcmp(t, "_t?") == 0) return NULL;     /* agregado en la auditoria: se acabo el pool */
    snprintf(lineas[*emitidas], 64, "%s = %s %c %s", t, izq, n->op, der);
    (*emitidas)++;
    return t;
}

int generar_tac(const Nodo *raiz, char lineas[][64], int max) {
    if (raiz == NULL || lineas == NULL || max <= 0) return 0;

    Pool p;
    p.usados = 0;
    int contador = 0, emitidas = 0;
    recorrer(raiz, lineas, max, &emitidas, &p, &contador);
    return emitidas;
}
