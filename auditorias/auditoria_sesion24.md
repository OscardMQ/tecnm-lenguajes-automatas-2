# Bitácora de Requerimientos y Auditoría IA — Sesión 24
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Control de Flujo: Sentencia if-else  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Saltos condicionales y evaluación booleana  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Aquí entendí por qué una condición compuesta no se evalúa completa. En `a && b`, si `a` es falso ya se sabe que todo es falso, y no tiene caso revisar `b`. Por eso cada operando lleva su propio salto: `IFFALSE a GOTO L9` y después `IFFALSE b GOTO L9`. Si los dos pasan, el programa sigue de frente y entra al cuerpo del if. Con `||` es al revés: basta con que uno sea verdadero, así que se usa `IFTRUE`.
>
> Lo que tuve que decidir fue qué hacer si no cabe todo. Si `max` es 1 y la condición es `a && b`, escribir solo el primer salto cambia el significado: el if quedaría como `if (a)`. Por eso reviso antes de escribir. O escribo todas las líneas o ninguna, y en ese caso regreso 0.
>
> Lo mismo con el largo de la línea. Después de la sesión 23 ya sé que un salto cortado puede ir a otra etiqueta. Antes de escribir mido cada línea con `snprintf(NULL, 0, ...)`, y si alguna pasa de 63 caracteres no escribo nada.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion24.h`: `int emitir_condicion(char lineas[][64], int max, const char *a, const char *b, char op, const char *etiqueta)`. |
| **Rol (R)** | Ingeniero de compiladores que traduce condiciones booleanas a saltos con cortocircuito. |
| **Especificación (E)** | Saltos condicionales con cortocircuito: con `&` cada operando lleva `IFFALSE x GOTO etiqueta`; con `\|` lleva `IFTRUE x GOTO etiqueta`; primero el salto de `a` y luego el de `b`; si `b` es `NULL` se emite un solo salto; nunca más de `max` líneas ni más de 63 caracteres; devuelve cuántas líneas escribió. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion24.h y no lo puedo cambiar:

    int emitir_condicion(char lineas[][64], int max, const char *a, const char *b,
                         char op, const char *etiqueta);

ROL: Actúa como ingeniero de compiladores que traduce condiciones booleanas a
saltos con cortocircuito.

ESPECIFICACIÓN:
1. Con op igual a '&' (AND), cada operando lleva la línea
   "IFFALSE <operando> GOTO <etiqueta>": si el operando es falso, se salta a la
   etiqueta de salida.
2. Con op igual a '|' (OR), cada operando lleva "IFTRUE <operando> GOTO <etiqueta>":
   si el operando es verdadero, se salta a la etiqueta.
3. Primero va el salto de a y después el de b. Si b es NULL se emite un solo
   salto; no se inventa una comparación.
4. Devuelve cuántas líneas escribió. Nunca escribe más de max líneas ni más de 63
   caracteres por línea.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 24 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion24.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Emite los saltos de una condicion compuesta con cortocircuito:
 *   op '&': IFFALSE <x> GOTO <etiqueta>  (un operando falso decide)
 *   op '|': IFTRUE  <x> GOTO <etiqueta>  (un operando verdadero decide)
 * Con b == NULL emite un solo salto. Devuelve el numero de lineas escritas;
 * 0 si algun argumento o el operador no son validos. */
int emitir_condicion(char lineas[][64], int max, const char *a, const char *b, char op, const char *etiqueta) {
    if (lineas == NULL || max <= 0 || a == NULL || etiqueta == NULL) {
        return 0;
    }

    const char *instr;
    if (op == '&') {
        instr = "IFFALSE";
    } else if (op == '|') {
        instr = "IFTRUE";
    } else {
        return 0;
    }

    int n = 0;
    snprintf(lineas[n++], 64, "%s %s GOTO %s", instr, a, etiqueta);
    if (b != NULL && n < max) {
        snprintf(lineas[n++], 64, "%s %s GOTO %s", instr, b, etiqueta);
    }
    return n;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Condiciones compuestas con AND y OR

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion24_c.c artesano_sesion24_c.c`:
```text
=== AUDITORIA SESION 24 - Saltos condicionales con cortocircuito ===
Caso                               esperado:              obtenido:              resultado
[ 1] a && b (primer salto)
     esperado: IFFALSE a GOTO L9
     obtenido: IFFALSE a GOTO L9
     -> OK
[ 2] a && b (segundo salto)
     esperado: IFFALSE b GOTO L9
     obtenido: IFFALSE b GOTO L9
     -> OK
[ 3] a && b (dos saltos)                esperado: 2            obtenido: 2            OK
[ 4] a || b (primer salto)
     esperado: IFTRUE a GOTO L9
     obtenido: IFTRUE a GOTO L9
     -> OK
[ 5] a || b (segundo salto)
     esperado: IFTRUE b GOTO L9
     obtenido: IFTRUE b GOTO L9
     -> OK
[ 6] condicion simple (b NULL)
     esperado: IFFALSE a GOTO L0
     obtenido: IFFALSE a GOTO L0
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion24.md
```

La celda con `ia_sesion24_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos y revisó algo que yo no: que el operador sea válido. Hice una prueba extra con `op = '+'`. Mi primera versión escribió `IFTRUE a GOTO L9` e `IFTRUE b GOTO L9` y regresó 2, porque yo decidía con `op == '&' ? "IFFALSE" : "IFTRUE"`, y todo lo que no era `&` lo trataba como OR. Un operador mal pasado se convertía en una condición OR sin ningún aviso. La IA regresó 0 con `'+'` y con `'a'`. Agregué esa revisión a mi versión y ahora las dos regresan 0.
>
> Pero en lo de no escribir a medias, la IA falló. Con `a && b` y `max` en 1, escribió solo `IFFALSE a GOTO L9` y regresó 1. Eso es justo lo que yo quería evitar: la condición se vuelve `if (a)`, y como 1 también es lo que regresa una condición simple, quien llama no puede saber que se perdió `b`. La mía regresa 0. Una llamada válida siempre escribe al menos una línea, así que el 0 sí se distingue.
>
> La otra prueba fue un operando de 47 caracteres con la etiqueta `L10`. La línea completa mide 64 caracteres y solo caben 63. La IA la cortó y la línea quedó terminando en `GOTO L1`, el mismo problema de la sesión 23: un salto a la etiqueta equivocada. La mía no escribió nada y regresó 0. Estas dos cosas no las cambié, porque la revisión ya estaba en mi versión.

---
**Firma digital:** OscardMQ
