# Bitácora de Requerimientos y Auditoría IA — Sesión 21
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Fundamentos de Código de 3 Direcciones  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Representación en Cuádruplos (op,a,b,r)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta es la primera sesión de código de tres direcciones. Lo primero fue el orden del contador: se usa y después se incrementa. Con el contador en 0, la primera llamada escribe `_t0` y lo deja en 1, así la siguiente ya es `_t1` y nunca se repite un temporal.
>
> Lo segundo fue que las funciones no imprimen, escriben en el buffer que les dan. Por eso usé `snprintf` con la capacidad, igual que en las sesiones 05 y 09, y nunca `printf` ni `malloc`.
>
> Lo tercero fue el `NULL` en el cuádruplo. Un `goto` no tiene segundo operando ni resultado, y el contrato pide escribir `_` en su lugar, no dejar el hueco vacío. Pasarle `NULL` a `%s` es comportamiento indefinido, así que cada campo pasa por `campo ? campo : "_"` antes de llegar a `snprintf`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion21.h`: `void nuevo_temporal(char *dst, int cap, int *contador)` y `void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1, const char *a2, const char *res)`. |
| **Rol (R)** | Ingeniero de compiladores que genera código intermedio de tres direcciones. |
| **Especificación (E)** | Fundamentos de código de 3 direcciones: `nuevo_temporal` escribe `_tN` con el valor actual del contador y después lo incrementa (la primera vez da `_t0`); `emitir_cuadruplo` escribe `(op, a1, a2, res)` con `_` en cada campo `NULL`; las dos escriben en `dst` respetando `cap`, sin reservar memoria ni usar globales. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion21.h y no lo puedo cambiar:

    void nuevo_temporal(char *dst, int cap, int *contador);
    void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1,
                          const char *a2, const char *res);

ROL: Actúa como ingeniero de compiladores que genera código intermedio de tres
direcciones.

ESPECIFICACIÓN:
1. nuevo_temporal escribe en dst el nombre "_tN", con N igual al valor actual de
   *contador, y después incrementa el contador. La primera llamada con el contador
   en 0 produce "_t0".
2. emitir_cuadruplo escribe en dst el cuádruplo con el formato "(op, a1, a2, res)".
   Por ejemplo: (+, a, b, _t0).
3. Un argumento NULL se escribe como guion bajo: (goto, L1, _, _). No se omite ni
   se inventa.
4. Las dos funciones escriben siempre en el buffer recibido, nunca más de cap - 1
   caracteres, sin reservar memoria ni usar variables globales.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion21.h"
#include <stdio.h>
#include <stddef.h>

/* O(1). Escribe "_tN" con N = *contador y luego incrementa el contador.
 * Si el nombre no cabe completo en dst, deja dst vacio y NO consume el
 * numero: un temporal recortado ("_t1" en lugar de "_t12") dejaria de
 * ser unico. */
void nuevo_temporal(char *dst, int cap, int *contador) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    dst[0] = '\0';
    if (contador == NULL || *contador < 0) {
        return;
    }

    int escritos = snprintf(dst, (size_t)cap, "_t%d", *contador);
    if (escritos < 0 || escritos >= cap) {
        dst[0] = '\0';
        return;
    }
    (*contador)++;
}

static const char *o_guion(const char *s) {
    return (s != NULL) ? s : "_";
}

/* O(L), L = largo del cuadruplo. Formato "(op, a1, a2, res)"; un campo
 * NULL se escribe como "_". Respeta cap y siempre termina en '\0'. */
void emitir_cuadruplo(char *dst, int cap, const char *op, const char *a1, const char *a2, const char *res) {
    if (dst == NULL || cap <= 0) {
        return;
    }
    int escritos = snprintf(dst, (size_t)cap, "(%s, %s, %s, %s)",
                            o_guion(op), o_guion(a1), o_guion(a2), o_guion(res));
    if (escritos < 0) {
        dst[0] = '\0';
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones aritméticas compuestas

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion21_c.c artesano_sesion21_c.c`:
```text
=== AUDITORIA SESION 21 - Cuadruplos y temporales unicos ===
Caso                               esperado:              obtenido:              resultado
[ 1] nuevo_temporal(_t0)                esperado: _t0          obtenido: _t0          OK
[ 2] nuevo_temporal(_t7)                esperado: _t7          obtenido: _t7          OK
[ 3] cuadruplo de a + b
     esperado: (+, a, b, _t0)
     obtenido: (+, a, b, _t0)
     -> OK
[ 4] cuadruplo de _t0 * c
     esperado: (*, _t0, c, _t1)
     obtenido: (*, _t0, c, _t1)
     -> OK
[ 5] asignacion simple                  esperado: (=, a, _, x) obtenido: (=, a, _, x) OK
[ 6] goto sin operandos
     esperado: (goto, L1, _, _)
     obtenido: (goto, L1, _, _)
     -> OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion21.md
```

La celda con `ia_sesion21_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos, y en `nuevo_temporal` cuidó algo que yo no había pensado: qué pasa si el nombre del temporal no cabe en el buffer. Lo probé con mi primera versión, con un buffer de 4 bytes. Con el contador en 12, `_t12` no cabe, así que `snprintf` lo cortó a `"_t1"` y el contador subió a 13. Después, con el contador en 1, salió otra vez `"_t1"`. O sea, dos temporales distintos quedaron con el mismo nombre, y eso rompe justo la regla de que los temporales son únicos.
>
> La IA revisa lo que regresa `snprintf`: si el nombre no cupo completo, deja el buffer vacío y no gasta el número del contador. Copié esa revisión a mi versión. Ahora, con el contador en 12 y buffer de 4, las dos dejan `""` y el contador se queda en 12. Un nombre vacío es fácil de detectar más adelante; uno repetido no.
>
> Lo que ninguna de las dos avisa es cuando el cuádruplo no cabe: con un buffer de 12, `(+, a, b, _t0)` queda como `"(+, a, b, _"` y la función es `void`. Como en las sesiones 05 y 09, convendría que regresara la longitud necesaria. También probé que un `op` en `NULL` sale como `_`: con todo en `NULL`, las dos escriben `(_, _, _, _)`.

---
**Firma digital:** OscardMQ
