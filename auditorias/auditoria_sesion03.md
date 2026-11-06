# Bitácora de Requerimientos y Auditoría IA — Sesión 03
**Materia:** Lenguajes y Autómatas II (SCD-1016)  
**Tema:** Google Colab, Mágicos y Flujo de Entrega  
**Docente Titular:** MCC. Iván Márquez Larios (`ijmarquezl`)  
**Estudiante:** Oscar Mauricio Matos Quintana  
**Matrícula:** 24390862  
**Track:** C (el harness evalúa C; la versión en Rust es opcional y no se califica con este harness)  

---

## 1. Fase 1: El Artesano (Dificultad Manual)
*Reto:* Escritura y conteo de líneas en archivos  
*Describe qué problema o restricción de memoria/sintaxis identificaste al codificar a mano:*
> Lo primero fue el tipo de la variable que lee. `fgetc` regresa `int` y no `char`, porque tiene que poder devolver cualquier byte y además `EOF`, que vale -1. Si guardo el resultado en un `char`, un byte `0xFF` se confunde con `EOF` y el conteo se corta antes de tiempo. Por eso `c` es `int`.
>
> Lo segundo fue la última línea. Si solo cuento los `'\n'`, `"a\nb"` da 1, pero son dos líneas. Guardo el último carácter que leí y, si al terminar no es `'\n'`, sumo una línea más. Lo inicio en `'\n'` para que el archivo vacío dé 0 y no 1.
>
> Lo tercero fue cerrar el archivo en todos los caminos. Si `fopen` falla regreso -1 sin cerrar nada, porque no hay nada abierto. Si se abrió, siempre llega a `fclose`. En `escribir_texto` también reviso lo que regresa `fclose`, porque ahí se vacía el buffer al disco y también puede fallar.

---

## 2. Fase 2: El Científico (Ingeniería de Requerimientos C-R-E-O)

### 2.1 Matriz de Especificación Formal
| Elemento C-R-E-O | Definición Técnica |
| :--- | :--- |
| **Contexto (C)** | C11 compilado con `gcc -std=c11 -Wall -Wextra -Werror`, solo biblioteca estándar. El archivo se enlaza con un harness externo que ya tiene `main()`. Las funciones son un paso de un pipeline: el paso siguiente decide si continúa según el código de retorno. Contrato en `api_sesion03.h`: `int escribir_texto(const char *ruta, const char *texto)` e `int contar_lineas(const char *ruta)`. |
| **Rol (R)** | Ingeniero de sistemas que escribe herramientas de línea de comandos para un pipeline de compilación. |
| **Especificación (E)** | Salida estándar y códigos de retorno ABI: 0 si `escribir_texto` escribió todo, -1 si no se pudo abrir o escribir. `contar_lineas` devuelve -1 si el archivo no existe, 0 si está vacío, y cuenta la última línea aunque no termine en `'\n'`. El archivo se cierra en todos los caminos. |
| **Output (O)** | Solo el código de las dos funciones, sin `main()`, con su complejidad. |

### 2.2 Prompt Estructurado
```text
CONTEXTO: Programo en C11 y compilo con gcc -std=c11 -Wall -Wextra -Werror, así que
cualquier advertencia del compilador rompe la compilación. Solo puedo usar la
biblioteca estándar. Mi archivo se enlaza con un harness externo que ya tiene su
propio main(), por lo tanto mi código no debe declarar main().
Estas funciones son un paso de un pipeline de
compilación: el paso siguiente decide si continúa o se detiene según el código
de retorno. El contrato está en api_sesion03.h y no lo puedo cambiar:

    int escribir_texto(const char *ruta, const char *texto);
    int contar_lineas(const char *ruta);

ROL: Actúa como ingeniero de sistemas que escribe herramientas de línea de comandos
para un pipeline de compilación en Unix.

ESPECIFICACIÓN: Implementa las dos funciones con las firmas exactas.
1. escribir_texto crea el archivo (o lo reemplaza si ya existe) y escribe el texto
   completo. Devuelve 0 si todo salió bien y -1 si no se pudo abrir o escribir.
2. contar_lineas devuelve cuántas líneas tiene el archivo, o -1 si no se puede
   abrir. Nunca debe abortar el programa.
3. Una última línea sin '\n' también cuenta: "a\nb" son 2 líneas. Un archivo vacío
   tiene 0 líneas.
4. El archivo se cierra en todos los caminos, también cuando hay error.
5. Una ruta o un texto en NULL se reportan con -1.

OUTPUT: Devuelve únicamente el bloque de código C con los #include necesarios y sin
main(), seguido de una línea por función con su complejidad.
```

