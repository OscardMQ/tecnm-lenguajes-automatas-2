# Bitácora de Requerimientos y Auditoría IA — Sesión 23
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Etiquetas y Saltos Incondicionales  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Generador de etiquetas e instrucciones goto  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión es casi igual a la 21, pero con etiquetas en lugar de temporales. `nueva_etiqueta` usa el valor del contador y después lo incrementa, así la primera es `L0`. Ya traía la lección de la 21: si `snprintf` no alcanza a escribir el nombre completo, dejo el buffer vacío y no gasto el número.
>
> Lo nuevo es que una etiqueta se usa de dos formas. `Lx:` marca el lugar a donde se llega y `GOTO Lx` es el salto que la usa. Las dos tienen que llevar exactamente el mismo nombre. Por eso la etiqueta se genera una sola vez y después se le pasa como texto a `emitir_flujo`.
>
> Pensando en eso, puse la misma revisión de `snprintf` en `emitir_flujo`. Aquí cortar es todavía peor que con un temporal. Si `GOTO L10` no cabe y se corta a `GOTO L1`, parece una instrucción válida, pero salta a otro lugar del programa. Prefiero dejar la cadena vacía, que es fácil de detectar.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion23.h`: `void nueva_etiqueta(char *dst, int cap, int *contador)` y `void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto)`. |
| **Rol (R)** | Ingeniero de compiladores que genera las etiquetas y los saltos del código intermedio. |
| **Especificación (E)** | Etiquetas y saltos incondicionales: `nueva_etiqueta` escribe `LN` con el valor actual del contador y después lo incrementa (la primera es `L0`); `emitir_flujo` escribe `GOTO Lx` si `es_salto` es distinto de cero y la marca `Lx:` si es cero; una etiqueta `NULL` da cadena vacía; nunca se escribe fuera de `cap`. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion23.h y no lo puedo cambiar:

    void nueva_etiqueta(char *dst, int cap, int *contador);
    void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto);

ROL: Actúa como ingeniero de compiladores que genera las etiquetas y los saltos del
código intermedio.

ESPECIFICACIÓN:
1. nueva_etiqueta escribe en dst el nombre "L<n>", con n igual al valor actual de
   *contador, y después incrementa el contador. La primera llamada con el contador
   en 0 produce "L0".
2. emitir_flujo con es_salto distinto de cero escribe "GOTO <etiqueta>"; con
   es_salto en cero escribe la marca de destino "<etiqueta>:".
3. Una etiqueta NULL produce la cadena vacía.
4. Las dos funciones escriben siempre en el buffer recibido, nunca más de cap - 1
   caracteres, sin reservar memoria ni usar variables globales.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 24 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion23.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Escribe "L<n>" con n = *contador y despues incrementa el contador.
 * Si la etiqueta no cabe completa, deja dst vacio y no consume el numero:
 * una etiqueta recortada podria repetirse. */
void nueva_etiqueta(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    dst[0] = '\0';
    if (contador == NULL || *contador < 0) {
        return;
    }

    int escritos = snprintf(dst, (size_t)cap, "L%d", *contador);
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
        return;
    }
    (*contador)++;
}

/* O(L), L = largo de la etiqueta. es_salto != 0 escribe "GOTO <etiqueta>";
 * es_salto == 0 escribe la marca "<etiqueta>:". Etiqueta NULL: cadena vacia. */
void emitir_flujo(char *dst, int cap, const char *etiqueta, int es_salto) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    if (etiqueta == NULL) {
        dst[0] = '\0';
        return;
    }
    if (es_salto) {
        snprintf(dst, (size_t)cap, "GOTO %s", etiqueta);
    } else {
        snprintf(dst, (size_t)cap, "%s:", etiqueta);
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Saltos adelante y atrás estructurados

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion23_c.c artesano_sesion23_c.c`:
```text
=== AUDITORIA SESION 23 - Etiquetas y saltos incondicionales ===
Caso                               esperado:              obtenido:              resultado
[ 1] nueva_etiqueta(L0)                 esperado: L0           obtenido: L0           OK
[ 2] nueva_etiqueta(L5)                 esperado: L5           obtenido: L5           OK
[ 3] emitir_flujo(GOTO L2)              esperado: GOTO L2      obtenido: GOTO L2      OK
[ 4] emitir_flujo(marca L2)             esperado: L2:          obtenido: L2:          OK
[ 5] emitir_flujo(NULL)                 esperado:              obtenido:              OK
[ 6] nueva_etiqueta(L10)                esperado: L10          obtenido: L10          OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion23.md
```

La celda con `ia_sesion23_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. En `nueva_etiqueta` hizo lo mismo que su versión de la sesión 21: si la etiqueta no cabe, deja el buffer vacío y no gasta el número. Además rechaza un contador negativo. Con el contador en -1, la mía escribe `L-1` y la de la IA deja la cadena vacía. El contrato no habla de contadores negativos, pero `L-1` no es un nombre de etiqueta válido, así que en eso la IA fue más cuidadosa.
>
> La diferencia importante está en `emitir_flujo`. Ahí la IA solo llama a `snprintf` y no revisa si el texto cupo. Hice una prueba extra con la etiqueta `L10` y un buffer de 8 bytes, y la IA escribió `GOTO L1`. Es una instrucción que se ve correcta, pero salta a `L1` en lugar de `L10`, y nadie lo notaría hasta que el programa hiciera algo raro. Con la marca pasó algo parecido: en un buffer de 4 escribió `L10`, sin los dos puntos. La mía deja la cadena vacía en los dos casos, porque revisa lo que regresa `snprintf`. Con 9 bytes `GOTO L10` ya cabe y las dos lo escriben completo. No cambié mi versión, porque esta vez la revisión ya la traía yo.
>
> También probé `es_salto` en 2 y en -1, y las dos escriben `GOTO`, porque el contrato dice "distinto de cero" y no "igual a uno". Lo que ninguna revisa es la etiqueta vacía: con `""` las dos escriben `GOTO ` y `:`, o sea, un salto sin destino. Habría que tratar la cadena vacía igual que `NULL`.

---
**Firma digital:** OscardMQ
