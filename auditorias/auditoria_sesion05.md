# Bitácora de Requerimientos y Auditoría IA — Sesión 05
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Recorridos del AST y Notación Polaca  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Impresión en Postorden / RPN  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> La parte difícil no fue el recorrido, que es izquierdo, derecho y raíz. Fue escribir en el buffer sin `printf` y sin pasarme de `cap`. Tuve que llevar la posición actual en un `int` y pasarla por puntero a la recursión, para que cada llamada sepa dónde terminó la anterior. Si la paso por valor, cada nivel empieza a escribir en el mismo lugar y el texto se encima.
>
> Para respetar la capacidad hice una función `anexar` que copia carácter por carácter y se detiene en `cap - 1`. Así siempre queda lugar para el `'\0'`. Los números los convertí con `snprintf` en un arreglo de 16, porque un `int` cabe en 11 caracteres contando el signo.
>
> El separador tiene su detalle. Si pongo un espacio antes de cada token, la salida empieza con espacio y ya no coincide con `"42"`. Por eso el espacio solo va cuando ya hay algo escrito, o sea, cuando la posición es mayor que 0.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion05.h`: el mismo `ASTNode` de la sesión 04 y `void postorden(ASTNode *n, char *buf, int cap)`. |
| **Rol (R)** | Ingeniero de compiladores que genera código para una máquina de pila. |
| **Especificación (E)** | Invariantes de recorrido recursivo: postorden (izquierdo, derecho, raíz) con tokens separados por un espacio, sin espacio al inicio ni al final; números con signo; nunca más de `cap-1` caracteres más `'\0'`; árbol `NULL` = cadena vacía; `buf` en `NULL` o `cap <= 0` no se cae; nada de `printf`. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad en tiempo y en pila. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion05.h y no lo puedo cambiar:

    typedef enum { NODO_NUM, NODO_BINOP } TipoNodo;
    typedef struct ASTNode {
        TipoNodo tipo;
        int valor;
        char op;
        struct ASTNode *izq;
        struct ASTNode *der;
    } ASTNode;

    void postorden(ASTNode *n, char *buf, int cap);

ROL: Actúa como ingeniero de compiladores que genera código para una máquina de pila.

ESPECIFICACIÓN: Implementa postorden() con la firma exacta.
1. Escribe en buf el recorrido postorden del árbol (izquierdo, derecho, raíz): en
   notación polaca inversa, 3+(5*2) se escribe "3 5 2 * +".
2. Los tokens van separados por un solo espacio, sin espacio al inicio ni al final.
   Los números pueden ser negativos.
3. No uses printf: todo se escribe en buf.
4. Nunca escribas más de cap-1 caracteres más el terminador. Si no cabe, corta.
5. Un árbol NULL produce la cadena vacía. Con buf NULL o cap <= 0, no hagas nada.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad en tiempo y en pila.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion05.h"
#include <stdio.h>
#include <stddef.h>

/* Estado de escritura: el buffer, su capacidad y cuantos caracteres van. */
typedef struct {
    char *buf;
    size_t cap;
    size_t len;
} Salida;

/* O(1) por token. snprintf devuelve lo que habria escrito; si eso no cabe,
 * len se fija en cap - 1 y las escrituras siguientes ya no hacen nada. */
static void emitir(Salida *s, const char *sep, ASTNode *n) {
    if (s->len >= s->cap - 1) {
        return;
    }

    size_t libre = s->cap - s->len;
    int escritos;
    if (n->tipo == NODO_NUM) {
        escritos = snprintf(s->buf + s->len, libre, "%s%d", sep, n->valor);
    } else {
        escritos = snprintf(s->buf + s->len, libre, "%s%c", sep, n->op);
    }

    if (escritos < 0) {
        return;
    }
    if ((size_t)escritos >= libre) {
        s->len = s->cap - 1;
    } else {
        s->len += (size_t)escritos;
    }
}

/* O(n) en nodos, O(h) de pila. */
static void visitar(Salida *s, ASTNode *n) {
    if (n == NULL) {
        return;
    }
    if (n->tipo == NODO_BINOP) {
        visitar(s, n->izq);
        visitar(s, n->der);
    }
    emitir(s, s->len == 0 ? "" : " ", n);
}

void postorden(ASTNode *n, char *buf, int cap) {
    if (buf == NULL || cap <= 0) {
        return;
    }

    buf[0] = '\0';
    Salida s = { buf, (size_t)cap, 0 };
    visitar(&s, n);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Validación contra desbordamiento de pila

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion05_c.c artesano_sesion05_c.c`:
```text
=== AUDITORIA SESION 05 - Recorrido postorden y RPN ===
Caso                               esperado:              obtenido:              resultado
[ 1] hoja 42                            esperado: 42           obtenido: 42           OK
[ 2] 3+(5*2)                            esperado: 3 5 2 * +    obtenido: 3 5 2 * +    OK
[ 3] arbol NULL                         esperado:              obtenido:              OK
[ 4] (1+2)*(3+4)
     esperado: 1 2 + 3 4 + *
     obtenido: 1 2 + 3 4 + *
     -> OK
[ 5] 8-3                                esperado: 8 3 -        obtenido: 8 3 -        OK

RESULTADO: 5/5 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion05.md
```

La celda con `ia_sesion05_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los cinco casos. En lugar de copiar carácter por carácter, usó `snprintf` directo sobre lo que queda del buffer, con un `struct` que guarda `buf`, `cap` y `len`. Lo interesante es cómo trató el valor de retorno. `snprintf` regresa lo que *hubiera* escrito, no lo que escribió. Si se suma ese número sin revisar, `len` se pasa de `cap` y la siguiente escritura cae fuera del buffer. La IA lo cuidó: si no cupo, fija `len` en `cap - 1` y ya no escribe más.
>
> Hice pruebas extra con buffers chicos. Con `cap = 6`, el árbol `3+(5*2)` dio `"3 5 2"` en las dos versiones, y con `cap = 1` dio la cadena vacía. No hubo desbordamiento, pero la RPN quedó cortada y no hay forma de saberlo, porque la función es `void`. Si una máquina de pila ejecutara `"3 5 2"`, terminaría con tres valores en la pila y sin operar. Para el proyecto convendría regresar el número de caracteres necesarios, como hace `snprintf`, para que el que llama detecte que el buffer no alcanzó.
>
> También probé un número negativo, `-7 - 3`, y salió `"-7 3 -"`. Se puede leer sin ambigüedad porque el signo va pegado al número y el operador va solo, separado por espacios.

---
**Firma digital:** OscardMQ
