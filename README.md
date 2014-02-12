TC8E (Text Mode Chip8 Emulator)
-------------------------------

Text mode Chip8 Emulator (TC8E) is another chip8 emulator but
in text mode. The emulator can be launched in console mode only or in
X based consoles. I totally forgot this project I began in December
2009. Many chip8 (and better) emulators exist, however, none of them 
are in text mode (or I didn't find them).

Currently, only chip8 instructions are managed, super chip8 support
will be implemented when I find a way to reduce the emulator display
(need to find a way to properly display UTF8 special characters, algos
are here).

This version is only in beta stage. For future, better keyboard
management must be implemented (currently, the keyboard management 
is a disaster) as well as super chip8 instructions. Display
will also be improved. Obviously, priority is the display.


## Compilation ##

TC8E only ncurses library (compiled with ncurses 5.7.2) and
for lastest tests, libcaca is also required (will be put in a different branch later).


## Usage ##

For information about options use '-h' or '--help' in the command line.
The emulator support two built-in keys:
'q' -> Quit the emulator
'+' -> Increase the emulator speed
'-' -> Decrease the emulator speed

The keyboard mapping is the following:
'r' -> key 0
UP  -> key 1
DWN -> key 1
't' -> key 3
LFT -> key 4
'u' -> key 5
RGT -> key 6
'y' -> key 7
'f' -> key 8
'g' -> key 9
'h' -> key 10
'j' -> key 11
'v' -> key 12
'b' -> key 13
'n' -> key 14
',' -> key 15
Theses keys can be changed in 'keyboard.h'.

The example in the "examples" directory comes from 
rivial studios (http://www.revival-studios.com). 
