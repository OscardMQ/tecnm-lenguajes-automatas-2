# Bitácora de Requerimientos y Auditoría IA — Sesión 26
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** TAC para Llamadas a Funciones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Instrucciones param, call y return  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Una llamada no es una sola instrucción. Primero se ponen los argumentos con `PARAM`, en orden, y después va el `CALL` con el nombre de la función y cuántos argumentos recibió. Si la función regresa algo, el `CALL` se asigna a un temporal: `_t0 = CALL f, 2`.
>
> El número del `CALL` lo calculo contando los `PARAM` que de verdad escribí, no con un 2 fijo. Así, si `a2` es `NULL`, sale `CALL f, 1` y los dos números siempre coinciden. Esa discrepancia es uno de los errores que menciona la presentación.
>
> En `f(g(x))` el orden importa: primero hay que emitir completa la llamada a `g`, que deja su resultado en `_t0`, y después la de `f` con `PARAM _t0`. Si se mezclaran los `PARAM` de las dos llamadas, la función interna se quedaría con los argumentos de la externa.
>
> El contrato no dice qué regresa `emitir_llamada`, así que decidí que regrese cuántas líneas agregó. También usé lo de las sesiones 24 y 25: todo o nada, revisar que `*n` no sea negativo y, en `emitir_return`, dejar la cadena vacía si no cabe, porque `RETURN _t10` cortado sería `RETURN _t1`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion26.h`: `int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion, const char *a1, const char *a2, const char *resultado)` y `void emitir_return(char *dst, int cap, const char *valor)`. |
| **Rol (R)** | Ingeniero de compiladores que genera el código intermedio de las llamadas a función. |
| **Especificación (E)** | TAC de llamadas y retornos: un `PARAM` por cada argumento que no sea `NULL`, en el orden `a1`, `a2`; después `res = CALL f, N` (o `CALL f, N` si no hay resultado), con N igual a los `PARAM` emitidos; `emitir_return` escribe `RETURN v` o solo `RETURN`; todo o nada si no cabe; `emitir_llamada` devuelve cuántas líneas agregó. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion26.h y no lo puedo cambiar:

    int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion,
                       const char *a1, const char *a2, const char *resultado);
    void emitir_return(char *dst, int cap, const char *valor);

ROL: Actúa como ingeniero de compiladores que genera el código intermedio de las
llamadas a función.

ESPECIFICACIÓN:
1. emitir_llamada agrega líneas a partir de lineas[*n] y avanza *n. Primero
   escribe "PARAM <arg>" por cada argumento que no sea NULL, en el orden a1, a2.
2. Después escribe "<resultado> = CALL <funcion>, <N>", donde N es el número de
   PARAM que se escribieron. Si resultado es NULL, escribe solo
   "CALL <funcion>, <N>".
3. emitir_llamada devuelve cuántas líneas agregó. Si no hay lugar para todas o
   alguna no cabe completa en 63 caracteres, no escribe nada y devuelve 0.
4. emitir_return escribe en dst "RETURN <valor>", o solo "RETURN" si valor es
   NULL. Si no cabe completo en cap, deja dst vacío.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 26 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion26.h"
#include <stdio.h>
#include <string.h>

#define ANCHO 64

/* Un texto cuenta como presente solo si no es NULL ni la cadena vacia:
 * "" como nombre de funcion, argumento o resultado produciria TAC mal formado. */
static int presente(const char *s) {
    return s != NULL && s[0] != '\0';
}

/* O(L), L = largo total de los textos. Emite un PARAM por cada argumento
 * presente (primero a1, luego a2) y despues "res = CALL f, N" o "CALL f, N",
 * donde N es el numero de PARAM emitidos. Todo o nada: si una linea no cabe o
 * no hay espacio para todas, no escribe. Devuelve las lineas agregadas. */
int emitir_llamada(char lineas[][64], int max, int *n, const char *funcion, const char *a1, const char *a2, const char *resultado) {
    if (lineas == NULL || n == NULL || !presente(funcion)) {
        return 0;
    }
    if (*n < 0 || max < 0 || *n > max) {
        return 0;
    }

    char tmp[3][ANCHO];
    int k = 0;
    const char *args[2] = { a1, a2 };
    for (int i = 0; i < 2; i++) {
        if (!presente(args[i])) {
            continue;
        }
        int escritos = snprintf(tmp[k], ANCHO, "PARAM %s", args[i]);
        if (escritos < 0 || escritos >= ANCHO) {
            return 0;
        }
        k++;
    }

    int params = k;
    int escritos_call = presente(resultado)
        ? snprintf(tmp[k], ANCHO, "%s = CALL %s, %d", resultado, funcion, params)
        : snprintf(tmp[k], ANCHO, "CALL %s, %d", funcion, params);
    if (escritos_call < 0 || escritos_call >= ANCHO) {
        return 0;
    }
    k++;

    if (k > max - *n) {
        return 0;
    }
    memcpy(lineas + *n, tmp, (size_t)k * ANCHO);
    *n += k;
    return k;
}

/* O(L). "RETURN <valor>" o "RETURN" si no hay valor. Si no cabe completo,
 * deja dst vacio: "RETURN _t10" recortado a "RETURN _t1" devolveria otro temporal. */
void emitir_return(char *dst, int cap, const char *valor) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    int escritos = presente(valor)
        ? snprintf(dst, (size_t)cap, "RETURN %s", valor)
        : snprintf(dst, (size_t)cap, "RETURN");
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Llamadas anidadas f(g(x))

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion26_c.c artesano_sesion26_c.c`:
```text
=== AUDITORIA SESION 26 - Llamadas, parametros y retorno ===
Caso                               esperado:              obtenido:              resultado
[ 1] PARAM x                            esperado: PARAM x      obtenido: PARAM x      OK
[ 2] PARAM y                            esperado: PARAM y      obtenido: PARAM y      OK
[ 3] CALL con dos argumentos
     esperado: _t0 = CALL f, 2
     obtenido: _t0 = CALL f, 2
     -> OK
[ 4] llamada anidada f(g(x))
     esperado: _t1 = CALL f, 1
     obtenido: _t1 = CALL f, 1
     -> OK
[ 5] CALL sin argumentos
     esperado: _t0 = CALL p, 0
     obtenido: _t0 = CALL p, 0
     -> OK
[ 6] RETURN con valor                   esperado: RETURN _t0   obtenido: RETURN _t0   OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion26.md
```

