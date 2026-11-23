# Bitácora de Requerimientos y Auditoría IA — Sesión 19
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Estructuras (struct) y Miembros  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Offsets de miembros en estructuras  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión junta la 12 y la 18: cada campo del struct se alinea a su propio ancho antes de darle offset. Con `{char a; int b; short c;}`, `a` queda en 0, `b` no puede empezar en 1 y salta a 4, y `c` queda en 8. El struct termina en 10.
>
> Lo que no sabía era el padding final. El tamaño no es 10, es 12, porque se redondea al ancho del campo más grande (4, por el `int`). Si no, en un arreglo de estos structs el `int` del segundo elemento quedaría en el byte 14 y ya no estaría alineado.
>
> Para no repetir código hice una función `alinear` y usé el mismo recorrido en las dos funciones. `offset_campo` se detiene cuando encuentra el nombre; `tamano_struct` llega al final y aplica el redondeo.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion19.h`: `struct Campo { const char *nombre; const char *tipo; }`, `offset_campo()` y `tamano_struct()`. Arquitectura x86-64. |
| **Rol (R)** | Ingeniero de compiladores que calcula la disposición en memoria de los `struct`. |
| **Especificación (E)** | Estructuras y miembros: cada campo se alinea a su propio ancho antes de recibir offset; el tamaño total se redondea al múltiplo del campo más grande; un struct sin campos mide 0; campo inexistente, lista `NULL` o tipo desconocido devuelven -1. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
Compilo para x86-64. El contrato está en api_sesion19.h y no lo puedo cambiar:

    typedef struct Campo {
        const char *nombre;
        const char *tipo;
    } Campo;

    int offset_campo(const Campo *campos, int total, const char *nombre);
    int tamano_struct(const Campo *campos, int total);

ROL: Actúa como ingeniero de compiladores que calcula la disposición en memoria de
los struct.

ESPECIFICACIÓN:
1. Los campos se acomodan en el orden de la lista. Cada uno se alinea a su propio
   ancho antes de recibir su offset: char 1, short 2, int 4, float 4, double 8,
   puntero 8.
2. offset_campo devuelve el offset del campo con ese nombre.
3. tamano_struct devuelve el tamaño total, redondeado al múltiplo del ancho del
   campo más grande (padding final). Un struct sin campos mide 0.
4. Un campo inexistente, una lista NULL o un tipo desconocido devuelven -1.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion19.h"
#include <stddef.h>
#include <string.h>

/* Ancho = alineacion en x86-64 (LP64) para los tipos escalares. */
static int ancho_tipo(const char *tipo) {
    static const struct { const char *nombre; int ancho; } TIPOS[] = {
        { "char", 1 }, { "short", 2 }, { "int", 4 },
        { "float", 4 }, { "double", 8 }, { "puntero", 8 },
    };
    if (tipo == NULL) {
        return -1;
    }
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++) {
        if (strcmp(tipo, TIPOS[i].nombre) == 0) {
            return TIPOS[i].ancho;
        }
    }
    return -1;
}

/* O(n). Recorre la disposicion completa del struct. Si 'objetivo' no es
 * NULL, guarda en *offset_objetivo el offset de ese campo (o deja -1).
 * Devuelve el tamano total con padding final, o -1 si algun tipo es
 * desconocido. */
static int disponer(const Campo *campos, int total, const char *objetivo, int *offset_objetivo) {
    int cursor = 0;
    int alineacion_max = 1;

    for (int i = 0; i < total; i++) {
        int ancho = ancho_tipo(campos[i].tipo);
        if (ancho < 0) {
            return -1;
        }
        cursor = (cursor + ancho - 1) / ancho * ancho;
        if (objetivo != NULL && *offset_objetivo < 0 &&
            campos[i].nombre != NULL && strcmp(campos[i].nombre, objetivo) == 0) {
            *offset_objetivo = cursor;
        }
        cursor += ancho;
        if (ancho > alineacion_max) {
            alineacion_max = ancho;
        }
    }
    return (cursor + alineacion_max - 1) / alineacion_max * alineacion_max;
}

/* O(n). -1 si la lista o el nombre son NULL, si un tipo es desconocido
 * o si el campo no existe. */
int offset_campo(const Campo *campos, int total, const char *nombre) {
    if (campos == NULL || nombre == NULL || total < 0) {
        return -1;
    }
    int offset = -1;
    if (disponer(campos, total, nombre, &offset) < 0) {
        return -1;
    }
    return offset;
}

/* O(n). 0 para un struct sin campos; -1 si la lista es NULL o un tipo es
 * desconocido. */
int tamano_struct(const Campo *campos, int total) {
    if (campos == NULL || total < 0) {
        return -1;
    }
    if (total == 0) {
        return 0;
    }
    return disponer(campos, total, NULL, NULL);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Acceso a miembros inexistentes

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion19_c.c artesano_sesion19_c.c`:
```text
=== AUDITORIA SESION 19 - Offsets de miembros y tamano de struct ===
Caso                               esperado:              obtenido:              resultado
[ 1] struct char e int: b               esperado: 4            obtenido: 4            OK
[ 2] struct de tres campos: c           esperado: 8            obtenido: 8            OK
[ 3] tamano total con padding           esperado: 12           obtenido: 12           OK
[ 4] double seguido de char             esperado: 16           obtenido: 16           OK
[ 5] miembro inexistente                esperado: -1           obtenido: -1           OK
[ 6] struct sin campos                  esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion19.md
```

La celda con `ia_sesion19_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Hizo una sola función `disponer()` que recorre todo el struct y, de paso, guarda el offset del campo buscado; las dos funciones públicas solo la llaman. Además, esa función revisa los tipos de todos los campos antes de dar cualquier resultado.
>
> Ahí estaba lo que se me pasó. Probé el struct `{int a; bool b;}`, donde `bool` no es un tipo válido. Mi primera versión regresaba 0 como offset de `a`, porque encontraba el campo antes de llegar al tipo malo y ya no seguía. La IA regresó -1, que es lo que dice el contrato: si hay un tipo desconocido, el struct es inválido. Agregué a mi versión una revisión de todos los tipos al principio (`tipos_validos`), y ahora las dos regresan -1 en los dos casos.
>
> En la misma prueba vi algo que explica por qué importa el orden de los campos: `{char, double, char}` mide 24 bytes y `{double, char, char}` mide 16. Son los mismos campos, pero en el primer orden se pierden 7 bytes de relleno antes del `double` y otros 7 al final. En C el compilador no puede reordenar los campos, así que eso le toca al programador.
>
> Un caso que ninguna de las dos revisa son dos campos con el mismo nombre: con `{char x; int x;}` las dos regresan 0, el offset del primero. Un compilador de verdad lo rechazaría como miembro duplicado antes de llegar aquí.

---
**Firma digital:** OscardMQ
