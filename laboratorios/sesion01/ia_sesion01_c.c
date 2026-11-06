/* ia_sesion01_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion01.h. No puede traer main().
 */
#include "api_sesion01.h"
#include <stddef.h>

/* O(n) tiempo, O(1) memoria: dos punteros que avanzan uno hacia el otro
 * y se detienen al cruzarse en la mitad de la cadena. */
void invertir(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }

    char *izq = s;
    char *der = s;
    while (*(der + 1) != '\0') {    /* der queda sobre el ultimo caracter */
        der++;
    }

    while (izq < der) {
        char tmp = *izq;
        *izq = *der;
        *der = tmp;
        izq++;
        der--;
    }
}
