# Bitácora de Requerimientos y Auditoría IA — Sesión 12
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Atributos de Símbolos y Memoria  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Estructura de registro de símbolos  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo que no sabía es que el compilador no puede poner las variables una tras otra sin más. Cada tipo se alinea a un múltiplo de su tamaño. Un `char` en 0 ocupa 1 byte, pero el `int` que sigue no empieza en 1, sino en 4. Si empezara en 1, el procesador tendría que leerlo en dos partes.
>
> Para redondear el cursor hacia arriba usé `(cursor + ancho - 1) / ancho * ancho`. Con cursor 1 e `int`: (1 + 3) / 4 * 4 = 4. Con cursor 4 y `double`: (4 + 7) / 8 * 8 = 8. La división entera es la que hace el redondeo.
>
> También me quedó claro que `asignar_offset` regresa dónde empieza la siguiente variable (offset + ancho) y no el offset. El offset se guarda en el registro.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion12.h`: `struct Simbolo { const char *nombre; const char *tipo; int offset; }`, `ancho_de_tipo()` y `asignar_offset()`. Arquitectura x86-64. |
| **Rol (R)** | Ingeniero de compiladores que asigna la memoria del marco de pila. |
| **Especificación (E)** | Cálculo de desplazamientos y alineación: char 1, short 2, int 4, float 4, double 8, puntero 8, cualquier otro -1; el offset guardado es múltiplo del ancho; el cursor nunca retrocede; registro `NULL`, cursor negativo o tipo desconocido devuelven -1. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion12.h y no lo puedo cambiar:

    typedef struct Simbolo {
        const char *nombre;
        const char *tipo;
        int offset;
    } Simbolo;

    int ancho_de_tipo(const char *tipo);
    int asignar_offset(Simbolo *simbolo, int cursor);

ROL: Actúa como ingeniero de compiladores que asigna la memoria del marco de pila
en x86-64.

ESPECIFICACIÓN:
1. ancho_de_tipo devuelve el tamaño en bytes: char 1, short 2, int 4, float 4,
   double 8, puntero 8. Cualquier otro tipo, o NULL, devuelve -1.
2. asignar_offset redondea el cursor hacia arriba al siguiente múltiplo del ancho
   del tipo, guarda ese offset en el registro y devuelve offset + ancho.
3. El offset guardado siempre es múltiplo del ancho y el cursor nunca retrocede.
4. Registro NULL, cursor negativo o tipo desconocido devuelven -1.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion12.h"
#include <limits.h>
#include <stddef.h>
#include <string.h>

typedef struct {
    const char *nombre;
    int ancho;
} TipoBasico;

/* Modelo de datos LP64 (x86-64 Linux). */
static const TipoBasico TIPOS[] = {
    { "char", 1 }, { "short", 2 }, { "int", 4 },
    { "float", 4 }, { "double", 8 }, { "puntero", 8 },
};

/* O(T), T = 6 tipos. -1 si el tipo es NULL o desconocido. */
int ancho_de_tipo(const char *tipo) {
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

/* O(1). Como el ancho es potencia de 2, alinear es redondear con una mascara:
 * offset = (cursor + ancho - 1) & ~(ancho - 1). Devuelve el cursor siguiente,
 * o -1 si el registro es NULL, el cursor es negativo, el tipo no existe o el
 * resultado no cabe en int. */
int asignar_offset(Simbolo *simbolo, int cursor) {
    if (simbolo == NULL || cursor < 0) {
        return -1;
    }

    int ancho = ancho_de_tipo(simbolo->tipo);
    if (ancho < 0) {
        return -1;
    }
    if (cursor > INT_MAX - 2 * ancho) {
        return -1;
    }

    int offset = (cursor + ancho - 1) & ~(ancho - 1);
    simbolo->offset = offset;
    return offset + ancho;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Alineación de tipos y tamaños en bytes

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion12_c.c artesano_sesion12_c.c`:
```text
=== AUDITORIA SESION 12 - Registro de simbolo y offsets alineados ===
Caso                               esperado:              obtenido:              resultado
[ 1] ancho_de_tipo("int")               esperado: 4            obtenido: 4            OK
[ 2] ancho_de_tipo("double")            esperado: 8            obtenido: 8            OK
[ 3] int en cursor 0                    esperado: 4            obtenido: 4            OK
[ 4] char e int: offset de b            esperado: 4            obtenido: 4            OK
[ 5] double desde cursor 4              esperado: 8            obtenido: 8            OK
[ 6] registro NULL                      esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion12.md
```

La celda con `ia_sesion12_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Guardó los tipos en una tabla de pares nombre-ancho y alineó con una máscara de bits, `(cursor + ancho - 1) & ~(ancho - 1)`. Funciona porque todos los anchos son potencias de 2. Es más rápido que mi división, pero si algún día hubiera un tipo de 3 o de 12 bytes la máscara daría un resultado equivocado, y mi fórmula no.
>
> Lo que agregó y yo no tenía fue el límite de `int`. Probé un `int` con el cursor en `INT_MAX - 2`. En mi versión, `cursor + ancho - 1` se desbordó: el offset salió -2147483648 y la función regresó otro número negativo. Eso rompe la regla de que el cursor nunca retrocede, y un offset negativo apuntaría fuera del marco. La IA revisa antes de calcular que `offset + ancho` quepa en `int` y, si no, regresa -1. Agregué esa revisión a mi versión y ahora las dos regresan -1.
>
> Un detalle que el contrato deja abierto es que `asignar_offset` alinea cada variable, pero no el final del marco. Con `char`, `double` y `char`, los offsets salen 0, 8 y 16, y el cursor termina en 17. En x86-64 la pila se alinea a 16 bytes en cada llamada, así que redondear el tamaño total le tocaría a quien arme el marco completo, no a esta función.

---
**Firma digital:** OscardMQ
