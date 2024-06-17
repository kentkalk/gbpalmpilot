; Project Palmpilot
; palmpilot.asm
; v0.1 - early design and testing
; My first assembler and GB project, airplane based SHUMP game

; INCLUDES
INCLUDE "hardware.inc"
INCLUDE "generic.inc"
INCLUDE "macros.inc"
INCLUDE "pp_func.inc"
INCLUDE "player.inc"
INCLUDE "enemies.inc"

SECTION	"Start", ROM0[$100]
    nop
    jp start

SECTION "Program", ROM0[$150]
start:
 ; ==============
 ; INITIALIZATION
 ; ==============

    di

    ;Wait for V-blank
    .wait_vblank
        ld a, [rLY]	
        cp $90
    jr nz, .wait_vblank

    ; Reset special registers
    xor a
    ld [rIF], a
    ld [rLCDC], a
    ld [rSTAT], a
    ld [rSCX], a
    ld [rSCY], a
    ld [rLYC], a
    ld [rIE], a

    ; Zero-write all the RAM
    ; RAM $C000-$DFFF
    ld hl, _RAM
    ld bc, $2000
    call FillRange
    ; Initialize stack in RAM
    ld sp, $E000
    ; HRAM $FF80-$FFFE
    ld hl, _HRAM
    ld bc, $80
    call FillRange
    ; VRAM $8000-$9FFF
    ld hl, _VRAM
    ld b, $20
    call FillRange

    ; Load DMA code to HRAM
    ld hl, DMA_CopySubr
    ld de, dmaHRAM
    ld bc, DMA_CopySubr.end-DMA_CopySubr
    call CopyRange     

    ; Set initial variable values
    ld a, ENEMY_SPR_BGN
    ldh [ptrEnemySprite], a
    ; FUTURE - perhaps move to player loading section
    ld a, PLAYER_SPAWN_Y
    ldh [plyrWpn1Y], a
    ld a, PLAYER_SPAWN_X
    ldh [plyrWpn1X], a

    ; VRAM Loads
    ; ----------
    ; FUTURE - This section definitely needs improvement
    
    ; Font
    ld hl, Font
    ld de, _VRAM9000+$200   ;add $200 to get to ASCII mapping
    ld bc, 1776         ;file size
    call CopyRange

    ; Load player tiles into VRAM
    ld hl, C150Tiles
    ld de, _VRAM + 16 ;skip tile 0
    ld bc, C150Tiles.end - C150Tiles
    call CopyRange

    ; Load enemy tiles into VRAM
    ld hl, Enemy_J20
    ld de, _VRAM + 160  ;skip to end of player
    ld bc, Enemy_J20.end - Enemy_J20
    call CopyRange



; ============
; GAME LOADING
; ============

    ; Configure window and align to a one tile at bottom row
    ld a, 135
    ld [rWY], a
    ld a, 7
    ld [rWX], a

    ; Initialize background collision variables
    ;ld bc, tblBGColl
    ;ld a, b
    ;ldh [bgCollWRAM], a
    ;ld a, c
    ;ldh [bgCollPtr], a

    ; Set Palettes
    ld a, %01000011
    ld [rBGP], a
    ld a, %11100100
    ld [rOBP0], a
    ld [rOBP1], a

    ; Fill BG0 with single tile
    ld a, $1F
    ld hl, _SCRN0
    ld bc, $400
    call FillRange

    ld hl, BGTile
    ld de, _VRAM9000 + $1F0
    ld bc, 16
    call CopyRange

    ; Type title on Window
    ld hl, title
    ld de, _SCRN1
    ld c, title_end-title
    call CopyRange

    ;PLAYER
    ; Load the player
    ld hl, sprCache | PLAYER_SPRITE_ADDR
    ld b, PLAYER_SPAWN_Y
    ld c, PLAYER_SPAWN_X
    ld d, PLAYER_TLR_TILE
    ld e, 0
    call SetSprite
    ld c, (PLAYER_SPAWN_X + 8)
    ld d, PLAYER_TC_TILE
    call SetSprite
    ld c, (PLAYER_SPAWN_X + 16)
    ld d, PLAYER_TLR_TILE
    ld e, OAMF_XFLIP
    call SetSprite
    ld b, (PLAYER_SPAWN_Y + 8)
    ld c, PLAYER_SPAWN_X
    ld d, PLAYER_BLR_TILE
    ld e, 0
    call SetSprite
    ld c, (PLAYER_SPAWN_X + 8)
    ld d, PLAYER_BC_TILE
    call SetSprite
    ld c, (PLAYER_SPAWN_X + 16)
    ld d, PLAYER_BLR_TILE
    ld e, OAMF_XFLIP
    call SetSprite




    ; Enable VBlank
    ld a, IEF_VBLANK
    ld [rIE], a

    ; Enable LCD with settings
    ld a, LCDCF_ON | LCDCF_WIN9C00 | LCDCF_WINON | LCDCF_BG8800 | LCDCF_BG9800 | LCDCF_OBJ8 | LCDCF_OBJON | LCDCF_BGON
    ld [rLCDC], a

    ; Enable interrupts
    ei


    ; Temporary level stuff
    ld b, 30
    ld c, 70
    call Spawn_J20

