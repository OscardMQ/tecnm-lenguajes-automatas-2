/* ia_sesion04_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion04.h. No puede traer main().
 */
#include "api_sesion04.h"
#include <stdlib.h>

/* O(1): reserva un nodo con todos los campos en cero/NULL. */
static ASTNode *nodo_nuevo(TipoNodo tipo) {
    ASTNode *n = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (n != NULL) {
        n->tipo = tipo;
    }
    return n;
}

/* O(1). */
ASTNode *crear_numero(int v) {
    ASTNode *n = nodo_nuevo(NODO_NUM);
    if (n != NULL) {
        n->valor = v;
    }
    return n;
}

/* O(1). Conserva el orden: izq es el operando izquierdo. */
ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der) {
    ASTNode *n = nodo_nuevo(NODO_BINOP);
    if (n != NULL) {
        n->op = op;
        n->izq = izq;
        n->der = der;
    }
    return n;
}

/* O(n) en nodos, O(h) de pila por la recursion (h = altura del arbol).
 * Devuelve -1 si el nodo es NULL o el operador no es + - *. */
int evaluar(ASTNode *n) {
    if (n == NULL) {
        return -1;
    }
    if (n->tipo == NODO_NUM) {
        return n->valor;
    }

    int izq = evaluar(n->izq);
    int der = evaluar(n->der);
    switch (n->op) {
    case '+':
        return izq + der;
    case '-':
        return izq - der;
    case '*':
        return izq * der;
    default:
        return -1;
    }
}
