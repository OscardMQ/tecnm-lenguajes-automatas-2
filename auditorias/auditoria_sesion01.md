# Bitácora de Requerimientos y Auditoría IA — Sesión 01
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Encuadre, Diagnóstico de Punteros y Metodología IA  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Inversión de buffer in-place (char*)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero que me costó fue el índice del final. Si la cadena mide `n`, el último carácter está en `n - 1` y no en `n`, porque en `s[n]` está el `'\0'`. Si muevo el nulo, la cadena se corta.
>
> También me di cuenta de que `strlen` regresa `size_t`, que no tiene negativos. Con la cadena vacía, `n - 1` no da -1, da un número enorme, y el ciclo se saldría del buffer. Por eso salgo antes cuando `n < 2`: la cadena vacía y la de un solo carácter ya están invertidas.
>
> Lo último fue `NULL`. `strlen(NULL)` truena, así que esa revisión va antes que todo lo demás.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato fijo en `api_sesion01.h`: `void invertir(char *s)`. |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores, especialista en punteros y manejo de memoria en C. |
| **Especificación (E)** | Punteros, O(n) tiempo y O(1) memoria: invertir en el mismo buffer sin `malloc` ni arreglos auxiliares, solo dos posiciones y un `char` temporal. El `'\0'` no se mueve. `invertir(NULL)` y la cadena vacía terminan sin caerse. |
| **Output (O)** | Solo el código de la función con sus `#include`, sin `main()`, y una línea con la complejidad en tiempo y memoria. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion01.h y no lo
puedo cambiar:

    void invertir(char *s);

ROL: Actúa como ingeniero de software de sistemas y compiladores, especialista en
punteros y manejo de memoria en C.

ESPECIFICACIÓN: Implementa invertir() para que la cadena quede al revés en su mismo
espacio de memoria. Requisitos obligatorios:
1. Nada de malloc ni arreglos auxiliares: solo dos posiciones y un char temporal.
2. Complejidad O(n) en tiempo y O(1) en memoria.
3. El terminador '\0' se queda en su lugar.
4. invertir(NULL) regresa sin hacer nada y sin caerse.
5. La cadena vacía y la de un solo carácter quedan iguales.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de una línea con la complejidad en tiempo y en memoria.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion01.h"
#include <stddef.h>

/* O(n) tiempo, O(1) memoria: dos punteros que avanzan uno hacia el otro
 * y se detienen al cruzarse en la mitad de la cadena. */
void invertir(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }

    char *izq = s;
    char *der = s;
    while (*(der + 1) != '\0') {    /* der queda sobre el ultimo caracter */
        der++;
    }

    while (izq < der) {
        char tmp = *izq;
        *izq = *der;
        *der = tmp;
        izq++;
        der--;
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Valores NULL, buffers vacios y longitudes impares

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion01_c.c artesano_sesion01_c.c`:
```text
=== AUDITORIA SESION 01 - Inversion de buffer in-place ===
Caso                               esperado:              obtenido:              resultado
[ 1] invertir("HOLA")                   esperado: ALOH         obtenido: ALOH         OK
[ 2] invertir("abc")                    esperado: cba          obtenido: cba          OK
[ 3] invertir("")                       esperado:              obtenido:              OK
[ 4] invertir("AB")                     esperado: BA           obtenido: BA           OK
[ 5] invertir("a")                      esperado: a            obtenido: a            OK
[ 6] invertir(NULL)                     esperado: sin crash    obtenido: sin crash    OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion01.md
```

La celda con `ia_sesion01_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Con el contrato completo, la IA no omitió ningún caso del harness. Revisó `NULL` y la cadena vacía en la misma condición. En lugar de `strlen`, buscó el último carácter con su propio ciclo. Sigue siendo O(n), y no tiene el problema del `size_t` que yo tuve que cuidar, porque nunca le resta 1 a una longitud cero.
>
> Lo que ni la IA ni yo podemos cubrir es que alguien llame `invertir("HOLA")` con una literal en lugar de un arreglo. La literal vive en memoria de solo lectura (en Linux, la sección `.rodata`), y escribir ahí es comportamiento indefinido: normalmente termina en *Segmentation fault*. El harness no lo prueba porque siempre usa arreglos (`char a[] = "HOLA"`), que sí son copias modificables en la pila.
>
> Desde adentro de la función no hay forma de saber si el buffer es de solo lectura. Por eso el contrato dice "cadena mutable" y esa responsabilidad queda del lado del que llama. Lo dejo anotado para cuando el compilador del proyecto tenga que modificar buffers.

---
**Firma digital:** OscardMQ
