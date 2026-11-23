/* ia_sesion17_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion17.h. No puede traer main().
 */
#include "api_sesion17.h"
#include <stddef.h>
#include <string.h>

enum { RET_OK = 0, RET_TIPO = 1, RET_RUTA = 2, RET_INVALIDO = -1 };

/* Jerarquia de ensanchamiento: char < short < int < long < float < double. */
static const char *const JERARQUIA[] = { "char", "short", "int", "long", "float", "double" };

/* O(T). Posicion en la jerarquia, o -1 si no es un tipo numerico conocido. */
static int nivel(const char *tipo) {
    for (int i = 0; i < (int)(sizeof JERARQUIA / sizeof JERARQUIA[0]); i++) {
        if (strcmp(tipo, JERARQUIA[i]) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(T). Primero valida los datos, luego el tipo y al final las rutas. */
int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion, int rutas_totales, int rutas_con_return) {
    if (tipo_funcion == NULL || tipo_expresion == NULL) {
        return RET_INVALIDO;
    }
    if (rutas_totales <= 0 || rutas_con_return < 0 || rutas_con_return > rutas_totales) {
        return RET_INVALIDO;
    }

    int funcion_void = (strcmp(tipo_funcion, "void") == 0);
    int expresion_void = (strcmp(tipo_expresion, "void") == 0);

    if (funcion_void || expresion_void) {
        if (funcion_void != expresion_void) {
            return RET_TIPO;
        }
    } else {
        int destino = nivel(tipo_funcion);
        int origen = nivel(tipo_expresion);
        if (destino < 0 || origen < 0) {
            return RET_INVALIDO;
        }
        if (destino < origen) {
            return RET_TIPO;
        }
    }

    if (rutas_con_return < rutas_totales) {
        return RET_RUTA;
    }
    return RET_OK;
}
