; NASM x86-64, Windows x64 calling convention
;
; void imgCvtGrayFloatToInt(const float *input,
;                           uint8_t *output,
;                           size_t count);
;
; RCX = input pointer
; RDX = output pointer
; R8  = pixel count
;
; Required scalar SIMD registers/instructions used:
; XMM0, XMM1
; MOVSS, MULSS, ADDSS, MINSS, MAXSS, CVTTSS2SI

bits 64
default rel

section .rdata
align 4
scale255: dd 255.0
half:     dd 0.5
zero:     dd 0.0
one:      dd 1.0

section .text
global imgCvtGrayFloatToInt

imgCvtGrayFloatToInt:
    xor r9, r9

.loop:
    cmp r9, r8
    jae .done

    ; Load one single-precision float pixel.
    movss xmm0, [rcx + r9*4]

    ; Clamp input into the valid [0.0, 1.0] range.
    movss xmm1, [zero]
    maxss xmm0, xmm1
    movss xmm1, [one]
    minss xmm0, xmm1

    ; integerPixel = round(floatPixel * 255.0)
    mulss xmm0, [scale255]
    addss xmm0, [half]
    cvttss2si eax, xmm0

    ; Store the low 8 bits as uint8_t.
    mov [rdx + r9], al

    inc r9
    jmp .loop

.done:
    ret
