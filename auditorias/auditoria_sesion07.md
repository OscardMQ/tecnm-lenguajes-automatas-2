# Bitácora de Requerimientos y Auditoría IA — Sesión 07
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Evaluación de Expresiones Aritméticas  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Evaluador recursivo de operaciones  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue el orden de las revisiones. El contrato dice que nunca se divide antes de revisar el divisor. Entonces evalúo los dos hijos, reviso si el derecho es 0 y solo después divido. Lo mismo con `%`, que también truena con 0.
>
> Lo segundo fue propagar el error. Uso una función `calcular` que recibe el mismo puntero `error` en toda la recursión. Después de cada hijo reviso si `error` cambió y, si cambió, regreso sin calcular nada más. Así el padre no opera con un valor que no sirve.
>
> El contrato no dice quién pone `error` en 0. Lo hago una sola vez en `evaluar`, antes de empezar, y no en cada nivel. Si cada nivel lo reiniciara, el error de un hijo se borraría al regresar al padre. También cubrí `error == NULL` con una variable local, para no escribir en `NULL`.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion07.h`: el mismo `ASTNode` y `int evaluar(ASTNode *n, int *error)`. |
| **Rol (R)** | Ingeniero de compiladores responsable del evaluador de expresiones constantes. |
| **Especificación (E)** | Manejo estricto de división entre cero: operadores `+ - * / %`; `*error` = 0 si todo bien, 1 si hay `/` o `%` entre cero, 2 si el nodo es `NULL` o inválido; el divisor se revisa antes de dividir; el error se propaga hacia arriba sin recalcular los padres; `evaluar(NULL, &e)` no se cae. |
| **Output (O)** | Solo el código, sin `main()`, con su complejidad en tiempo y en pila. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion07.h y no lo puedo cambiar:

    typedef enum { NODO_NUM, NODO_BINOP } TipoNodo;
    typedef struct ASTNode {
        TipoNodo tipo;
        int valor;
        char op;
        struct ASTNode *izq;
        struct ASTNode *der;
    } ASTNode;

    int evaluar(ASTNode *n, int *error);

ROL: Actúa como ingeniero de compiladores responsable del evaluador de expresiones
constantes.

ESPECIFICACIÓN: Implementa evaluar() con la firma exacta.
1. Calcula el valor del árbol con los operadores +, -, *, / y %.
2. Al terminar, *error vale 0 si todo salió bien, 1 si hubo una división o un
   módulo entre cero, y 2 si el árbol o algún nodo es NULL o inválido.
3. Nunca ejecutes la división antes de comprobar el divisor.
4. El error se propaga hacia arriba: en cuanto un hijo falla, los padres no
   vuelven a calcular nada.
5. evaluar(NULL, &e) no debe caerse.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad en tiempo y en pila.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion07.h"
#include <limits.h>
#include <stddef.h>

enum { EVAL_OK = 0, EVAL_DIV_CERO = 1, EVAL_NODO_INVALIDO = 2 };

/* O(n) en nodos, O(h) de pila. Se detiene en el primer error: el padre
 * no vuelve a calcular nada cuando un hijo ya fallo. */
static int evaluar_rec(const ASTNode *n, int *error) {
    if (n == NULL) {
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }

    switch (n->tipo) {
    case NODO_NUM:
        return n->valor;
    case NODO_BINOP:
        break;
    default:
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }

    int izq = evaluar_rec(n->izq, error);
    if (*error != EVAL_OK) {
        return 0;
    }
    int der = evaluar_rec(n->der, error);
    if (*error != EVAL_OK) {
        return 0;
    }

    switch (n->op) {
    case '+':
        return izq + der;
    case '-':
        return izq - der;
    case '*':
        return izq * der;
    case '/':
    case '%':
        if (der == 0) {
            *error = EVAL_DIV_CERO;
            return 0;
        }
        /* INT_MIN / -1 no cabe en int: en x86 provoca la misma excepcion
         * de hardware que dividir entre cero. */
        if (izq == INT_MIN && der == -1) {
            *error = EVAL_DIV_CERO;
            return 0;
        }
        return (n->op == '/') ? izq / der : izq % der;
    default:
        *error = EVAL_NODO_INVALIDO;
        return 0;
    }
}

int evaluar(ASTNode *n, int *error) {
    int local = EVAL_OK;
    if (error == NULL) {
        error = &local;
    }
    *error = EVAL_OK;
    return evaluar_rec(n, error);
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Expresiones complejas y división por 0

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion07_c.c artesano_sesion07_c.c`:
```text
=== AUDITORIA SESION 07 - Evaluador recursivo con deteccion de errores ===
Caso                               esperado:              obtenido:              resultado
[ 1] (2+3)*4                            esperado: 20           obtenido: 20           OK
[ 2] 7/0 -> error 1                     esperado: 1            obtenido: 1            OK
[ 3] 7%0 -> error 1                     esperado: 1            obtenido: 1            OK
[ 4] arbol NULL -> error 2              esperado: 2            obtenido: 2            OK
[ 5] 10-2-3                             esperado: 5            obtenido: 5            OK
[ 6] (8/2)+1                            esperado: 5            obtenido: 5            OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion07.md
```

La celda con `ia_sesion07_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos y además cubrió algo que yo no había considerado: `INT_MIN / -1`. El resultado sería 2,147,483,648, que no cabe en un `int`. En x86, la instrucción de división lanza la misma excepción de hardware que al dividir entre cero, y en Linux eso se ve como *Floating point exception*. Para comprobarlo, probé mi versión sin esa condición con `INT_MIN / -1` y el programa se cayó justo en la división: imprimió el mensaje que puse antes y nada más. Copié la condición a mi versión y la marqué como error 1, igual que la IA. Con eso, mi prueba extra dio `e = 1` para `INT_MIN / -1` y para `INT_MIN % -1` en las dos versiones.
>
> Lo que ninguna de las dos cubre es el desbordamiento en `+`, `-` y `*`. En mi prueba, `INT_MAX + 1` dio -2147483648 con error 0. En C eso es comportamiento indefinido: hoy da la vuelta, pero con optimizaciones el compilador puede hacer otra cosa. No lo corregí porque el contrato solo define los errores 1 y 2. Lo dejo anotado para el Hito 1, donde convendría agregar un código de error para desbordamiento.

---
**Firma digital:** OscardMQ
