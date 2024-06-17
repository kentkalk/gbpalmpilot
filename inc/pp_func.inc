; Project Palmpilot
; pp_func.asm
; More game-specific functions will be put here

SECTION "Game Functions", ROM0

; ----------------
; Tilemap Routines
; ----------------

XYtoTblByte:
    ; Calculates byte offset given a tile X, Y value
    ; BC - tile location (XY) in 20x14 tiles
    ; returns DE - offset
    ; This is used to get to the correct byte in a 20x14 (280 byte) screen collision map
    ; Essentially this calculates (20*Y) + X by doing (4*Y) + X + (16*Y)
    ld d, 0     ;clear D
    sla c
    sla c
    ld a, c
    add b       ;A is the (4*Y), add the X
    sla c
    sla c
    add c       ;add the (16*Y) to A
    ld e, a
    ret nc      ;it's possible to overflow as the largest offset value is $117
    inc d       ;only get here if there is an overflow, and can only be by 1 byte
    ret

XYtoMapByte:
    ; Calculates byte offset given a tile X, Y value
    ; BC - tile location (XY) in 32x32 tiles
    ; returns DE - offset
    ; This is used just like the screen table function but to get the byte offset of the 32x32 tilemap
    ; It just calculates (32*Y) + X
    xor a       ;Need A at 0 for the shifting
    ld l, b     ;Go ahead and put B in HL
    ld b, a     ;Clear some more registers while A is 0
    ld h, a  
    sla c       ;2*Y, max here is 62
    sla c       ;4*Y, max here is 124
    sla c       ;8*Y, max here is 248
    sla c       ;16*Y, max here is 496, so need to rotate into A
    rla
    sla c       ;32*Y, max here is 992
    rla
    ld b, a     ;BC = 32*Y
    add hl, bc  ;HL = 32*Y + X
    push hl
    pop de
    ret 




    

Coord2Addr:
    ; FUTURE - DEPRECATE
    ; Converts X and Y coordinates to a tilemap location
    ; offset by the scroll, such that the coordinates apply to the actual on-screen tile
    ; BC - coordinates (YX)
    ; returns destination address in HL

    ld h, HIGH(_SCRN0)  ;if in future want to use on the other tilemap this can be re-designed, but for now only works on SCRN0, also only need the top byte so save 4 cycles

    ; work with X first
    srl c               ;divide C by 8, also rounds down to the nearest tile
    srl c
    srl c
    ld a, l
    add c
    ld l, a

    ; now do Y
    ldh a, [rSCY]        ;start with the scroll offset, because this game only scrolls vertically we don't do this on X
    add b               ;add the screen position to the offset
    and %11111000       ;chop off the last 3 bits (rounds down to nearest tile)
    ld e, a
    xor a               ;set to 0
    sla b               ;the next instructions essentially do the following ([Y-coord]/8)*32, because every row is 32 bytes, simplified to 4*y done by two shifts
    rla                 ;equation is simplified to (4*[Y-coord]) and the RLA allows it to be a 16-bit number, as it rotates the carry bits into the top byte (A)
    sla b
    rla
    ld d, a             ;at this point DE represents the full 16-bit representing the rows

    ; create the HL output
    add hl, de          ;16-bit add, HL already has the x-offset address in it, just add DE from the Y portion

    ret



;----------------
;Player Functions
;----------------

MoveMegaSprite:
    ; Moves grouped sprites as one unit
    ; BC - new top left coordinate (YX)
    ; D - sprite count (4 bits, 4 unused XXHHXXWW, this matches the first byte in the sprite variable table)
    ; HL - first sprite address
    ld a, d
    and %00000011
    ld e, a
    ldh [tmpWVal], a
    ld a, d
    swap a
    and %00000011
    ld d, a
    ld a, c
    ldh [tmpcVal], a
    ; D is the height and E is the width now
    .heightloop
        .widthloop
            ld [hl], b
            inc l
            ld [hl], c
            inc l
            inc l
            inc l
            ld a, c
            add 8
            ld c, a
            dec e
            jr nz, .widthloop
        ldh a, [tmpWVal]
        ld e, a
        ldh a, [tmpcVal]
        ld c, a
        ld a, b
        add 8
        ld b, a
        dec d
        jr nz, .heightloop        
    ret


;---------------------
;General NPC Functions
;---------------------

GetNextNPCAddr:
    ; Checks sprite variable table for the first unused sprite and returns its address in hl
    ; Does this by checking the bit 0 of the first byte in the attribute table
    ld a, %01011111                         ;Complement of ($9F+1), used to check when at end of table
    ;ld hl, tblSprVars + PLAYER_WIDTH + PLAYER_HEIGHT - 1     ;Skip the player sprites, but start minus 1 because this will get incremented
.checknext
    inc l
    and l
    jr z, .badend
    ld b, [hl]
    bit 0, b
    jr nz, .checknext
    ret
.badend     ;Get here if the sprite table has no available, will just return the last address in the table
    dec l
    ret

AddNPC:
    ; Add an NPC to the sprite and variable tables
    ; b - attribute byte
    ; c - health and score data
    ; de - code location
    ; hl - NPC table location (typically will run GetNextNPCAddr immediately prior to this)
    ld a, l         ;Store l to put back at end of function
    ld [hl], b
    inc l
    ld [hl], c
    inc l
    ld [hl], d
    inc l
    ld [hl], e
    ld hl, sprCache     ;Will typically want to set the sprite itself next, so going ahead and setting hl while data is loaded
    ld l, a
    ret

SECTION "pp_func HRAM", HRAM
    tmpWVal:    DS 1    ;Temporary value used by MoveMegaSprite function
    tmpcVal:    DS 1    ;Temporary value used by MoveMegaSprite function


; ---------------
; V-Blank Routine
; ---------------

SECTION "V-Blank HRAM", HRAM
    dmaHRAM:    DS 10   ;Sprite DMA code block

SECTION "V-Blank Code", ROM0[$1000]
    
;DMA ROUTINES
    DMA_CopySubr:
        ; Copy sprite cache via DMA
        ; To be run by interrupt during V-blank
        ld a, HIGH(sprCache)
        ld [rDMA], a
        ld a, 40
        .wait
            dec a
            jr nz, .wait
        ret
    .end

    VBIHandler:
    ; VBlank Interrupt handler
    ; dmaHRAM should be defined as the location for the DMA copy subroutine to be placed
    call dmaHRAM
    reti

