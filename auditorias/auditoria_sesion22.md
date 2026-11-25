# Bitácora de Requerimientos y Auditoría IA — Sesión 22
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Generación de TAC para Expresiones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Traductor de nodos AST a TAC  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión conecta el AST de la unidad 1 con el código de tres direcciones de la 21. La clave fue el orden del recorrido. Si escribo el operador antes de recorrer sus hijos, todavía no sé en qué temporal quedó cada lado. Por eso es postorden: primero el hijo izquierdo, luego el derecho y al final la línea del operador.
>
> Lo que más me costó fue decidir qué regresa la función recursiva. No basta con escribir la línea, porque el padre necesita saber dónde quedó el resultado de su hijo. Si el hijo es una hoja, el resultado es su propio nombre (`a`) y no se escribe nada. Si es un operador, es el temporal que se acaba de crear (`_t0`). Así, en `a + b * c`, el `+` recibe `a` y `_t0` y escribe `_t1 = a + _t0`.
>
> La precedencia sale sola, porque ya viene en la forma del árbol: el parser dejó la multiplicación más abajo. El traductor no necesita una tabla de precedencias.
>
> Para no pasarme de `max`, llevo la cuenta de líneas en un entero que paso por puntero. Si ya no cabe una línea, regreso NULL para que el padre tampoco escriba.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion22.h`: `struct Nodo { char tipo; char op; char valor[16]; struct Nodo *izq, *der; }` e `int generar_tac(const Nodo *raiz, char lineas[][64], int max)`. Los temporales salen de `nuevo_temporal()`, con un pool de 64 nombres que ya viene en el esqueleto. |
| **Rol (R)** | Ingeniero de compiladores que genera código intermedio a partir del AST. |
| **Especificación (E)** | Generación de TAC para expresiones: recorrido en postorden y una línea `_tN = izq op der` por operador; una hoja no escribe nada y aporta su propio nombre; los temporales salen solo de `nuevo_temporal`; nunca más de `max` líneas ni más de 63 caracteres por línea; devuelve cuántas líneas escribió. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion22.h y no lo puedo cambiar. En un nodo, tipo vale
'H' si es una hoja y 'B' si es un operador binario:

    typedef struct Nodo {
        char tipo;
        char op;
        char valor[16];
        struct Nodo *izq;
        struct Nodo *der;
    } Nodo;

    int generar_tac(const Nodo *raiz, char lineas[][64], int max);

Los temporales ya están resueltos y los tengo que usar tal cual:

    typedef struct {
        char nombre[64][16];
        int usados;
    } Pool;

    const char *nuevo_temporal(Pool *p, int *contador) {
        if (p->usados >= 64) return "_t?";
        snprintf(p->nombre[p->usados], sizeof p->nombre[0], "_t%d", (*contador)++);
        return p->nombre[p->usados++];
    }

ROL: Actúa como ingeniero de compiladores que genera código intermedio a partir
del AST.

ESPECIFICACIÓN:
1. Recorre el árbol en postorden: primero el hijo izquierdo, después el derecho y
   al final el operador.
2. Por cada operador escribe en lineas[n] la línea "_tN = izq op der", donde izq y
   der son el nombre de la hoja o el temporal donde quedó cada hijo. Por ejemplo,
   a + b * c produce "_t0 = b * c" y luego "_t1 = a + _t0".
3. Una hoja no escribe nada. Si la raíz es una hoja, la función devuelve 0.
4. Cada operador usa un temporal nuevo de nuevo_temporal; no inventes nombres.
5. Devuelve cuántas líneas escribió. Nunca escribe más de max líneas ni más de 63
   caracteres por línea.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 24 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion22.h"
#include <stdio.h>
#include <string.h>

/* Pool de temporales que da el contrato; se usa sin cambios. */
typedef struct {
    char nombre[64][16];
    int usados;
} Pool;

const char *nuevo_temporal(Pool *p, int *contador) {
    if (p->usados >= 64) return "_t?";
    snprintf(p->nombre[p->usados], sizeof p->nombre[0], "_t%d", (*contador)++);
    return p->nombre[p->usados++];
}

typedef struct {
    char (*lineas)[64];
    int max;
    int emitidas;
    int error;
    Pool pool;
    int contador;
} Generador;

/* O(n), n = nodos del arbol: cada nodo se visita una vez, en postorden.
 * Devuelve el nombre que guarda el valor del subarbol (hoja o temporal)
 * o NULL si hubo un error; despues de un error ya no se emite nada. */
static const char *traducir(Generador *g, const Nodo *n) {
    if (g->error || n == NULL) {
        g->error = 1;
        return NULL;
    }
    if (n->tipo != 'B') {
        return n->valor;
    }

    const char *izq = traducir(g, n->izq);
    const char *der = traducir(g, n->der);
    if (g->error) {
        return NULL;
    }
    if (g->emitidas >= g->max) {
        g->error = 1;
        return NULL;
    }

    const char *t = nuevo_temporal(&g->pool, &g->contador);
    if (strcmp(t, "_t?") == 0) {
        /* El pool se agoto: repetir "_t?" romperia la unicidad de los temporales. */
        g->error = 1;
        return NULL;
    }
    snprintf(g->lineas[g->emitidas], 64, "%s = %s %c %s", t, izq, n->op, der);
    g->emitidas++;
    return t;
}

/* O(n). Escribe una linea TAC por operador y devuelve cuantas escribio. */
int generar_tac(const Nodo *raiz, char lineas[][64], int max) {
    if (raiz == NULL || lineas == NULL || max <= 0) {
        return 0;
    }
    Generador g;
    g.lineas = lineas;
    g.max = max;
    g.emitidas = 0;
    g.error = 0;
    g.pool.usados = 0;
    g.contador = 0;

    traducir(&g, raiz);
    return g.emitidas;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones anidadas con paréntesis

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion22_c.c artesano_sesion22_c.c`:
```text
=== AUDITORIA SESION 22 - Traductor de AST a TAC con precedencia ===
Caso                               esperado:              obtenido:              resultado
[ 1] a + b * c                          esperado: _t0 = b * c  obtenido: _t0 = b * c  OK
[ 2] a + b * c (2a linea)
     esperado: _t1 = a + _t0
     obtenido: _t1 = a + _t0
     -> OK
[ 3] (a + b) * c
     esperado: _t1 = _t0 * c
     obtenido: _t1 = _t0 * c
     -> OK
[ 4] x * (y - z) + w
     esperado: _t2 = _t1 + w
     obtenido: _t2 = _t1 + w
     -> OK
[ 5] hoja sin operaciones               esperado: 0            obtenido: 0            OK
[ 6] (a + b) * (c - d)
     esperado: _t2 = _t0 * _t1
     obtenido: _t2 = _t0 * _t1
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion22.md
```

