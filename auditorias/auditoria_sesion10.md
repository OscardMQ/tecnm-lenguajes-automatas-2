# Bitácora de Requerimientos y Auditoría IA — Sesión 10
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Hito 1: Mini-Evaluador de Expresiones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Integración AST y Tipado Estático  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> El analizador ya venía armado y solo tenía que llenar `primaria`, `termino` y `expr`. Lo que más me costó fue entender que la precedencia sale del orden en que se llaman. `expr` le pide sus operandos a `termino`, y `termino` se los pide a `primaria`. Por eso en `2+3*4` el `*` se resuelve antes que el `+`, sin que yo tenga que comparar operadores.
>
> Lo segundo fue la asociatividad. `5-10-2` tiene que calcularse como `(5-10)-2`. Si lo hago con recursión del lado derecho sale `5-(10-2)`, que es otro número. Con un ciclo que va acumulando en `v`, la operación anterior siempre queda del lado izquierdo.
>
> Lo tercero fue no perder el primer error. Si hay una división entre cero, el código tiene que ser 1 aunque después venga basura. Por eso cada función revisa `p->error` antes de seguir y se detiene en cuanto deja de ser 0. Y el divisor lo reviso antes de dividir, no después.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion10.h`: `long evaluar_expresion(const char *fuente, int *error)`. El `Parser`, `saltar()` y `evaluar_expresion()` ya vienen armados; solo se completan `primaria`, `termino` y `expr`. |
| **Rol (R)** | Ingeniero de compiladores que escribe el frontend de un evaluador de expresiones. |
| **Especificación (E)** | Contrato formal de Frontend Semántico: `expr := termino { (+|-) termino }`, `termino := primaria { (*|/) primaria }`, `primaria := NUMERO | ( expr )`; precedencia y asociatividad a la izquierda; espacios ignorados; `*error` = 1 en división entre cero y 2 en sintaxis inválida; se conserva el primer error. |
| **Output (O)** | El archivo completo con las tres funciones, sin `main()` y sin cambiar `evaluar_expresion`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion10.h y no lo puedo cambiar:

    long evaluar_expresion(const char *fuente, int *error);

El analizador ya viene armado: typedef struct { const char *s; int error; } Parser;
una función saltar(Parser *p) que brinca espacios, y evaluar_expresion(), que llama
a expr(), salta espacios y marca error 2 si sobra texto. Solo debo completar:

    long primaria(Parser *p);
    long termino(Parser *p);
    long expr(Parser *p);

ROL: Actúa como ingeniero de compiladores que escribe el frontend de un evaluador
de expresiones con un parser descendente recursivo.

ESPECIFICACIÓN:
1. Gramática:
     expr     := termino { ("+" | "-") termino }
     termino  := primaria { ("*" | "/") primaria }
     primaria := NUMERO | "(" expr ")"
2. Solo enteros sin signo y los operadores + - * /. No hay variables ni menos unario.
3. Respeta la precedencia y la asociatividad a la izquierda: 5-10-2 es (5-10)-2.
4. Los espacios entre tokens se ignoran.
5. p->error = 1 si hay una división entre cero; p->error = 2 si la sintaxis es
   inválida (paréntesis sin cerrar, operador sin operando, carácter desconocido).
6. Se conserva el primer error que aparezca. El divisor se revisa ANTES de dividir.

OUTPUT: Devuelve el archivo completo (lo que ya venía armado más las tres funciones),
sin main(), seguido de la complejidad.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion10.h"
#include <ctype.h>
#include <limits.h>
#include <stddef.h>

/* Gramatica (descendente recursivo, LL(1)):
 *   expr     := termino  { ("+" | "-") termino }
 *   termino  := primaria { ("*" | "/") primaria }
 *   primaria := NUMERO | "(" expr ")"
 * error: 0 = bien, 1 = division entre cero, 2 = sintaxis invalida.
 * Solo se conserva el primer error. */
typedef struct {
    const char *s;
    int error;
} Parser;

enum { ERR_NINGUNO = 0, ERR_DIV_CERO = 1, ERR_SINTAXIS = 2 };

static void saltar(Parser *p) {
    while (*p->s == ' ') {
        p->s++;
    }
}

static void fallar(Parser *p, int codigo) {
    if (p->error == ERR_NINGUNO) {
        p->error = codigo;
    }
}

/* Devuelve 1 y consume c si es el siguiente caracter no blanco. */
static int aceptar(Parser *p, char c) {
    saltar(p);
    if (*p->s == c) {
        p->s++;
        return 1;
    }
    return 0;
}

long expr(Parser *p);

/* O(d), d = digitos. Un literal que no cabe en long se trata como sintaxis invalida. */
long primaria(Parser *p) {
    if (p->error != ERR_NINGUNO) {
        return 0;
    }
    if (aceptar(p, '(')) {
        long v = expr(p);
        if (!aceptar(p, ')')) {
            fallar(p, ERR_SINTAXIS);
            return 0;
        }
        return v;
    }

    saltar(p);
    if (!isdigit((unsigned char)*p->s)) {
        fallar(p, ERR_SINTAXIS);
        return 0;
    }

    long v = 0;
    while (isdigit((unsigned char)*p->s)) {
        int d = *p->s - '0';
        if (v > (LONG_MAX - d) / 10) {
            fallar(p, ERR_SINTAXIS);
            return 0;
        }
        v = v * 10 + d;
        p->s++;
    }
    return v;
}

long termino(Parser *p) {
    long v = primaria(p);
    for (;;) {
        if (p->error != ERR_NINGUNO) {
            return 0;
        }
        if (aceptar(p, '*')) {
            v *= primaria(p);
        } else if (aceptar(p, '/')) {
            long d = primaria(p);
            if (p->error != ERR_NINGUNO) {
                return 0;
            }
            if (d == 0) {
                fallar(p, ERR_DIV_CERO);
                return 0;
            }
            v /= d;
        } else {
            return v;
        }
    }
}

long expr(Parser *p) {
    long v = termino(p);
    for (;;) {
        if (p->error != ERR_NINGUNO) {
            return 0;
        }
        if (aceptar(p, '+')) {
            v += termino(p);
        } else if (aceptar(p, '-')) {
            v -= termino(p);
        } else {
            return v;
        }
    }
}

/* O(n), n = longitud de la cadena: cada caracter se consume una sola vez. */
long evaluar_expresion(const char *fuente, int *error) {
    static int sin_error;
    if (error == NULL) {
        error = &sin_error;
    }
    *error = ERR_NINGUNO;
    if (fuente == NULL) {
        *error = ERR_SINTAXIS;
        return 0;
    }

    Parser p = { fuente, ERR_NINGUNO };
    long v = expr(&p);
    saltar(&p);
    if (p.error == ERR_NINGUNO && *p.s != '\0') {
        p.error = ERR_SINTAXIS;
    }
    *error = p.error;
    return (p.error != ERR_NINGUNO) ? 0 : v;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Precedencia, paréntesis, división entre cero y sintaxis inválida

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion10_c.c artesano_sesion10_c.c`:
```text
=== AUDITORIA SESION 10 - Mini-evaluador de expresiones (Hito 1) ===
Caso                               esperado:              obtenido:              resultado
[ 1] "2+3*4"                            esperado: 14           obtenido: 14           OK
[ 2] "(2+3)*4"                          esperado: 20           obtenido: 20           OK
[ 3] "10/0" -> error 1                  esperado: 1            obtenido: 1            OK
[ 4] "2+" -> error 2                    esperado: 2            obtenido: 2            OK
[ 5] "  7 "                             esperado: 7            obtenido: 7            OK
[ 6] "5-10"                             esperado: -5           obtenido: -5           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion10.md
```

