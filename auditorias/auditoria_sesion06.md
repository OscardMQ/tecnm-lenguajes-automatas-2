# Bitácora de Requerimientos y Auditoría IA — Sesión 06
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Sistema de Tipos Primitivos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Chequeo de compatibilidad de tipos  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Me di cuenta de que la jerarquía `CHAR < INT < FLOAT` ya está en los números del enum: 1, 2 y 3. Entonces `compatible` es una comparación: se puede asignar si el destino es igual o más ancho que el origen.
>
> El problema es `TIPO_ERROR`, que vale 0. Si comparo directo, `compatible(INT, ERROR)` da 1 porque 2 >= 0, y eso es justo lo que prohíbe el caso 4 del harness. Lo comprobé quitando la validación y, en efecto, salió 1. Por eso hice `es_valido`, que solo acepta del 1 al 3. Así `ERROR` y cualquier valor raro, como `(Tipo)7`, nunca son compatibles.
>
> En `nombre_tipo` usé un `switch` con los cuatro casos y dejé el `return "desconocido"` afuera, para los valores que no estén en el enum.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion06.h`: `enum Tipo { TIPO_ERROR = 0, TIPO_CHAR = 1, TIPO_INT = 2, TIPO_FLOAT = 3 }` con `compatible` y `nombre_tipo`. |
| **Rol (R)** | Ingeniero de compiladores que diseña el verificador de tipos de un analizador semántico. |
| **Especificación (E)** | Matriz de compatibilidad estática: jerarquía `CHAR < INT < FLOAT`; se acepta asignar a un tipo igual o más ancho y se rechaza a uno más angosto (requiere cast explícito); `TIPO_ERROR` nunca es compatible; `nombre_tipo` devuelve `"error"`, `"char"`, `"int"`, `"float"` y `"desconocido"` fuera de rango. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion06.h y no lo puedo cambiar:

    typedef enum { TIPO_ERROR = 0, TIPO_CHAR = 1, TIPO_INT = 2, TIPO_FLOAT = 3 } Tipo;

    int compatible(Tipo destino, Tipo origen);
    const char *nombre_tipo(Tipo t);

ROL: Actúa como ingeniero de compiladores que diseña el verificador de tipos de un
analizador semántico.

ESPECIFICACIÓN: Implementa las dos funciones con las firmas exactas.
1. compatible devuelve 1 si una asignación destino = origen es legal sin cast y 0
   si no. La jerarquía es CHAR < INT < FLOAT.
2. Asignar a un tipo igual o más ancho es legal. Asignar a un tipo más angosto se
   rechaza, porque requiere cast explícito.
3. TIPO_ERROR nunca es compatible con nada, ni consigo mismo.
4. nombre_tipo devuelve "error", "char", "int" o "float". Para un valor fuera del
   enum devuelve "desconocido".
5. Representa las reglas como una matriz estática de compatibilidad.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion06.h"

#define NUM_TIPOS 4

/* Matriz estatica [destino][origen]: 1 = asignacion implicita permitida.
 * Fila y columna 0 (TIPO_ERROR) son siempre 0. */
static const int COMPATIBLE[NUM_TIPOS][NUM_TIPOS] = {
    /* origen:   ERROR CHAR INT FLOAT */
    /* ERROR */ { 0,    0,   0,  0 },
    /* CHAR  */ { 0,    1,   0,  0 },
    /* INT   */ { 0,    1,   1,  0 },
    /* FLOAT */ { 0,    1,   1,  1 },
};

static const char *const NOMBRES[NUM_TIPOS] = { "error", "char", "int", "float" };

static int en_rango(Tipo t) {
    return (int)t >= 0 && (int)t < NUM_TIPOS;
}

/* O(1): una consulta a la matriz. */
int compatible(Tipo destino, Tipo origen) {
    if (!en_rango(destino) || !en_rango(origen)) {
        return 0;
    }
    return COMPATIBLE[destino][origen];
}

/* O(1). */
const char *nombre_tipo(Tipo t) {
    if (!en_rango(t)) {
        return "desconocido";
    }
    return NOMBRES[t];
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Rechazo de asignaciones incompatibles

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion06_c.c artesano_sesion06_c.c`:
```text
=== AUDITORIA SESION 06 - Tabla de compatibilidad de tipos ===
Caso                               esperado:              obtenido:              resultado
[ 1] compatible(INT,INT)                esperado: 1            obtenido: 1            OK
[ 2] compatible(FLOAT,INT)              esperado: 1            obtenido: 1            OK
[ 3] compatible(INT,FLOAT)              esperado: 0            obtenido: 0            OK
[ 4] compatible(INT,ERROR)              esperado: 0            obtenido: 0            OK
[ 5] nombre_tipo(INT)                   esperado: int          obtenido: int          OK
[ 6] nombre_tipo(ERROR)                 esperado: error        obtenido: error        OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion06.md
```

La celda con `ia_sesion06_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA resolvió lo mismo con una matriz `[destino][origen]` de 4 por 4 y un arreglo de nombres. Cumplió los seis casos y además revisa el rango antes de indexar. Eso es importante: sin esa revisión, `compatible((Tipo)7, TIPO_INT)` leería fuera de la matriz. En mi prueba extra, las dos versiones dieron 0 con `(Tipo)7` y `"desconocido"` como su nombre.
>
> La diferencia que me llevo es de diseño. Mi versión depende de que el enum esté numerado en orden y de que todos los tipos formen una sola línea. Si después se agrega un tipo como `TIPO_BOOL`, mi `es_valido` lo rechazaría siempre. Para aceptarlo tendría que acomodarlo en algún lugar de la línea `CHAR < INT < FLOAT`, aunque no tenga sentido decir que `bool` es más ancho o más angosto que `char`. Con la matriz, cada combinación está escrita a mano, y agregar un tipo solo obliga a llenar su fila y su columna. Para el proyecto me quedo con la idea de la matriz.

---
**Firma digital:** OscardMQ
