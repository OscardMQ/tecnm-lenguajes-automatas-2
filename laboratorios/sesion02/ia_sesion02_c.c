/* ia_sesion02_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion02.h. No puede traer main().
 */
#include "api_sesion02.h"
#include <stdlib.h>
#include <string.h>

/* O(k), k = longitud del lexema acotada a 15: reserva y llena un nodo. */
static NodoToken *nuevo_nodo(const char *lexema) {
    NodoToken *nodo = (NodoToken *)malloc(sizeof(NodoToken));
    if (nodo == NULL) {
        return NULL;
    }

    size_t largo = strlen(lexema);
    if (largo > sizeof(nodo->lexema) - 1U) {
        largo = sizeof(nodo->lexema) - 1U;
    }
    memcpy(nodo->lexema, lexema, largo);
    nodo->lexema[largo] = '\0';
    nodo->siguiente = NULL;
    return nodo;
}

/* O(n): inserta al final y devuelve la cabeza (nueva si la lista estaba vacia).
 * Si malloc falla, devuelve la lista sin cambios. */
NodoToken *agregar_token(NodoToken *cabeza, const char *lexema) {
    if (lexema == NULL) {
        return cabeza;
    }

    NodoToken *nodo = nuevo_nodo(lexema);
    if (nodo == NULL) {
        return cabeza;
    }

    NodoToken **enlace = &cabeza;
    while (*enlace != NULL) {
        enlace = &(*enlace)->siguiente;
    }
    *enlace = nodo;
    return cabeza;
}

/* O(n). */
int contar_tokens(NodoToken *cabeza) {
    int total = 0;
    while (cabeza != NULL) {
        total++;
        cabeza = cabeza->siguiente;
    }
    return total;
}

/* O(n), iterativa para no arriesgar desbordamiento de pila. */
void liberar_lista(NodoToken *cabeza) {
    while (cabeza != NULL) {
        NodoToken *siguiente = cabeza->siguiente;
        free(cabeza);
        cabeza = siguiente;
    }
}
