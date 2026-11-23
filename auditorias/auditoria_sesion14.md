# Bitácora de Requerimientos y Auditoría IA — Sesión 14
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Declaración vs Uso de Variables  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Detección de variables no declaradas  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión es la regla de "declarar antes de usar". Lo importante fue la comparación exacta. Si comparara con `strncmp` usando el largo del nombre declarado, `"xy"` empezaría igual que `"x"` y `usar` diría que sí existe. Con `strcmp` se comparan las cadenas completas, hasta el `'\0'`.
>
> Lo segundo fue que una redeclaración no debe agregar otro elemento. Por eso primero busco y solo inserto si el nombre no estaba; así `total` se queda en 1 aunque declare `x` dos veces.
>
> El contrato no dice qué hacer si la tabla de 16 ya está llena. Decidí regresar -1 para no escribir fuera del arreglo.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion14.h`: `struct Tabla { const char *nombres[16]; int total; }`, `declarar()` y `usar()`. |
| **Rol (R)** | Ingeniero de compiladores que implementa la verificación de declaraciones del analizador semántico. |
| **Especificación (E)** | Invariante de definición previa: `declarar` devuelve 1 si insertó, 0 si el nombre ya estaba y -1 con `NULL` o nombre vacío; `usar` devuelve 1 solo si el nombre fue declarado y 0 en cualquier otro caso; comparación exacta (`"x"` y `"xy"` son distintos). |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion14.h y no lo puedo cambiar:

    #define TABLA_MAX 16
    typedef struct Tabla {
        const char *nombres[TABLA_MAX];
        int total;
    } Tabla;

    int declarar(Tabla *tabla, const char *nombre);
    int usar(const Tabla *tabla, const char *nombre);

ROL: Actúa como ingeniero de compiladores que implementa la verificación de
declaraciones de un analizador semántico.

ESPECIFICACIÓN:
1. declarar devuelve 1 si insertó, 0 si el nombre ya estaba (sin agregarlo otra vez)
   y -1 si recibe NULL o un nombre vacío.
2. usar devuelve 1 solo si el nombre ya fue declarado; en cualquier otro caso, 0.
3. La comparación es exacta: "x" y "xy" son nombres distintos.
4. Si la tabla está llena, no escribas fuera del arreglo.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion14.h"
#include <stddef.h>
#include <string.h>

/* O(n). Devuelve la posicion del nombre en la tabla o -1 si no esta.
 * La comparacion es exacta (strcmp), nunca por prefijo. */
static int indice_de(const Tabla *tabla, const char *nombre) {
    for (int i = 0; i < tabla->total; i++) {
        if (tabla->nombres[i] != NULL && strcmp(tabla->nombres[i], nombre) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(n). 1 = insertado, 0 = ya declarado, -1 = argumento invalido o tabla llena. */
int declarar(Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL || nombre[0] == '\0') {
        return -1;
    }
    if (indice_de(tabla, nombre) >= 0) {
        return 0;
    }
    if (tabla->total < 0 || tabla->total >= TABLA_MAX) {
        return -1;
    }
    tabla->nombres[tabla->total++] = nombre;
    return 1;
}

/* O(n). 1 solo si el nombre ya fue declarado. */
int usar(const Tabla *tabla, const char *nombre) {
    if (tabla == NULL || nombre == NULL) {
        return 0;
    }
    return indice_de(tabla, nombre) >= 0 ? 1 : 0;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Uso antes de declarar y redeclaración

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion14_c.c artesano_sesion14_c.c`:
```text
=== AUDITORIA SESION 14 - Declaracion previa y redeclaracion ===
Caso                               esperado:              obtenido:              resultado
[ 1] usar sin declarar                  esperado: 0            obtenido: 0            OK
[ 2] usar despues de declarar           esperado: 1            obtenido: 1            OK
[ 3] redeclarar el mismo nombre         esperado: 0            obtenido: 0            OK
[ 4] total tras redeclarar              esperado: 1            obtenido: 1            OK
[ 5] prefijo no es el mismo nombre      esperado: 0            obtenido: 0            OK
[ 6] declarar nombre vacio              esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion14.md
```

La celda con `ia_sesion14_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Hizo una función `indice_de` que regresa la posición del nombre o -1, y la usó en las dos funciones. Con la tabla llena regresó -1, igual que yo.
>
> Justo ahí está lo que encontré: ese -1 mezcla dos problemas distintos. Probé declarar 17 nombres en la tabla de 16. El 17 regresó -1 y `total` se quedó en 16, sin escribir fuera del arreglo, lo cual está bien. Pero quien llama recibe el mismo -1 que con un `NULL` o un nombre vacío, y no puede saber si el problema es su argumento o que ya no hay lugar. En un compilador son mensajes distintos: uno es un error de programación del propio compilador y el otro es "demasiadas variables en este ámbito". Si el contrato lo permitiera, usaría otro código para la tabla llena. Por ahora lo dejé con -1, porque el contrato solo define 1, 0 y -1.
>
> También confirmé que la comparación distingue mayúsculas y espacios: con `x` declarada, `usar("X")` y `usar("x ")` dan 0, y `usar(NULL)` da 0 sin caerse. Eso coincide con Lenguajito, donde `suma` y `Suma` son variables distintas.

---
**Firma digital:** OscardMQ
