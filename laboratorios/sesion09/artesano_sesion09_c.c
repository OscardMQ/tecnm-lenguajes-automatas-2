/* artesano_sesion09_c.c - Fase 1: El Artesano (sin IA)
 * Diagnostico con linea y columna.
 *
 * Implementa lo que declara api_sesion09.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion09.h"
#include <stdio.h>
#include <string.h>

/* Formato de GCC: archivo:linea:columna: error: mensaje
 * snprintf corta en cap - 1 y siempre pone el '\0', asi que el buffer
 * del llamador nunca se desborda. */
void reportar(const char *archivo, int linea, int columna, const char *mensaje, char *salida, int cap) {
    if (salida == NULL || cap <= 0) return;     /* no hay donde escribir */
    if (archivo == NULL) archivo = "";          /* %s con NULL es comportamiento indefinido */
    if (mensaje == NULL) mensaje = "";

    snprintf(salida, (size_t)cap, "%s:%d:%d: error: %s", archivo, linea, columna, mensaje);
}
