# Player Sprite
Player is 3W x 2H
||Left|Center|Right|
|--|--|--|--|
|Top|MAIN_TLR|MAIN_TC|MAIN_TLR X-flipped|
|Bottom|MAIN_BLR|MAIN_BC|MAIN_BLR X-flipped|

# Sprite Variables
Sprite variables are used for the game logic to control sprites.  It's a 160 byte block of memory and the low bite should start in zero, same as the Sprite Cache.  The memory locations themselves are used for numbering.

example - if Sprite Cache is $D000-$D09F and Sprite Variables are $D100-D19F then sprite number 20 would have a cache address of $D020 and variable address of $D120

## Sprite #s
- 00-05: reserved for player

## Sprite variable bytes
### 1st byte - flags and settings
These flags are used as quick checks without having to load extra data, and also a place to store the mega-sprite dimensions, as both will be needed often (generally every frame by enemy update code)
- 07: action flag, 0 if sprite has a script call in bytes 3 and 4, 1 if not
- 06: killed flag, 0 default, 1 if sprite has been killed, used by some sprite update code
- 05-04: mega-sprite height
- 03-02: flags for use by specific update code
- 01-00: mega-sprite width
### 2nd byte - sprite data
- 00-03: health left
- 04-07: score provided
### 3rd and 4th bytes - memory location of code to manipulate sprite