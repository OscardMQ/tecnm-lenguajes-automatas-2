/* ia_sesion14_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion14.h. No puede traer main().
 */
#include "api_sesion14.h"
#include <stddef.h>
#include <string.h>

/* O(n). Devuelve la posicion del nombre en la tabla o -1 si no esta.
 * La comparacion es exacta (strcmp), nunca por prefijo. */
static int indice_de(const Tabla *tabla, const char *nombre) {
    for (int i = 0; i < tabla->total; i++) {
        if (tabla->nombres[i] != NULL && strcmp(tabla->nombres[i], nombre) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(n). 1 = insertado, 0 = ya declarado, -1 = argumento invalido o tabla llena. */
int declarar(Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL || nombre[0] == '\0') {
        return -1;
    }
    if (indice_de(tabla, nombre) >= 0) {
        return 0;
    }
    if (tabla->total < 0 || tabla->total >= TABLA_MAX) {
        return -1;
    }
    tabla->nombres[tabla->total++] = nombre;
    return 1;
}

/* O(n). 1 solo si el nombre ya fue declarado. */
int usar(const Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL) {
        return 0;
    }
    return indice_de(tabla, nombre) >= 0 ? 1 : 0;
}
