/* artesano_sesion03_c.c - Fase 1: El Artesano (sin IA)
 * Archivos: escribir y contar lineas.
 *
 * Implementa lo que declara api_sesion03.h.
 * El harness compila este archivo junto con el suyo: no escribas main().
 */
#include "api_sesion03.h"
#include <stdio.h>
#include <string.h>

/* Abre en modo "w" (si existe lo vacia), escribe y cierra. 0 = bien, -1 = error. */
int escribir_texto(const char *ruta, const char *texto) {
    if (ruta == NULL || texto == NULL) return -1;

    FILE *f = fopen(ruta, "w");
    if (f == NULL) return -1;               /* ruta invalida o sin permiso */

    int ok = (fputs(texto, f) != EOF);
    if (fclose(f) != 0) ok = 0;             /* fclose tambien puede fallar al vaciar el buffer */
    return ok ? 0 : -1;
}

/* Cuenta los '\n' y suma una linea mas si el archivo no termina en salto. */
int contar_lineas(const char *ruta) {
    if (ruta == NULL) return -1;

    FILE *f = fopen(ruta, "r");
    if (f == NULL) return -1;               /* inexistente: -1, sin abortar */

    int lineas = 0;
    int c;                                  /* int y no char: EOF tiene que caber */
    int anterior = '\n';                    /* archivo vacio = 0 lineas */
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') lineas++;
        anterior = c;
    }
    if (anterior != '\n') lineas++;         /* ultima linea sin salto final */

    fclose(f);
    return lineas;
}
