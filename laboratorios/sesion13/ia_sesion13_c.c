/* ia_sesion13_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion13.h. No puede traer main().
 */
#include "api_sesion13.h"
#include <stddef.h>
#include <string.h>

/* O(n), n = nombres del ambito (n <= AMBITO_MAX). */
static int contiene(const Ambito *ambito, const char *nombre) {
    for (int i = 0; i < ambito->total; i++) {
        if (ambito->nombres[i] != NULL && strcmp(ambito->nombres[i], nombre) == 0) {
            return 1;
        }
    }
    return 0;
}

/* O(n). Inserta solo en el ambito actual: un nombre puede repetirse en
 * niveles distintos (shadowing) pero no dos veces en el mismo nivel. */
int declarar(Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL || *nombre == '\0') {
        return -1;
    }
    if (contiene(ambito, nombre)) {
        return 0;
    }
    if (ambito->total < 0 || ambito->total >= AMBITO_MAX) {
        return -1;
    }
    ambito->nombres[ambito->total++] = nombre;
    return 1;
}

/* O(d * n), d = profundidad de la cadena. Devuelve el nombre del ambito
 * mas cercano que declara el identificador, o NULL si ninguno lo declara. */
const char *buscar(const Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL) {
        return NULL;
    }
    for (const Ambito *actual = ambito; actual != NULL; actual = actual->padre) {
        if (contiene(actual, nombre)) {
            return actual->nombre;
        }
    }
    return NULL;
}
