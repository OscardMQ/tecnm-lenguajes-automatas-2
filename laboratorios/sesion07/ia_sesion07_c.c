/* ia_sesion07_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion07.h. No puede traer main().
 */
#include "api_sesion07.h"
#include <limits.h>
#include <stddef.h>

enum { EVAL_OK = 0, EVAL_DIV_CERO = 1, EVAL_NODO_INVALIDO = 2 };

/* O(n) en nodos, O(h) de pila. Se detiene en el primer error: el padre
 * no vuelve a calcular nada cuando un hijo ya fallo. */
static int evaluar_rec(const ASTNode *n, int *error) {
    if (n == NULL) {
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }

    switch (n->tipo) {
    case NODO_NUM:
        return n->valor;
    case NODO_BINOP:
        break;
    default:
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }

    int izq = evaluar_rec(n->izq, error);
    if (*error != EVAL_OK) {
        return 0;
    }
    int der = evaluar_rec(n->der, error);
    if (*error != EVAL_OK) {
        return 0;
    }

    switch (n->op) {
    case '+':
        return izq + der;
    case '-':
        return izq - der;
    case '*':
        return izq * der;
    case '/':
    case '%':
        if (der == 0) {
            *error = EVAL_DIV_CERO;
            return 0;
        }
        /* INT_MIN / -1 no cabe en int: en x86 provoca la misma excepcion
         * de hardware que dividir entre cero. */
        if (izq == INT_MIN && der == -1) {
            *error = EVAL_DIV_CERO;
            return 0;
        }
        return (n->op == '/') ? izq / der : izq % der;
    default:
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }
}

int evaluar(ASTNode *n, int *error) {
    int local = EVAL_OK;
    if (error == NULL) {
        error = &local;
    }
    *error = EVAL_OK;
    return evaluar_rec(n, error);
}
