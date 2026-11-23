/* ia_sesion18_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion18.h. No puede traer main().
 */
#include "api_sesion18.h"
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *nombre;
    int ancho;
} TipoBasico;

/* Modelo LP64 (x86-64 Linux). */
static const TipoBasico TIPOS[] = {
    { "char", 1 }, { "short", 2 }, { "int", 4 },
    { "float", 4 }, { "double", 8 }, { "puntero", 8 },
};

/* O(T), T = 6 tipos. Devuelve ancho * elementos, o -1 si el tipo es
 * desconocido o el numero de elementos es negativo. */
long bytes_arreglo(const char *tipo, int elementos) {
    if (tipo == NULL || elementos < 0) {
        return -1;
    }
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++) {
        if (strcmp(tipo, TIPOS[i].nombre) == 0) {
            return (long)TIPOS[i].ancho * (long)elementos;
        }
    }
    return -1;
}

/* O(1). 1 si 0 <= indice < elementos; 0 en cualquier otro caso,
 * incluido un arreglo de cero o menos elementos. */
int indice_en_rango(int indice, int elementos) {
    if (elementos <= 0) {
        return 0;
    }
    return (indice >= 0 && indice < elementos) ? 1 : 0;
}
