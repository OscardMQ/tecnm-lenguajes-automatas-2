/* artesano_sesion14_c.c - Fase 1: El Artesano (sin IA)
 * Declaración previa y redeclaración.
 *
 * Implementa lo que declara api_sesion14.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion14.h"
#include <stdio.h>
#include <string.h>

/* Busca el nombre completo con strcmp: asi "x" y "xy" no se confunden. */
static int existe(const Tabla *tabla, const char *nombre) {
    for (int i = 0; i < tabla->total; i++) {
        if (strcmp(tabla->nombres[i], nombre) == 0) return 1;
    }
    return 0;
}

/* 1 = insertado, 0 = ya estaba, -1 = NULL, nombre vacio o tabla llena. */
int declarar(Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL || nombre[0] == '\0') return -1;
    if (existe(tabla, nombre)) return 0;            /* redeclaracion: no se agrega otra vez */
    if (tabla->total >= TABLA_MAX) return -1;       /* el contrato no lo dice; sin lugar no se inserta */
    tabla->nombres[tabla->total] = nombre;
    tabla->total++;
    return 1;
}

/* 1 solo si ya fue declarado; cualquier otra cosa es 0. */
int usar(const Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL) return 0;
    return existe(tabla, nombre);
}
