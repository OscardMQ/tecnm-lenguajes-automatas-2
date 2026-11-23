# Bitácora de Requerimientos y Auditoría IA — Sesión 11
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Arquitectura de la Tabla de Símbolos  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Tabla Hash con encadenamiento  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue el desbordamiento en djb2. `h * 33 + c` se pasa de 32 bits muy rápido, pero como `h` es `unsigned int`, el resultado da la vuelta y está bien definido en C. Con `int` sería comportamiento indefinido. También convertí cada carácter a `unsigned char`, para que una letra con acento no sume un número negativo.
>
> Lo segundo fue entender que nunca se sobrescribe una cubeta. Si dos nombres caen en el mismo índice, el nuevo se encadena con `sig`. Yo lo puse al inicio de la cadena porque es O(1): el nodo nuevo apunta a lo que ya había y la cubeta apunta al nuevo.
>
> Lo tercero fue el campo `char nombre[16]`. Un nombre de 16 caracteres o más no cabe. Si lo corto, dos variables distintas pueden quedar iguales, como vi en la sesión 02. Preferí rechazarlo con -1, junto con el nombre vacío.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Contrato en `api_sesion11.h`: `struct Simbolo { char nombre[16]; int tipo; int offset; struct Simbolo *sig; }`, `hash_djb2()` e `insertar_simbolo()`. |
| **Rol (R)** | Ingeniero de compiladores que diseña la tabla de símbolos del analizador semántico. |
| **Especificación (E)** | Función de dispersión y colisiones O(1): djb2 (`h = h * 33 + c`, inicio 5381) sobre `unsigned int`; índice `hash % cubetas`; las colisiones se encadenan y nunca se sobrescriben; un nombre repetido devuelve 0 sin crear nodo; tabla `NULL`, nombre `NULL` o cero cubetas devuelven -1. |
| **Output (O)** | Solo el código de las dos funciones, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
El contrato está en api_sesion11.h y no lo puedo cambiar:

    typedef struct Simbolo {
        char nombre[16];
        int tipo;
        int offset;
        struct Simbolo *sig;
    } Simbolo;

    unsigned int hash_djb2(const char *clave);
    int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas,
                         const char *nombre, int tipo, int offset);

ROL: Actúa como ingeniero de compiladores que diseña la tabla de símbolos de un
analizador semántico.

ESPECIFICACIÓN:
1. hash_djb2 empieza en 5381 y por cada carácter hace h = h * 33 + c, sobre
   unsigned int. hash_djb2("") = 5381 y hash_djb2("abc") = 193485963.
2. insertar_simbolo calcula la cubeta con hash % cubetas.
3. Si el nombre ya existe, devuelve 0 y no crea ningún nodo.
4. Si dos nombres distintos caen en la misma cubeta, se encadenan con sig; nunca se
   sobrescribe una entrada.
5. Con tabla NULL, nombre NULL o cero cubetas devuelve -1. Si insertó, devuelve 1.
6. El nombre se copia al campo de 16 bytes y no puede desbordarlo.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de la complejidad de cada función.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 23 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion11.h"
#include <stdlib.h>
#include <string.h>

/* O(k), k = longitud de la clave. h = h * 33 + c, con aritmetica modular
 * de unsigned int (el desbordamiento esta definido por el estandar). */
unsigned int hash_djb2(const char *clave) {
    unsigned int h = 5381U;
    if (clave == NULL) {
        return h;
    }
    while (*clave != '\0') {
        h = ((h << 5) + h) + (unsigned char)*clave;
        clave++;
    }
    return h;
}

/* O(L) en promedio, L = largo de la cadena de la cubeta.
 * 1 = insertado, 0 = ya existia, -1 = argumentos invalidos o sin memoria. */
int insertar_simbolo(Simbolo *tabla[], unsigned int cubetas, const char *nombre, int tipo, int offset) {
    if (tabla == NULL || nombre == NULL || cubetas == 0U) {
        return -1;
    }

    size_t largo = strlen(nombre);
    if (largo == 0U || largo >= sizeof(((Simbolo *)0)->nombre)) {
        return -1;
    }

    unsigned int indice = hash_djb2(nombre) % cubetas;

    Simbolo **enlace = &tabla[indice];
    while (*enlace != NULL) {
        if (strcmp((*enlace)->nombre, nombre) == 0) {
            return 0;
        }
        enlace = &(*enlace)->sig;
    }

    Simbolo *nuevo = (Simbolo *)malloc(sizeof(Simbolo));
    if (nuevo == NULL) {
        return -1;
    }
    memcpy(nuevo->nombre, nombre, largo + 1U);
    nuevo->tipo = tipo;
    nuevo->offset = offset;
    nuevo->sig = NULL;
    *enlace = nuevo;
    return 1;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Manejo de claves duplicadas y colisiones

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion11_c.c artesano_sesion11_c.c`:
```text
=== AUDITORIA SESION 11 - Tabla hash con encadenamiento ===
Caso                               esperado:              obtenido:              resultado
[ 1] hash_djb2("")                      esperado: 5381         obtenido: 5381         OK
[ 2] hash_djb2("abc")                   esperado: 193485963    obtenido: 193485963    OK
[ 3] insertar x dos veces               esperado: 0            obtenido: 0            OK
[ 4] dos claves, una cubeta             esperado: 2            obtenido: 2            OK
[ 5] offset de beta en cadena           esperado: 4            obtenido: 4            OK
[ 6] insertar en tabla NULL             esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion11.md
```

La celda con `ia_sesion11_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

El contrato no incluye una función para liberar la tabla, así que los nodos que crea el harness no se liberan. Mi implementación no pierde ningún nodo que reserva.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. Calculó djb2 con `(h << 5) + h`, que es lo mismo que multiplicar por 33, y encadenó el nodo nuevo al final de la cadena con un puntero al enlace, como en la sesión 02. Yo lo puse al inicio. En mi prueba extra con tres nombres en una sola cubeta, la cadena de la IA quedó `alpha beta gamma` y la mía `gamma beta alpha`. El harness no revisa el orden, y la complejidad es la misma porque las dos versiones recorren la cadena completa para buscar duplicados.
>
> Lo que encontré viene del contrato: dice que un nombre repetido "en cualquier cubeta" devuelve 0. Las dos versiones solo revisan la cubeta donde cae el nombre, y eso funciona mientras `cubetas` sea siempre el mismo. Lo probé insertando `x` con 8 cubetas y luego otra vez con 4 en la misma tabla: `hash % 8` da 5 y `hash % 4` da 1, así que las dos versiones lo insertaron dos veces y la tabla quedó con dos nodos `x`.
>
> En una tabla real esto pasa cuando crece: si se cambia el número de cubetas, hay que volver a acomodar todos los nodos (rehash). No lo cambié, porque revisar todas las cubetas en cada inserción haría la búsqueda O(n) y le quitaría el sentido a la tabla hash. Lo dejo como regla: una tabla siempre se usa con el mismo número de cubetas.

---
**Firma digital:** OscardMQ
