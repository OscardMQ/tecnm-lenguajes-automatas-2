/* ia_sesion03_c.c - Fase 2: El Cientifico
 *
 * Pega aqui el codigo que te devolvio la IA, con la MISMA firma
 * que declara api_sesion03.h. No puede traer main().
 */
#include "api_sesion03.h"
#include <stdio.h>
#include <string.h>

/* O(n), n = bytes del texto. Devuelve 0 si todo se escribio y el archivo
 * se cerro bien; -1 en cualquier otro caso. */
int escribir_texto(const char *ruta, const char *texto) {
    if (ruta == NULL || texto == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL) {
        return -1;
    }

    size_t largo = strlen(texto);
    size_t escritos = fwrite(texto, 1, largo, archivo);
    int error = (escritos != largo);

    if (fclose(archivo) != 0) {
        error = 1;
    }
    return error ? -1 : 0;
}

/* O(n), n = bytes del archivo. Una linea es una secuencia terminada en '\n'
 * o el ultimo fragmento no vacio sin '\n'. Devuelve -1 si no se puede abrir. */
int contar_lineas(const char *ruta) {
    if (ruta == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        return -1;
    }

    int lineas = 0;
    int en_linea = 0;
    int c;
    while ((c = fgetc(archivo)) != EOF) {
        if (c == '\n') {
            lineas++;
            en_linea = 0;
        } else {
            en_linea = 1;
        }
    }
    if (en_linea) {
        lineas++;
    }

    fclose(archivo);
    return lineas;
}
