; A neat fibonacci program in pseudo-assembly
; Program instructions
$0x00
OUT 0xE0  ; Output a,b
OUT 0xE1
LDA 0xF1
MBA
LDA 0xF0
SUB
STA 0xE4  ; imax = 10 - 1
LDA 0xE3  ; While loop entry
MBA
LDA 0xE4
SUB
JN  0x1C  ; While exit
LDA 0xE1
MBA
LDA 0xE0
ADD
STA 0xE2  ; c = a + b
OUT 0xE2  ; Output c
LDA 0xE1
STA 0xE0  ; a = b
LDA 0xE2
STA 0xE1  ; b = c
LDA 0xF1
MBA
LDA 0xE3
ADD
STA 0xE3  ; i++
JMP 0x07  ; Check while condition
HLT

; Volatile values
$0xE0
0x1     ; a
0x1     ; b
0x0     ; c
0x0     ; i
0x0     ; imax

; Constants
$0xF0
0xA     ; const 10
0x1     ; const 1
