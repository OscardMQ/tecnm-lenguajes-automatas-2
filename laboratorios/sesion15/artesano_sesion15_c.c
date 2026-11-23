/* artesano_sesion15_c.c - Fase 1: El Artesano (sin IA)
 * Push y pop de ámbitos.
 *
 * Implementa lo que declara api_sesion15.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion15.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Reserva un ambito nuevo en el heap y lo cuelga del actual. */
Ambito *push_ambito(Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) return NULL;

    Ambito *nuevo = malloc(sizeof *nuevo);
    if (nuevo == NULL) return NULL;
    nuevo->nombre = nombre;
    nuevo->padre = actual;          /* el enlace al padre es lo que deja regresar despues */
    return nuevo;
}

/* Libera el ambito actual y regresa a su padre.
 * La raiz no la creo push (no tiene padre): no se libera y se regresa NULL.
 * Agregado en la auditoria: todo ambito con padre debe venir de push_ambito;
 * si viene de la pila, el free truena. */
Ambito *pop_ambito(Ambito *actual) {
    if (actual == NULL) return NULL;
    if (actual->padre == NULL) return NULL;     /* raiz: no es nuestra memoria */

    Ambito *padre = actual->padre;              /* se guarda ANTES del free */
    free(actual);
    return padre;
}
