# Agon Build Suite for MPAGD
## About
The Agon Build Suite adds support for the Agon Light/Console 8 computer to the Multi-Platform Arcade Designer. It is based on the ZX Spectrum engine and is designed to allow Spectrum MPAGD games to be ported to the Agon with minimal changes.

The Agon Build Suite includes a new GUI (AgonAGD.exe) which is a re-implemented version of the MPAGD WinAGD.exe application. It includes support for the Agon in its list of machines plus additional features and bug fixes.

The suite also includes:
- ez80asm assembler: https://github.com/AgonPlatform/agon-ez80asm/
- fab-agon-emulator: https://github.com/tomm/fab-agon-emulator/

The engine, compiler and GUI are based on the ZX Spectrum MPAGD suite:\
https://jonathan-cauldwell.itch.io/multi-platform-arcade-game-designer/

For more information on the Agon please visit:\
https://agonplatform.github.io/agon-docs/

## Installation
First download and install MPAGD v0.7.10 which is available from https://jonathan-cauldwell.itch.io/multi-platform-arcade-game-designer/ then unzip the latest AgonMPAGD zip file into the MPAGD installation directory.

You will need to use the AgonAGD.exe application instead of WinAGD.exe to build and edit Agon MPAGD games.

## Differences from the Spectrum MPAGD
### Sounds
#### The SOUND command
The SOUND command can be used to play sampled sound effects. It takes one parameter which is the number of the sound effect to play. The default sound effects are based on the MSX version of MPAGD and are:

0) Jumping
1) Explosion
2) Shoot
3) Pickup 
4) Event
5) Laser
6) Jet

e.g.
```
; Laser sound
SOUND 5
```

#### The DEFSOUND command
Different sound effect samples can be loaded using the DEFSOUND command. The syntax is:
```
DEFSOUND type "filename.raw"
```
Currently only one type ( 0 ) is supported, RAW (header-less) Signed 8-bit PCM, Mono, at 16000Hz. The sample files should be stored in the "Agon Suite\ez80asm\sounds" directory.\
e.g.
```
DEFSOUND 0 "walk.raw"
DEFSOUND 0 "shoot.raw"
DEFSOUND 0 "zap.raw"
```
To add the DEFSOUND commands to your project use the "Custom Defines" option on the Editor menu.
If the DEFSOUND command is used the default sound effects are not loaded so in the above example three new sounds number 0 to 2 would be created.

#### The CRASH command
CRASH takes one parameter which defines both the pitch and duration of white noise to be played.
The pitch should be between 0-15 and the duration from 1-15, then use 16*pitch+duration as the parameter.
The duration is in 40ms increments.\
e.g.
```
; play medium pitched white-noise for 240ms
CRASH $86
```
### INI configuration files
Configuration settings for the AgonAGD application are stored in the agonagd.ini file.

When a project is saved a project specfic ini file is created using the project name and a .ini extension (e.g. myproject.ini).
Any settings in this file will override the default ones in the agonagd.ini.
By default the file will only contain the setting for adventure_mode but you can manually add other settings using a text editor.

For example you could add a [build] section and use a custom build script just for that project.\
e.g.
```
[build]
agon = debug-agon.bat
```





 





 

 
