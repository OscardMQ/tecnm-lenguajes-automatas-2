/* ia_sesion11_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion11.h. No puede traer main().
 */
#include "api_sesion11.h"
#include <stdlib.h>
#include <string.h>

/* O(k), k = longitud de la clave. h = h * 33 + c, con aritmetica modular
 * de unsigned int (el desbordamiento esta definido por el estandar). */
unsigned int hash_djb2(const char *clave) {
    unsigned int h = 5381U;
    if (clave == NULL) {
        return h;
    }
    while (*clave != '\0') {
        h = ((h << 5) + h) + (unsigned char)*clave;
        clave++;
    }
    return h;
}

/* O(L) en promedio, L = largo de la cadena de la cubeta.
 * 1 = insertado, 0 = ya existia, -1 = argumentos invalidos o sin memoria. */
int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas, const char *nombre, int tipo, int offset) {
    if (tabla == NULL || nombre == NULL || cubetas == 0U) {
        return -1;
    }

    size_t largo = strlen(nombre);
    if (largo == 0U || largo >= sizeof(((Simbolo *)0)->nombre)) {
        return -1;
    }

    unsigned int indice = hash_djb2(nombre) % cubetas;

    Simbolo **enlace = &tabla[indice];
    while (*enlace != NULL) {
        if (strcmp((*enlace)->nombre, nombre) == 0) {
            return 0;
        }
        enlace = &(*enlace)->sig;
    }

    Simbolo *nuevo = (Simbolo *)malloc(sizeof(Simbolo));
    if (nuevo == NULL) {
        return -1;
    }
    memcpy(nuevo->nombre, nombre, largo + 1U);
    nuevo->tipo = tipo;
    nuevo->offset = offset;
    nuevo->sig = NULL;
    *enlace = nuevo;
    return 1;
}
