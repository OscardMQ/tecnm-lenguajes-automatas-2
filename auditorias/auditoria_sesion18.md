# Bitácora de Requerimientos y Auditoría IA — Sesión 18
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Arreglos y Verificación de Límites  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Cálculo de tamaño de arreglo en memoria  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión se parece a la 12, así que reusé la tabla de anchos. Lo que tuve que pensar fue el tipo del resultado. Si multiplico `int` por `int`, un arreglo grande de `double` se pasa de 2,147,483,647 y se desborda. Por eso convierto el ancho a `long` antes de multiplicar.
>
> Lo segundo fue el borde del rango. En un arreglo de 5 elementos los índices válidos van de 0 a 4. El 5 ya se sale, aunque sea el número de elementos. Por eso la condición es `indice < elementos` y no `<=`. Ese error de uno de más es justo el que provoca desbordamientos de buffer.
>
> Y el contrato distingue dos cosas que parecen iguales: cero elementos es válido y pesa 0 bytes, pero un número negativo es inválido y regresa -1.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion18.h`: `long bytes_arreglo(const char *tipo, int elementos)` e `int indice_en_rango(int indice, int elementos)`. Arquitectura x86-64 Linux. |
| **Rol (R)** | Ingeniero de compiladores que verifica arreglos en el análisis semántico. |
| **Especificación (E)** | Arreglos y verificación de límites: char 1, short 2, int 4, float 4, double 8, puntero 8, cualquier otro -1; elementos negativos devuelven -1 y cero elementos pesan 0 bytes; un índice es válido de 0 a elementos - 1. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
Compilo para x86-64 Linux. El contrato está en api_sesion18.h y no lo puedo cambiar:

    long bytes_arreglo(const char *tipo, int elementos);
    int indice_en_rango(int indice, int elementos);

ROL: Actúa como ingeniero de compiladores que verifica arreglos en el análisis
semántico.

ESPECIFICACIÓN:
1. bytes_arreglo devuelve el tamaño total del arreglo en bytes. Los tipos válidos
   son char (1), short (2), int (4), float (4), double (8) y puntero (8); cualquier
   otro nombre devuelve -1.
2. Un número negativo de elementos es inválido y devuelve -1; cero elementos pesa
   cero bytes.
3. indice_en_rango devuelve 1 si el índice puede leerse y 0 si no. El índice válido
   más alto es elementos - 1, nunca elementos.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion18.h"
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *nombre;
    int ancho;
} TipoBasico;

/* Modelo LP64 (x86-64 Linux). */
static const TipoBasico TIPOS[] = {
    { "char", 1 }, { "short", 2 }, { "int", 4 },
    { "float", 4 }, { "double", 8 }, { "puntero", 8 },
};

/* O(T), T = 6 tipos. Devuelve ancho * elementos, o -1 si el tipo es
 * desconocido o el numero de elementos es negativo. */
long bytes_arreglo(const char *tipo, int elementos) {
    if (tipo == NULL || elementos < 0) {
        return -1;
    }
    for (size_t i = 0; i < sizeof TIPOS / sizeof TIPOS[0]; i++) {
        if (strcmp(tipo, TIPOS[i].nombre) == 0) {
            return (long)TIPOS[i].ancho * (long)elementos;
        }
    }
    return -1;
}

/* O(1). 1 si 0 <= indice < elementos; 0 en cualquier otro caso,
 * incluido un arreglo de cero o menos elementos. */
int indice_en_rango(int indice, int elementos) {
    if (elementos <= 0) {
        return 0;
    }
    return (indice >= 0 && indice < elementos) ? 1 : 0;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Límites estáticos y dimensiones inválidas

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion18_c.c artesano_sesion18_c.c`:
```text
=== AUDITORIA SESION 18 - Tamano de arreglo y limites ===
Caso                               esperado:              obtenido:              resultado
[ 1] int[10]                            esperado: 40           obtenido: 40           OK
[ 2] double[3]                          esperado: 24           obtenido: 24           OK
[ 3] int[-4]                            esperado: -1           obtenido: -1           OK
[ 4] indice 0 de 5                      esperado: 1            obtenido: 1            OK
[ 5] indice 5 de 5                      esperado: 0            obtenido: 0            OK
[ 6] indice negativo                    esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion18.md
```

La celda con `ia_sesion18_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos con la misma idea que yo. Guardó los tipos en una tabla y en `indice_en_rango` agregó un caso que yo cubría sin escribirlo: con cero elementos regresa 0 para cualquier índice. En la mía sale igual, porque ningún índice cumple `0 <= i < 0`.
>
> Lo que encontré es de portabilidad y lo tienen las dos versiones. El contrato regresa `long` porque en Linux x86-64 un `long` es de 64 bits, y ahí `8 × 2,147,483,647` cabe sin problema. Pero yo programo en Windows, donde `long` es de 32 bits. Hice una prueba extra en mi computadora con `double[300000000]`, que debería pesar 2,400,000,000 bytes: las dos versiones regresaron -1894967296. La multiplicación se desbordó en silencio y el resultado salió negativo, que además se confunde con el -1 de error.
>
> En Colab no pasa, y por eso el harness sale en verde. No lo cambié, porque el contrato fija `long` y el curso compila en Linux. Si el compilador del proyecto tuviera que correr en Windows, habría que usar `long long` o revisar antes de multiplicar que `elementos <= LONG_MAX / ancho`.
>
> También probé los bordes: índice 4 de 5 da 1, índice 0 de un arreglo vacío da 0, `INT_MIN` da 0, `bool[3]` da -1 y `char[0]` pesa 0.

---
**Firma digital:** OscardMQ
