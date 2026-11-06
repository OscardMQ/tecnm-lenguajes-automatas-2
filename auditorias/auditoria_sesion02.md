# Bitácora de Requerimientos y Auditoría IA — Sesión 02
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Gestión en GitHub Web y Nodos Dinámicos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Lista enlazada de tokens en Heap  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> El contrato cambió respecto a la primera versión del laboratorio. Ya no existe `crear_nodo`, y `agregar_token` recibe la cabeza y la regresa. Lo que más me costó entender fue por qué la regresa. En C los parámetros se pasan por valor: si la lista está vacía y adentro de la función hago `cabeza = nodo`, ese cambio se queda en mi copia. Por eso la función devuelve la cabeza y el que llama tiene que escribir `c = agregar_token(c, ...)`. Si se le olvida reasignar, la lista se queda en `NULL` y el nodo se pierde en el heap.
>
> La segunda restricción fue el campo `char lexema[16]`. Es un arreglo dentro del nodo, no un puntero, así que caben 15 caracteres más el `'\0'`. Usé `strncpy` con `sizeof nodo->lexema - 1` y escribí el nulo a mano, porque `strncpy` no lo agrega cuando el texto es más largo.
>
> La tercera fue el orden del `free` en `liberar_lista`. Si libero el nodo y después leo `cabeza->siguiente`, estoy leyendo memoria que ya devolví (*use-after-free*). Por eso guardo el siguiente en una variable antes de liberar.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato fijo en `api_sesion02.h`: `struct NodoToken { char lexema[16]; struct NodoToken *siguiente; }` y las firmas de `agregar_token`, `contar_tokens` y `liberar_lista`. |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores, especialista en gestión manual de memoria dinámica. |
| **Especificación (E)** | Gestión de memoria dinámica: cada token en un nodo reservado con `malloc` e insertado al final; `agregar_token` devuelve la cabeza (el nodo nuevo si la lista estaba vacía); copia acotada a 15 caracteres más `'\0'`; `contar_tokens(NULL)` = 0; `liberar_lista(NULL)` no se cae; sin fugas ni *use-after-free*. |
| **Output (O)** | Solo el código de las tres funciones, sin `main()` ni arreglos globales, con la complejidad de cada una. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion02.h y no puedo modificarlo:

    typedef struct NodoToken {
        char lexema[16];
        struct NodoToken *siguiente;
    } NodoToken;

    NodoToken *agregar_token(NodoToken *cabeza, const char *lexema);
    int contar_tokens(NodoToken *cabeza);
    void liberar_lista(NodoToken *cabeza);

ROL: Actúa como ingeniero de software de sistemas y compiladores, especialista en
gestión manual de memoria dinámica.

ESPECIFICACIÓN: Implementa las tres funciones respetando las firmas exactas.
Requisitos obligatorios:
1. Cada token vive en un nodo reservado con malloc; nada de arreglos globales ni
   estáticos.
2. agregar_token inserta al final para conservar el orden de llegada y devuelve la
   cabeza de la lista. Si la lista estaba vacía (cabeza == NULL), devuelve el nodo
   nuevo.
3. El campo siguiente del último nodo debe valer NULL.
4. La copia del lexema no puede desbordar los 16 bytes del campo y siempre debe
   quedar terminada en el byte nulo.
5. contar_tokens(NULL) devuelve 0 y liberar_lista(NULL) retorna sin caerse.
6. liberar_lista no debe leer memoria ya liberada.
7. Verifica el valor de retorno de malloc antes de usarlo.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de una línea por función con su complejidad temporal en notación O.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion02.h"
#include <stdlib.h>
#include <string.h>

/* O(k), k = longitud del lexema acotada a 15: reserva y llena un nodo. */
static NodoToken *nuevo_nodo(const char *lexema) {
    NodoToken *nodo = (NodoToken *)malloc(sizeof(NodoToken));
    if (nodo == NULL) {
        return NULL;
    }

    size_t largo = strlen(lexema);
    if (largo > sizeof(nodo->lexema) - 1U) {
        largo = sizeof(nodo->lexema) - 1U;
    }
    memcpy(nodo->lexema, lexema, largo);
    nodo->lexema[largo] = '\0';
    nodo->siguiente = NULL;
    return nodo;
}

/* O(n): inserta al final y devuelve la cabeza (nueva si la lista estaba vacia).
 * Si malloc falla, devuelve la lista sin cambios. */
NodoToken *agregar_token(NodoToken *cabeza, const char *lexema) {
    if (lexema == NULL) {
        return cabeza;
    }

    NodoToken *nodo = nuevo_nodo(lexema);
    if (nodo == NULL) {
        return cabeza;
    }

    NodoToken **enlace = &cabeza;
    while (*enlace != NULL) {
        enlace = &(*enlace)->siguiente;
    }
    *enlace = nodo;
    return cabeza;
}

/* O(n). */
int contar_tokens(NodoToken *cabeza) {
    int total = 0;
    while (cabeza != NULL) {
        total++;
        cabeza = cabeza->siguiente;
    }
    return total;
}

/* O(n), iterativa para no arriesgar desbordamiento de pila. */
void liberar_lista(NodoToken *cabeza) {
    while (cabeza != NULL) {
        NodoToken *siguiente = cabeza->siguiente;
        free(cabeza);
        cabeza = siguiente;
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Prevención de Use-After-Free y cadenas nulas

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion02_c.c artesano_sesion02_c.c`:
```text
=== AUDITORIA SESION 02 - Lista enlazada de tokens en heap ===
Caso                               esperado:              obtenido:              resultado
[ 1] agregar_token(NULL,"if")           esperado: if           obtenido: if           OK
[ 2] contar_tokens(3 tokens)            esperado: 3            obtenido: 3            OK
[ 3] orden: segundo token               esperado: dos          obtenido: dos          OK
[ 4] ultimo->siguiente == NULL          esperado: 1            obtenido: 1            OK
[ 5] contar_tokens(NULL)                esperado: 0            obtenido: 0            OK
[ 6] liberar_lista(NULL)                esperado: sin crash    obtenido: sin crash    OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion02.md
```

La celda con `ia_sesion02_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Con el contrato completo, la IA cumplió los seis casos. La diferencia contra mi versión es de estilo. Usó un puntero al enlace (`NodoToken **enlace`) en lugar de caminar hasta el último nodo, y así no necesita el caso especial de la lista vacía. También separó la creación del nodo en una función `static`, que tiene sentido porque ya no forma parte del contrato.
>
> Lo que encontré al auditar no truena, pero sí importa en un compilador: el campo de 16 bytes corta los lexemas largos sin avisar. Hice una prueba extra con `"contador_total_a"` y `"contador_total_b"`, que tienen 16 caracteres. Las dos versiones guardaron `"contador_total_"` en ambos nodos. O sea, dos identificadores distintos quedan iguales en la lista. Cuando esta lista alimente la tabla de símbolos, el compilador va a creer que son la misma variable. El contrato no tiene cómo reportar ese error, porque `agregar_token` solo regresa la cabeza. Para el proyecto lo voy a resolver guardando el lexema con `malloc` del tamaño exacto, en lugar del arreglo fijo, o rechazando el token con un diagnóstico.
>
> En la misma prueba confirmé el riesgo de no reasignar: `agregar_token(d, "x")` con `d` en `NULL` deja `d` en `NULL` y el nodo queda perdido en el heap.

---
**Firma digital:** OscardMQ
