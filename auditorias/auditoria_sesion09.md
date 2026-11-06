# Bitácora de Requerimientos y Auditoría IA — Sesión 09
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Reporte Formal de Errores Semánticos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Diagnóstico con línea y columna  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Esta fue la más corta, porque `snprintf` hace casi todo: arma el texto con el formato, nunca escribe más de `cap - 1` caracteres y siempre pone el `'\0'`. Con `cap = 10`, el harness espera `"main.c:14"`, que son 9 caracteres más el nulo.
>
> Lo que sí tuve que revisar fue el tipo de `cap`. El contrato lo pide como `int`, pero `snprintf` recibe `size_t`. Si `cap` llega negativo y lo convierto directo, -1 se vuelve un número enorme y `snprintf` creería que tiene todo el espacio del mundo. Por eso reviso `cap <= 0` antes de convertir.
>
> También cubrí `archivo` y `mensaje` en `NULL`. Pasar `NULL` a `%s` es comportamiento indefinido. Algunas bibliotecas imprimen `(null)`, pero no es algo con lo que se pueda contar.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion09.h`: `void reportar(const char *archivo, int linea, int columna, const char *mensaje, char *salida, int cap)`. |
| **Rol (R)** | Ingeniero de compiladores encargado de los diagnósticos del frontend. |
| **Especificación (E)** | Estructura estándar de errores GCC: `archivo:linea:columna: error: mensaje`, sin espacios extra ni punto final; nunca más de `cap-1` caracteres; el resultado siempre termina en `'\0'`; `salida` en `NULL` o `cap <= 0` no se cae. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion09.h y no lo puedo cambiar:

    void reportar(const char *archivo, int linea, int columna,
                  const char *mensaje, char *salida, int cap);

ROL: Actúa como ingeniero de compiladores encargado de los diagnósticos del frontend.

ESPECIFICACIÓN: Implementa reportar() con la firma exacta.
1. Escribe en salida un diagnóstico con el formato exacto de GCC:
   archivo:linea:columna: error: mensaje
   Ejemplo: main.c:14:5: error: tipo incompatible
2. Sin espacios extra ni punto final.
3. El buffer es del que llama: nunca escribas más de cap-1 caracteres. Si no cabe,
   corta el texto.
4. El resultado siempre termina en '\0'.
5. Con salida NULL o cap <= 0, no hagas nada.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion09.h"
#include <stdio.h>
#include <stddef.h>

/* O(L), L = largo del diagnostico. El mensaje entra como argumento de %s,
 * nunca como formato, asi que un '%' dentro del mensaje no se interpreta. */
void reportar(const char *archivo, int linea, int columna, const char *mensaje, char *salida, int cap) {
    if (salida == NULL || cap <= 0) {
        return;
    }

    const char *arch = (archivo != NULL) ? archivo : "<desconocido>";
    const char *msg = (mensaje != NULL) ? mensaje : "";

    int n = snprintf(salida, (size_t)cap, "%s:%d:%d: error: %s", arch, linea, columna, msg);
    if (n < 0) {
        salida[0] = '\0';
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Múltiples errores sin abortar parser

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion09_c.c artesano_sesion09_c.c`:
```text
=== AUDITORIA SESION 09 - Diagnostico con linea y columna ===
Caso                               esperado:              obtenido:              resultado
[ 1] main.c:14:5
     esperado: main.c:14:5: error: tipo incompatible
     obtenido: main.c:14:5: error: tipo incompatible
     -> OK
[ 2] a.c:1:1
     esperado: a.c:1:1: error: x
     obtenido: a.c:1:1: error: x
     -> OK
[ 3] linea 0
     esperado: x.c:0:0: error: y
     obtenido: x.c:0:0: error: y
     -> OK
[ 4] cap=10 trunca                      esperado: main.c:14    obtenido: main.c:14    OK

RESULTADO: 4/4 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion09.md
```

La celda con `ia_sesion09_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los cuatro casos con la misma idea que yo: un solo `snprintf`. En su comentario explicó algo que vale la pena: el mensaje entra como argumento de `%s` y no como formato. Si alguien escribiera `snprintf(salida, cap, mensaje)` y el mensaje trajera un `%`, la función intentaría leer argumentos que no existen. En mi prueba extra pasé `"100% seguro"` y las dos versiones lo copiaron tal cual.
>
> Las dos versiones difieren cuando `archivo` es `NULL`. La mía deja el nombre vacío y la salida empieza con `":1:1:"`. La de la IA pone `"<desconocido>"`, que se entiende mejor, así que voy a usar esa idea en el proyecto.
>
> El caso que me preocupa es el truncado. Con `cap = 10` la salida queda en `"main.c:14"`: ya no dice qué error es, y la función no tiene cómo avisar porque es `void`. Además, el contrato acepta líneas negativas. Probé `linea = -1` y salió `"x.c:-1:-1: error: y"`, que no tiene sentido. Para el proyecto, `reportar` debería regresar la longitud necesaria, como `snprintf`, y validar que la línea y la columna no sean negativas.

---
**Firma digital:** OscardMQ
