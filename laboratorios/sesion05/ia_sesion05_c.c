/* ia_sesion05_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion05.h. No puede traer main().
 */
#include "api_sesion05.h"
#include <stdio.h>
#include <stddef.h>

/* Estado de escritura: el buffer, su capacidad y cuantos caracteres van. */
typedef struct {
    char *buf;
    size_t cap;
    size_t len;
} Salida;

/* O(1) por token. snprintf devuelve lo que habria escrito; si eso no cabe,
 * len se fija en cap - 1 y las escrituras siguientes ya no hacen nada. */
static void emitir(Salida *s, const char *sep, ASTNode *n) {
    if (s->len >= s->cap - 1) {
        return;
    }

    size_t libre = s->cap - s->len;
    int escritos;
    if (n->tipo == NODO_NUM) {
        escritos = snprintf(s->buf + s->len, libre, "%s%d", sep, n->valor);
    } else {
        escritos = snprintf(s->buf + s->len, libre, "%s%c", sep, n->op);
    }

    if (escritos < 0) {
        return;
    }
    if ((size_t)escritos >= libre) {
        s->len = s->cap - 1;
    } else {
        s->len += (size_t)escritos;
    }
}

/* O(n) en nodos, O(h) de pila. */
static void visitar(Salida *s, ASTNode *n) {
    if (n == NULL) {
        return;
    }
    if (n->tipo == NODO_BINOP) {
        visitar(s, n->izq);
        visitar(s, n->der);
    }
    emitir(s, s->len == 0 ? "" : " ", n);
}

void postorden(ASTNode *n, char *buf, int cap) {
    if (buf == NULL || cap <= 0) {
        return;
    }

    buf[0] = '\0';
    Salida s = { buf, (size_t)cap, 0 };
    visitar(&s, n);
}
