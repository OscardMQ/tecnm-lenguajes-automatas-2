/* artesano_sesion16_c.c - Fase 1: El Artesano (sin IA)
 * Registro de prototipos y aridad.
 *
 * Implementa lo que declara api_sesion16.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion16.h"
#include <stdio.h>
#include <string.h>

/* 1 = llamada valida, 0 = el nombre existe pero ninguna firma tiene esa
 * aridad, -1 = el nombre no existe o los argumentos son invalidos.
 * Recorre TODO el catalogo: si hay dos firmas con el mismo nombre,
 * basta con que una coincida. */
int verificar_llamada(const Funcion *catalogo, int total, const char *nombre, int nargs) {
    if (catalogo == NULL || nombre == NULL || nargs < 0 || total < 0) return -1;

    int encontrada = 0;
    for (int i = 0; i < total; i++) {
        if (catalogo[i].nombre == NULL) continue;
        if (strcmp(catalogo[i].nombre, nombre) != 0) continue;
        encontrada = 1;
        if (catalogo[i].aridad == nargs) return 1;      /* nombre y aridad coinciden */
    }
    return encontrada ? 0 : -1;
}
