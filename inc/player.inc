; Project Palmpilot
; player.res
; Player sprite and weapon resource definitions

; Use constants for lots of player details
DEF PLAYER_SPAWN_X EQU 20 ;76
DEF PLAYER_SPAWN_Y EQU 20 ;128
DEF PLAYER_MOVE_SPEED EQU 2

; Screen boundaries
DEF PLAYER_TOP_BOUND EQU 16
DEF PLAYER_BOTTOM_BOUND EQU 136
DEF PLAYER_LEFT_BOUND EQU 8
DEF PLAYER_RIGHT_BOUND EQU 144

; Player sprite addresses
; These are based on the assumption that player sprites will be in the $00-$05 range of the OAM table
; In many cases quicker to load the 8-bit value into L than calculate it
DEF PLAYER_SPRITE_ADDR EQU $00
DEF PLAYER_TL EQU $00
DEF PLAYER_TC EQU $04
DEF PLAYER_TR EQU $08
DEF PLAYER_BL EQU $0C
DEF PLAYER_BC EQU $10
DEF PLAYER_BR EQU $14
DEF SPRITE_TILE_OFFSET EQU $02
DEF SPRITE_ATTR_OFFSET EQU $03

; Special sprite size constant to use in the MegaSprite functions
DEF PLAYER_SPRITE_SIZE EQU %00100011

; Player tile indexes
; Also used because easier to load with constants the left and right turning of player sprites
DEF PLAYER_TC_TILE EQU 1
DEF PLAYER_TLR_TILE EQU 3
DEF PLAYER_BLR_TILE EQU 6
DEF PLAYER_BC_TILE EQU 8


SECTION "Player Sprites", ROM0

; C150 Player Sprite Tiles
C150Tiles:
    DB $0E,$0E,$18,$18,$3C,$24,$FF,$FF
    DB $81,$FF,$81,$FF,$81,$FF,$81,$FF
    DB $00,$00,$00,$00,$00,$00,$0F,$0F
    DB $1C,$1B,$18,$17,$14,$1B,$18,$17
    DB $00,$00,$00,$00,$00,$00,$1F,$1F
    DB $39,$37,$35,$2B,$39,$27,$35,$2B
    DB $00,$00,$00,$00,$00,$00,$3F,$3F
    DB $79,$67,$75,$4B,$79,$47,$75,$4B
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $00,$00,$00,$00,$00,$00,$00,$00
    DB $7E,$42,$7E,$42,$3C,$24,$3C,$24
    DB $3C,$24,$3C,$24,$7E,$66,$FF,$81
    DB $7E,$42,$7A,$46,$3C,$24,$3C,$24
    DB $3C,$24,$3C,$24,$7C,$64,$FA,$86
.end