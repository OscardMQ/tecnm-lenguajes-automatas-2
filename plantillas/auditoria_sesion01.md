# Bitácora de Requerimientos y Auditoría IA — Sesión 01
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Encuadre, Diagnóstico de Punteros y Metodología IA  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** [Nombre Completo]  
**Matrícula:** [No. de Control]  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Inversión de buffer in-place (char*)  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Al inicio la verdad no entendia bien las instrucciones pero fui leyendo y le fui entendiendo

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C99 / GCC o Rust 2021 sin bibliotecas no estándar |
| **Rol (R)** | Ingeniero de software de sistemas y compiladores |
| **Especificación (E)** | Punteros, O(n) tiempo, O(1) memoria |
| **Output (O)** | Solo código modular y análisis de complejidad |

### 2.2 Prompt Estructurado
```text
CONTEXTO:
Estoy en un laboratorio de Lenguajes y Autómatas II. Necesito una función en C11
que se compila junto con un programa de pruebas usando:
gcc -std=c11 -Wall -Wextra -Werror. Cualquier advertencia cuenta como error.

ROL:
Actúa como un programador experto en C y manejo seguro de memoria.

ESPECIFICACIÓN:
- Firma exacta, sin cambiarla: void invertir(char *s);
- Debe invertir la cadena en su mismo espacio de memoria (in-place).
- Prohibido usar malloc o arreglos auxiliares: solo dos índices y un char temporal.
- Si recibe NULL, debe terminar sin fallar.
- Debe funcionar con cadena vacía, de 1 carácter y de longitud par e impar.
- No incluyas main().
- Debe incluir #include "api_sesion01.h".

OUTPUT:
Devuélveme únicamente el código del archivo .c, sin explicaciones.
```

---

## 3. Código Devuelto por el Modelo
```c# include "api_sesion01.h"
# include <stddef.h>

void invertir(char *s) {
    if (s == NULL) {
        return;
    }

    int i = 0;
    int j = 0;
    char temp;

    while (s[j] != '\0') {
        j++;
    }

    if (j == 0) {
        return;
    }
    j--;

    while (i < j) {
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        i++;
        j--;
    }
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Valores NULL, buffers vacios y longitudes impares

### Salida del harness
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

* [ ] Caso base y límites superados sin *Crash*.
* [ ] No existen accesos fuera de límites ni *Memory Leaks*.
* [ ] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> Ninguno, el prompt que le brinde me dio el codigo solo me hizo falta especificar algunas cosas
> pero se solucionaron
---
**Firma digital:** [OscardMQ]
