# Bitácora de Requerimientos y Auditoría IA — Sesión 13
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Manejo de Ámbitos (Scope Global/Local)  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Pila de tablas de símbolos (Stack Scopes)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue entender que `declarar` solo mira el ámbito actual. Si revisara también los padres, no se podría declarar `x` en un bloque cuando ya existe una `x` global, y el shadowing dejaría de funcionar.
>
> Lo segundo fue que `buscar` regresa el nombre del ámbito donde encontró la variable, no la variable. Subo con `padre` desde el actual hasta la raíz y regreso en el primero que la tiene. Por eso en el caso del shadowing gana `bloque` y no `global`.
>
> Lo tercero fue el orden de las revisiones en `declarar`. Primero reviso si el nombre ya está y después si el ámbito está lleno. Así, repetir un nombre en un ámbito lleno da 0 ("ya estaba"), que dice más que -1.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion13.h`: `struct Ambito { const char *nombre; const char *nombres[8]; int total; struct Ambito *padre; }`, `declarar()` y `buscar()`. |
| **Rol (R)** | Ingeniero de compiladores responsable de la resolución de nombres. |
| **Especificación (E)** | Ocultamiento de variables (shadowing): `declarar` inserta solo en el ámbito actual (1 insertó, 0 ya estaba en ese ámbito, -1 lleno, `NULL` o nombre vacío); `buscar` sube por `padre` y devuelve el nombre del ámbito más cercano que declara el identificador, o `NULL`. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion13.h y no lo puedo cambiar:

    #define AMBITO_MAX 8
    typedef struct Ambito {
        const char *nombre;
        const char *nombres[AMBITO_MAX];
        int total;
        struct Ambito *padre;
    } Ambito;

    int declarar(Ambito *ambito, const char *nombre);
    const char *buscar(const Ambito *ambito, const char *nombre);

ROL: Actúa como ingeniero de compiladores responsable de la resolución de nombres.

ESPECIFICACIÓN:
1. declarar inserta el nombre solo en el ámbito que recibe, sin consultar a los
   padres: el mismo nombre puede existir una vez en cada nivel (shadowing).
2. declarar devuelve 1 si insertó, 0 si el nombre ya estaba en ese ámbito y -1 si
   el ámbito está lleno o recibe NULL o un nombre vacío.
3. buscar empieza en el ámbito actual y sube por padre hasta la raíz.
4. buscar devuelve el nombre del ámbito más cercano que declara el identificador
   (no el identificador), o NULL si ninguno lo declara.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion13.h"
#include <stddef.h>
#include <string.h>

/* O(n), n = nombres del ambito (n <= AMBITO_MAX). */
static int contiene(const Ambito *ambito, const char *nombre) {
    for (int i = 0; i < ambito->total; i++) {
        if (ambito->nombres[i] != NULL && strcmp(ambito->nombres[i], nombre) == 0) {
            return 1;
        }
    }
    return 0;
}

/* O(n). Inserta solo en el ambito actual: un nombre puede repetirse en
 * niveles distintos (shadowing) pero no dos veces en el mismo nivel. */
int declarar(Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL || *nombre == '\0') {
        return -1;
    }
    if (contiene(ambito, nombre)) {
        return 0;
    }
    if (ambito->total < 0 || ambito->total >= AMBITO_MAX) {
        return -1;
    }
    ambito->nombres[ambito->total++] = nombre;
    return 1;
}

/* O(d * n), d = profundidad de la cadena. Devuelve el nombre del ambito
 * mas cercano que declara el identificador, o NULL si ninguno lo declara. */
const char *buscar(const Ambito *ambito, const char *nombre) {
    if (ambito == NULL || nombre == NULL) {
        return NULL;
    }
    for (const Ambito *actual = ambito; actual != NULL; actual = actual->padre) {
        if (contiene(actual, nombre)) {
            return actual->nombre;
        }
    }
    return NULL;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Búsqueda jerárquica de identificadores

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion13_c.c artesano_sesion13_c.c`:
```text
=== AUDITORIA SESION 13 - Pila de ambitos y busqueda jerarquica ===
Caso                               esperado:              obtenido:              resultado
[ 1] declarar(&g, "x")                  esperado: 1            obtenido: 1            OK
[ 2] buscar en el mismo ambito          esperado: global       obtenido: global       OK
[ 3] buscar subiendo al padre           esperado: global       obtenido: global       OK
[ 4] shadowing en el hijo               esperado: bloque       obtenido: bloque       OK
[ 5] tres niveles hasta la raiz         esperado: raiz         obtenido: raiz         OK
[ 6] nombre no declarado                esperado: (nulo)       obtenido: (nulo)       OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion13.md
```

La celda con `ia_sesion13_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos con la misma idea que yo: una función auxiliar que revisa un solo ámbito y un ciclo que sube por `padre`. Además revisó que `total` no fuera negativo, algo que no consideré porque el harness siempre lo inicia con `memset`.
>
> Lo que encontré lo tienen las dos versiones y viene del contrato: el arreglo guarda punteros (`const char *nombres[]`), no copias de los nombres. Hice una prueba extra: declaré `x` usando un arreglo `char buf[8]` y después escribí `y` en ese mismo arreglo, como haría un lexer que reutiliza su buffer para el siguiente token. Después de eso, `buscar("x")` regresó `NULL` y `buscar("y")` regresó `global`, aunque nunca se declaró `y`. La tabla apuntaba al buffer y el nombre cambió por debajo.
>
> Con las literales del harness no pasa, porque viven todo el programa. En el compilador del proyecto sí importa: los lexemas salen de un buffer, así que hay que guardar una copia (`malloc` + `strcpy`) y liberarla al cerrar el ámbito. No lo cambié aquí porque el contrato fija el tipo del arreglo.
>
> También probé el límite: el noveno nombre en un ámbito de 8 regresa -1, y repetir uno que ya estaba regresa 0 aunque el ámbito esté lleno.

---
**Firma digital:** OscardMQ
