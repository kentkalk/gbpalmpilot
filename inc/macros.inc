; Project Palmpilot
; macros.inc
; RGBDS Macros

MACRO Bdiv8
    ; These are just simple division, used to divide by 8 for conversion from screen coordinates to tile locations
    srl b
    srl b
    srl b
ENDM

MACRO Cdiv8
    ; Same as Bdiv8 but with C
    srl c
    srl c
    srl c
ENDM

MACRO CScrollOffs
    ; Offsets C by the scroll value
    ldh a, [rSCY]
    add c
    ld c, a
ENDM