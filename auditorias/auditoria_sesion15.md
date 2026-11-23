# Bitácora de Requerimientos y Auditoría IA — Sesión 15
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Bloques Anidados y Ciclo de Vida  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Operaciones Push y Pop de ámbito  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo que más me costó fue el `pop` sobre la raíz. El ámbito global no lo crea `push_ambito`: en el harness es una variable local, en la pila. Si le hago `free`, el programa truena, porque `free` solo sirve para memoria que vino de `malloc`. Por eso, si el ámbito no tiene padre, regreso `NULL` sin liberar nada.
>
> Lo segundo fue el orden dentro de `pop`. Tengo que guardar `actual->padre` en una variable antes del `free`. Si libero primero y luego leo el padre, estoy leyendo memoria que ya devolví, igual que en la lista de la sesión 02.
>
> Y en `push`, el ámbito nuevo apunta a su padre y no al revés. Así cada bloque sabe a dónde regresar sin tener que guardar una lista de hijos.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion15.h`: `struct Ambito { const char *nombre; struct Ambito *padre; }`, `push_ambito()` y `pop_ambito()`. |
| **Rol (R)** | Ingeniero de compiladores responsable del ciclo de vida de los ámbitos. |
| **Especificación (E)** | Liberación de memoria al cerrar bloque: `push_ambito` reserva un ámbito hijo del actual (`NULL` si el actual o el nombre son `NULL`); `pop_ambito` libera el actual y devuelve el padre; sobre la raíz devuelve `NULL` y no libera memoria que no le pertenece; cada push se empareja con un pop. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion15.h y no lo puedo cambiar:

    typedef struct Ambito {
        const char *nombre;
        struct Ambito *padre;
    } Ambito;

    Ambito *push_ambito(Ambito *actual, const char *nombre);
    Ambito *pop_ambito(Ambito *actual);

ROL: Actúa como ingeniero de compiladores responsable del ciclo de vida de los
ámbitos.

ESPECIFICACIÓN:
1. push_ambito reserva con malloc un ámbito nuevo, hijo del actual, y lo devuelve.
   Devuelve NULL si el ámbito actual o el nombre son NULL.
2. pop_ambito libera el ámbito actual y devuelve su padre.
3. Sobre la raíz (sin padre), pop_ambito devuelve NULL y no libera memoria que no
   le pertenece: la raíz la crea quien llama, no push_ambito.
4. Cada push se empareja con un pop: no deben quedar ámbitos colgando.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion15.h"
#include <stdlib.h>

/* O(1). Crea un ambito hijo del actual. NULL si algun argumento es NULL
 * o si no hay memoria. */
Ambito *push_ambito(Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) {
        return NULL;
    }
    Ambito *hijo = (Ambito *)malloc(sizeof(Ambito));
    if (hijo == NULL) {
        return NULL;
    }
    hijo->nombre = nombre;
    hijo->padre = actual;
    return hijo;
}

/* O(1). Libera el ambito actual y devuelve su padre. La raiz (padre NULL)
 * no fue creada por push_ambito, asi que no se libera: se devuelve NULL.
 * Precondicion: todo ambito con padre fue creado por push_ambito. */
Ambito *pop_ambito(Ambito *actual) {
    if (actual == NULL || actual->padre == NULL) {
        return NULL;
    }
    Ambito *padre = actual->padre;
    free(actual);
    return padre;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Aislamiento de variables fuera de bloque

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion15_c.c artesano_sesion15_c.c`:
```text
=== AUDITORIA SESION 15 - Push y pop de ambitos ===
Caso                               esperado:              obtenido:              resultado
[ 1] nombre del ambito nuevo            esperado: if           obtenido: if           OK
[ 2] profundidad con dos push           esperado: 3            obtenido: 3            OK
[ 3] pop devuelve el padre              esperado: 1            obtenido: 1            OK
[ 4] pop sobre la raiz                  esperado: 1            obtenido: 1            OK
[ 5] push con padre NULL                esperado: 1            obtenido: 1            OK
[ 6] profundidad tras pop               esperado: 2            obtenido: 2            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion15.md
```

La celda con `ia_sesion15_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA escribió prácticamente lo mismo que yo: las dos funciones son O(1) y pasan los seis casos. La diferencia está en un comentario: la IA dejó escrita una precondición, "todo ámbito con padre fue creado por `push_ambito`". Quise ver qué pasa si no se cumple. Armé en la pila un ámbito `bloque` cuyo padre es la raíz, sin usar `push`, y le hice `pop`. Las dos versiones intentaron hacer `free` sobre memoria de la pila y el programa se cayó: imprimió el mensaje que puse antes del `pop` y nunca llegó a la siguiente línea.
>
> O sea, `pop_ambito` no tiene forma de saber de dónde salió la memoria. Solo distingue la raíz porque no tiene padre. La regla correcta es que los ámbitos internos solo se crean con `push` y solo se cierran con `pop`. En el compilador del proyecto eso se cumple si cada `{` abre con `push` y cada `}` cierra con `pop`, incluso cuando hay un error de sintaxis a la mitad del bloque.
>
> No se puede arreglar dentro de la función sin cambiar el contrato, así que agregué esa precondición como comentario en mi versión, igual que la IA.

---
**Firma digital:** OscardMQ
