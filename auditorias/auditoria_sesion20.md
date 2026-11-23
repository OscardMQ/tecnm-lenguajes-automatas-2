# Bitácora de Requerimientos y Auditoría IA — Sesión 20
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Hito 2: Analizador Semántico y Scopes  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Integración de Tabla de Símbolos y AST  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta es la sesión del Hito 2 y junta lo de las sesiones 13 y 15. Para `profundidad_de` subo por `padre` contando niveles y regreso en el primer ámbito que declara el nombre. Por eso el más interno siempre gana: si `x` está en el bloque y en el global, regresa 0 y nunca llega al global.
>
> `contar_visibles` fue lo que más me costó. No basta con sumar los símbolos de todos los niveles, porque un nombre que aparece en dos niveles se vería dos veces. Lo que hice fue contar cada nombre solo la primera vez que aparece, viendo desde el ámbito actual hacia afuera. Me lo salto si ya estaba en un ámbito más interno (lo tapa) o si ya apareció antes en el mismo ámbito.
>
> El contrato dice qué regresa `profundidad_de` con `NULL` (-2), pero no dice qué regresa `contar_visibles(NULL)`. Usé -2 también, para ser consistente.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion20.h`: `struct Ambito { const char *nombre; const char *simbolos[8]; int total; struct Ambito *padre; }`, `profundidad_de()` y `contar_visibles()`. |
| **Rol (R)** | Ingeniero de compiladores que integra la tabla de símbolos con el árbol de ámbitos. |
| **Especificación (E)** | Analizador semántico y scopes: `profundidad_de` devuelve el nivel del ámbito más cercano que declara el nombre (0 es el actual), -1 si no existe y -2 si algún argumento es `NULL`; `contar_visibles` cuenta cada nombre visible una sola vez aunque esté en varios niveles; el ámbito más interno siempre gana. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion20.h y no lo puedo cambiar:

    #define AMBITO_MAX 8
    typedef struct Ambito {
        const char *nombre;
        const char *simbolos[AMBITO_MAX];
        int total;
        struct Ambito *padre;
    } Ambito;

    int profundidad_de(const Ambito *actual, const char *nombre);
    int contar_visibles(const Ambito *actual);

ROL: Actúa como ingeniero de compiladores que integra la tabla de símbolos con el
árbol de ámbitos.

ESPECIFICACIÓN:
1. profundidad_de devuelve a cuántos niveles hacia arriba está declarado el nombre:
   0 es el ámbito actual. Devuelve -1 si el nombre no existe y -2 si algún
   argumento es NULL.
2. contar_visibles cuenta los nombres distintos que se ven desde el ámbito actual,
   una sola vez cada uno aunque estén en varios ámbitos.
3. El ámbito más interno siempre gana: el shadowing no se promedia ni se suma.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion20.h"
#include <stddef.h>
#include <string.h>

/* O(k), k = simbolos del ambito. Indice de la primera aparicion del
 * nombre en ese ambito, o -1 si no esta. */
static int indice_en(const Ambito *a, const char *nombre) {
    for (int i = 0; i < a->total && i < AMBITO_MAX; i++) {
        if (a->simbolos[i] != NULL && strcmp(a->simbolos[i], nombre) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(d * k), d = profundidad de la cadena. -2 con argumentos NULL,
 * -1 si el nombre no es visible, o el nivel del ambito mas cercano. */
int profundidad_de(const Ambito *actual, const char *nombre) {
    if (actual == NULL || nombre == NULL) {
        return -2;
    }
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        if (indice_en(a, nombre) >= 0) {
            return nivel;
        }
        nivel++;
    }
    return -1;
}

/* O(N * d * k), N = total de simbolos en la cadena. Un simbolo del nivel d
 * cuenta solo si profundidad_de lo resuelve justo en ese nivel (nadie mas
 * interno lo oculta) y si es su primera aparicion dentro del ambito. */
int contar_visibles(const Ambito *actual) {
    if (actual == NULL) {
        return 0;
    }
    int visibles = 0;
    int nivel = 0;
    for (const Ambito *a = actual; a != NULL; a = a->padre) {
        for (int i = 0; i < a->total && i < AMBITO_MAX; i++) {
            const char *s = a->simbolos[i];
            if (s != NULL && profundidad_de(actual, s) == nivel && indice_en(a, s) == i) {
                visibles++;
            }
        }
        nivel++;
    }
    return visibles;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Casos complejos de shadowing y ámbito

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion20_c.c artesano_sesion20_c.c`:
```text
=== AUDITORIA SESION 20 - Resolucion de nombres y visibilidad ===
Caso                               esperado:              obtenido:              resultado
[ 1] nombre en el ambito actual         esperado: 0            obtenido: 0            OK
[ 2] nombre dos niveles arriba          esperado: 2            obtenido: 2            OK
[ 3] shadowing gana el interno          esperado: 0            obtenido: 0            OK
[ 4] nombre no visible                  esperado: -1           obtenido: -1           OK
[ 5] visibles sin repetir               esperado: 3            obtenido: 3            OK
[ 6] ambito NULL                        esperado: -2           obtenido: -2           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion20.md
```

La celda con `ia_sesion20_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, pero resolvió `contar_visibles` de otra forma. En lugar de buscar repetidos a mano, reutilizó su propia `profundidad_de`: un símbolo del nivel *d* cuenta solo si `profundidad_de` lo resuelve justo en ese nivel (o sea, nadie más interno lo tapa) y si es su primera aparición en ese ámbito. Me gustó porque usa la misma regla del shadowing en las dos funciones. También puso un tope `i < AMBITO_MAX` en los ciclos, por si `total` viniera dañado.
>
> La diferencia que encontré es justo un caso que el contrato no define: `contar_visibles(NULL)`. La mía regresa -2, como `profundidad_de`, y la de la IA regresa 0, con la idea de que "desde ningún ámbito no se ve nada". Las dos interpretaciones son defendibles, y el harness no lo prueba. Lo dejé en -2 para ser consistente con la otra función del mismo contrato, pero es una buena pregunta para el profe.
>
> En las pruebas extra coincidieron en lo demás. Si el bloque declara una copia de `"x"` (mismo texto, otra dirección de memoria) y el global tiene otra `x`, las dos cuentan 1 visible y dan profundidad 0, porque comparan con `strcmp` y no con la dirección del puntero. Con `a` repetida dos veces en el mismo ámbito, también cuentan 1.

---
**Firma digital:** OscardMQ