La celda con `ia_sesion26_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, y en lo que pedí en el prompt coincidimos. Probé `f(g(x), h(y))` llamando tres veces a la función, y las dos escribieron la misma secuencia de siete líneas: `PARAM x`, `_t0 = CALL g, 1`, `PARAM y`, `_t1 = CALL h, 1`, `PARAM _t0`, `PARAM _t1` y `_t2 = CALL f, 2`. Las llamadas internas quedan completas antes de los `PARAM` de `f`, y cada una deja su resultado en un temporal distinto. También coincidieron en la llamada sin resultado (`CALL imprimir, 1`), en `RETURN _t10` con un buffer de 10, que las dos dejaron vacío, y en `n = -1`, que las dos rechazaron sin tocar la memoria.
>
> La diferencia fue con las cadenas vacías. La IA trata `""` igual que `NULL` en el nombre de la función, en los argumentos, en el resultado y en el valor del `RETURN`; la mía solo revisaba `NULL`. Con la función `""`, mi primera versión escribió `_t0 = CALL , 1`. Con `a1 = ""` y resultado `""`, escribió un `PARAM` sin argumento y ` = CALL f, 2`. Y `emitir_return(b, 32, "")` dejó `RETURN` con un espacio al final. Todo eso es TAC mal formado que el backend no sabría leer, y es lo mismo que anoté en la sesión 23 sin corregirlo. Esta vez agregué una función `hay()` que trata `""` como `NULL`, y ahora las dos versiones dan lo mismo en todas las pruebas.
>
> Lo que ninguna revisa es un hueco en los argumentos. Con `a1 = NULL` y `a2 = "y"`, las dos escriben `PARAM y` y `_t0 = CALL f, 1`, así que `y` pasa a ser el primer argumento. Es lo que dice el contrato ("cada argumento no NULL genera un PARAM"), pero una llamada `f(?, y)` probablemente es un error de quien llama, y convendría rechazarla.

---
**Firma digital:** OscardMQ
