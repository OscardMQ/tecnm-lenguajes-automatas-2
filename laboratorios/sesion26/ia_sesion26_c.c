/* ia_sesion26_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion26.h. No puede traer main().
 */
#include "api_sesion26.h"
#include <stdio.h>
#include <string.h>

#define ANCHO 64

/* Un texto cuenta como presente solo si no es NULL ni la cadena vacia:
 * "" como nombre de funcion, argumento o resultado produciria TAC mal formado. */
static int presente(const char *s) {
    return s != NULL && s[0] != '\0';
}

/* O(L), L = largo total de los textos. Emite un PARAM por cada argumento
 * presente (primero a1, luego a2) y despues "res = CALL f, N" o "CALL f, N",
 * donde N es el numero de PARAM emitidos. Todo o nada: si una linea no cabe o
 * no hay espacio para todas, no escribe. Devuelve las lineas agregadas. */
int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion, const char *a1, const char *a2, const char *resultado) {
    if (lineas == NULL || n == NULL || !presente(funcion)) {
        return 0;
    }
    if (*n < 0 || max < 0 || *n > max) {
        return 0;
    }

    char tmp[3][ANCHO];
    int k = 0;
    const char *args[2] = { a1, a2 };
    for (int i = 0; i < 2; i++) {
        if (!presente(args[i])) {
            continue;
        }
        int escritos = snprintf(tmp[k], ANCHO, "PARAM %s", args[i]);
        if (escritos < 0 || escritos >= ANCHO) {
            return 0;
        }
        k++;
    }

    int params = k;
    int escritos_call = presente(resultado)
        ? snprintf(tmp[k], ANCHO, "%s = CALL %s, %d", resultado, funcion, params)
        : snprintf(tmp[k], ANCHO, "CALL %s, %d", funcion, params);
    if (escritos_call < 0 || escritos_call >= ANCHO) {
        return 0;
    }
    k++;

    if (k > max - *n) {
        return 0;
    }
    memcpy(lineas + *n, tmp, (size_t)k * ANCHO);
    *n += k;
    return k;
}

/* O(L). "RETURN <valor>" o "RETURN" si no hay valor. Si no cabe completo,
 * deja dst vacio: "RETURN _t10" recortado a "RETURN _t1" devolveria otro temporal. */
void emitir_return(char *dst, int cap, const char *valor) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    int escritos = presente(valor)
        ? snprintf(dst, (size_t)cap, "RETURN %s", valor)
        : snprintf(dst, (size_t)cap, "RETURN");
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
    }
}
