# Bitácora de Requerimientos y Auditoría IA — Sesión 25
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Control de Flujo: Bucles while y for  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Estructura de ciclo en instrucciones TAC  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión junta la 23 (etiquetas) y la 24 (saltos). Un `while` necesita dos etiquetas: una de inicio, a donde se regresa en cada vuelta, y una de fin, por donde se sale. Lo importante es el orden. La marca de inicio va **antes** de la condición, para que la condición se vuelva a evaluar en cada vuelta. Si la pusiera después, el ciclo nunca revisaría otra vez si debe terminar.
>
> El `for` no lo hice desde cero, lo desazucaré: `for (inicio; cond; post)` es lo mismo que `inicio; while (cond) { cuerpo; post; }`. Por eso el `inicio` va antes de la marca, porque solo se ejecuta una vez, y el `post` va justo antes del `GOTO` de regreso. Si el post se pierde, el iterador nunca avanza y el ciclo es infinito.
>
> Volví a usar el "todo o nada" de la sesión 24. Preparo todas las líneas en un arreglo temporal, reviso que cada una quepa completa y que haya lugar para todas, y solo entonces las copio. Las etiquetas también se gastan solo si el ciclo de verdad se escribió.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion25.h`: `void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas)` y `void emitir_for(char lineas[][64], int max, int *n, const char *inicio, const char *cond, const char *post, int *etiquetas)`. |
| **Rol (R)** | Ingeniero de compiladores que genera el código intermedio de los ciclos. |
| **Especificación (E)** | Esqueleto TAC de while y for: el while emite `L_inicio:`, `IFFALSE cond GOTO L_fin`, `GOTO L_inicio` y `L_fin:`; el for se desazucara como `inicio; while (cond) { post; }` y omite `inicio` o `post` si son `NULL`; cada ciclo consume dos etiquetas de `*etiquetas`; las líneas se agregan desde `lineas[*n]` y avanzan `*n`; todo o nada si falta espacio o una línea no cabe en 63 caracteres. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion25.h y no lo puedo cambiar:

    void emitir_while(char lineas[][64], int max, int *n, const char *cond,
                      int *etiquetas);
    void emitir_for(char lineas[][64], int max, int *n, const char *inicio,
                    const char *cond, const char *post, int *etiquetas);

ROL: Actúa como ingeniero de compiladores que genera el código intermedio de los
ciclos.

ESPECIFICACIÓN:
1. Las dos funciones agregan líneas a partir de lineas[*n] y avanzan *n. Las
   etiquetas se numeran con *etiquetas: cada ciclo usa dos (L<e> de inicio y
   L<e+1> de fin) y le suma 2 al contador.
2. emitir_while escribe cuatro líneas: "L<inicio>:", "IFFALSE <cond> GOTO L<fin>",
   "GOTO L<inicio>" y "L<fin>:".
3. emitir_for desazucara for (inicio; cond; post) como inicio; while (cond)
   { post; }: escribe <inicio>, la marca de inicio, el IFFALSE, <post>, el GOTO
   de regreso y la marca de fin. Si inicio o post son NULL, esa línea no se
   escribe.
4. Todo o nada: si no hay lugar para todas las líneas o alguna no cabe completa en
   63 caracteres, no escribe nada, no mueve *n y no gasta etiquetas.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 26 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion25.h"
#include <stdio.h>
#include <string.h>

#define ANCHO 64

/* Una linea solo sirve si snprintf la escribio completa. */
static int cabe(int escritos) {
    return escritos >= 0 && escritos < ANCHO;
}

/* La posicion de escritura debe estar dentro del arreglo (0 <= *n <= max)
 * y deben quedar al menos k lineas libres. */
static int hay_espacio(int max, const int *n, int k) {
    return *n >= 0 && max >= 0 && *n <= max && k <= max - *n;
}

/* O(L), L = largo de la condicion. Emite:
 *   L_inicio:
 *   IFFALSE <cond> GOTO L_fin
 *   GOTO L_inicio
 *   L_fin:
 * Todo o nada: si falta espacio o una linea no cabe, no escribe ni consume etiquetas. */
void emitir_while(char lineas[][64], int max, int *n, const char *cond, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL || *etiquetas < 0) {
        return;
    }
    if (!hay_espacio(max, n, 4)) {
        return;
    }

    int inicio = *etiquetas;
    int fin = *etiquetas + 1;
    char tmp[4][ANCHO];
    if (!cabe(snprintf(tmp[0], ANCHO, "L%d:", inicio)) ||
        !cabe(snprintf(tmp[1], ANCHO, "IFFALSE %s GOTO L%d", cond, fin)) ||
        !cabe(snprintf(tmp[2], ANCHO, "GOTO L%d", inicio)) ||
        !cabe(snprintf(tmp[3], ANCHO, "L%d:", fin))) {
        return;
    }

    memcpy(lineas + *n, tmp, sizeof tmp);
    *n += 4;
    *etiquetas += 2;
}

/* O(L). Desazucara for (inicio; cond; post) en:
 *   <inicio>                     (se omite si es NULL)
 *   L_inicio:
 *   IFFALSE <cond> GOTO L_fin
 *   <post>                       (se omite si es NULL; va antes del GOTO de regreso)
 *   GOTO L_inicio
 *   L_fin:
 * Misma regla de todo o nada que emitir_while. */
void emitir_for(char lineas[][64], int max, int *n, const char *inicio, const char *cond, const char *post, int *etiquetas) {
    if (lineas == NULL || n == NULL || cond == NULL || etiquetas == NULL || *etiquetas < 0) {
        return;
    }
    int total = 4 + (inicio != NULL) + (post != NULL);
    if (!hay_espacio(max, n, total)) {
        return;
    }

    int l_inicio = *etiquetas;
    int l_fin = *etiquetas + 1;
    char tmp[6][ANCHO];
    int k = 0;

    if (inicio != NULL && !cabe(snprintf(tmp[k++], ANCHO, "%s", inicio))) {
        return;
    }
    if (!cabe(snprintf(tmp[k++], ANCHO, "L%d:", l_inicio)) ||
        !cabe(snprintf(tmp[k++], ANCHO, "IFFALSE %s GOTO L%d", cond, l_fin))) {
        return;
    }
    if (post != NULL && !cabe(snprintf(tmp[k++], ANCHO, "%s", post))) {
        return;
    }
    if (!cabe(snprintf(tmp[k++], ANCHO, "GOTO L%d", l_inicio)) ||
        !cabe(snprintf(tmp[k++], ANCHO, "L%d:", l_fin))) {
        return;
    }

    memcpy(lineas + *n, tmp, (size_t)k * ANCHO);
    *n += k;
    *etiquetas += 2;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Bucles anidados y condiciones de parada

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion25_c.c artesano_sesion25_c.c`:
```text
=== AUDITORIA SESION 25 - Esqueleto TAC de while y for ===
Caso                               esperado:              obtenido:              resultado
[ 1] while: marca de inicio             esperado: L0:          obtenido: L0:          OK
[ 2] while: condicion
     esperado: IFFALSE a < b GOTO L1
     obtenido: IFFALSE a < b GOTO L1
     -> OK
[ 3] dos ciclos sin colision            esperado: L2:          obtenido: L2:          OK
[ 4] for: inicializacion                esperado: i = 0        obtenido: i = 0        OK
[ 5] for: actualizacion                 esperado: i = i + 1    obtenido: i = i + 1    OK
[ 6] for sin post (NULL)                esperado: 5            obtenido: 5            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion25.md
```

