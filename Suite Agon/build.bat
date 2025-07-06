@echo off

rem Compile AGD file
 copy AGDsource\%1.agd AGD
 cd AGD
 CompilerAgon %1 -a
 copy %1.asm ..\ez80asm\
 del %1.agd
 del %1.asm

rem Assemble game
 cd ..\ez80asm
 ez80asm.exe %1.asm
 copy %1.bin ..\fabemu\sdcard\mpagd\game.bin
 del %1.asm
 del %1.bin

rem Start emulator
 cd ..\fabemu
 fab-agon-emulator.exe
 cd ..
