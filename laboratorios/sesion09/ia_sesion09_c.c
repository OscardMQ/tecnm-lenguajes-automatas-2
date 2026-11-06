/* ia_sesion09_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion09.h. No puede traer main().
 */
#include "api_sesion09.h"
#include <stdio.h>
#include <stddef.h>

/* O(L), L = largo del diagnostico. El mensaje entra como argumento de %s,
 * nunca como formato, asi que un '%' dentro del mensaje no se interpreta. */
void reportar(const char *archivo, int linea, int columna, const char *mensaje, char *salida, int cap) {
    if (salida == NULL || cap <= 0) {
        return;
    }

    const char *arch = (archivo != NULL) ? archivo : "<desconocido>";
    const char *msg = (mensaje != NULL) ? mensaje : "";

    int n = snprintf(salida, (size_t)cap, "%s:%d:%d: error: %s", arch, linea, columna, msg);
    if (n < 0) {
        salida[0] = '\0';
    }
}
