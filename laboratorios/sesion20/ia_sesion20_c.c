/* ia_sesion20_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion20.h. No puede traer main().
 */
#include "api_sesion20.h"
#include <stddef.h>
#include <string.h>

/* O(k), k = simbolos del ambito. Indice de la primera aparicion del
 * nombre en ese ambito, o -1 si no esta. */
static int indice_en(const Ambito *a, const char *nombre) {
    for (int i = 0; i < a->total && i < AMBITO_MAX; i++) {
        if (a->simbolos[i] != NULL && strcmp(a->simbolos[i], nombre) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(d * k), d = profundidad de la cadena. -2 con argumentos NULL,
 * -1 si el nombre no es visible, o el nivel del ambito mas cercano. */
int profundidad_de(const Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) {
        return -2;
    }
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        if (indice_en(a, nombre) >= 0) {
            return nivel;
        }
        nivel++;
    }
    return -1;
}

/* O(N * d * k), N = total de simbolos en la cadena. Un simbolo del nivel d
 * cuenta solo si profundidad_de lo resuelve justo en ese nivel (nadie mas
 * interno lo oculta) y si es su primera aparicion dentro del ambito. */
int contar_visibles(const Ambito *actual) {
    if (actual == NULL) {
        return 0;
    }
    int visibles = 0;
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        for (int i = 0; i < a->total && i < AMBITO_MAX; i++) {
            const char *s = a->simbolos[i];
            if (s != NULL && profundidad_de(actual, s) == nivel && indice_en(a, s) == i) {
                visibles++;
            }
        }
        nivel++;
    }
    return visibles;
}
