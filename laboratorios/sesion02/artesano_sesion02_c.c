/* artesano_sesion02_c.c - Fase 1: El Artesano (sin IA)
 * Lista enlazada de tokens en heap.
 *
 * Implementa lo que declara api_sesion02.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion02.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Crea el nodo al final y regresa la cabeza. Quien llama debe hacer
 * c = agregar_token(c, ...), porque la cabeza cambia en la primera insercion. */
NodoToken *agregar_token(NodoToken *cabeza, const char *lexema) {
    if (lexema == NULL) return cabeza;

    NodoToken *nodo = malloc(sizeof *nodo);
    if (nodo == NULL) return cabeza;                /* sin memoria: la lista queda igual */

    strncpy(nodo->lexema, lexema, sizeof nodo->lexema - 1);
    nodo->lexema[sizeof nodo->lexema - 1] = '\0';   /* strncpy no pone el nulo si el lexema es largo */
    nodo->siguiente = NULL;                         /* nodo nuevo = fin de la lista */

    if (cabeza == NULL) return nodo;                /* lista vacia: el nodo es la cabeza */

    NodoToken *ultimo = cabeza;
    while (ultimo->siguiente != NULL) {             /* camina hasta el ultimo nodo */
        ultimo = ultimo->siguiente;
    }
    ultimo->siguiente = nodo;
    return cabeza;                                  /* la cabeza no cambia */
}

/* Recorre la lista contando nodos; 0 si la cabeza es NULL. */
int contar_tokens(NodoToken *cabeza) {
    int total = 0;
    for (NodoToken *actual = cabeza; actual != NULL; actual = actual->siguiente) {
        total++;
    }
    return total;
}

/* Guarda el siguiente antes del free para no leer memoria ya liberada. */
void liberar_lista(NodoToken *cabeza) {
    while (cabeza != NULL) {
        NodoToken *siguiente = cabeza->siguiente;
        free(cabeza);
        cabeza = siguiente;
    }
}
