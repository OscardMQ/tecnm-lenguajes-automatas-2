# Bitácora de Requerimientos y Auditoría IA — Sesión 27
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Indexación de Arreglos en TAC  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Cálculo de desplazamientos base + offset  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> El procesador no entiende índices, entiende direcciones. Para leer `A[i]` hay que convertir el índice en bytes: se multiplica por el ancho del elemento y ese offset se suma a la base. Por eso la primera línea siempre es `_tK = i * ancho`, aunque el ancho sea 1, y la segunda es la lectura `_tK+1 = A[_tK]`.
>
> En 2D primero se aplana. `M[i][j]` se vuelve `i * filas + j`, y ese índice lineal se multiplica por el ancho. Me confundió el nombre del parámetro: en la fórmula de la diapositiva, para `A[N][M]` se multiplica `i * M`, o sea, por lo largo de cada fila. Así que `filas` aquí es cuántos elementos tiene una fila. Si se intercambia el orden, se lee otra celda sin ningún aviso.
>
> Cada línea crea un temporal nuevo, así que el contador avanza lo mismo que las líneas escritas: 2 en 1D y 4 en 2D. La función regresa cuántas líneas escribió. Volví a usar el "todo o nada" de las sesiones 24 a 26: preparo las líneas aparte y solo las copio si todas caben y hay lugar.
>
> La diapositiva habla de desbordamiento de enteros en el offset. Mi función no calcula el offset, solo escribe las instrucciones; la multiplicación pasa en ejecución. Donde sí hay un entero que se puede desbordar es en el contador de temporales, así que reviso que no sea negativo y que no esté a punto de pasarse de `INT_MAX`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion27.h`: `int emitir_acceso(char lineas[][64], int max, const char *base, const char *indice, int ancho, int *temporales)` y `int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i, const char *j, int filas, int ancho, int *temporales)`. |
| **Rol (R)** | Ingeniero de compiladores que genera el código intermedio para leer arreglos. |
| **Especificación (E)** | Lectura indexada en TAC: en 1D, `_tK = indice * ancho` y `_tK+1 = base[_tK]`; en 2D, aplanado `i * filas + j`, después por el ancho y la lectura (cuatro líneas); escriben desde `lineas[0]` y devuelven cuántas líneas escribieron; cada línea crea un temporal y `*temporales` avanza lo mismo; base o índices `NULL` o vacíos devuelven 0; todo o nada si falta espacio o una línea no cabe en 63 caracteres. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion27.h y no lo puedo cambiar:

    int emitir_acceso(char lineas[][64], int max, const char *base,
                      const char *indice, int ancho, int *temporales);
    int emitir_acceso_2d(char lineas[][64], int max, const char *base,
                         const char *i, const char *j, int filas, int ancho,
                         int *temporales);

ROL: Actúa como ingeniero de compiladores que genera el código intermedio para
leer arreglos.

ESPECIFICACIÓN:
1. Las dos funciones escriben desde lineas[0] y devuelven cuántas líneas
   escribieron (0 si no escribieron nada). Los temporales se numeran desde
   *temporales: cada línea crea un temporal nuevo y *temporales avanza lo mismo.
2. emitir_acceso escribe "_tK = <indice> * <ancho>" y "_tK+1 = <base>[_tK]".
3. emitir_acceso_2d aplana en orden i * filas + j: "_tK = <i> * <filas>",
   "_tK+1 = _tK + <j>", "_tK+2 = _tK+1 * <ancho>" y "_tK+3 = <base>[_tK+2]".
4. Si base, indice, i o j son NULL o cadena vacía, devuelve 0 sin escribir.
5. Todo o nada: si no hay lugar para todas las líneas o alguna no cabe completa en
   63 caracteres, no escribe nada, devuelve 0 y no mueve *temporales.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 27 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion27.h"
#include <stdio.h>
#include <string.h>

#define ANCHO_LINEA 64
#define MAX_LINEAS_ACCESO 4

/* Un texto es valido solo si no es NULL ni la cadena vacia. */
static int texto_valido(const char *s) {
    return s != NULL && *s != '\0';
}

/* Copia las lineas preparadas al destino solo si todas estan completas
 * y hay espacio para todas (todo o nada). Devuelve cuantas copio. */
static int confirmar(char lineas[][64], int max, char buffer[][ANCHO_LINEA],
                     const int *escritos, int cantidad) {
    if (max < cantidad) return 0;
    for (int k = 0; k < cantidad; k++) {
        if (escritos[k] < 0 || escritos[k] >= ANCHO_LINEA) return 0;  /* linea truncada */
    }
    for (int k = 0; k < cantidad; k++) {
        memcpy(lineas[k], buffer[k], ANCHO_LINEA);
    }
    return cantidad;
}

int emitir_acceso(char lineas[][64], int max, const char *base, const char *indice,
                  int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL) return 0;
    if (!texto_valido(base) || !texto_valido(indice) || ancho <= 0) return 0;

    char buffer[MAX_LINEAS_ACCESO][ANCHO_LINEA];
    int escritos[2];
    int t = *temporales;

    /* Offset: el indice siempre se multiplica por el ancho del elemento. */
    escritos[0] = snprintf(buffer[0], ANCHO_LINEA, "_t%d = %s * %d", t, indice, ancho);
    /* Lectura indexada desde la base con el offset calculado. */
    escritos[1] = snprintf(buffer[1], ANCHO_LINEA, "_t%d = %s[_t%d]", t + 1, base, t);

    int copiadas = confirmar(lineas, max, buffer, escritos, 2);
    if (copiadas > 0) *temporales = t + 2;
    return copiadas;
}

int emitir_acceso_2d(char lineas[][64], int max, const char *base, const char *i,
                     const char *j, int filas, int ancho, int *temporales) {
    if (lineas == NULL || temporales == NULL) return 0;
    if (!texto_valido(base) || !texto_valido(i) || !texto_valido(j)) return 0;
    if (filas <= 0 || ancho <= 0) return 0;

    char buffer[MAX_LINEAS_ACCESO][ANCHO_LINEA];
    int escritos[4];
    int t = *temporales;

    /* Aplanado en orden de filas: i * filas + j. */
    escritos[0] = snprintf(buffer[0], ANCHO_LINEA, "_t%d = %s * %d", t, i, filas);
    escritos[1] = snprintf(buffer[1], ANCHO_LINEA, "_t%d = _t%d + %s", t + 1, t, j);
    /* Escalado por el ancho del elemento y lectura indexada. */
    escritos[2] = snprintf(buffer[2], ANCHO_LINEA, "_t%d = _t%d * %d", t + 2, t + 1, ancho);
    escritos[3] = snprintf(buffer[3], ANCHO_LINEA, "_t%d = %s[_t%d]", t + 3, base, t + 2);

    int copiadas = confirmar(lineas, max, buffer, escritos, 4);
    if (copiadas > 0) *temporales = t + 4;
    return copiadas;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Arreglos multidimensionales en TAC

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion27_c.c artesano_sesion27_c.c`:
```text
=== AUDITORIA SESION 27 - Desplazamientos para arreglos 1D y 2D ===
Caso                               esperado:              obtenido:              resultado
[ 1] 1D: i * 4                          esperado: _t0 = i * 4  obtenido: _t0 = i * 4  OK
[ 2] 1D: A[_t0]                         esperado: _t1 = A[_t0] obtenido: _t1 = A[_t0] OK
[ 3] 2D: i * 3                          esperado: _t0 = i * 3  obtenido: _t0 = i * 3  OK
[ 4] 2D: offset por ancho
     esperado: _t2 = _t1 * 4
     obtenido: _t2 = _t1 * 4
     -> OK
[ 5] 2D: M[_t2]                         esperado: _t3 = M[_t2] obtenido: _t3 = M[_t2] OK
[ 6] base NULL (sin acceso)             esperado: 0            obtenido: 0            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion27.md
```