La celda con `ia_sesion10_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Cambió la forma pero no la idea: hizo una función `aceptar()` que salta espacios y consume un carácter si coincide, y otra `fallar()` que solo guarda el primer error. Con eso `termino` y `expr` le quedaron más cortas que las mías.
>
> Lo que sí agregó y yo no tenía es el desbordamiento del literal. Hice una prueba extra con `99999999999999999999`, que no cabe en un `long`. Mi versión armó el número dígito por dígito, se desbordó y regresó un valor sin sentido con error 0, como si todo estuviera bien. En C eso además es comportamiento indefinido, porque es un desbordamiento con signo. La IA revisa antes de cada dígito que `v * 10 + d` todavía quepa y, si no, marca error 2. Copié esa condición a mi versión y ahora las dos dan error 2.
>
> En la misma prueba confirmé que se conserva el primer error: `10/0+` da error 1 y no 2, aunque después del `+` no venga nada. `2*(3+4` y `2 3` dan error 2 en las dos versiones.
>
> Lo que ninguna de las dos revisa es el desbordamiento en las operaciones: `9999999999*9999999999` no cabe en un `long` de 64 bits y el resultado se calcula igual. El contrato no tiene un código para eso, así que lo dejo anotado. Otro detalle viene del esqueleto: `saltar()` solo brinca espacios, así que con un tabulador antes del número (`\t7`) las dos versiones dan error 2. El contrato dice "espacios" y no lo cambié, pero en Lenguajito el lexer sí ignora tabuladores y saltos de línea.

---
**Firma digital:** OscardMQ