; =========
; GAME LOOP
; =========

    GameLoop:
        halt                                        ;This is skipped when there's a V-blank interrupt, so it's necessary for timing
        ;nop - will be added by RGBFix

        ; ***** BEGIN PLAYER CODE *****
        call ReadInput                              ;Read joypad and update input variables
    
        ; PLAYER MOVEMENT
        ; ---------------
        ldh a, [joyState]
        and %00001111
        jr z, .skipmovement                         ;Check D-pad, if not pressed can save a ton of cycle
        ld d, a                                     ;D - joypad button state
        ld a, PLAYER_MOVE_SPEED                     
        ld e, a                                     ;E - player speed modifier
        ld hl, sprCache | PLAYER_SPRITE_ADDR      ;HL - sprite being manipulated

        ld b, [hl]                                  ;B - current Y position
        inc l
        ld c, [hl]                                  ;B - current X position
        dec l                                       ;Put L back at the Y position as it's expected to be there

        ; Read Y input and re-direct
        bit 2, d
        jr nz, .moveup
        bit 3, d
        jr nz, .movedown
        jr .xinput                                  ;Skip to X check if no up-down direction
        
        .moveup
            ld a, b
            sub e
            ld b, a                                 ;B is now the new Y-value
            ld a, PLAYER_TOP_BOUND                  ;Now keep player on screen
            cp b
            jr c, .xinput                           ;If B > the top bound then skip, otherwise replace with the top bound
            ld b, a
            jr .xinput
        .movedown
            ld a, b
            add e
            ld b, a                                 ;B is now the new Y-value
            ld a, PLAYER_BOTTOM_BOUND               ;Now keep player on screen
            cp b
            jr nc, .xinput                          ;If B < the bottom bound then skip, otherwise replace with the bottom bound
            ld b, a

        ; Read X input and re-direct
        .xinput
        bit 0, d
        jr nz, .moveright
        bit 1, d
        jr nz, .moveleft
        jr .endmovement                             ;Skip to setting position if no left-right direction

        .moveright
            ld a, c
            add e
            ld c, a                                 ;C is now the new Y-value
            ld a, PLAYER_RIGHT_BOUND                ;Now keep player on screen
            cp c
            jr nc, .endmovement                     ;If C < right bound then skip, otherwise replace with the right bound
            ld c, a
            jr .endmovement
        .moveleft
            ld a, c
            sub e
            ld c, a                                 ;C is now the new Y-value
            ld a, PLAYER_LEFT_BOUND                 ;Now keep player on screen
            cp c
            jr c, .endmovement                      ;If C > left bound then skip, otherwise replace with left bound
            ld c, a
        .endmovement
        
        ; Update player location (currently in BC)
        ld a, b
        ldh [plyrWpn1Y], a
        ld a, c
        ldh [plyrWpn1X], a
        ld hl, sprCache | PLAYER_SPRITE_ADDR
        ld d, PLAYER_SPRITE_SIZE                    ;D - special constant for player sprite dimensions
        call MoveMegaSprite
        
        .skipmovement                               ;Will be jumped to here if D-pad is not pressed

        ; BUTTON PRESS ACTIONS
        ; --------------------
        ld hl, sprCache | PLAYER_SPRITE_ADDR
        ldh a, [joyPress]
        and %11111111
        jr z, .skipbuttons                          ;Skip a lot if buttons not pressed
        ld d, a
        bit 0, d
        jr nz, .pressright
        bit 1, d
        jr nz, .pressleft
        jr .endLR

        .pressright
            ld l, (PLAYER_TL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE + 1
            ld l, (PLAYER_TR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE - 1
            ld l, (PLAYER_BL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE + 1
            ld l, (PLAYER_BR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE - 1
            ld l, (PLAYER_BC + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BC_TILE + 1
            jr .endLR
        .pressleft
            ld l, (PLAYER_TL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE - 1
            ld l, (PLAYER_TR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE + 1
            ld l, (PLAYER_BL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE - 1
            ld l, (PLAYER_BR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE + 1
            ld l, (PLAYER_BC + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BC_TILE + 1
            ld l, (PLAYER_BC + SPRITE_ATTR_OFFSET)
            set 5, [hl]
        .endLR

        bit 4, d
        jr nz, .pressA
        bit 5, d
        jr nz, .pressB
        jr .skipbuttons

        .pressA
            ; Temp for a test
            Bdiv8
            CScrollOffs
            Cdiv8
            call XYtoMapByte
            ld hl, _SCRN0
            add hl, de
            ld [hl], $2A
            ; Temp, disables enemy action
            ;ld hl, tblSprVars
            ;ldh a, [ptrEnemySprite]
            ;ld l, a
            ;ld a, [hl]
            ;res 7, a
            ;ld [hl], a
            jr .skipbuttons
        .pressB
            ; Temp, enables enemy action
            ld hl, tblSprVars
            ldh a, [ptrEnemySprite]
            ld l, a
            ld a, [hl]
            set 7, a
            ld [hl], a

        .skipbuttons

        ; PLAYER ANIMATION
        ; ----------------

        ; Reset animation if player not moving
        ldh a, [joyState]
        and %00000011
        jr nz, .ismoving
            ld l, (PLAYER_TL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE
            ld l, (PLAYER_TR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_TLR_TILE
            ld l, (PLAYER_BL + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE
            ld l, (PLAYER_BR + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BLR_TILE
            ld l, (PLAYER_BC + SPRITE_TILE_OFFSET)
            ld [hl], PLAYER_BC_TILE
            ld l, (PLAYER_BC + SPRITE_ATTR_OFFSET)
            res 5, [hl]
        .ismoving

        ; Propeller animation
        ld l, (PLAYER_TC + SPRITE_ATTR_OFFSET)
        bit 5, [hl]
        jr nz, .animset0
        set 5, [hl]
        jr .animend
        .animset0
        res 5, [hl]
        .animend

        ; *****END PLAYER CODE*****

    ; Scroll Background
        ld hl, rSCY
        dec [hl]

    ; Collisions with Background
        ;ldh a, [bgCollWRAM]
        ;ld h, a
        ;ldh a, [bgCollPtr]
        ;ld l, a
        ;ldh a, [plyrYTL]
        ;and %11111000   ; drop last three bits to get the row number
        ;rra             ; the next instructions convert into the memory location offset for the collision table pointer
        ;rra             ; it's a right shift 3 times to remove the trailing zeroes then multiplied to get the row
        ;rra
        ;ld b, a         ; then triple the value because there's three bytes per row in the table
        ;add b           
        ;add b
        ;add l           ; offset the L pointer now
        ;ld l, a
        ; FUTURE - HL is now positioned at the row the first tile to check is.  check however many tiles here
        ; may need to adjust to account for the screen offset, either here or when writing the variable
        ; also optimize this, can use an offset like for sprites and not have to block out 256 bytes

        
    ; Weapons
    ;    ldh a, [joyPress]
    ;    bit 4, a
    ;    jr z, .skip
    ;    ld a, 16
    ;    ld b, 32
    ;    ld c, 32
    ;    ld d, 3
    ;    ld e, 0
    ;    call SetSprite
    ;    .skip

        ; *****BEGIN ENEMY CODE*****

        EnemyLoop:
            ld hl, tblSprVars + ENEMY_SPR_BGN
            ld b, 4                     ; Saves around 200 cycles to load these next two items once
            ld c, ENEMY_SPR_END + 1     ; as they are used every cycle of the start loop that runs 26 times per frame
            .start
                ld a, [hl]
                bit 7, a
                jr nz, .trigger
                .backtoloop
                ld a, l
                cp c
                jr z, .end
                add b
                ld l, a
                jr .start
            .trigger
            push bc                     ; Save BC and HL before calling sprite script
            push hl
            ld e, l                     ; E - original L value, might as well save it here because the sprite script is likely to want it
            ; This next section calls an address placed in bytes 3 and 4 of the tblSprVars
            ; the code in those addresses is where each enemy sprite AI is implemented
            ld a, [hl]
            ld d, a                     ; D - first byte of the sprite variables, almost all will want this so go ahead and keep it here
            inc l                       ; unfortunately can't really keep the 2nd byte anywhere, sprite update code will have to get itself if it wants
            inc l                       ; Move to 3rd byte
            ld b, [hl]                  ; Load value into B
            inc l                       ; Move to 4th byte
            ld c, [hl]
            push bc                     ; need to get BC into HL now
            pop hl
            jp hl                       ; Jump to address
            ; At the end of the enemy sprite AI block, it should jump back to EnemyLoop.return
            .return
            pop hl                      ; Restore old BC and HL before jumping back
            pop bc
            jr .backtoloop
            .end

        ; *****END ENEMY CODE*****

    jp GameLoop

; =====
; TILES
; =====
SECTION "Tiles", ROMX

    ; Text Tiles
Font:
    INCBIN "font_8x8.chr"	

BGTile:
    DB $FF,$75,$FF,$7D,$FF,$EF,$FF,$AB
    DB $FF,$BB,$FF,$FF,$FF,$DE,$FF,$D6


; ====
; DATA
; ====
SECTION "Data", ROM0 

title:
    DB " Project  Palmpilot "
title_end:

; ===
; RAM
; ===
SECTION "HRAM", HRAM
ptrEnemySprite:     DS 1    ; Points to the 8-bit L address of the next available enemy sprite in the range $FE18-FE7F, this range is worked through backwards
plyrWpn1Y:          DS 1    ; Player Y-value, for weapon 1 (top-left sprite), this could be calculated when needed, but since it's used by multiple things it's stored here every frame
plyrWpn1X:          DS 1    ; Player X-value, ditto as above
;bgCollWRAM:     DS 1    ;Block assigned to background RAM collision table
;bgCollPtr:      DS 1    ;Current position of background RAM collision table
;plyrYTL:        DS 1    ;Player top left coordinate



; SPRITES
SECTION "Sprite Variables", WRAM0, ALIGN[8]
; Table storing different characteristics to coincide with a sprite
; This is aligned same as the sprite cache so the last 8 bits of the address will always match the sprite cache
tblSprVars:    DS 160

SECTION "Sprite Storage", WRAM0, ALIGN[8]
; Table storing data saved by a sprite routine
; Each coded sprite can use this how it wishes
; This is aligned same as the sprite cache so the last 8 bits of the address will always match the sprite cache
tblSprStor:    DS 160


; BACKGROUND
SECTION "Background Collision Map", WRAM0, ALIGN[8]
; Tilemap used to determine if player collides with background
; It's a bit wasteful to use an entire 256 bytes for this, as 51 are all that's needed but this is done
; to allow the collision map addressing to scroll with the background with a simple pointer rather than
; having to move data around
tblBGColl:  DS 256