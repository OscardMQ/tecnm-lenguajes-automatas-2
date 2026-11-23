/* artesano_sesion13_c.c - Fase 1: El Artesano (sin IA)
 * Pila de ámbitos y búsqueda jerárquica.
 *
 * Implementa lo que declara api_sesion13.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion13.h"
#include <stdio.h>
#include <string.h>

/* Revisa solo el ambito que le pasan, sin subir al padre. */
static int esta_en(const Ambito *a, const char *nombre) {
    for (int i = 0; i < a->total; i++) {
        if (strcmp(a->nombres[i], nombre) == 0) return 1;
    }
    return 0;
}

/* 1 = insertado, 0 = ya estaba en ESTE ambito, -1 = lleno o argumentos malos. */
int declarar(Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL || nombre[0] == '\0') return -1;
    if (esta_en(ambito, nombre)) return 0;          /* el padre no se consulta: eso permite el shadowing */
    if (ambito->total >= AMBITO_MAX) return -1;     /* ya no caben mas nombres */
    ambito->nombres[ambito->total] = nombre;
    ambito->total++;
    return 1;
}

/* Sube del ambito actual hacia la raiz; el primero que tenga el nombre gana. */
const char *buscar(const Ambito *ambito, const char *nombre) {
    if (nombre == NULL) return NULL;
    for (const Ambito *a = ambito; a != NULL; a = a->padre) {
        if (esta_en(a, nombre)) return a->nombre;   /* se regresa el nombre del ambito, no el simbolo */
    }
    return NULL;
}
