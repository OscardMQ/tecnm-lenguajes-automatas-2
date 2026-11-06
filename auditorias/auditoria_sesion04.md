# Bitácora de Requerimientos y Auditoría IA — Sesión 04
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Anatomía de un Árbol Sintáctico (AST)  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Estructura de nodo binario (ASTNode)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo que más me costó fue entender que el árbol ya guarda la precedencia. Para `3+5*2` no tengo que decidir qué operador va primero: el `*` queda más abajo en el árbol, así que la recursión lo evalúa antes. Mi función solo tiene dos casos. El caso base es la hoja, que regresa su número. El caso recursivo evalúa los dos hijos y después aplica el operador.
>
> Otra cosa fue el orden de los hijos. En la resta, `izq - der` no es lo mismo que `der - izq`, y el harness lo revisa con `10-4`. Por eso `crear_binario` guarda los hijos en el orden en que llegan, sin acomodarlos.
>
> También llené todos los campos del nodo aunque no se usen. Una hoja tiene `op` en `'\0'` y los hijos en `NULL`. Si los dejo con la basura que trae `malloc`, cualquier recorrido que revise `izq` podría seguir un puntero inválido.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion04.h`: `enum TipoNodo { NODO_NUM, NODO_BINOP }` y `struct ASTNode { tipo, valor, op, izq, der }`, con `crear_numero`, `crear_binario` y `evaluar`. |
| **Rol (R)** | Ingeniero de compiladores especializado en árboles sintácticos abstractos. |
| **Especificación (E)** | Gramática `E -> E + T | T` con tipos de nodo: `crear_numero` y `crear_binario` reservan en el heap e inicializan todos los campos; `crear_binario` respeta el orden izquierdo/derecho; `evaluar` aplica `+ - *` después de evaluar ambos hijos; `evaluar(NULL)` devuelve -1 sin caerse. |
| **Output (O)** | Solo el código de las tres funciones, sin `main()`, con su complejidad en tiempo y en pila. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion04.h y no lo puedo cambiar:

    typedef enum { NODO_NUM, NODO_BINOP } TipoNodo;
    typedef struct ASTNode {
        TipoNodo tipo;
        int valor;
        char op;
        struct ASTNode *izq;
        struct ASTNode *der;
    } ASTNode;

    ASTNode *crear_numero(int v);
    ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der);
    int evaluar(ASTNode *n);

ROL: Actúa como ingeniero de compiladores especializado en árboles sintácticos
abstractos.

ESPECIFICACIÓN: Implementa las tres funciones con las firmas exactas.
1. Cada nodo se reserva en el heap; verifica el retorno del allocator.
2. El campo tipo distingue hoja de operación. Inicializa todos los campos, aunque el
   tipo de nodo no los use.
3. crear_binario respeta el orden: izq es el operando izquierdo y der el derecho.
4. evaluar es recursiva: una hoja devuelve su valor y un nodo binario aplica su
   operador después de evaluar ambos hijos. Solo se soportan +, - y *.
5. evaluar(NULL) devuelve -1 y no debe caerse.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función en tiempo y en pila.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion04.h"
#include <stdlib.h>

/* O(1): reserva un nodo con todos los campos en cero/NULL. */
static ASTNode *nodo_nuevo(TipoNodo tipo) {
    ASTNode *n = (ASTNode *)calloc(1, sizeof(ASTNode));
    if (n != NULL) {
        n->tipo = tipo;
    }
    return n;
}

/* O(1). */
ASTNode *crear_numero(int v) {
    ASTNode *n = nodo_nuevo(NODO_NUM);
    if (n != NULL) {
        n->valor = v;
    }
    return n;
}

/* O(1). Conserva el orden: izq es el operando izquierdo. */
ASTNode *crear_binario(char op, ASTNode *izq, ASTNode *der) {
    ASTNode *n = nodo_nuevo(NODO_BINOP);
    if (n != NULL) {
        n->op = op;
        n->izq = izq;
        n->der = der;
    }
    return n;
}

/* O(n) en nodos, O(h) de pila por la recursion (h = altura del arbol).
 * Devuelve -1 si el nodo es NULL o el operador no es + - *. */
int evaluar(ASTNode *n) {
    if (n == NULL) {
        return -1;
    }
    if (n->tipo == NODO_NUM) {
        return n->valor;
    }

    int izq = evaluar(n->izq);
    int der = evaluar(n->der);
    switch (n->op) {
    case '+':
        return izq + der;
    case '-':
        return izq - der;
    case '*':
        return izq * der;
    default:
        return -1;
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Árbol vacío y evaluación de nodo raíz

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion04_c.c artesano_sesion04_c.c`:
```text
=== AUDITORIA SESION 04 - Nodo binario de AST ===
Caso                               esperado:              obtenido:              resultado
[ 1] evaluar(nodo 42)                   esperado: 42           obtenido: 42           OK
[ 2] evaluar(3+5*2)                     esperado: 13           obtenido: 13           OK
[ 3] hijo izquierdo de (* 5 2)          esperado: 5            obtenido: 5            OK
[ 4] operador guardado                  esperado: *            obtenido: *            OK
[ 5] evaluar(NULL)                      esperado: -1           obtenido: -1           OK
[ 6] evaluar(10-4)                      esperado: 6            obtenido: 6            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion04.md
```

La celda con `ia_sesion04_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

El contrato de esta sesión no incluye una función para liberar el árbol, así que los nodos que crea el harness no se liberan. Mi implementación no reserva nada que no regrese al que llama.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Usó `calloc` en una función auxiliar, así que todos los campos nacen en cero y no tuvo que llenarlos uno por uno. También indicó que la recursión usa O(h) de pila, donde h es la altura del árbol, algo en lo que yo no había pensado.
>
> El problema que encontré viene del contrato y lo tienen las dos versiones: -1 sirve al mismo tiempo como resultado y como error. Hice una prueba extra. `evaluar(2-3)` da -1, que es correcto, pero es el mismo valor que `evaluar(NULL)`. Peor aún, un nodo `+` con el hijo derecho en `NULL` dio 4, porque calcula `5 + (-1)` sin avisar. Y `8/2` dio -1 porque esta sesión no soporta `/`. Quien llama no tiene forma de distinguir esos casos de un resultado normal.
>
> No lo cambié porque el harness exige -1 para `NULL`. Pero es justo lo que resuelve la sesión 07, donde `evaluar` recibe un `int *error` aparte del valor. El otro pendiente es la memoria: el contrato no tiene función para liberar el árbol y el harness no libera los nodos. Para el Hito 1 hace falta un `liberar_ast` que recorra en postorden.

---
**Firma digital:** OscardMQ
