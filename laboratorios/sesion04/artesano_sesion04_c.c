/* artesano_sesion04_c.c - Fase 1: El Artesano (sin IA)
 * Nodo binario de AST.
 *
 * Implementa lo que declara api_sesion04.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion04.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Hoja del arbol: guarda el numero y no tiene hijos. */
ASTNode *crear_numero(int v) {
    ASTNode *n = malloc(sizeof *n);
    if (n == NULL) return NULL;

    n->tipo = NODO_NUM;
    n->valor = v;
    n->op = '\0';                   /* una hoja no tiene operador */
    n->izq = NULL;
    n->der = NULL;
    return n;
}

/* Nodo interno: guarda el operador y enlaza los hijos en el orden recibido. */
ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der) {
    ASTNode *n = malloc(sizeof *n);
    if (n == NULL) return NULL;

    n->tipo = NODO_BINOP;
    n->valor = 0;
    n->op = op;
    n->izq = izq;                   /* el izquierdo es el primer operando */
    n->der = der;
    return n;
}

/* Caso base: la hoja regresa su valor. Caso recursivo: evalua los dos
 * hijos y despues aplica el operador. */
int evaluar(ASTNode *n) {
    if (n == NULL) return -1;
    if (n->tipo == NODO_NUM) return n->valor;

    int a = evaluar(n->izq);
    int b = evaluar(n->der);
    switch (n->op) {
        case '+': return a + b;
        case '-': return a - b;     /* el orden importa: izq - der */
        case '*': return a * b;
        default:  return -1;        /* operador que no soporta esta sesion */
    }
}
