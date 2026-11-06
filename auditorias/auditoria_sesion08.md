# Bitácora de Requerimientos y Auditoría IA — Sesión 08
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Inferencia y Promoción de Tipos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Reglas de casteo implícito  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta sesión se parece a la 06, así que reusé la idea de `es_valido`. `promover` regresa el más ancho de los dos, que con este enum es el de número mayor. Lo que tuve que pensar fue el caso de `ERROR`. Si uno de los operandos ya es `ERROR`, regreso `ERROR`. Si regresara el otro tipo, el error se perdería y la expresión parecería válida más arriba en el árbol.
>
> En `hay_perdida` fue al revés. Hay pérdida cuando el destino es más angosto que el origen, por ejemplo guardar un `float` en un `int` y perder los decimales. Con `ERROR` regreso 0: ese error ya se reportó en otro lado, y si aquí también avisara, el usuario vería dos mensajes por el mismo problema.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion08.h`: el mismo `enum Tipo` de la sesión 06 con `promover`, `hay_perdida` y `nombre_tipo`. |
| **Rol (R)** | Ingeniero de compiladores que implementa las conversiones implícitas del verificador de tipos. |
| **Especificación (E)** | Promoción entera y ensanchamiento: jerarquía `CHAR < INT < FLOAT`; `promover` devuelve el tipo más ancho, o `ERROR` si alguno es `ERROR`; `hay_perdida` = 1 cuando el destino es más angosto que el origen; `nombre_tipo` con `"error"`, `"char"`, `"int"` y `"float"`. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion08.h y no lo puedo cambiar:

    typedef enum { TIPO_ERROR = 0, TIPO_CHAR = 1, TIPO_INT = 2, TIPO_FLOAT = 3 } Tipo;

    Tipo promover(Tipo a, Tipo b);
    int hay_perdida(Tipo destino, Tipo origen);
    const char *nombre_tipo(Tipo t);

ROL: Actúa como ingeniero de compiladores que implementa las conversiones implícitas
del verificador de tipos.

ESPECIFICACIÓN: Implementa las tres funciones con las firmas exactas.
1. La jerarquía es CHAR < INT < FLOAT.
2. promover(a, b) devuelve el tipo común de una operación binaria: el más ancho de
   los dos. Si alguno es TIPO_ERROR, devuelve TIPO_ERROR.
3. hay_perdida(destino, origen) devuelve 1 cuando convertir de origen a destino
   estrecha el tipo (el destino es más angosto) y 0 en otro caso.
4. nombre_tipo devuelve "error", "char", "int" o "float".
5. No dependas de que los valores del enum estén ordenados.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion08.h"

/* Rango de cada tipo en la jerarquia CHAR < INT < FLOAT.
 * -1 marca TIPO_ERROR o cualquier valor fuera del enum. */
static int rango(Tipo t) {
    switch (t) {
    case TIPO_CHAR:
        return 1;
    case TIPO_INT:
        return 2;
    case TIPO_FLOAT:
        return 3;
    case TIPO_ERROR:
    default:
        return -1;
    }
}

/* O(1). Devuelve el tipo de mayor rango; TIPO_ERROR si alguno es invalido. */
Tipo promover(Tipo a, Tipo b) {
    if (rango(a) < 0 || rango(b) < 0) {
        return TIPO_ERROR;
    }
    return (rango(a) >= rango(b)) ? a : b;
}

/* O(1). 1 si convertir origen -> destino estrecha el tipo. Con un tipo
 * invalido devuelve 0: el error ya lo reporta el verificador de tipos. */
int hay_perdida(Tipo destino, Tipo origen) {
    if (rango(destino) < 0 || rango(origen) < 0) {
        return 0;
    }
    return rango(destino) < rango(origen);
}

/* O(1). */
const char *nombre_tipo(Tipo t) {
    switch (t) {
    case TIPO_ERROR:
        return "error";
    case TIPO_CHAR:
        return "char";
    case TIPO_INT:
        return "int";
    case TIPO_FLOAT:
        return "float";
    default:
        return "desconocido";
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Mezcla de tipos enteros y flotantes

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion08_c.c artesano_sesion08_c.c`:
```text
=== AUDITORIA SESION 08 - Reglas de promocion y perdida ===
Caso                               esperado:              obtenido:              resultado
[ 1] promover(CHAR,INT)                 esperado: int          obtenido: int          OK
[ 2] promover(INT,FLOAT)                esperado: float        obtenido: float        OK
[ 3] promover(FLOAT,FLOAT)              esperado: float        obtenido: float        OK
[ 4] promover(ERROR,INT)                esperado: error        obtenido: error        OK
[ 5] hay_perdida(CHAR,INT)              esperado: 1            obtenido: 1            OK
[ 6] hay_perdida(INT,CHAR)              esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion08.md
```

La celda con `ia_sesion08_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. En lugar de comparar los números del enum, hizo una función `rango` con un `switch`, así que no depende de cómo esté numerado. Con `ERROR`, `hay_perdida` regresó 0 en las dos versiones, por la misma razón que yo: no duplicar el diagnóstico.
>
> El problema que encontré está en la regla, no en el código. Según la jerarquía, pasar de `INT` a `FLOAT` nunca pierde datos, y las dos versiones dicen `hay_perdida(FLOAT, INT) = 0`. Pero un `float` de 32 bits solo tiene 24 bits de mantisa. Hice una prueba extra con 16777217 (2^24 + 1): al convertirlo a `float` quedó 16777216.0, y al regresarlo a `int` dio 16777216. O sea, sí se perdió un dato y nadie avisó. A partir de 2^24, un `float` ya no alcanza a guardar todos los enteros seguidos, así que muchos se redondean al más cercano que sí puede representar. Lo bueno es que gcc sí lo detecta si se le pide: con `-Wconversion`, al compilar `float g = x;` avisa *conversion from 'int' to 'float' may change value*, y con la constante 16777217 dice que el valor cambia a `1.6777216e+7f`.
>
> No lo cambié porque el contrato y el harness definen la jerarquía así. Para el proyecto, la opción sería avisar cuando se convierte una constante entera grande a `float`.

---
**Firma digital:** OscardMQ
