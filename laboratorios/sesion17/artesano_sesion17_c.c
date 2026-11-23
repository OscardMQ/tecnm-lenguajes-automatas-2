/* artesano_sesion17_c.c - Fase 1: El Artesano (sin IA)
 * Validación de retornos y rutas.
 *
 * Implementa lo que declara api_sesion17.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion17.h"
#include <stdio.h>
#include <string.h>

/* Rango de cada tipo: entre mayor, mas ancho. void es 0 y no se mezcla con nada.
 * -1 si el tipo no lo conocemos. */
static int rango(const char *tipo) {
    if (strcmp(tipo, "void") == 0)   return 0;
    if (strcmp(tipo, "char") == 0)   return 1;
    if (strcmp(tipo, "short") == 0)  return 2;
    if (strcmp(tipo, "int") == 0)    return 3;
    if (strcmp(tipo, "long") == 0)   return 4;
    if (strcmp(tipo, "float") == 0)  return 5;
    if (strcmp(tipo, "double") == 0) return 6;
    return -1;
}

/* 0 = bien, 1 = tipo incompatible, 2 = ruta sin retorno, -1 = datos incoherentes. */
int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion, int rutas_totales, int rutas_con_return) {
    if (tipo_funcion == NULL || tipo_expresion == NULL) return -1;
    if (rutas_totales <= 0 || rutas_con_return < 0 || rutas_con_return > rutas_totales) return -1;

    int destino = rango(tipo_funcion);
    int origen = rango(tipo_expresion);
    if (destino < 0 || origen < 0) return -1;           /* tipo que no existe */

    if (destino == 0) {                                 /* funcion void */
        return (origen == 0) ? 0 : 1;                   /* devolver un valor es error; sin return esta bien */
    }
    if (origen == 0 || destino < origen) return 1;      /* falta el valor, o el destino es mas angosto */
    if (rutas_con_return < rutas_totales) return 2;     /* una ruta llega al final sin return */
    return 0;
}
