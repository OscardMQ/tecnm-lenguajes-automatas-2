# Bitácora de Requerimientos y Auditoría IA — Sesión 17
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Comprobación Semántica de Retornos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Validación de tipo de sentencia return  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta función junta dos revisiones: la del tipo y la de las rutas. Para el tipo hice un rango por tipo (`char` 1 hasta `double` 6) y `void` aparte, con 0. La regla es la de la sesión 08: se acepta si el destino es igual o más ancho que el origen.
>
> El caso que más pensé fue `void`. Si la función es `void` y la expresión trae un valor, es error de tipo. Pero una función `void` puede terminar sin `return` en cualquier ruta, porque no tiene nada que regresar. Por eso, con `void` regreso 0 sin revisar las rutas.
>
> También tuve que decidir el orden. Si el tipo está mal y además falta un `return`, regreso 1, porque el tipo es lo primero que se revisa en la sentencia `return`. Y antes de todo valido los datos: cero rutas, rutas negativas o más `return` que rutas regresan -1.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion17.h`: `int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion, int rutas_totales, int rutas_con_return)`. |
| **Rol (R)** | Ingeniero de compiladores que comprueba las sentencias `return` en el análisis semántico. |
| **Especificación (E)** | Rutas de ejecución con retorno garantizado: 0 si todo está bien; 1 si el tipo es incompatible (el destino debe ser igual o más ancho que el origen, y devolver un valor en una función `void` siempre es error); 2 si queda una ruta sin `return`; firma `NULL`, cero rutas o más retornos que rutas devuelven -1. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion17.h y no lo puedo cambiar:

    int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion,
                             int rutas_totales, int rutas_con_return);

ROL: Actúa como ingeniero de compiladores que comprueba las sentencias return en el
análisis semántico.

ESPECIFICACIÓN:
1. Devuelve 0 si todo está bien, 1 si el tipo es incompatible y 2 si queda una ruta
   de ejecución sin return.
2. La jerarquía es char < short < int < long < float < double. La conversión solo
   se acepta cuando el tipo de la función es igual o más ancho que el de la
   expresión.
3. Devolver un valor en una función void es siempre un error de tipo.
4. Una firma NULL o un conteo incoherente (cero rutas, rutas negativas o más
   retornos que rutas) devuelve -1.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion17.h"
#include <stddef.h>
#include <string.h>

enum { RET_OK = 0, RET_TIPO = 1, RET_RUTA = 2, RET_INVALIDO = -1 };

/* Jerarquia de ensanchamiento: char < short < int < long < float < double. */
static const char *const JERARQUIA[] = { "char", "short", "int", "long", "float", "double" };

/* O(T). Posicion en la jerarquia, o -1 si no es un tipo numerico conocido. */
static int nivel(const char *tipo) {
    for (int i = 0; i < (int)(sizeof JERARQUIA / sizeof JERARQUIA[0]); i++) {
        if (strcmp(tipo, JERARQUIA[i]) == 0) {
            return i;
        }
    }
    return -1;
}

/* O(T). Primero valida los datos, luego el tipo y al final las rutas. */
int diagnosticar_retorno(const char *tipo_funcion, const char *tipo_expresion, int rutas_totales, int rutas_con_return) {
    if (tipo_funcion == NULL || tipo_expresion == NULL) {
        return RET_INVALIDO;
    }
    if (rutas_totales <= 0 || rutas_con_return < 0 || rutas_con_return > rutas_totales) {
        return RET_INVALIDO;
    }

    int funcion_void = (strcmp(tipo_funcion, "void") == 0);
    int expresion_void = (strcmp(tipo_expresion, "void") == 0);

    if (funcion_void || expresion_void) {
        if (funcion_void != expresion_void) {
            return RET_TIPO;
        }
    } else {
        int destino = nivel(tipo_funcion);
        int origen = nivel(tipo_expresion);
        if (destino < 0 || origen < 0) {
            return RET_INVALIDO;
        }
        if (destino < origen) {
            return RET_TIPO;
        }
    }

    if (rutas_con_return < rutas_totales) {
        return RET_RUTA;
    }
    return RET_OK;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Funciones sin retorno o retornos mixtos

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion17_c.c artesano_sesion17_c.c`:
```text
=== AUDITORIA SESION 17 - Validacion de retornos y rutas ===
Caso                               esperado:              obtenido:              resultado
[ 1] int con int y dos rutas            esperado: 0            obtenido: 0            OK
[ 2] void devuelve valor                esperado: 1            obtenido: 1            OK
[ 3] ruta sin retorno                   esperado: 2            obtenido: 2            OK
[ 4] double acepta int                  esperado: 0            obtenido: 0            OK
[ 5] int no acepta double               esperado: 1            obtenido: 1            OK
[ 6] cero rutas de ejecucion            esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion17.md
```

La celda con `ia_sesion17_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, pero hay un caso que el harness no prueba y donde las dos versiones no coinciden: una función `void` que no tiene `return` en todas sus rutas. Lo probé con `("void", "void", 2, 0)`. Mi versión regresa 0 y la de la IA regresa 2, "ruta sin retorno". En C, una función `void` puede llegar al final sin `return` y es perfectamente válida, así que la IA marcaría como error a casi cualquier procedimiento.
>
> La causa es que la IA aplicó la regla de las rutas a todas las funciones por igual. El contrato dice "2 si queda una ruta sin retorno" y la IA lo tomó literal, sin pensar que para `void` no hace falta. Mi versión regresa antes de revisar las rutas cuando la función es `void`. Para que la IA lo hiciera bien, la especificación tendría que decir que las rutas solo se revisan en funciones que devuelven un valor; me faltó escribirlo en el prompt.
>
> En lo demás coincidieron: una función `int` que no devuelve valor da 1, `char` no acepta `int` (1), un tipo desconocido como `bool` da -1, y con el tipo mal y además una ruta sin `return`, las dos regresan 1 porque revisan primero el tipo.

---
**Firma digital:** OscardMQ