La celda con `ia_sesion25_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos con el mismo orden que yo. Esta vez puse en el prompt la regla de todo o nada que aprendí en las sesiones 23 y 24, y la IA la cumplió: con una condición de 50 caracteres, la línea `IFFALSE` mide 66 y ninguna de las dos versiones escribió nada ni gastó etiquetas. Lo mismo con `n = 6` y `max = 8`, donde el while ya no cabe.
>
> El harness no revisa a dónde apuntan los saltos: solo mira la marca de inicio, la condición, el init y el post. Así que imprimí el for completo: `i = 0`, `L0:`, `IFFALSE i < 10 GOTO L1`, `i = i + 1`, `GOTO L0` y `L1:`. Las dos versiones regresan a `L0`, que está después del init. Eso importa, porque si el `GOTO` regresara antes de `i = 0`, el iterador se reiniciaría en cada vuelta y el ciclo nunca terminaría. Es uno de los puntos de falla de la presentación: la etiqueta de inicio mal colocada.
>
> Lo que se me pasó fue el valor inicial de `*n`. Yo revisaba que no se pasara de `max`, pero no que fuera negativo. Lo probé con un arreglo más grande, pasándole a la función un apuntador a la fila 4 para que `lineas[-1]` cayera en memoria del propio probe. Con `n = -1`, mi primera versión escribió `L0:` en la fila anterior al arreglo que recibió, o sea, una escritura fuera de los límites. La IA revisa que `*n` no sea negativo antes de escribir y no tocó nada. Agregué esa revisión a mi versión y ahora las dos dejan todo intacto.
>
> Lo que ninguna maneja es una condición `NULL`. Las dos la rechazan y no escriben nada, pero en C `for (;;)` sin condición es válido: es un ciclo infinito que solo termina con un `break`. Para soportarlo habría que omitir el `IFFALSE`. El contrato solo habla de `inicio` y `post` en `NULL`, así que lo dejé así.

---
**Firma digital:** OscardMQ
