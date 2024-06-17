# Background
In normal case _SCRN0 is the game background and _SCRN1 is the window

# Background Collision Maps

Memory map of on screen is laid out as follows.  Offset by the current value and uses an entire 256 byte area so that this table can be scrolled along with the background.  Collision maps to be used for enemy projectiles, clouds, buildings, etc anything that could need to be collision checked with the background.  Helps to allow a lot more going on with less worry about the 10-sprite limit as all the work is done on the BG layer and will be automatically scrolled with the background.

|Y|X-byte 1|X-byte 2|X-byte-3|
|--|--|--|--|
|0|$00|$01|$02|
|1|$03|$04|$05|
|2|$06|$06|$08|
|3|$09|$0A|$0B|
|4|$0C|$0D|$0E|
|5|$0F|$10|$11|
|6|$12|$13|$14|
|7|$15|$16|$17|
|8|$18|$19|$1A|
|9|$1B|$1C|$1D|
|10|$1E|$1F|$20|
|11|$21|$22|$23|
|12|$24|$25|$26|
|13|$27|$28|$29|
|14|$2A|$2B|$2C|
|15|$2D|$2E|$2F|
|16|$3A|$3B|$3C|
|17|$3D|$3E|$3F|

The X-bytes are as follows for a row:
|7|6|5|4|3|2|1|0|||7|6|5|4|3|2|1|0|||7|6|5|4|3|2|1|0|
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
|C|C|C|C|C|C|C|C|||C|C|C|C|C|C|C|C|||C|C|C|C|-|-|-|-|

The last four bits are wasted.  

Two rows (6 bytes) will be checked every loop because the player is 2 rows high.  Collisions summed up.  May try to run two of these maps if RAM and cycles allow to allow for different levels of damage (or no damage at all) per collision.