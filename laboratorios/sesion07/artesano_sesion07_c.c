/* artesano_sesion07_c.c - Fase 1: El Artesano (sin IA)
 * Evaluador recursivo con deteccion de errores.
 *
 * Implementa lo que declara api_sesion07.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion07.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

/* error = 0 todo bien, 1 division (o modulo) entre cero, 2 nodo invalido.
 * En cuanto un hijo marca error, se regresa sin calcular nada mas. */
static int calcular(ASTNode *n, int *error) {
    if (n == NULL) { *error = 2; return 0; }
    if (n->tipo == NODO_NUM) return n->valor;
    if (n->tipo != NODO_BINOP) { *error = 2; return 0; }

    int a = calcular(n->izq, error);
    if (*error != 0) return 0;              /* el error sube tal cual */
    int b = calcular(n->der, error);
    if (*error != 0) return 0;

    switch (n->op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
        case '%':
            if (b == 0) { *error = 1; return 0; }               /* revisar ANTES de dividir */
            if (a == INT_MIN && b == -1) { *error = 1; return 0; } /* agregado en la auditoria: tambien truena */
            return (n->op == '/') ? a / b : a % b;
        default:
            *error = 2;                     /* operador desconocido = nodo invalido */
            return 0;
    }
}

int evaluar(ASTNode *n, int *error) {
    int sin_uso = 0;
    if (error == NULL) error = &sin_uso;    /* si no me dan donde reportar, no me caigo */
    *error = 0;
    return calcular(n, error);
}
