# Bitácora de Requerimientos y Auditoría IA — Sesión 28
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Optimización: Constant Folding  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Plegado y simplificación de constantes  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> La función tiene tres salidas posibles: un literal calculado, una identidad resuelta o la instrucción original. Lo primero es decidir si un operando es un entero completo. Acepto un `-` opcional seguido solo de dígitos. No acepto espacios, `+`, `0x` ni texto sobrante, porque el contrato dice "nada de prefijos". Además, el número tiene que caber en un `int`, que es el tamaño en el programa destino.
>
> Para plegar, calculo en `long long` y solo uso el resultado si cabe en un `int`. Así `2147483647 + 1` o `-2147483648 / -1` se quedan como estaban, que es lo que dice la diapositiva sobre preservar la semántica: el optimizador no puede cambiar lo que pasaría en ejecución. Una división entre cero tampoco se pliega.
>
> El contrato pide `x + 0`, `x * 1` y `x * 0`. Agregué `0 + x`, `1 * x`, `0 * x`, `x - 0` y `x / 1`, porque son la misma regla con los operandos al revés. Lo que no toco es `0 - x`, que es `-x`, ni `0 / x`: si `x` vale 0 en ejecución, el programa original falla, y plegarlo a 0 escondería ese error.
>
> Si el resultado no cabe completo en `dst`, lo dejo vacío, igual que en las sesiones anteriores.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion28.h`: `void plegar(char *dst, int cap, const char *res, const char *op, const char *a, const char *b)`. |
| **Rol (R)** | Ingeniero de compiladores que escribe el optimizador local del código intermedio. |
| **Especificación (E)** | Plegado de constantes en una instrucción TAC `res = a op b` con `+ - * /`: dos literales enteros completos se sustituyen por `res = <resultado>`; la división entre cero no se pliega; identidades `x + 0` y `x * 1` a `res = x`, y `x * 0` a `res = 0`; si no se puede simplificar, la instrucción original; textos `NULL` o vacíos, o un resultado que no cabe en `cap`, dejan `dst` vacío. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion28.h y no lo puedo cambiar:

    void plegar(char *dst, int cap, const char *res, const char *op,
                const char *a, const char *b);

ROL: Actúa como ingeniero de compiladores que escribe el optimizador local del
código intermedio.

ESPECIFICACIÓN:
1. plegar recibe una instrucción TAC "res = a op b", con op igual a +, -, * o /,
   y escribe en dst la versión más simple que conserve el significado.
2. Si a y b son literales enteros completos (sin prefijos ni texto sobrante),
   escribe "res = <resultado>".
3. Si el divisor es cero, no pliega: deja la instrucción original y no provoca
   ninguna excepción.
4. Identidades: "x + 0" y "x * 1" se escriben como "res = x"; "x * 0" como
   "res = 0".
5. Si no se puede simplificar, escribe la instrucción original "res = a op b".
6. Si algún texto es NULL o vacío, o el resultado no cabe completo en cap, deja
   dst vacío.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 27 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion28.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Devuelve 1 si s es un literal entero completo y guarda su valor. */
static int es_entero(const char *s, long *valor) {
    if (s == NULL || *s == '\0') return 0;
    char *fin = NULL;
    errno = 0;
    long v = strtol(s, &fin, 10);
    if (errno == ERANGE || fin == s || *fin != '\0') return 0;  /* texto sobrante o fuera de rango */
    *valor = v;
    return 1;
}

/* Escribe en dst sin truncar: si no cabe completo, deja la cadena vacia. */
static void escribir(char *dst, int cap, int escritos) {
    if (escritos < 0 || escritos >= cap) dst[0] = '\0';
}

void plegar(char *dst, int cap, const char *res, const char *op, const char *a, const char *b) {
    if (dst == NULL || cap <= 0) return;
    dst[0] = '\0';
    if (res == NULL || op == NULL || a == NULL || b == NULL) return;
    if (*res == '\0' || *op == '\0' || *a == '\0' || *b == '\0') return;

    long va = 0, vb = 0;
    int a_const = es_entero(a, &va);
    int b_const = es_entero(b, &vb);
    char o = (op[1] == '\0') ? op[0] : '\0';

    /* Plegado de constantes: los dos operandos son literales. */
    if (a_const && b_const && o != '\0') {
        long r = 0;
        int plegable = 1;
        switch (o) {
            case '+': r = va + vb; break;
            case '-': r = va - vb; break;
            case '*': r = va * vb; break;
            case '/':
                if (vb == 0) plegable = 0;   /* division entre cero: no se pliega */
                else r = va / vb;
                break;
            default: plegable = 0; break;
        }
        if (plegable) {
            escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %ld", res, r));
            return;
        }
    }

    /* Identidades algebraicas. */
    if (o == '+' && b_const && vb == 0) {               /* x + 0 -> x */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s", res, a));
        return;
    }
    if (o == '*' && b_const && vb == 1) {               /* x * 1 -> x */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s", res, a));
        return;
    }
    if (o == '*' && b_const && vb == 0) {               /* x * 0 -> 0 */
        escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = 0", res));
        return;
    }

    /* Sin simplificacion posible: la instruccion original. */
    escribir(dst, cap, snprintf(dst, (size_t)cap, "%s = %s %s %s", res, a, op, b));
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Simplificación de expresiones constantes

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion28_c.c artesano_sesion28_c.c`:
```text
=== AUDITORIA SESION 28 - Plegado de constantes e identidades ===
Caso                               esperado:              obtenido:              resultado
[ 1] 3 + 5                              esperado: _t0 = 8      obtenido: _t0 = 8      OK
[ 2] 2 * 3                              esperado: _t0 = 6      obtenido: _t0 = 6      OK
[ 3] 7 - 10                             esperado: _t0 = -3     obtenido: _t0 = -3     OK
[ 4] x + 0                              esperado: _t0 = x      obtenido: _t0 = x      OK
[ 5] x + y (sin plegar)                 esperado: _t0 = x + y  obtenido: _t0 = x + y  OK
[ 6] 10 / 0 (no plegar)                 esperado: _t0 = 10 / 0 obtenido: _t0 = 10 / 0 OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion28.md
```

