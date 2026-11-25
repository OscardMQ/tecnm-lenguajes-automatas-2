/* ia_sesion22_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion22.h. No puede traer main().
 */
#include "api_sesion22.h"
#include <stdio.h>
#include <string.h>

/* Pool de temporales que da el contrato; se usa sin cambios. */
typedef struct {
    char nombre[64][16];
    int usados;
} Pool;

const char *nuevo_temporal(Pool *p, int *contador) {
    if (p->usados >= 64) return "_t?";
    snprintf(p->nombre[p->usados], sizeof p->nombre[0], "_t%d", (*contador)++);
    return p->nombre[p->usados++];
}

typedef struct {
    char (*lineas)[64];
    int max;
    int emitidas;
    int error;
    Pool pool;
    int contador;
} Generador;

/* O(n), n = nodos del arbol: cada nodo se visita una vez, en postorden.
 * Devuelve el nombre que guarda el valor del subarbol (hoja o temporal)
 * o NULL si hubo un error; despues de un error ya no se emite nada. */
static const char *traducir(Generador *g, const Nodo *n) {
    if (g->error || n == NULL) {
        g->error = 1;
        return NULL;
    }
    if (n->tipo != 'B') {
        return n->valor;
    }

    const char *izq = traducir(g, n->izq);
    const char *der = traducir(g, n->der);
    if (g->error) {
        return NULL;
    }
    if (g->emitidas >= g->max) {
        g->error = 1;
        return NULL;
    }

    const char *t = nuevo_temporal(&g->pool, &g->contador);
    if (strcmp(t, "_t?") == 0) {
        /* El pool se agoto: repetir "_t?" romperia la unicidad de los temporales. */
        g->error = 1;
        return NULL;
    }
    snprintf(g->lineas[g->emitidas], 64, "%s = %s %c %s", t, izq, n->op, der);
    g->emitidas++;
    return t;
}

/* O(n). Escribe una linea TAC por operador y devuelve cuantas escribio. */
int generar_tac(const Nodo *raiz, char lineas[][64], int max) {
    if (raiz == NULL || lineas == NULL || max <= 0) {
        return 0;
    }
    Generador g;
    g.lineas = lineas;
    g.max = max;
    g.emitidas = 0;
    g.error = 0;
    g.pool.usados = 0;
    g.contador = 0;

    traducir(&g, raiz);
    return g.emitidas;
}
