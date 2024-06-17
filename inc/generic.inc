; Generic Functions Include
; generic.inc
; Copyright (C) 2021 Kent Kalkbrenner
;
; This contains more generic functions that could be used in any game build.

;===============
;HEADER SECTIONS
;===============
; May improve this later, but these three sections allow the cartridge header to stay blank
; This is currently filled in by RGBFIX
SECTION "Header", ROM0[$0000]
    hdrspace: DS 64
SECTION "Header2", ROM0[$0043]
    hdrspace2: DS 189
SECTION "Header3", ROM0[$104]
    hdrspace3: DS 76

SECTION "VBlankInt", ROM0[$0040]
    jp VBIHandler

SECTION "Generic.inc HRAM", HRAM
    joyPrev:    DS 1    ;Joypad variables
    joyPress:   DS 1
    joyState:   DS 1

SECTION "Sprite Cache", WRAM0, ALIGN[8]
    ; Sprite Cache to be loaded via DMA
    sprCache:   DS 160
    

;=============
;CODE SECTIONS
;=============
SECTION "Generic Functions", ROM0

    ;IO ROUTINES
    ReadInput:
        ; Read input from buttons and joypad into HRAM variables joyPress and joyState
        ; output bits:
        ; | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
        ; |St |Sl | B | A | D | U | L | R |
        ; 0 if up, 1 if down
        ; joyState - current state
        ; joyPress - only when released and re-pressed
        ld a, P1F_GET_BTN       ;buttons
        ld [rP1], a
        ld a, [rP1]             ;waste cycles
        ld a, [rP1]
        cpl                     ;invert
        and $0F                 ;keep first 4 bits
        swap a                  ;swap to last 4
        ld b, a                 ;put in B
        ld a, P1F_GET_DPAD      ;joypad
        ld [rP1], a
        ld a, [rP1]             ;waste more cycles
        ld a, [rP1]
        ld a, [rP1]
        ld a, [rP1]
        ld a, [rP1]
        ld a, [rP1]
        cpl                     ;invert
        and $0F                 ;keep only first 4 bits
        or b                    ;combine A into B (buttons + joy in one byte)
        ldh [joyState], a
        ; now calculate the single-press values
        ld b, a
        ldh a, [joyPrev]        ;read old data
        xor b                   ;xor compare to B (current data)
        and b                   ;and to only keep bits that changed and were also still pressed, not checking for buttons being released
        ldh [joyPress], a
        ld a, b                 ;copy previous value from B to A
        ldh [joyPrev], a        ;store the previous value
        ; reset joypad
        ld a, P1F_GET_NONE
        ld [rP1], a
        ret


    ; MEMORY MOVEMENT ROUTINES
    CopyRange:
        ; Copy memory range
        ; HL - source address
        ; DE - destination address
        ; BC - length
        inc b
        inc c
        jr .skip
        .copy
            ld a, [hl+]
            ld [de], a
            inc de
        .skip
            dec c
            jr nz, .copy
            dec b
            jr nz, .copy
            ret

    FillRange:
        ; Fill memory range
        ; A - data to fill with
        ; HL - start address
        ; BC - length
        inc b
        inc c
        jr .skip
        .fill
            ld [hl+], a
        .skip
            dec c
            jr nz, .fill
            dec b
            jr nz, .fill
            ret


    ; OAM (SPRITE) ROUTINES
    SetSprite:
        ; BC - screen location (YX)
        ; D - tile ID number
        ; E - attributes
        ; HL - sprite cache address
        ld [hl], b
        inc l
        ld [hl], c
        inc l
        ld [hl], d
        inc l
        ld [hl], e
        inc l
        ret