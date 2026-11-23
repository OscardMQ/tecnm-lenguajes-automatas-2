/* artesano_sesion18_c.c - Fase 1: El Artesano (sin IA)
 * Tamaño de arreglo y límites.
 *
 * Implementa lo que declara api_sesion18.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion18.h"
#include <stdio.h>
#include <string.h>

/* Ancho de cada tipo en x86-64; -1 si no existe. */
static int ancho_de(const char *tipo) {
    if (strcmp(tipo, "char") == 0)    return 1;
    if (strcmp(tipo, "short") == 0)   return 2;
    if (strcmp(tipo, "int") == 0)     return 4;
    if (strcmp(tipo, "float") == 0)   return 4;
    if (strcmp(tipo, "double") == 0)  return 8;
    if (strcmp(tipo, "puntero") == 0) return 8;
    return -1;
}

/* Bytes del arreglo = ancho del tipo por numero de elementos.
 * La multiplicacion se hace en long para que no se desborde en int. */
long bytes_arreglo(const char *tipo, int elementos) {
    if (tipo == NULL || elementos < 0) return -1;   /* cero elementos si se vale: pesa 0 */
    int ancho = ancho_de(tipo);
    if (ancho < 0) return -1;
    return (long)ancho * elementos;
}

/* Un indice es valido de 0 a elementos - 1; elementos ya se sale. */
int indice_en_rango(int indice, int elementos) {
    return (indice >= 0 && indice < elementos) ? 1 : 0;
}
