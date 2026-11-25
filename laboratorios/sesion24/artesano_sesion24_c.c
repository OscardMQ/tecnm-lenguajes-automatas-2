/* artesano_sesion24_c.c - Fase 1: El Artesano (sin IA)
 * Saltos condicionales con cortocircuito.
 *
 * Implementa lo que declara api_sesion24.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion24.h"
#include <stdio.h>
#include <string.h>

/* Cortocircuito con una sola etiqueta de salida:
 *   &  ->  IFFALSE x GOTO salida   (si uno es falso, el otro ya no se revisa)
 *   |  ->  IFTRUE  x GOTO salida   (si uno es verdadero, el otro ya no se revisa)
 * Regresa cuantas lineas escribio: todas o ninguna. */
int emitir_condicion(char lineas[][64], int max, const char *a, const char *b, char op, const char *etiqueta) {
    if (lineas == NULL || a == NULL || etiqueta == NULL) return 0;
    if (op != '&' && op != '|') return 0;       /* agregado en la auditoria */

    int necesarias = (b != NULL) ? 2 : 1;
    if (max < necesarias) return 0;             /* medio salto cambia la condicion */

    const char *salto = (op == '&') ? "IFFALSE" : "IFTRUE";
    const char *operandos[2] = { a, b };

    /* Primero reviso que todo quepa; un salto cortado iria a otra etiqueta. */
    for (int i = 0; i < necesarias; i++) {
        int n = snprintf(NULL, 0, "%s %s GOTO %s", salto, operandos[i], etiqueta);
        if (n < 0 || n >= 64) return 0;
    }
    for (int i = 0; i < necesarias; i++)
        snprintf(lineas[i], 64, "%s %s GOTO %s", salto, operandos[i], etiqueta);
    return necesarias;
}