La celda con `ia_sesion22_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos con el mismo recorrido en postorden que yo. La diferencia es que guardó todo el estado en un struct `Generador` (las líneas, el contador, el pool y una bandera de error) en lugar de pasar seis parámetros, y que revisa lo que regresa `nuevo_temporal`.
>
> Eso último fue lo que se me pasó. El pool del esqueleto solo tiene 64 nombres, y cuando se acaban, `nuevo_temporal` no falla: regresa `"_t?"`. Hice una prueba extra con la cadena `v0 + v1 + ... + v65`, que tiene 65 operadores, y le di espacio para 80 líneas. Mi primera versión escribió 65 líneas, y la última fue `_t? = _t63 + v65`. Como `nuevo_temporal` regresa `"_t?"` siempre que el pool está lleno, cualquier operador después de ese se llamaría igual, y dos temporales distintos con el mismo nombre rompen el TAC. La IA se detuvo en 64 líneas. Agregué a mi versión la misma revisión y ahora las dos escriben 64.
>
> En las demás pruebas casi coincidieron. Con `a + b * c` y `max` en 1, las dos escriben solo `_t0 = b * c` y regresan 1. El resultado queda a medias y quien llama no tiene forma de saberlo, porque 1 también es una respuesta válida. Convendría regresar -1 en ese caso, pero el contrato solo pide las líneas escritas. Con un operador sin hijo izquierdo (`NULL + (b * c)`), la IA no escribe nada y regresa 0, y la mía alcanza a escribir la línea del hijo derecho y regresa 1. Ninguna truena, y el contrato no dice qué hacer con un árbol mal formado.
>
> Por último probé el punto de falla que marca la presentación: invertir los operandos de un operador que no es conmutativo. El harness no revisa ninguna línea con resta ni con división, así que lo probé aparte con `a - b`, `a / b`, `(a - b) - c` y `a - (b - c)`. Las dos versiones respetan el orden. Por ejemplo, `a - (b - c)` da `_t0 = b - c` y luego `_t1 = a - _t0`, porque las dos escriben el lugar del hijo izquierdo a la izquierda del operador y el del derecho a la derecha.

---
**Firma digital:** OscardMQ
