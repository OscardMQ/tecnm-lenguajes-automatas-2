# Bitácora de Requerimientos y Auditoría IA — Sesión 16
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Firmas de Función y Parámetros  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Registro de prototipos y aridad  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue separar los tres resultados: 1 cuando nombre y aridad coinciden, 0 cuando el nombre existe pero con otro número de argumentos, y -1 cuando el nombre no existe. Al principio pensé en regresar en cuanto encuentro el nombre, pero el contrato dice que se recorre el catálogo completo. Si hubiera dos firmas con el mismo nombre, como `suma` de 2 y de 3 parámetros, la primera no tiene por qué ser la buena.
>
> Por eso uso una bandera `encontrada`. Si alguna firma coincide en nombre y aridad, regreso 1 de inmediato. Si termino el recorrido y solo coincidió el nombre, regreso 0. Si nunca apareció, -1.
>
> También salto las entradas del catálogo con `nombre` en `NULL`, para no pasárselas a `strcmp`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion16.h`: `struct Funcion { const char *nombre; int aridad; }` y `verificar_llamada()`. |
| **Rol (R)** | Ingeniero de compiladores que verifica las llamadas a función en el análisis semántico. |
| **Especificación (E)** | Coincidencia estricta de parámetros: búsqueda exacta del nombre recorriendo el catálogo completo; 1 llamada válida, 0 aridad incorrecta, -1 nombre inexistente; una función de cero parámetros solo acepta cero argumentos; catálogo `NULL`, nombre `NULL` o aridad negativa devuelven -1. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion16.h y no lo puedo cambiar:

    typedef struct Funcion {
        const char *nombre;
        int aridad;
    } Funcion;

    int verificar_llamada(const Funcion *catalogo, int total,
                          const char *nombre, int nargs);

ROL: Actúa como ingeniero de compiladores que verifica las llamadas a función en el
análisis semántico.

ESPECIFICACIÓN:
1. La búsqueda del nombre es exacta y recorre el catálogo completo.
2. Devuelve 1 si la llamada es válida (nombre y número de argumentos coinciden),
   0 si el nombre existe pero la aridad no coincide y -1 si el nombre no existe.
3. Una función registrada con cero parámetros solo acepta cero argumentos.
4. Un catálogo NULL, un nombre NULL o una aridad negativa devuelven -1.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion16.h"
#include <stddef.h>
#include <string.h>

enum { LLAMADA_INVALIDA = -1, ARIDAD_INCORRECTA = 0, LLAMADA_VALIDA = 1 };

/* O(n), n = total de firmas en el catalogo. Busca por nombre exacto y
 * compara la aridad de la primera firma con ese nombre. */
int verificar_llamada(const Funcion *catalogo, int total, const char *nombre, int nargs) {
    if (catalogo == NULL || nombre == NULL || nargs < 0 || total <= 0) {
        return LLAMADA_INVALIDA;
    }

    for (int i = 0; i < total; i++) {
        const Funcion *f = &catalogo[i];
        if (f->nombre != NULL && strcmp(f->nombre, nombre) == 0) {
            return (f->aridad == nargs) ? LLAMADA_VALIDA : ARIDAD_INCORRECTA;
        }
    }
    return LLAMADA_INVALIDA;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Llamadas con número incorrecto de args

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion16_c.c artesano_sesion16_c.c`:
```text
=== AUDITORIA SESION 16 - Registro de prototipos y aridad ===
Caso                               esperado:              obtenido:              resultado
[ 1] suma con 3 argumentos              esperado: 1            obtenido: 1            OK
[ 2] suma con 2 argumentos              esperado: 0            obtenido: 0            OK
[ 3] suma con 4 argumentos              esperado: 0            obtenido: 0            OK
[ 4] funcion no registrada              esperado: -1           obtenido: -1           OK
[ 5] main sin argumentos                esperado: 1            obtenido: 1            OK
[ 6] nombre NULL                        esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion16.md
```

La celda con `ia_sesion16_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, pero se saltó una parte del contrato: regresa en cuanto encuentra el primer nombre igual. El harness no lo nota porque cada catálogo tiene una sola firma. Hice una prueba con el catálogo `{suma/2, suma/3}` y la llamada a `suma` con 3 argumentos. Mi versión regresó 1, porque recorre todo y encuentra la segunda firma. La de la IA regresó 0, o sea, diría que la aridad está mal aunque sí existe una `suma` de 3 parámetros.
>
> El prompt decía "recorre el catálogo completo", pero la IA lo interpretó como "busca en todo el catálogo hasta encontrarlo". Lo detecté comparando su ciclo con el mío: su `return` está dentro del `if` del nombre. La forma de corregirlo es la que usé: una bandera que recuerde si el nombre apareció, y regresar 0 solo al terminar el recorrido.
>
> Si el lenguaje no permite sobrecarga, dos firmas con el mismo nombre deberían ser un error de redeclaración al registrarlas, y entonces la versión de la IA bastaría. Pero eso no lo revisa esta función. En lo demás coincidieron: con `total` en 0, con aridad -1 y con `Suma` en mayúscula, las dos regresan -1.

---
**Firma digital:** OscardMQ
