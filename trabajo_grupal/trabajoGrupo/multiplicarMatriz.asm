
section .text
    global multiplicarMatriz

multiplicarMatriz:
; matriz RDI
; vector RSI
; resultado RDX
; filas ECX
; columnas R8D
push r12;guardamos el valor de r12
mov r9d, 0; Iniciamos el contador de filas a 0
mov r10, [rdi];Hacemos que r10 apunte a matriz[0]
_mainLoop:
    mov rax, 0; Inicamos el contador de columnas a 0
    mov r11, 0; Iniciamos el sumatorio a 0
    push rsi; Guardamos el valor de la variable rsi en la pila
    _rowLoop:
        mov r12d, [rsi]; leemos los primeros 32 bits de la dirección rsi en r12d
        imul r12d, [r10]; multiplicamos r12d por la dirección de r10
        add r11d, r12d; sumamos el valor de r12d al sumatorio
        inc eax; aumentamos el contador y los punteros
        add r10, 4
        add rsi,4
        cmp eax, r8d; si eax es menor que r8d sigue el bucle
        jl _rowLoop
    pop rsi;recuperamos el valor de rsi para la proxima iteración
    mov [rdx], r11d;guardamos el valor del sumatorio en el vector de salida
    add rdx,4;aumentamos la posición del puntero
    inc r9d;aumentamos el contador de filas
    cmp r9d, ecx ;si el contador de filas es menor que ecx seguimos en el bucle
    jl _mainLoop

pop r12;recuperamos el valor de r12
ret




