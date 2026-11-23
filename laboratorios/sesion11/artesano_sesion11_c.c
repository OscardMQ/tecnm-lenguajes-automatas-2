/* artesano_sesion11_c.c - Fase 1: El Artesano (sin IA)
 * Tabla hash con encadenamiento.
 *
 * Implementa lo que declara api_sesion11.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion11.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* djb2: empieza en 5381 y por cada caracter hace h * 33 + c.
 * Con unsigned int el desbordamiento da la vuelta y esta bien definido. */
unsigned int hash_djb2(const char *clave) {
    unsigned int h = 5381;
    if (clave == NULL) return h;
    for (const unsigned char *c = (const unsigned char *)clave; *c != '\0'; c++) {
        h = h * 33 + *c;
    }
    return h;
}

int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas, const char *nombre, int tipo, int offset) {
    if (tabla == NULL || nombre == NULL || cubetas == 0) return -1;
    if (nombre[0] == '\0' || strlen(nombre) >= sizeof tabla[0]->nombre) return -1; /* vacio o no cabe en 15 */

    unsigned int i = hash_djb2(nombre) % cubetas;

    /* el mismo nombre siempre cae en la misma cubeta: basta revisar esa cadena */
    for (Simbolo *p = tabla[i]; p != NULL; p = p->sig) {
        if (strcmp(p->nombre, nombre) == 0) return 0;   /* ya existe: no se duplica */
    }

    Simbolo *nuevo = malloc(sizeof *nuevo);
    if (nuevo == NULL) return -1;
    strcpy(nuevo->nombre, nombre);                      /* ya sabemos que cabe */
    nuevo->tipo = tipo;
    nuevo->offset = offset;
    nuevo->sig = tabla[i];                              /* se encadena al inicio, nunca se sobrescribe */
    tabla[i] = nuevo;
    return 1;
}
