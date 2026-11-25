/* ia_sesion24_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion24.h. No puede traer main().
 */
#include "api_sesion24.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Emite los saltos de una condicion compuesta con cortocircuito:
 *   op '&': IFFALSE <x> GOTO <etiqueta>  (un operando falso decide)
 *   op '|': IFTRUE  <x> GOTO <etiqueta>  (un operando verdadero decide)
 * Con b == NULL emite un solo salto. Devuelve el numero de lineas escritas;
 * 0 si algun argumento o el operador no son validos. */
int emitir_condicion(char lineas[][64], int max, const char *a, const char *b, char op, const char *etiqueta) {
    if (lineas == NULL || max <= 0 || a == NULL || etiqueta == NULL) {
        return 0;
    }

    const char *instr;
    if (op == '&') {
        instr = "IFFALSE";
    } else if (op == '|') {
        instr = "IFTRUE";
    } else {
        return 0;
    }

    int n = 0;
    snprintf(lineas[n++], 64, "%s %s GOTO %s", instr, a, etiqueta);
    if (b != NULL && n < max) {
        snprintf(lineas[n++], 64, "%s %s GOTO %s", instr, b, etiqueta);
    }
    return n;
}
