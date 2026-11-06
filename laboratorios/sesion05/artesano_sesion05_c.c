/* artesano_sesion05_c.c - Fase 1: El Artesano (sin IA)
 * Recorrido postorden y RPN.
 *
 * Implementa lo que declara api_sesion05.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion05.h"
#include <stdio.h>
#include <string.h>

/* Copia texto al final del buffer caracter por caracter y se detiene
 * en cap - 1 para que siempre quepa el '\0'. */
static void anexar(const char *texto, char *buf, int cap, int *pos) {
    while (*texto != '\0' && *pos < cap - 1) {
        buf[*pos] = *texto;
        (*pos)++;
        texto++;
    }
    buf[*pos] = '\0';
}

/* Postorden: primero el hijo izquierdo, luego el derecho y al final la raiz. */
static void recorrer(ASTNode *n, char *buf, int cap, int *pos) {
    if (n == NULL) return;

    char token[16];
    if (n->tipo == NODO_BINOP) {
        recorrer(n->izq, buf, cap, pos);
        recorrer(n->der, buf, cap, pos);
        token[0] = n->op;
        token[1] = '\0';
    } else {
        snprintf(token, sizeof token, "%d", n->valor);  /* un int cabe en 11 caracteres */
    }

    if (*pos > 0) anexar(" ", buf, cap, pos);           /* separador solo entre tokens */
    anexar(token, buf, cap, pos);
}

void postorden(ASTNode *n, char *buf, int cap) {
    if (buf == NULL || cap <= 0) return;                /* sin buffer no hay donde escribir */

    buf[0] = '\0';                                      /* arbol NULL = cadena vacia */
    int pos = 0;
    recorrer(n, buf, cap, &pos);
}
