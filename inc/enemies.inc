; Project Palmpilot
; enemies.res
; Enemy sprite and weapon resource definitions

; Sprite table range used for enemies
DEF ENEMY_SPR_BGN EQU $18
DEF ENEMY_SPR_END EQU $7F

; Temporary definitions
DEF J20_TILE1 EQU $0A

SECTION "Enemy Sprites", ROM0

Enemy_J20:
    DB $06,$06,$FF,$FF,$86,$F9,$46,$79
    DB $26,$39,$12,$1D,$08,$0F,$04,$07
    DB $02,$03,$02,$03,$09,$0E,$05,$06
    DB $03,$02,$00,$01,$00,$01,$01,$01
    .end

; ENEMY CODE
SECTION "Enemy Code", ROM0

; Enemy spawn functions
; BC - contains YX coordinates of where to spawn if wanted
;
; Enemy update functions
; D - contains contents byte 1 of the sprite variables, which is mainly important flags
; E - contains 2nd byte of sprite address, needed so the update code will know where to access data

; ***** START - J20 Fighter *****
; FUTURE - The spawn code in particular is a mess

DEF J20_MOVE_SPEED EQU 1

Spawn_J20:
ld hl, sprCache
ldh a, [ptrEnemySprite]
ld l, a
ld d, J20_TILE1
ld e, 0
call SetSprite
ld a, c
add 8
ld c, a
ld e, OAMF_XFLIP
call SetSprite
ld a, b
add 8
ld b, a
ld a, c
sub 8
ld c, a
ld d, J20_TILE1 + 1
ld e, 0
call SetSprite
ld a, c
add 8
ld c, a
ld e, OAMF_XFLIP
call SetSprite

ld hl, tblSprVars
ldh a, [ptrEnemySprite]
ld l, a
ld a, %10100010
ld [hl], a
inc l
inc l
ld bc, Update_J20
ld [hl], b
inc l
ld [hl], c

;ld hl, tblSprStor
;ldh a, [ptrEnemySprite]
;ld l, a
;inc l
;ld [hl], 20
ret

Update_J20:
    ; Called by the enemy loop when a sprite of this type is active
    ld h, HIGH(sprCache)
    ld l, e                     ; E has the first byte of the sprite in it at the moment
    ld b, [hl]                  ; B - current Y value, this stays at whatever level the enemy was spawned
    ; This enemy is designed to follow the player horizontally, so we need to set the player as the destination
    ldh a, [plyrWpn1X]
    and %11111000               ; Want to move to nearest tile, background-based weapons fire at a tile position
    add 4                       ; Offset from the player to align with the center
    ld c, a                     ; C - destination X value
    inc l
    ld a, [hl]                  ; A - current X value

    ; Now move in a direction
    cp c
    jr c, .moveright            ; cur-X (A) < dest-X (C)
    jr z, .nomove               ; cur-X (A) == dest-X (C)
    .moveleft                   ; cur-X (A) > dest-X (C)
        sub J20_MOVE_SPEED  
        IF J20_MOVE_SPEED > 1       ; these instructions are only needed if enemy is faster than one per frame
            cp c                    ; see if moved too far
            jr nc, .nomove
            ld a, c                 ; if too far, replace with destination X from C
        ENDC
        jr .nomove
    .moveright                 
        add J20_MOVE_SPEED
        IF J20_MOVE_SPEED > 1
            cp c                    ; see if moved too far
            jr c, .nomove
            ld a, c                 ; if too far, just replace with the destination X from C
        ENDC
    .nomove
    ld c, a                     ; put current X (A) in destination X (C)

    ld h, HIGH(sprCache)        ; Need sprite cache again
    dec l                       ; Go back to byte 0
    call MoveMegaSprite
    jp EnemyLoop.return
; ***** END - J20 Fighter *****