La celda con `ia_sesion28_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

Las casillas son de mi versión. La de la IA sí se cae con `-2147483648 / -1`, como explico en el reporte.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, y en varias pruebas extra coincidimos. Las dos dejaron como estaban `0x10 + 1`, `5abc + 1`, `3.5 * 2` y el operador `%`. Las dos dieron `-3` para `-7 / 2`, porque C trunca hacia cero. Y con `cap = 8`, `_t0 = x + y` no cabe y las dos dejaron la cadena vacía.
>
> La diferencia importante fue el desbordamiento. La IA calcula en `long`, y en Windows `long` es de 32 bits, igual que `int`. Con `2147483647 + 1` escribió `_t0 = -2147483648`: el propio optimizador se desbordó, que en C es comportamiento indefinido, y puso ese número como si fuera el resultado. Con `-2147483648 / -1` fue peor: el probe se cerró con el código `0xC0000095`, que es la excepción de desbordamiento de enteros de Windows, antes de imprimir el resultado. Es justo el punto de falla de la presentación, "provocar la caída del propio compilador". La IA sí protegió la división entre cero, pero no la otra división que se desborda. Mi versión calcula en `long long` y solo pliega si el resultado cabe en un `int`, así que esas dos instrucciones se quedan como estaban. Esto no lo puse en el prompt, y la IA no lo cuidó por su cuenta.
>
> El error que tenía yo estaba en el `-` solo. Mi primera revisión de enteros se saltaba el signo y luego revisaba los dígitos, pero si no había ninguno, el ciclo no corría y el valor quedaba en 0. Así, `- + 5` salía como `_t0 = 5`. La IA usa `strtol` y revisa que haya leído algo, así que lo rechazó. Agregué la revisión de que haya al menos un dígito.
>
> Hay dos cosas en las que la IA es más permisiva. `strtol` se salta los espacios del inicio y acepta un `+`, así que `" 3" + 5` salió como `_t0 = 8` y `+5 + 1` como `_t0 = 6`. El resultado no está mal, pero esconde un operando mal formado, y el contrato dice "nada de prefijos". La otra es que la IA solo aplica las tres identidades del prompt, así que `0 + x`, `1 * x`, `0 * x`, `x - 0` y `x / 1` se quedan igual. Ninguna de las dos versiones tocó `0 - x` ni `0 / x`, y eso está bien.

---
**Firma digital:** OscardMQ