La celda con `ia_sesion27_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Probé un acceso 1D y después uno 2D con el mismo contador, y las dos versiones dieron `_t0` y `_t1` para `A[i]` y luego `_t2` a `_t5` para `M[i][j]`, sin repetir nombres. También coincidieron en lo que no se debe escribir: el acceso 2D con `max = 3` y una base de 56 caracteres, donde la lectura ya no cabe en 63. En los dos casos regresaron 0 y el contador se quedó igual.
>
> La primera diferencia fue el ancho. Mi primera versión aceptaba un ancho de 0 y escribía `_t0 = i * 0` y `_t1 = A[_t0]`. Eso lee `A[0]` para cualquier `i`, sin ningún aviso. Es el primer punto de falla de la presentación, "olvidar multiplicar por el ancho", pero disfrazado: la multiplicación sí está, solo que por 0. Con `-4` también escribía las dos líneas, y el offset quedaba negativo, antes del inicio del arreglo. Con `filas = 0` en 2D, `i` dejaba de importar. La IA rechazó `ancho <= 0` y `filas <= 0` sin que yo lo pidiera en el prompt. Agregué esas dos revisiones a mi versión.
>
> La segunda diferencia fue el contador, y aquí la IA falló. No revisa `*temporales`. Con `-1` escribió `_t-1 = i * 4` y `_t0 = A[_t-1]`, un nombre de temporal que no es válido. Con `INT_MAX` el contador se desbordó: escribió `_t2147483647 = i * 4` y `_t-2147483648 = A[_t2147483647]`, y `*temporales` terminó en `-2147483647`. En C eso es comportamiento indefinido. Mi versión rechaza los dos casos porque revisa que el contador no sea negativo ni esté cerca de `INT_MAX`, como hice con `*n` en la sesión 25. En este laboratorio, ese es el desbordamiento de enteros del que habla la diapositiva.
>
> Lo que ninguna hace es escribir en un arreglo. La presentación menciona confundir la lectura con la escritura en `A[i] = B[j]`. Las dos funciones solo leen: para esa asignación habría que leer `B[j]` en un temporal y luego escribir `A[_tK] = _tM`, que es otra instrucción. Si se usara `emitir_acceso` para el lado izquierdo, saldría una lectura de `A` en vez de una escritura.

---
**Firma digital:** OscardMQ