---

## 3. Código Devuelto por el Modelo
Modelo consultado: Claude (Anthropic), 6 de noviembre de 2026, con el prompt de la sección 2.2.

```c
#include "api_sesion03.h"
#include <stdio.h>
#include <string.h>

/* O(n), n = bytes del texto. Devuelve 0 si todo se escribio y el archivo
 * se cerro bien; -1 en cualquier otro caso. */
int escribir_texto(const char *ruta, const char *texto) {
    if (ruta == NULL || texto == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL) {
        return -1;
    }

    size_t largo = strlen(texto);
    size_t escritos = fwrite(texto, 1, largo, archivo);
    int error = (escritos != largo);

    if (fclose(archivo) != 0) {
        error = 1;
    }
    return error ? -1 : 0;
}

/* O(n), n = bytes del archivo. Una linea es una secuencia terminada en '\n'
 * o el ultimo fragmento no vacio sin '\n'. Devuelve -1 si no se puede abrir. */
int contar_lineas(const char *ruta) {
    if (ruta == NULL) {
        return -1;
    }

    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        return -1;
    }

    int lineas = 0;
    int en_linea = 0;
    int c;
    while ((c = fgetc(archivo)) != EOF) {
        if (c == '\n') {
            lineas++;
            en_linea = 0;
        } else {
            en_linea = 1;
        }
    }
    if (en_linea) {
        lineas++;
    }

    fclose(archivo);
    return lineas;
}
```

---

## 4. Fase 3: El Auditor (Resultados del Test Harness)
*Pruebas ejecutadas:* Archivo vacío, sin salto final y ruta inexistente

### Salida del harness
Celda `gcc -std=c11 -Wall -Wextra -Werror harness_sesion03_c.c artesano_sesion03_c.c`:
```text
=== AUDITORIA SESION 03 - Archivos: escribir y contar lineas ===
Caso                               esperado:              obtenido:              resultado
[ 1] escribir_texto(...)==0             esperado: 0            obtenido: 0            OK
[ 2] contar_lineas(3 lineas)            esperado: 3            obtenido: 3            OK
[ 3] contar_lineas(sin salto final)     esperado: 2            obtenido: 2            OK
[ 4] contar_lineas(archivo vacio)       esperado: 0            obtenido: 0            OK
[ 5] contar_lineas(inexistente)         esperado: -1           obtenido: -1           OK
[ 6] escribir_texto(ruta invalida)      esperado: -1           obtenido: -1           OK

RESULTADO: 6/6 pruebas superadas
TODO EN VERDE. Documenta tu auditoria en auditorias/auditoria_sesion03.md
```

La celda con `ia_sesion03_c.c` dio exactamente la misma salida, línea por línea, y las dos compilaron sin advertencias.

* [x] Caso base y límites superados sin *Crash*.
* [x] No existen accesos fuera de límites ni *Memory Leaks*.
* [x] La complejidad teórica coincide con la implementada.

### Reporte Crítico
*¿Qué caso borde o problema omitió la IA y cómo lo solucionaste?*
> La IA cumplió los seis casos. En `escribir_texto` usó `fwrite` y comparó los bytes escritos contra `strlen`, que es más estricto que mi `fputs`. En `contar_lineas` usó una bandera `en_linea` en lugar de guardar el último carácter. Las dos ideas dan el mismo resultado.
>
> Hice tres pruebas extra que el harness no tiene. Un archivo con saltos de Windows (`"uno\r\ndos\r\n"`) dio 2 en las dos versiones, porque cada línea sigue teniendo su `'\n'` y el `'\r'` cuenta como un carácter más. Un archivo que solo tiene `"\n"` dio 1, o sea, una línea vacía, y me parece correcto. Y con ruta o texto en `NULL` las dos regresan -1 en lugar de pasarle `NULL` a `fopen`.
>
> Lo que sí noté es que nuestro conteo no coincide con `wc -l` de Linux. `wc -l` cuenta saltos de línea, así que para `"a\nb"` dice 1 y el harness espera 2. No es un error, pero hay que tenerlo claro si algún día comparamos la salida del compilador contra herramientas de Unix. Otro detalle es que `contar_lineas` regresa `int`, así que un archivo con más de 2,147,483,647 líneas se desbordaría. Con archivos fuente eso no pasa, por eso lo dejé así.

---
**Firma digital:** OscardMQ
