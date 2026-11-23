/* ia_sesion15_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion15.h. No puede traer main().
 */
#include "api_sesion15.h"
#include <stdlib.h>

/* O(1). Crea un ambito hijo del actual. NULL si algun argumento es NULL
 * o si no hay memoria. */
Ambito *push_ambito(Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) {
        return NULL;
    }
    Ambito *hijo = (Ambito *)malloc(sizeof(Ambito));
    if (hijo == NULL) {
        return NULL;
    }
    hijo->nombre = nombre;
    hijo->padre = actual;
    return hijo;
}

/* O(1). Libera el ambito actual y devuelve su padre. La raiz (padre NULL)
 * no fue creada por push_ambito, asi que no se libera: se devuelve NULL.
 * Precondicion: todo ambito con padre fue creado por push_ambito. */
Ambito *pop_ambito(Ambito *actual) {
    if (actual == NULL || actual->padre == NULL) {
        return NULL;
    }
    Ambito *padre = actual->padre;
    free(actual);
    return padre;
}
