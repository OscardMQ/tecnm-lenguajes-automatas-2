/* ia_sesion16_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion16.h. No puede traer main().
 */
#include "api_sesion16.h"
#include <stddef.h>
#include <string.h>

enum { LLAMADA_INVALIDA = -1, ARIDAD_INCORRECTA = 0, LLAMADA_VALIDA = 1 };

/* O(n), n = total de firmas en el catalogo. Busca por nombre exacto y
 * compara la aridad de la primera firma con ese nombre. */
int verificar_llamada(const Funcion *catalogo, int total, const char *nombre, int nargs) {
    if (catalogo == NULL || nombre == NULL || nargs < 0 || total <= 0) {
        return LLAMADA_INVALIDA;
    }

    for (int i = 0; i < total; i++) {
        const Funcion *f = &catalogo[i];
        if (f->nombre != NULL && strcmp(f->nombre, nombre) == 0) {
            return (f->aridad == nargs) ? LLAMADA_VALIDA : ARIDAD_INCORRECTA;
        }
    }
    return LLAMADA_INVALIDA;
}
