; Game engine code --------------------------------------------------------------

; Arcade Game Designer.
; (C) 2008 - 2020 Jonathan Cauldwell.
; ZX Spectrum Engine v0.7.10

; AgonLight/Console8 port v0.1 by Christian Pinder.


                ASSUME  ADL=1
                org     $40000

                jp      begin$
        
                align   64
                db      "MOS",0,1
begin$:
                push    ix
                push    iy
                call    start
                pop     iy
                pop     ix
                or      a
                sbc     hl,hl
                ret


; Block characteristics.

PLATFM:         equ     1               ; platform.
WALL:           equ     PLATFM + 1      ; solid wall.
LADDER:         equ     WALL + 1        ; ladder.
FODDER:         equ     LADDER + 1      ; fodder block.
DEADLY:         equ     FODDER + 1      ; deadly block.
CUSTOM:         equ     DEADLY + 1      ; custom block.
WATER:          equ     CUSTOM + 1      ; water block.
COLECT:         equ     WATER + 1       ; collectable block.
NUMTYP:         equ     COLECT + 1      ; number of types.

; Sprites.

NUMSPR:         equ 12                  ; number of sprites.
TABSIZ:         equ 20                  ; size of each entry.
SPRBUF:         equ NUMSPR * TABSIZ     ; size of entire table.
NMESIZ:         equ 4                   ; bytes stored in nmetab for each sprite.
X:              equ 8                   ; new x coordinate of sprite.
Y:              equ X + 1               ; new y coordinate of sprite.
PAM1ST:         equ 5                   ; first sprite parameter, old x (ix+5).

; Particle engine.

NUMSHR:			equ 55              	; pieces of shrapnel.
SHRSIZ:			equ 6               	; bytes per particle.

; Object data definition
OB_NUMBER:		EQU		0
OB_SCREEN:      EQU     1
OB_X:           EQU     2
OB_Y:           EQU     3
OB_INIT_SCR:    EQU     4
OB_INIT_X:      EQU     5
OB_INIT_Y:      EQU     6
OBJSIZ:         EQU     7					; size of each entry

; Game starts here.
start:
				ld		a,8					; mos_sysvars
				rst.lil	$08
				ld		(sys_timer_addr), ix
				ld		(mos_vars_addr),ix

				ld		a,$1e
				rst.lil	$08
				ld		(mos_keymap_addr),ix

				call	init_50hz_timer		; setup a 50HZ timer for controlling game speed

				call	init_vdp			; setup screen mode and sound channels
				call	load_font			; load the custom font

				ld		a,(numbl)
				ld		b,a					; B = number of blocks
				ld		iy,blkptrs			; IY = address of bitmap array
				ld		hl,$B100			; HL = starting bitmap ID for blocks
				call	create_bitmaps		; load in the block bitmaps

				ld		a,(numob)
				ld		b,a					; B = number of objects
				ld		iy,objbmpptrs		; IY = address of bitmap array
				ld		hl,$B200			; HL = starting bitmap ID for objects
				call	create_bitmaps		; load in the object bitmaps

				ld		a,(numsprbmp)
				ld		b,a					; B = number of sprite bitmaps
				ld		iy,sprbmpptrs		; IY = address of bitmap array
				ld		hl,$B300			; HL = starting bitmap ID for sprites
				call	create_bitmaps		; load in the sprite bitmaps

; if you wish to exit to MOS at the end of each game, change "jp gamelp" to "jp game".
				jp      gamelp       		; start the game.



; Modify for inventory.
minve:
				ld 		hl,invdis       	; routine address.
       			ld 		(mod0+1),hl     	; set up menu routine.
       			ld 		(mod2+1),hl     	; set up count routine.
       			ld 		hl,fopt         	; find option from available objects.
       			ld 		(mod1+1),hl     	; set up routine.
       			jr 		dbox            	; do menu routine.


; Modify for menu.
mmenu:
				ld		hl,always        	; routine address.
       			ld		(mod0+1),hl      	; set up routine.
       			ld		(mod2+1),hl      	; set up count routine.
       			ld		hl,fstd          	; standard option selection.
       			ld		(mod1+1),hl      	; set up routine.

; Drop through into box routine.

; Work out size of box for message or menu.

dbox:			ld 		hl,msgdat   		; pointer to messages.
				call 	getwrd         		; get message number.
				push 	hl             		; store pointer to message.
				ld 		d,1             	; height.
				xor 	a              	 	; start at object zero.
				ld 		(combyt),a      	; store number of object in combyt.
				ld 		e,a             	; maximum width.
dbox5:  		ld 		b,0             	; this line's width.
mod2:   		call 	always         		; item in player's possession? (WARNING! self-modifying code)
       			jr 		nz,dbox6        	; not in inventory, skip this line.
       			inc 	d               	; add to tally.
dbox6:  		ld 		a,(hl)      		; get character.
				inc 	hl              	; next character.
				cp 		','          		; reached end of line?
				jr 		z,dbox3      		; yes.
				cp 		13           		; reached end of line?
				jr 		z,dbox3      		; yes.
				inc 	b               	; add to this line's width.
				and 	a               	; end of message?
				jp 		m,dbox4         	; yes, end count.
				jr 		dbox6           	; repeat until we find the end.
dbox3:  		ld 		a,e             	; maximum line width.
				cp 		b               	; have we exceeded longest so far?
				jr 		nc,dbox5        	; no, carry on looking.
				ld 		e,b             	; make this the widest so far.
				jr 		dbox5           	; keep looking.
dbox4:  		ld 		a,e             	; maximum line width.
				cp 		b               	; have we exceeded longest so far?
				jr 		nc,dbox8        	; no, carry on looking.
				ld 		e,b             	; final line is the longest so far.
dbox8:  		dec	 	d               	; decrement items found.
				jp 		z,dbox15        	; total was zero.
				ld 		a,e             	; longest line.
				and 	a               	; was it zero?
				jp 		z,dbox15        	; total was zero.
				ld		a,e
				ld 		(bwid),a       		; set up width.
				ld		a,d
				ld 		(blen),a       		; set up height.

; That's set up our box size.

				ld 		a,(winhgt)       	; window height in characters.
				sub 	d               	; subtract height of box.
				rra                 		; divide by 2.
				ld 		hl,wintop        	; top edge of window.
				add 	a,(hl)          	; add displacement.
				ld 		(btop),a         	; set up box top.
				ld 		a,(winwid)       	; window width in characters.
				sub 	e               	; subtract box width.
				rra                 		; divide by 2.
				inc 	hl              	; left edge of window.
				add 	a,(hl)          	; add displacement.
				ld 		(blft),a         	; box left.
				pop 	hl              	; restore message pointer.
				ld 		a,(btop)         	; box top.
				ld 		(dispx),a        	; set display coordinate.
				xor 	a               	; start at object zero.
				ld 		(combyt),a       	; store number of object in combyt.
dbox2:  		ld 		a,(combyt)       	; get object number.
mod0:  			call 	always         		; check inventory for display. (WARNING! self-modifying code)
       			jp 		nz,dbox13        	; not in inventory, skip this line.

				ld 		a,(blft)         	; box left.
				ld 		(dispy),a        	; set left display position.
				ld 		a,(bwid)         	; box width.
				ld 		b,a              	; store width.
dbox0:  		ld 		a,(hl)           	; get character.
				cp 		','              	; end of line?
				jr 		z,dbox1          	; yes, next one.
				cp 		13               	; end of option?
				jr 		z,dbox1          	; yes, on to next.
				dec 	b               	; one less to display.
				and 	127             	; remove terminator.
				push 	bc             		; store characters remaining.
				push 	hl             		; store address on stack.
				call 	pchr           		; display on screen.
				pop 	hl              	; retrieve address of next character.
				pop 	bc              	; chars left for this line.
				ld 		a,(hl)           	; get character.
				inc 	hl              	; next character.
				cp 		128           		; end of message?
				jp 		nc,dbox7         	; yes, job done.
				ld 		a,b              	; chars remaining.
				and 	a               	; are any left?
				jr 		nz,dbox0         	; yes, continue.

; Reached limit of characters per line.
dbox9:
				ld 		a,(hl)           	; get character.
				inc 	hl              	; next one.
				cp 		','              	; another line?
				jr 		z,dbox10         	; yes, do next line.
				cp 		13               	; another line?
				jr 		z,dbox10         	; yes, on to next.
				cp 		128              	; end of message?
				jr 		nc,dbox11        	; yes, finish message.
				jr 		dbox9

; Fill box to end of line.
dboxf:
 	 			push 	hl             		; store address on stack.
				push 	bc             		; store characters remaining.
				ld 		a,32             	; space character.
				call 	pchr           		; display character.
				pop 	bc              	; retrieve character count.
				pop 	hl              	; retrieve address of next character.
				djnz 	dboxf          		; repeat for remaining chars on line.
				ret
dbox1:
				inc 	hl              	; skip character.
       			call 	dboxf          		; fill box out to right side.
dbox10:
 				ld 		a,(dispx)        	; x coordinate.
       			inc 	a               	; down a line.
       			ld 		(dispx),a        	; next position.
       			jp 		dbox2            	; next line.
dbox7:
				ld 		a,b              	; chars remaining.
				and 	a               	; are any left?
				jr 		z,dbox11        	; no, nothing to draw.
				call 	dboxf          		; fill message to line.

; Drawn the box menu, now select option.
dbox11:
 				ld 		a,(btop)         	; box top.
       			ld 		(dispx),a        	; set bar position.
dbox14:
				call 	joykey         		; get controls.
       			and 	31              	; anything pressed?
       			jr 		nz,dbox14       	; yes, debounce it.
       			call 	dbar           		; draw bar.
dbox12:
				call 	joykey         		; get controls.
				and 	28              	; anything pressed?
				jr 		z,dbox12       		; no, nothing.
				and 	16              	; fire button pressed?
				jr		z,dboxnf			; no, move bar
@relfire:
				call	joykey				; get controls
				and		16					; fire pressed?
				jr		nz,@relfire			; yes, loop until released

mod1:			jp 		fstd          		; job done. (WARNING! self-modifying code)
dboxnf:
				call 	dbar           		; delete bar.
				ld 		a,(joyval)       	; joystick reading.
				and 	8               	; going up?
				jr 		nz,dboxu         	; yes, go up.
				ld 		a,(dispx)        	; vertical position of bar.
				inc 	a               	; look down.
				ld 		hl,btop          	; top of box.
				sub 	(hl)            	; find distance from top.
				dec 	hl              	; point to height.
				cp 		(hl)             	; are we at end?
				jp 		z,dbox14         	; yes, go no further.
				ld 		hl,dispx         	; coordinate.
				inc 	(hl)            	; move bar.
				jr 		dbox14           	; continue.
dboxu:
  				ld 		a,(dispx)        	; vertical position of bar.
				ld 		hl,btop          	; top of box.
				cp 		(hl)             	; are we at the top?
				jp 		z,dbox14         	; yes, go no further.
				ld 		hl,dispx         	; coordinate.
				dec 	(hl)            	; move bar.
				jr 		dbox14           	; continue.
fstd:
				ld 		a,(dispx)        	; bar position.
				ld 		hl,btop          	; top of menu.
				sub 	(hl)            	; find selected option.
				ld 		(varopt),a       	; store the option.
				jp 		redraw           	; redraw the screen.

; Option not available.  Skip this line.
dbox13:
				ld 		a,(hl)           	; get character.
				inc 	hl              	; next one.
				cp 		','              	; another line?
				jp 		z,dbox2          	; yes, do next line.
				cp 		13               	; another line?
				jp 		z,dbox2          	; yes, on to next line.
				and 	a               	; end of message?
				jp 		m,dbox11         	; yes, finish message.
				jr 		dbox13
dbox15:
				pop		hl              	; pop message pointer from the stack.
       			ret

; draw the menu bar
dbar:
				ld		a,(blft)         	; box left X pos
				ld		l,a
				ld		h,16
				mlt		hl
				ld		a,l
				ld		(vdu_bar_x),a
				ld		a,h
				ld		(vdu_bar_x+1),a

				ld		a,(dispx)			; bar top Y pos
				ld		l,a
				ld		h,16
				mlt		hl
				ld		a,l
				ld		(vdu_bar_y),a
				ld		a,h
				ld		(vdu_bar_y+1),a

				ld		a,(bwid)         	; box width.
				ld		l,a
				ld		h,16
				mlt		hl
				dec		hl
				ld		a,l
				ld		(vdu_bar_w),a
				ld		a,h
				ld		(vdu_bar_w+1),a

				ld		hl,vdu_bar
				ld		bc,vdu_bar_end - vdu_bar
				call	batchvdu
				call	gfx_present
				ret

; VDU codes for drawing a solid coloured rectangle
vdu_bar:		db		18,3,15				; gcol xor colour 15
				db		25, 4	 			; MOVE x,y
vdu_bar_x:		dw		0
vdu_bar_y:		dw		0
				db		25,$61				; RECTANGLE relative co-ords
vdu_bar_w:		dw		0
vdu_bar_h:		dw		15
				dw		18,0,15				; gcol paint colour 15
vdu_bar_end:


invdis:
				push 	hl             		; store message text pointer.
       			push 	de             		; store de pair for line count.
       			ld 		hl,combyt       	; object number.
       			ld 		a,(hl)          	; get object number.
       			inc 	(hl)            	; ready for next one.
       			call	gotob          		; check if we have object.
       			pop 	de              	; retrieve de pair from stack.
       			pop 	hl              	; retrieve text pointer.
       			ret

; Find option selected.
fopt:
				ld 		a,(dispx)
				ld 		hl,btop          	; top of menu.
				sub 	(hl)            	; find selected option.
				inc 	a               	; object 0 needs one iteration, 1 needs 2 and so on.
				ld 		b,a              	; option selected in b register.
				ld 		hl,combyt        	; object number.
				ld 		(hl),0           	; set to first item.
fopt0:
				push 	bc             		; store option counter in b register.
				call 	fobj           		; find next object in inventory.
				pop 	bc              	; restore option counter.
				djnz 	fopt0          		; repeat for relevant steps down the list.
				ld 		a,(combyt)       	; get option.
				dec 	a               	; one less, due to where we increment combyt.
				ld 		(varopt),a       	; store the option.
				jp 		redraw           	; redraw the screen.

fobj:
  				ld 		hl,combyt        	; object number.
				ld 		a,(hl)           	; get object number.
				inc 	(hl)            	; ready for next item.
				ret 	z               	; in case we loop back to zero.
				call 	gotob          		; do we have this item?
				ret 	z               	; yes, it's on the list.
				jr 		fobj             	; repeat until we find next item in pockets.

bwid:   		db 0              			; box/menu width.
blen:   		db 0              			; box/menu height.
btop:   		db 0              			; box coordinates.
blft:   		db 0

; Wait for keypress.
prskey:
				call	debkey         		; debounce key.
prsky0:
				call	vsync          		; vertical synch.
				call	read_key
				jr		nc,prsky0
				ret

; Debounce keypress. Wait until no keys are pressed.
debkey:
				;call vsync          		; update scrolling, sounds etc.
				call	read_key
				jr		c,debkey
				ret

; Delay routine.
delay:
				push	bc     				; store loop counter.
       			call	vsync    			; wait for interrupt.
       			pop		bc              	; restore counter.
       			djnz	delay     			; repeat.
       			ret


; Clear sprite table.
xspr:
                ld      hl,sprtab       	; sprite table.
                ld      b,SPRBUF        	; length of table.
xspr0:
                ld      (hl),255        	; clear one byte.
                inc     hl              	; move to next byte.
                djnz    xspr0           	; repeat for rest of table.
                ret

; Silence sound channels.
silenc:
				ret

; Initialise all objects.
iniob:
                ld 		ix,objdta           ; objects table.
                ld 		a,(numob)           ; number of objects in the game.
                ld 		b,a                 ; loop counter.
                ld 		de,OBJSIZ           ; distance between objects.
iniob0:
                ld 		a,(ix+OB_INIT_SCR)  ; start screen.
                ld 		(ix+OB_SCREEN),a    ; set start screen.
                ld 		a,(ix+OB_INIT_X)    ; find start x.
                ld 		(ix+OB_X),a         ; set start x.
                ld 		a,(ix+OB_INIT_Y)    ; get initial y.
                ld 		(ix+OB_Y),a         ; set y coord.
                add 	ix,de               ; point to next object.
                djnz 	iniob0          	; repeat.
                ret

; Screen synchronisation.
vsync:
				call	gfx_present			; send drawing commands to GPU
@skipvdu:
  				call	joykey				; read joystick/keyboard.

				ld		hl,clock
				ld		a,(hl)
@wait:
				cp		(hl)
				jr		z, @wait			; wait for 50HZ clock to tick over 

       			jp		proshr				; shrapnel and stuff.


; Redraw the screen.
; Remove old copy of all sprites for redraw.
redraw:
				push 	ix             		; place sprite pointer on stack.
				call 	gfx_present
				call 	droom          		; show screen layout.
				call 	shwob          		; draw objects.
numsp0:
				ld 		b,NUMSPR         	; sprites to draw.
       			ld 		ix,sprtab        	; sprite table.
redrw0:
				ld 		a,(ix+0)         	; old sprite type.
				inc 	a               	; is it enabled?
				jr 		z,redrw1         	; no, find next one.
				ld 		a,(ix+3)         	; sprite x.
				cp 		177              	; beyond maximum?
				jr 		nc,redrw1        	; yes, nothing to draw.
				push 	bc             		; store sprite counter.
				call 	sspria         		; show single sprite.
				pop 	bc              	; retrieve sprite counter.
redrw1:
 				ld 		de,TABSIZ        	; distance to next odd/even entry.
				add 	ix,de           	; next sprite.
				djnz 	redrw0         		; repeat for remaining sprites.
				call 	rbloc          		; redraw blocks if in adventure mode.
				call 	dshrp          		; redraw shrapnel.
				pop 	ix              	; retrieve sprite pointer.
				ret

; Clear the screen
cls:
				ld		hl,@vdu_cls
				ld		bc,1
				call	batchvdu
                ret
@vdu_cls:		db		12


fdchk:
				ld 		a,(hl)           	; fetch cell.
       			cp 		FODDER           	; is it fodder?
       			ret 	nz              	; no.
				ld 		(hl),0           	; rewrite block type.
				push 	hl             		; store pointer to block.
				ld 		de,MAP           	; address of map.
				and 	a               	; clear carry flag for subtraction.
				sbc 	hl,de           	; find simple displacement for block.
				ld 		a,l              	; low byte is y coordinate.
				and 	31              	; column position 0 - 31.
				ld 		(dispy),a        	; set up y position.
				add 	hl,hl           	; multiply displacement by 8.
				add 	hl,hl
				add 	hl,hl
				ld 		a,h              	; x coordinate now in h.
				ld 		(dispx),a        	; set the display coordinate.
				xor 	a               	; block to write.
				call 	pattr          		; write block.
				pop 	hl              	; restore block pointer.
				ret

; Colour a sprite.
cspr:
				ld		a,c					; A = new sprite colour
				and		15					; limit it to 0..15 range
				ld      (ix+19),a			; set new sprite colour
				ret


; Specialist routines.
; Process shrapnel.
; TODO
proshr:
				ret


; Explosion shrapnel.
; TODO
shrap:
				ret

; Check coordinates are good before redrawing at new position.
; TODO
chkxy:
				ret

; TODO
trail:
				ret

; TODO
laser:
				ret

; Shoot a laser.
; TODO
shoot:
				ret

; Create a bit of vapour trail.
vapour:
				ret

; Create a user particle.
; TODO
ptusr:
				ret

; Create a vertical or horizontal star.
TODO:
star:
				ret

; Find particle slot for lasers or vapour trail.
; Can't use alternate accumulator.
; TODO
fpslot:
				ret

; Create an explosion at sprite position.
; TODO
explod:
				ret


; Display all shrapnel.
; TODO
dshrp:
				ret

; Particle engine.
; TODO
inishr:
				ret


; Check for collision between laser and sprite.
; TODO
lcol:
				ret


; Main game engine code starts here.
gamelp:
                call    game
                jr      gamelp


game:
rpblc2:			call 	inishr				; initialise particle engine.
evintr:
				call	batchoff
                call	evnt12         		; call intro/menu event.
				call	batchon

                ld		hl,MAP           	; block properties.
                ld 		de,MAP+1         	; next byte.
                ld 		bc,767           	; size of property map.
                ld 		(hl),WALL        	; write default property.
                ldir

                call 	iniob          		; initialise objects.
                xor 	a               	; put zero in accumulator.
                ld 		(gamwon),a      	; reset game won flag.

                ld 		hl,score         	; score.
                call 	inisc          		; init the score.
mapst:
                ld 		a,(stmap)        	; start position on map.
                ld 		(roomtb),a       	; set up position in table, if there is one.
inipbl:
                call 	ibloc          		; set up first screen.
                ld 		ix,ssprit        	; default to spare sprite in table.
evini:
                call 	evnt13         		; initialisation.

; Two restarts.
; First restart - clear all sprites and initialise everything.
rstrt:
                call 	rsevt          		; restart events.
                call 	xspr           		; clear sprite table.
                call 	sprlst         		; fetch pointer to screen sprites.
                call 	ispr           		; initialise sprite table.
                jr 		rstrt0

; Second restart - clear all but player, and don't initialise him.
rstrtn:
                call 	rsevt          		; restart events.
                call 	nspr           		; clear all non-player sprites.
                call 	sprlst         		; fetch pointer to screen sprites.
                call 	kspr           		; initialise sprite table, no more players.


; Set up the player and/or enemy sprites.
rstrt0:
                xor 	a               	; zero in accumulator.
                ld 		(nexlev),a       	; reset next level flag.
                ld 		(restfl),a       	; reset restart flag.
                ld 		(deadf),a        	; reset dead flag.
                call 	droom          		; show screen layout.
                call 	rbloc          		; redraw blocks if in adventure mode.
                call 	inishr         		; initialise particle engine.
                call 	shwob          		; draw objects.
                ld 		ix,sprtab        	; address of sprite table, even sprites.
                call 	dspr           		; display sprites.
                ld 		ix,sprtab+TABSIZ 	; address of first odd sprite.
                call 	dspr           		; display sprites.
mloop:
                call 	vsync          		; synchronise with display.

				;call check_cheat

                ld 		ix,sprtab        	; address of sprite table, even sprites.
                call 	dspr           		; display even sprites.

                call 	plsnd          		; play sounds.
                call 	vsync          		; synchronise with display.
                ld 		ix,sprtab+TABSIZ 	; address of first odd sprite.
                call 	dspr           		; display odd sprites.
                ld 		ix,ssprit        	; point to spare sprite for spawning purposes.
evlp1:
                call 	evnt10         		; called once per main loop.
                call 	pspr           		; process sprites.

; Main loop events.

                ld 		ix,ssprit        	; point to spare sprite for spawning purposes.
evlp2: 
                call 	evnt11         		; called once per main loop.
bsortx:
                call 	bsort          		; sort sprites.
                ld 		a,(nexlev)       	; finished level flag.
                and 	a               	; has it been set?
                jr 		nz,newlev        	; yes, go to next level.
                ld 		a,(gamwon)       	; finished game flag.
                and 	a               	; has it been set?
                jr 		nz,evwon         	; yes, finish the game.
                ld 		a,(restfl)       	; finished level flag.
                dec 	a               	; has it been set?
                jp 		z,rstrt          	; yes, go to next level.
                dec 	a               	; has it been set?
                jp 		z,rstrtn         	; yes, go to next level.

                ld		a,(deadf)        	; dead flag.
                and 	a               	; is it non-zero?
                jr 		nz,pdead         	; yes, player dead.

                ld 		hl,frmno         	; game frame.
                inc 	(hl)            	; advance the frame.

; Back to start of main loop.

                jp 		mloop            	; debug or back to mloop.


newlev:
				ld 		a,(scno)         	; current screen.
       			ld 		hl,numsc         	; total number of screens.
       			inc 	a               	; next screen.
       			cp 		(hl)             	; reached the limit?
       			jr 		nc,evwon         	; yes, game finished.
       			ld 		(scno),a         	; set new level number.
       			jp 		rstrt            	; restart, clearing all aliens.
evwon:
				call 	evnt18         		; game completed.
    			jp 		tidyup        		; tidy up and return to BASIC/calling routine.


; Player dead.
pdead:
				xor 	a            		; zeroise accumulator.
       			ld 		(deadf),a     		; reset dead flag.
evdie:  		call 	evnt16        		; death subroutine.
       			ld 		a,(numlif)   		; number of lives.
       			and 	a               	; reached zero yet?
       			jp 		nz,rstrt     		; restart game.
evfail: 		call 	evnt17      		; failure event.
tidyup: 		ld 		hl,hiscor    		; high score.
       			ld 		de,score     		; player's score.
       			ld 		b,6          		; digits to check.
tidyu2: 		ld 		a,(de)       		; get score digit.
       			cp 		(hl)        		; are we larger than high score digit?
       			jr 		c,tidyu0    		; high score is bigger.
       			jr 		nz,tidyu1   		; score is greater, record new high score.
       			inc 	hl              	; next digit of high score.
       			inc 	de              	; next digit of score.
       			djnz 	tidyu2     			; repeat for all digits.
tidyu0:
       			ld 		bc,score    		; return pointing to score.
       			ret
tidyu1:
				ld 		hl,score    		; score.
       			ld 		de,hiscor  			; high score.
       			ld 		bc,6       			; digits to copy.
       			ldir                		; copy score to high score.
evnewh:
				call 	evnt19				; new high score event.
       			jr 		tidyu0				; tidy up.

; Restart event.
rsevt:
                ld      ix,ssprit       	; default to spare element in table.
evrs:
                jp      evnt14          	; call restart event.


; Copy number passed in a to string position bc, right-justified.
num2ch:
				ld		hl,0        		; blank high byte of hl.
 				ld		l,a        			; put accumulator in l.
				ld		a,32       			; leading spaces.
numdg3:
				ld		de,100    			; hundreds column.
				call	numdg     			; show digit.
numdg2:
				ld		de,10       		; tens column.
       			call	numdg        		; show digit.
       			or		16        			; last digit is always shown.
       			ld		de,1        		; units column.
numdg:
				and		48         			; clear carry, clear digit.
numdg1:
				sbc		hl,de        		; subtract from column.
       			jr		c,numdg0     		; nothing to show.
       			or		16           		; something to show, make it a digit.
       			inc		a          			; increment digit.
       			jr		numdg1    			; repeat until column is zero.
numdg0:
				add		hl,de       		; restore total.
       			cp		32          		; leading space?
       			ret		z           		; yes, don't write that.
       			ld		(bc),a       		; write digit to buffer.
       			inc		bc         			; next buffer position.
       			ret
num2dd:
       			ld		hl,0         		; blank high byte of hl.
				ld		l,a         		; put accumulator in l.
       			ld		a,32        		; leading spaces.
       			ld		de,100      		; hundreds column.
       			call	numdg      			; show digit.
       			or 		16          		; second digit is always shown.
       			jr		numdg2
num2td:
				ld		hl,0        		; blank high byte of hl.
				ld		l,a         		; put accumulator in l.
				ld		a,48         		; leading spaces.
				jr		numdg3

; Initialise the score to '000000'
inisc:
                ld      b,6             	; digits to initialise.
inisc0:
                ld      (hl),'0'        	; write zero digit.
                inc     hl              	; next column.
                djnz    inisc0          	; repeat for all digits.
                ret

; Multiply h by d and return in hl.
imul:
				ld		l,d
				mlt		hl
				ret

; Divide d by e and return in d, remainder in a.
idiv:
				xor		a
				ld		b,8        			; bits to shift.
idiv0:
				sla		d               	; multiply d by 2.
       			rla              			; shift carry into remainder.
       			cp		e          			; test if e is smaller.
       			jr		c,idiv1    			; e is greater, no division this time.
       			sub		e               	; subtract it.
       			inc		d               	; rotate into d.
idiv1:
				djnz	idiv0
       			ret


; Initialise a sound.
; TODO:
isnd:
				ret


; Objects handling.
; 1 for bitmap number
; 3 for room, x and y
; 3 for starting room, x and y.
; 254 = disabled.
; 255 = object in player's pockets.

; Show objects in current room
shwob:
				ld 		hl,objdta			; objects table.
				inc		hl					; point to room data.
				ld		a,(numob)			; number of objects in the game.
				ld		b,a					; loop counter.
shwob0:
				push	bc					; store count.
				push	hl					; store item pointer.
				ld		a,(scno)			; current location.
				cp		(hl)				; same as an item?
				call	z,dobjc				; yes, display object in colour.
				pop		hl              	; restore pointer.
				pop		bc              	; restore counter.
				ld		de,OBJSIZ    		; distance to next item.
				add		hl,de           	; point to it.
				djnz	shwob0				; repeat for others.
				ret
dobjc:
				inc		hl
				ld		a,(hl)
				ld		(dispx),a
				inc		hl
				ld		a,(hl)
				ld		(dispy),a
dobj1:			dec		hl
				dec		hl
				dec		hl
				ld		a,(hl)
				call	draw_object
				ret

;cobj:
;                ret

; Remove an object.
remob:
				ld		hl,numob   			; number of objects in game.
				cp		(hl)       			; are we checking past the end?
       			ret		nc        			; yes, can't get non-existent item.
				push	af        			; remember object.
				call	getob     			; pick it up if we haven't already got it.
				pop		af              	; retrieve object number.
				call	gotob     			; get its address.
				ld		(hl),254    		; remove it.
				ret

; Pick up object number held in the accumulator.
getob:
                ld      hl,numob        	; number of objects in game.
                cp      (hl)            	; are we checking past the end?
                ret     nc              	; yes, can't get non-existent item.
                call    gotob           	; check if we already have it.
                ret     z               	; we already do.
                ex      de,hl           	; object address in de.
                ld      hl,scno         	; current screen.
                cp      (hl)            	; is it on this screen?
                ex      de,hl           	; object address back in hl.
                jr      nz,getob0       	; not on screen, so nothing to delete.
                ld      (hl),255        	; pick it up.
                inc     hl              	; point to x coord.
getob1:
                ld      a,(hl)          	; x coord.
                ld      (dispx),a
                inc     hl              	; back to y coord.
                ld      a,(hl)          	; y coord.
                ld      (dispy),a       	; set display coords.
				dec		hl
				dec		hl
				dec		hl
				ld		a,(hl)				; A = object number
				call	draw_object
				ret
getob0:
                ld      (hl),255        	; pick it up.
                ret

; Got object check.
; Call with object in accumulator, returns zero set if in pockets.
gotob:
                ld      hl,numob        	; number of objects in game.
                cp      (hl)            	; are we checking past the end?
                jr      nc,gotob0       	; yes, we can't have a non-existent object.
                call    findob          	; find the object.
gotob1:
                cp      255             	; in pockets?
                ret
gotob0:
                ld      a,254           	; missing.
                jr      gotob1

findob:
                ld      hl,objdta       	; objects.
				ld		d,a
                ld      e,OBJSIZ 			; size of each object.
				mlt		de
				add		hl,de
				inc		hl
                ld      a,(hl)          	; fetch status.
                ret

; Drop object number at (dispy, dispx).
drpob:
				ld		hl,numob			; number of objects in game.
				cp		(hl) 				; are we checking past the end?
				ret		nc              	; yes, can't drop non-existent item.
				call	gotob   			; make sure object is in inventory.
				ld		a,(scno)   			; screen number.
				cp		(hl)      			; already on this screen?
				ret		z               	; yes, nothing to do.
				ld		(hl),a      		; bring onto screen.
				inc		hl              	; point to x coord.
				ld		a,(dispx)    		; sprite x coordinate.
				ld		(hl),a       		; set x coord.
				inc		hl              	; point to object y.
				ld		a,(dispy)    		; sprite y coordinate.
				ld		(hl),a       		; set the y position.
				jp		dobj1				; display object


; Seek objects at sprite position.
skobj:
				ld		hl,objdta  			; pointer to objects.
				ld		de,1      			; distance to room number.
				add		hl,de           	; point to room data.
				ld		de,OBJSIZ  			; size of each object.
				ld		a,(numob)  			; number of objects in game.
				ld		b,a         		; set up the loop counter.
skobj0:
				ld		a,(scno)    		; current room number.
				cp		(hl)        		; is object in here?
				call	z,skobj1  			; yes, check coordinates.
				add		hl,de           	; point to next object in table.
				djnz	skobj0     			; repeat for all objects.
				ld		a,255      			; end of list and nothing found, return 255.
				ret
skobj1:
				inc		hl              	; point to x coordinate.
				ld		a,(hl)     			; get coordinate.
				sub		(ix+8)          	; subtract sprite x.
				add		a,15            	; add sprite height minus one.
				cp		31          		; within range?
				jp		nc,skobj2  			; no, ignore object.
				inc		hl              	; point to y coordinate now.
				ld		a,(hl)      		; get coordinate.
				sub		(ix+9)          	; subtract the sprite y.
				add		a,15            	; add sprite width minus one.
				cp		31           		; within range?
				jp		nc,skobj3   		; no, ignore object.
				pop		de              	; remove return address from stack.
				ld 		a,(numob)   		; objects in game.
				sub		b               	; subtract loop counter.
				ret                 		; accumulator now points to object.
skobj3:			dec 	hl              	; back to y position.
skobj2:			dec 	hl              	; back to room.
       			ret


; Spawn a new sprite.
spawn:
				ld		hl,sprtab   		; sprite table.
				push	bc         			; store parameters on stack.
				ld		b,NUMSPR     		; number of sprites.
       			ld		de,TABSIZ    		; size of each entry.
spaw0:
       			ld		a,(hl)       		; get sprite type.
       			inc		a               	; is it an unused slot?
       			jr		z,spaw1      		; yes, we can use this one.
       			add		hl,de           	; point to next sprite in table.
       			djnz	spaw0         		; keep going until we find a slot.

; Didn't find one but drop through and set up a dummy sprite instead.
spaw1:
				pop		bc					; take image and type back off the stack.
  				push 	ix          		; existing sprite address on stack.
				ld 		(spptr),hl   		; store spawned sprite address.
				ld 		(hl),c      		; set the type.
				inc 	hl              	; point to image.
				ld 		(hl),b       		; set the image.
				inc 	hl              	; next byte.
				ld 		(hl),0       		; frame zero.
				inc 	hl              	; next byte.
				ld 		a,(ix+X)      		; x coordinate.
				ld 		(hl),a        		; set sprite coordinate.
				inc 	hl              	; next byte.
				ld 		a,(ix+Y)       		; y coordinate.
				ld 		(hl),a        		; set sprite coordinate.
				inc 	hl              	; next byte.
				ex 		de,hl          		; swap address into de.
				ld 		hl,(spptr)     		; restore address of details.
				ld 		bc,5         		; number of bytes to duplicate.
				ldir                		; copy first version to new version.
				ex 		de,hl        		; swap address into de.
				ld 		a,(ix+10)    		; direction of original.
				ld 		(hl),a       		; set the direction.
				inc 	hl              	; next byte.
				ld 		(hl),b       		; reset parameter.
				inc 	hl              	; next byte.
				ld 		(hl),b        		; reset parameter.
				inc 	hl              	; next byte.
				ld 		(hl),b       		; reset parameter.
				inc 	hl              	; next byte.
				ld 		(hl),b          	; reset parameter.
;rtssp:
				ld		ix,(spptr)       	; address of new sprite.
				ld		(ix+18),15			; set new sprite colour
				ld		(ix+19),15			; set new sprite colour
;evis1:
				call	evnt09    			; call sprite initialisation event.
       			ld		ix,(spptr)   		; address of new sprite.
       			call	sspria        		; display the new sprite.
       			pop 	ix              	; address of original sprite.
       			ret

spptr:  		dl 		0              		; spawned sprite pointer.

checkx:
				ld		a,e       			; x position.
       			cp		24          		; off screen?
       			ret		c               	; no, it's okay.
       			pop		hl              	; remove return address from stack.
       			ret

; Displays the current score.
dscor:
				call	preprt				; set up font and print position.
       			call	checkx				; make sure we're in a printable range.
       			;ld 	a,(prtmod)   		; get print mode.
       			;and	a           		; standard size text?
       			;jp		nz,bscor0    		; no, show double-height.
dscor0:
				push	bc            		; place counter onto the stack.
				push	hl
				ld		a,(hl)       		; fetch character.
				call	pchar        		; display character.
				ld		hl,dispy       		; y coordinate.
				inc		(hl)            	; move along one.
				pop		hl
				inc		hl              	; next score column.
				pop		bc              	; retrieve character counter.
				djnz	dscor0         		; repeat for all digits.
dscor2:
				ld		a,(dispx)       	; general coordinates.
       			ld		(charx),a       	; set up display coordinates.
				ld		a,(dispy)       	; general coordinates.
       			ld		(chary),a       	; set up display coordinates.
       			ret


; Adds number in the hl pair to the score.
addsc:
				ex		de,hl
				ld		hl,0
				ld		h,d
				ld		l,e

				ld		de,score+1    		; ten thousands column.
       			ld		bc,10000    		; amount to add each time.
       			call	incsc      			; add to score.
       			inc		de        			; thousands column.
       			ld		bc,1000    			; amount to add each time.
       			call	incsc      			; add to score.
       			inc		de         			; hundreds column.
       			ld		bc,100     			; amount to add each time.
       			call	incsc     			; add to score.
       			inc		de          		; tens column.
       			ld		bc,10       		; amount to add each time.
       			call	incsc      			; add to score.
       			inc		de        			; units column.
       			ld		bc,1        		; units.
incsc:
				push	hl        			; store amount to add.
       			and		a          			; clear the carry flag.
       			sbc		hl,bc       		; subtract from amount to add.
       			jr		c,incsc0    		; too much, restore value.
       			pop		af          		; delete the previous amount from the stack.
       			push	de         			; store column position.
       			call	incsc2    			; do the increment.
       			pop		de         			; restore column.
       			jp		incsc       		; repeat until all added.
incsc0:
				pop		hl       			; restore previous value.
       			ret
incsc2:
				ld		a,(de)     			; get amount.
				inc		a          			; add one to column.
				ld		(de),a      		; write new column total.
				cp		'9'+1       		; gone beyond range of digits?
				ret		c           		; no, carry on.
				ld		a,'0'       		; make it zero.
				ld		(de),a       		; write new column total.
				dec		de           		; back one column.
				jr		incsc2

; Add bonus to score.
addbo:
				ld		de,score+5       	; last score digit.
				ld		hl,bonus+5       	; last bonus digit.
				and		a               	; clear carry.
				ld		b,6					; 6 digits to add
				ld		c,48				; ASCII '0' in c.
addbo0:
				ld 		a,(de)           	; get score.
				adc 	a,(hl)          	; add bonus.
				sub 	c               	; 0 to 18.
				ld 		(hl),c           	; zeroise bonus.
				dec 	hl              	; next bonus.
				cp 		58               	; carried?
				jr 		c,addbo1         	; no, do next one.
				sub 	10              	; subtract 10.
addbo1:
				ld 		(de),a           	; write new score.
				dec 	de              	; next score digit.
				ccf                 		; set carry for next digit.
				djnz 	addbo0         		; repeat for all 6 digits.
				ret

; Swap score and bonus.
swpsb:
				ld 		de,score         	; first score digit.
				ld 		hl,bonus         	; first bonus digit.
				ld 		b,6              	; digits to add.
swpsb0:
 				ld 		a,(de)           	; get score and bonus digits.
				ld 		c,(hl)
				ex 		de,hl            	; swap pointers.
				ld 		(hl),c           	; write bonus and score digits.
				ld 		(de),a
				inc 	hl              	; next score and bonus.
				inc 	de
				djnz 	swpsb0         		; repeat for all 6 digits.
				ret

; Get property buffer address of char at (dispy, dispx) in hl.
pradd:
				ld		a,(dispx)        	; Y coordinate
				ld		e,a
				ld		d,32
				mlt		de					; DE = Y * 32
				ld		a,(dispy)			; fetch X coordinate
       			and		31              	; should be in range 0 - 31.
				or		a
				sbc		hl,hl
				ld		l,a					; HL = X
				add		hl,de				; HL = Y * 32 + X
				ld		de,MAP
				add		hl,de				; HL = MAP + (Y * 32 + X)
				ret

; print character in A at (dispy, dispx)
pchar:
				ld		bc,4
				cp		127
				jr		nz,@skip
				inc		bc
				ld		(@txt_char2),a
				ld		a,27
@skip:
				ld		(@txt_char),a
				ld		a,(dispx)
				ld		(@txt_row),a
				ld		a,(dispy)
				ld		(@txt_col),a
				ld		hl,@vdu_txt
				call	batchvdu
				ret
@vdu_txt:
				db		31
@txt_col:		db		0
@txt_row:		db		0
@txt_char:		db		0
@txt_char2:		db		0


; Print attributes, properties and pixels.

colpat:			db		0

pattr:
				ld		b,a        			; store cell in b register for now.
				ld		de,0        		; no high byte.
				ld		e,a        			; displacement in e.
				ld		hl,(proptr)  		; pointer to properties.
				add		hl,de           	; property cell address.
				ld		c,(hl)      		; fetch byte.
				ld		a,c         		; put into accumulator.
				cp		COLECT       		; is it a collectable?
				jp		nz,pattr1    		; no, carry on as normal.
				ld		a,b          		; restore cell.
				ld		(colpat),a   		; store collectable block.
pattr1:
				call	pradd      			; get property buffer address.
				ld		(hl),c      		; write property.
				ld		a,b          		; get block number.
				call	draw_block
				ld 		hl,dispy         	; X coordinate.
       			inc 	(hl)            	; move along one.
				ret


; Print character pixels, no more.
pchr:
				call	pchar      			; show character in accumulator.
				ld		hl,dispy    		; X coordinate.
       			inc		(hl)            	; move along one.
       			ret

;sprite:
;                ret


; Get room address.
groom:
				ld		a,(scno)    		; screen number.
groomx:			ld		de,0      			; start at zero.
				ld		bc,0				; clear BC
       			ld		hl,(scrptr)  		; pointer to screens.
       			and		a               	; is it the first one?
groom1:
				jr		z,groom0     		; no more screens to skip.
				ld		c,(hl)       		; low byte of screen size.
				inc		hl              	; point to high byte.
				ld		b,(hl)       		; high byte of screen size.
				inc		hl              	; next address.
				ex		de,hl        		; put total in hl, pointer in de.
				add		hl,bc           	; skip a screen.
				ex		de,hl        		; put total in de, pointer in hl.
				dec		a               	; one less iteration.
				jr		groom1       		; loop until we reach the end.
groom0:
				ld		hl,(scrptr)  		; pointer to screens.
				add		hl,de    			; add displacement.
				ld		a,(numsc)   		; number of screens.
				ld		de,0         		; zeroise high bytes.
				ld		e,a          		; displacement in de.
				add		hl,de           	; add double displacement to address.
				add		hl,de
				ret


; Draw present room.
droom:
                ld      a,(wintop)      	; window top.
                ld      (dispx),a       	; set x coordinate.
droom2:
                call    groom           	; get address of current room.
                xor     a               	; zero in accumulator.
                ld      (comcnt),a      	; reset compression counter.
                ld      a,(winhgt)      	; height of window.
droom0:
                push    af              	; store row counter.
                ld      a,(winlft)      	; window left edge.
                ld      (dispy),a       	; set cursor position.
                ld      a,(winwid)      	; width of window.
droom1:
                push    af              	; store column counter.
                call    flbyt           	; decompress next byte on the fly.
                push    hl              	; store address of cell.
                call    pattr				; show attributes and block.
                pop     hl              	; restore cell address.
                pop     af              	; restore loop counter.
                dec     a               	; one less column.
                jr      nz,droom1       	; repeat for entire line.
                ld      a,(dispx)       	; Y coord.
                inc     a               	; move down one line.
                ld      (dispx),a       	; set new position.
                pop     af              	; restore row counter.
                dec     a               	; one less row.
                jr      nz,droom0       	; repeat for all rows.
				call	gfx_present
                ret

; Decompress bytes on-the-fly.
flbyt:
				ld 		a,(comcnt)     		; compression counter.
				and 	a              		; any more to decompress?
				jr 		nz,flbyt1      		; yes.
				ld 		a,(hl)          	; fetch next byte.
				inc 	hl             		; point to next cell.
				cp 		255             	; is this byte a control code?
				ret 	nz              	; no, this byte is uncompressed.
				ld 		a,(hl)          	; fetch byte type.
				ld 		(combyt),a      	; set up the type.
				inc 	hl              	; point to quantity.
				ld 		a,(hl)          	; get quantity.
				inc 	hl              	; point to next byte.
flbyt1:
				dec 	a               	; one less.
       			ld 		(comcnt),a      	; store new quantity.
       			ld 		a,(combyt)      	; byte to expand.
       			ret

; Ladder down check.
laddd:
				ld 		a,(ix+8)    		; x coordinate.
				and 	254             	; make it even.
				ld 		(ix+8),a     		; reset it.
				ld 		h,(ix+9)     		; y coordinate.
numsp5:
				add 	a,16            	; look down 16 pixels.
       			ld 		l,a          		; coords in hl.
       			jr 		laddv

; Ladder up check.
laddu:
				ld 		a,(ix+8)     		; x coordinate.
				and 	254          		; make it even.
				ld 		(ix+8),a     		; reset it.
				ld 		h,(ix+9)     		; y coordinate.
numsp6:			add 	a,14            	; look 2 pixels above feet.
       			ld 		l,a          		; coords in hl.
laddv:
				ld		a,h
				ld		(dispy),a
				ld		a,l
				ld 		(dispx),a   		; set up test coordinates.
				call 	tstbl       		; get map address.
				call 	ldchk       		; standard ladder check.
				ret 	nz              	; no way through.
				inc 	hl              	; look right one cell.
				call 	ldchk        		; do the check.
				ret 	nz              	; impassable.
				ld 		a,(dispy)    		; y coordinate.
				and 	7               	; position straddling block cells.
				ret 	z               	; no more checks needed.
				inc 	hl              	; look to third cell.
				call 	ldchk        		; do the check.
				ret                 		; return with zero flag set accordingly.

; Can go up check.
cangu:
                ld      a,(ix+8)        	; x coordinate.
                sub     2               	; look up 2 pixels.
                ld      (dispx),a
                ld      a,(ix+9)        	; y coordinate.
                ld      (dispy),a
                call    tstbl           	; get map address.
                call    lrchk           	; standard left/right check.
                ret     nz              	; no way through.
                inc     hl              	; look right one cell.
                call    lrchk           	; do the check.
                ret     nz              	; impassable.
                ld      a,(dispy)       	; y coordinate.
                and     7               	; position straddling block cells.
                ret     z               	; no more checks needed.
                inc     hl              	; look to third cell.
                call    lrchk           	; do the check.
                ret                     	; return with zero flag set accordingly.

; Can go down check.
cangd:          ld      a,(ix+8)        	; x coordinate.
                add     a,16            	; look down 16 pixels.
                ld      (dispx),a
                ld      a,(ix+9)        	; y coordinate.
                ld      (dispy),a
                call    tstbl           	; get map address.
                call    plchk           	; block, platform check.
                ret     nz              	; no way through.
                inc     hl              	; look right one cell.
                call    plchk           	; block, platform check.
                ret     nz              	; impassable.
                ld      a,(dispy)       	; y coordinate.
                and     7               	; position straddling block cells.
                ret     z               	; no more checks needed.
                inc     hl              	; look to third cell.
                call    plchk           	; block, platform check.
                ret                     	; return with zero flag set accordingly.

; Can go left check.
cangl:
				ld		l,(ix+8)  			; x coordinate.
       			ld		a,(ix+9)   			; y coordinate.
       			sub		2               	; look left 2 pixels.
       			ld		h,a        			; coords in hl.
       			jr		cangh      			; test if we can go there.

; Can go right check.
cangr:
				ld		l,(ix+8) 			; x coordinate.
       			ld		a,(ix+9)   			; y coordinate.
       			add		a,16            	; look right 16 pixels.
       			ld		h,a        			; coords in hl.
cangh:
				ld		a,l
				ld		(dispx),a
				ld		a,h
				ld		(dispy),a
cangh2:
                ld      b,3             	; default rows to write.
                ld      a,l             	; x position.
                and     7               	; does x straddle cells?
                jr      nz,cangh0       	; yes, loop counter is good.
                dec     b               	; one less row to write.
cangh0:
                call    tstbl           	; get map address.
                ld      de,32           	; distance to next cell.
cangh1:
                call    lrchk           	; standard left/right check.
                ret     nz              	; no way through.
                add     hl,de           	; look down.
                djnz    cangh1
                ret

; Check left/right movement is okay.
; HL = map cell address
lrchk:
                ld      a,(hl)          	; fetch map cell.
                cp      WALL            	; is it passable?
                jr      z,lrchkx        	; no.
                cp      FODDER          	; fodder has to be dug.
                jr      z,lrchkx        	; not passable.
always:
                xor     a               	; report it as okay.
                ret
lrchkx:
                xor     a               	; reset all bits.
                inc     a
                ret

; Check platform or solid item is not in way.
; HL = map cell address
plchk:
                ld      a,(hl)          	; fetch map cell.
                cp      WALL            	; is it passable?
                jr      z,lrchkx        	; no.
                cp      FODDER          	; fodder has to be dug.
                jr      z,lrchkx        	; not passable.
                cp      PLATFM          	; platform is solid.
                jr      z,plchkx        	; not passable.
                cp      LADDER          	; is it a ladder?
                jr      z,lrchkx        	; on ladder, deny movement.
plchk0:
                xor     a               	; report it as okay.
                ret
plchkx:
                ld      a,(dispx)       	; x coordinate.
                and     7               	; position straddling blocks.
                jr      z,lrchkx        	; on platform, deny movement.
                jr      plchk0

; Check ladder is available.
ldchk:
				ld		a,(hl)     			; fetch cell.
       			cp		LADDER      		; is it a ladder?
       			ret                 		; return with zero flag set accordingly.


; Get collectables.
getcol:
				ld 		b,COLECT       		; collectable blocks.
       			call 	tded           		; test for collectable blocks.
       			cp 		b                	; did we find one?
       			ret 	nz              	; none were found, job done.
       			call	gtblk          		; get block.
       			call 	evnt20         		; collected block event.
       			jr 		getcol           	; repeat until none left.

; Get collectable block.
gtblk:
				ld 		(hl),0     			; make it empty now.
				ld 		de,MAP     			; map address.
				and 	a               	; clear carry.
				sbc 	hl,de           	; find cell number.
				ld 		a,l        			; get low byte of cell number.
				and 	31              	; 0 - 31 is column.
				ld 		d,a         		; store X in d register.
				add 	hl,hl           	; multiply by 8.
				add 	hl,hl
				add 	hl,hl       		; Y is now in h.
				ld 		e,h         		; put Y in e.
				ld		a,e
				ld 		(dispx),a       	; set display coordinates.
				ld		a,d
				ld 		(dispy),a       	; set display coordinates.
       			ld		a,(colpat)       	; get collectable block used on this screen.
; remove the block from the screen
				call	draw_block
       			ret

; Touched deadly block check.
; Returns with DEADLY (must be non-zero) in accumulator if true.
tded:
                ld      a,(ix+8)        	; Y coordinate.
                ld      (dispx),a
                ld      a,(ix+9)        	; X coordinate.
                ld      (dispy),a
                call    tstbl           	; get map address.
                ld      de,31           	; default distance to next line down.
                cp      b               	; is this the required block?
                ret     z               	; yes.
                inc     hl              	; next cell.
                ld      a,(hl)          	; fetch type.
                cp      b               	; is this deadly/custom?
                ret     z               	; yes.
                ld      a,(dispy)       	; horizontal position.
                ld      c,a             	; store column in c register.
                and     7               	; is it straddling cells?
                jr      z,tded0         	; no.
                inc     hl              	; last cell.
                ld      a,(hl)          	; fetch type.
                cp      b               	; is this the block?
                ret     z               	; yes.
                dec     de              	; one less cell to next row down.
tded0:
                add     hl,de           	; point to next row.
                ld      a,(hl)          	; fetch left cell block.
                cp      b               	; is this fatal?
                ret     z               	; yes.
                inc     hl              	; next cell.
                ld      a,(hl)          	; fetch type.
                cp      b               	; is this fatal?
                ret     z               	; yes.
                ld      a,c             	; horizontal position.
                and     7               	; is it straddling cells?
                jr      z,tded1         	; no.
                inc     hl              	; last cell.
                ld      a,(hl)          	; fetch type.
                cp      b               	; is this fatal?
                ret     z               	; yes.
tded1:
                ld      a,(dispx)       	; vertical position.
                and     7               	; is it straddling cells?
                ret     z               	; no, job done.
                add     hl,de           	; point to next row.
                ld      a,(hl)          	; fetch left cell block.
                cp      b               	; is this fatal?
                ret     z               	; yes.
                inc     hl              	; next cell.
                ld      a,(hl)          	; fetch type.
                cp      b               	; is this fatal?
                ret     z               	; yes.
                ld      a,c             	; horizontal position.
                and     7               	; is it straddling cells?
                ret     z               	; no.
                inc     hl              	; last cell.
                ld      a,(hl)          	; fetch final type.
                ret                     	; return with final type in accumulator.

; Fetch block type at (dispy, dispx).
tstbl:
                ld      a,(dispx)       	; fetch Y coord.
                rlca                   	 	; divide by 8,
                rlca                    	; and multiply by 32.
                ld      de,0				; clear uDE high byte
                ld      d,a             	; store in d.
                and     224             	; mask off high bits.
                ld      e,a             	; low byte.
                ld      a,d             	; restore shift result.
                and     3               	; high bits.
                ld      d,a             	; got displacement in de.
                ld      a,(dispy)       	; X coord.
                rra                     	; divide by 8.
                rra
                rra
                and     31              	; only want 0 - 31.
                add     a,e             	; add to displacement.
                ld      e,a             	; displacement in de.
                ld      hl,MAP          	; position of dummy screen.
                add     hl,de           	; point to address.
                ld      a,(hl)          	; fetch byte there.
                ret

; Jump - if we can.
; Requires initial speed to be set up in accumulator prior to call.
jump:
				neg              			; switch sign so we jump up.
				ld		c,a          		; store in c register.
				ld		a,(ix+13)    		; jumping flag.
				and		a               	; is it set?
				ret		nz              	; already in the air.
				inc		(ix+13)         	; set it.
				ld		(ix+14),c			; set jump height.
				ret

hop:
				ld		a,(ix+13)   		; jumping flag.
       			and		a           		; is it set?
				ret		nz           		; already in the air.
				ld		(ix+13),255   		; set it.
				ld		(ix+14),0    		; set jump table displacement.
				ret

; Random numbers code.
; Pseudo-random number generator, 8-bit.
random:
				ld 		hl,seed     		; set up seed pointer.
				ld 		a,(hl)      		; get last random number.
				ld 		b,a         		; copy to b register.
				rrca                		; multiply by 32.
				rrca
				rrca
				xor 	31
				add 	a,b
				sbc 	a,255
				ld 		(hl),a       		; store new seed.
				ld 		(varrnd),a   		; return number in variable.
				ret

; Keyboard test routine.
ktest:
				call	inkey
				jr		z,@setcarry
				or		a
				ret
@setcarry:
				scf
				ret

; Joystick and keyboard reading routines.
joykey:
				ld a,(contrl)       		; control flag.
       			dec a               		; is it the keyboard?
       			jr z,joyjoy         		; no, it's joystick.
       			dec a               		; also joystick
       			jr z,joyjoy       			; read joystick.

; Keyboard controls
				ld		hl,keys+6  			; address of last key.
				ld		e,0      			; zero reading.
				ld		d,7          		; keys to read.
joyke0:
				ld		a,(hl)    			; get key from table.
       			call	ktest     			; being pressed?
       			ccf              			; complement the carry.
       			rl		e          			; rotate into reading.
       			dec		hl       			; next key.
       			dec		d          			; one less to do.
       			jr		nz,joyke0   		; repeat for all keys.
				ld		a,e         		; copy e register to accumulator.
				ld		(joyval),a  		; remember value.
       			ret

; Joystick/Joypad controls
joyjoy:
				call	read_joy1
joyjo3:
				ld		e,a      			; copy to e register.
				ld		a,(keys+5)   		; key six.
       			call	ktest          		; being pressed?
       			jr		c,joyjo0    		; not pressed.
       			set		5,e         		; set bit d5.
joyjo0: 		ld		a,(keys+6)   		; key seven.
       			call	ktest       		; being pressed?
       			jr		c,joyjo1   			; not pressed.
       			set		6,e      			; set bit d6.
joyjo1: 		ld		a,e        			; copy e register to accumulator.
joyjo2: 		ld		(joyval),a  		; remember value.
       			ret

; Display message.
dmsg: 
				ld		hl,msgdat   		; pointer to messages.
       			call	getwrd     			; get message number.
dmsg3:
				call	preprt     			; pre-printing stuff.
				call	checkx     			; make sure we're in a printable range.
				;ld 	a,(prtmod)   		; print mode.
				;and	a               	; standard size?
				;jp 	nz,bmsg1      		; no, double-height text.
dmsg0:
				push	hl          		; store string pointer.
				ld		a,(hl)     			; fetch byte to display.
				and		127             	; remove any end marker.
				cp		13           		; newline character?
				jr		z,dmsg1
				call 	pchar      			; display character.
				call	nexpos      		; display position.
				jr		nz,dmsg2     		; not on a new line.
				call	nexlin      		; next line down.
dmsg2:
				pop		hl
       			ld		a,(hl)     			; fetch last character.
				rla              			; was it the end?
				jr		c,dmsg5    			; yes, job done.
				inc		hl              	; next character to display.
				jr		dmsg0
dmsg1:
				ld		hl,dispx    		; x coordinate.
       			inc		(hl)            	; newline.
       			ld		a,(hl)       		; fetch position.
       			cp		24           		; past screen edge?
       			jr		c,dmsg4      		; no, it's okay.
       			ld		(hl),0       		; restart at top.
dmsg4:
				xor		a
				ld		(dispy),a			; carriage return
       			jr		dmsg2
dmsg5:
				ld		a,(dispx)       	; general coordinates.
       			ld 		(charx),a       	; set up display coordinates.
				ld		a,(dispy)       	; general coordinates.
       			ld 		(chary),a       	; set up display coordinates.
       			ret

; Display a character.
achar:
				ld 		b,a          		; copy to b.
				call 	preprt        		; get ready to print.
				;ld 	a,(prtmod)       	; print mode.
				;and 	a               	; standard size?
				ld 		a,b              	; character in accumulator.
				;jp 	nz,bchar         	; no, double-height text.
				call 	pchar          		; display character.
				call 	nexpos         		; display position.
				jp 		z,achar3         	; next line down.
				jp 		dscor2           	; tidy up line and column variables.
achar3:
				inc 	(hl)            	; newline.
       			call 	nexlin         		; next line check.
				jp 		dscor2           	; tidy up line and column variables.


; Get next print column position.
nexpos:
				ld		hl,dispy    		; display position.
       			ld		a,(hl)       		; get coordinate.
       			inc		a               	; move along one position.
       			and		31              	; reached edge of screen?
       			ld		(hl),a       		; set new position.
       			dec		hl              	; point to x now.
       			ret                 		; return with status in zero flag.

; Get next print line position.
nexlin:
				inc		(hl)         		; newline.
       			ld		a,(hl)       		; vertical position.
       			cp		24           		; past screen edge?
       			ret		c               	; no, still okay.
       			ld		(hl),0        		; restart at top.
       			ret

; Pre-print preliminaries.
preprt:
;prescr:
				ld		a,(charx)       	; display coordinates.
				ld		(dispx),a       	; set up general coordinates.
				ld		e,a
				ld		a,(chary)       	; display coordinates.
				ld		(dispy),a       	; set up general coordinates.
				ld		d,a
				ret

; On entry: HL points to word list
;           A contains word number.
getwrd:
				and		a         			; first word in list?
				ret		z               	; yep, don't search.
				ld		b,a
getwd0:
				ld		a,(hl)
				inc		hl
				cp		128           		; found end?
				jr		c,getwd0     		; no, carry on.
       			djnz	getwd0         		; until we have right number.
       			ret


; Bubble sort sprites
bsort:
				ld		b,NUMSPR - 1 		; sprites to swap.
       			ld		ix,sprtab      		; sprite table.
bsort0:
				push	bc             		; store loop counter for now.

				ld		a,(ix+0)         	; first sprite type.
				inc		a               	; is it switched off?
				jr		z,swemp          	; yes, may need to switch another in here.

				ld		a,(ix+TABSIZ)    	; check next slot exists.
				inc		a               	; is it enabled?
				jr		z,bsort2         	; no, nothing to swap.

				ld		a,(ix+3+TABSIZ)		; fetch next sprite's coordinate.
				cp		(ix+3)           	; compare with this x coordinate.
				jr		c,bsort1         	; next sprite is higher - may need to switch.
bsort2:
				ld		de,TABSIZ        	; distance to next odd/even entry.
				add		ix,de           	; next sprite.
				pop		bc              	; retrieve loop counter.
				djnz	bsort0         		; repeat for remaining sprites.
				ret
bsort1:
				ld		a,(ix+TABSIZ)    	; sprite on/off flag.
       			inc		a               	; is it enabled?
       			jr		z,bsort2         	; no, nothing to swap.
       			call	swspr          		; swap positions.
       			jr		bsort2
swemp:
				ld 		a,(ix+TABSIZ)    	; next table entry.
       			inc		a               	; is that one on?
       			jr		z,bsort2         	; no, nothing to swap.
       			call	swspr          		; swap positions.
       			jr		bsort2

; Swap sprites.
swspr:
				lea		hl,ix+TABSIZ		; point to second sprite entry
				lea		de,ix+0
       			ld		b,TABSIZ			; bytes to swap.
swspr0:
				ld		c,(hl)        		; fetch second byte.
       			ld		a,(de)       		; fetch first byte.
       			ld		(hl),a      		; copy to second.
       			ld		a,c          		; second byte in accumulator.
       			ld		(de),a       		; copy to first sprite entry.
       			inc		de              	; next byte.
       			inc		hl              	; next byte.
       			djnz	swspr0      		; swap all bytes in table entry.
       			ret

; Process sprites.
pspr:
				ld 		b,NUMSPR   			; sprites to process.
       			ld 		ix,sprtab    		; sprite table.
pspr1:
				push 	bc         			; store loop counter for now.
				ld 		a,(ix+0)     		; fetch sprite type.
				cp 		9            		; within range of sprite types?
				call 	c,pspr2     		; yes, process this one.
				ld 		de,TABSIZ    		; distance to next odd/even entry.
				add 	ix,de           	; next sprite.
				pop 	bc              	; retrieve loop counter.
				djnz	pspr1      			; repeat for remaining sprites.
				ret
pspr2:
				ld 		(ogptr),ix   		; store original sprite pointer.
       			call	pspr3      			; do the routine.
rtorg:		  	ld 		ix,(ogptr)   		; restore original pointer to sprite.
rtorg0:			ret
pspr3:
				ld		hl,evtyp0     		; sprite type events list.
pspr4:
				ld		e,a
				ld		d,3
				mlt		de					; DE = A * 3
				add		hl,de				; HL = evtyp0 + (A * 3)
				ld		hl,(hl)				; HL = address of event routine
				jp 		(hl)        		; go there

ogptr:			dl		0              		; original sprite pointer.

; Address of each sprite type's routine.	

evtyp0: 		dl 	evnt00
evtyp1: 		dl 	evnt01
evtyp2: 		dl 	evnt02
evtyp3: 		dl 	evnt03
evtyp4: 		dl 	evnt04
evtyp5: 		dl 	evnt05
evtyp6: 		dl 	evnt06
evtyp7: 		dl 	evnt07
evtyp8: 		dl 	evnt08

; Display sprites.
dspr:
				ld		b,NUMSPR/2   		; number of sprites to display.
dspr0:
				push	bc          		; store loop counter for now.
       			ld		a,(ix+0)     		; get sprite type.
       			inc		a               	; is it enabled?
       			jr		nz,dspr1     		; yes, it needs deleting.
dspr5:
				ld		a,(ix+5)     		; new type.
       			inc		a              		; is it enabled?
       			jr		nz,dspr3     		; yes, it needs drawing.
dspr2:
				lea		hl,ix+5				; point to new properties.
				lea		de,ix+0
				ldi                 		; copy to old positions.
				ldi
				ldi
				ldi
				ldi
				ld		a,(ix+19)
				ld		(ix+18),a			; copy colour to old colour
				lea		ix,ix+TABSIZ*2		; next sprite
				pop		bc              	; retrieve loop counter.
				djnz	dspr0       		; repeat for remaining sprites.
				ret
dspr1:
				ld		a,(ix+5)     		; type of new sprite.
       			inc		a               	; is this enabled?
       			jr		nz,dspr4     		; yes, display both.
dspr6:			call	sspria      		; show single sprite.
       			jr		dspr2


; Displaying two sprites.  Don't bother redrawing if nothing has changed.
dspr4:
				ld		a,(ix+4)     		; old y.
				cp		(ix+9)      		; compare with new value.
				jr		nz,dspr7    		; they differ, need to redraw.
				ld		a,(ix+3)     		; old x.
				cp		(ix+8)       		; compare against new value.
				jr		nz,dspr7     		; they differ, need to redraw.
				ld		a,(ix+2)     		; old frame.
				cp		(ix+7)       		; compare against new value.
				jr		nz,dspr7     		; they differ, need to redraw.
				ld		a,(ix+18)			; old colour
				cp		(ix+19)				; compare against new value
				jr		nz,dspr7     		; they differ, need to redraw.
				ld		a,(ix+1)     		; old image.
				cp		(ix+6)       		; compare against new value.
				jp		z,dspr2     		; everything is the same, don't redraw.
dspr7:
				call	sspric     			; delete old sprite, draw new one simultaneously.
       			jp 		dspr2
dspr3:
				call	ssprib     			; show single sprite.
       			jp		dspr2

; Get sprite address calculations.
; gspran = new sprite, gsprad = old sprite.
gspran:
				ld		a,(ix+8)			; new x coordinate.
				ld		(dispx),a       	; set display coordinates.
				ld		a,(ix+9) 			; new y coordinate.
				ld		(dispy),a       	; set display coordinates.
				ld		a,(ix+6)   			; new sprite image.
				call	gfrm      			; fetch start frame for this sprite.
				ld		a,(hl)     			; frame in accumulator.
				add		a,(ix+7)        	; new add frame number.
				ld		c,(ix+19)			; new sprite colour
				ret

gsprad:
				ld		a,(ix+3)   			; x coordinate.
				ld		(dispx),a       	; set display coordinates.
       			ld		a,(ix+4)     		; y coordinate.
				ld		(dispy),a       	; set display coordinates.
				ld		a,(ix+1)     		; sprite image.
				call	gfrm       			; fetch start frame for this sprite.
				ld		a,(hl)       		; frame in accumulator.
				add		a,(ix+2)        	; add frame number.
				ld		c,(ix+18)			; old sprite colour
				ret

; These are the sprite routines.
; sspria = single sprite, old (ix).
; ssprib = single sprite, new (ix+5).
; sspric = both sprites, old (ix) and new (ix+5).

sspria:
				call 	gsprad      		; get old sprite address
				call	draw_sprite
				ret

ssprib:
				call	gspran				; get new sprite address.
				call	draw_sprite
				ret

sspric:
				call 	gsprad      		; get old sprite address
				call	draw_sprite
				call	gspran				; get new sprite address.
				call	draw_sprite
				ret

; Animates a sprite.
animsp:
				ld		hl,frmno     		; game frame.
				and		(hl)            	; is it time to change the frame?
				ret		nz              	; not this frame.
				ld		a,(ix+6)     		; sprite image.
				call	gfrm         		; get frame data.
				inc		hl              	; point to frames.
				ld		a,(ix+7)    		; sprite frame.
				inc		a               	; next one along.
				cp		(hl)         		; reached the last frame?
				jr		c,anims0      		; no, not yet.
				xor		a               	; start at first frame.
anims0:
				ld		(ix+7),a      		; new frame.
       			ret
animbk:
				ld		hl,frmno      		; game frame.
				and		(hl)            	; is it time to change the frame?
				ret		nz              	; not this frame.
				ld		a,(ix+6)     		; sprite image.
				call	gfrm           		; get frame data.
				inc		hl              	; point to frames.
				ld		a,(ix+7)     		; sprite frame.
				and		a               	; first one?
				jr		nz,rtanb0     		; yes, start at end.
				ld		a,(hl)        		; last sprite.
rtanb0:
				dec		a               	; next one along.
       			jr		anims0       		; set new frame.

; Check for collision with other sprite, strict enforcement.
sktyp:
				ld		a,(cheat_mode)
				or		a
				ret		nz

				ld		hl,sprtab     		; sprite table.
numsp2:
				ld		c,NUMSPR     		; number of sprites.
sktyp0:
       			ld		(skptr),hl   		; store pointer to sprite.
       			ld		a,(hl)        		; get sprite type.
       			cp		b            		; is it the type we seek?
       			jr		z,coltyp     		; yes, we can use this one.
sktyp1:
				ld		hl,(skptr)    		; retrieve sprite pointer.
       			ld		de,TABSIZ     		; size of each entry.
       			add		hl,de           	; point to next sprite in table.
       			dec		c               	; one less iteration.
       			jr		nz,sktyp0    		; keep going until we find a slot.
       			ld		hl,0          		; default to ROM address - no sprite.
       			ld		(skptr),hl    		; store pointer to sprite.
       			or		h             		; don't return with zero flag set.
       			ret                 		; didn't find one.

skptr:			dl 		0           		; search pointer.

coltyp:
				ld		a,(ix+0)     		; current sprite type.
       			cp		b            		; seeking sprite of same type?
       			jr		z,colty1     		; yes, need to check we're not detecting ourselves.
colty0:
				ld		de,X         		; distance to x position in table.
				add		hl,de           	; point to coords.
       			ld		e,(hl)       		; fetch x coordinate.
       			inc		hl              	; now point to y.
       			ld		d,(hl)       		; that's y coordinate.

; Drop into collision detection.

colc16:
				ld		a,(ix+X)     		; x coord.
				sub		e              		; subtract x.
				jr		nc,colc1a    		; result is positive.
				neg                 		; make negative positive.
colc1a:
				cp		16           		; within x range?
       			jr		nc,sktyp1    		; no - they've missed.
       			ld		h,a           		; store difference.
       			ld		a,(ix+Y)      		; y coord.
       			sub		d               	; subtract y.
       			jr		nc,colc1b     		; result is positive.
       			neg                 		; make negative positive.
colc1b:
				cp		16           		; within y range?
				jr		nc,sktyp1     		; no - they've missed.
       			add		a,h           		; add x difference.
       			cp		26            		; only 5 corner pixels touching?
       			ret		c               	; carry set if there's a collision.
       			jp		sktyp1       		; try next sprite in table.
colty1:
				push	ix          		; base sprite address onto stack.
				pop		de              	; pop it into de.
				ex		de,hl         		; flip hl into de.
				sbc		hl,de           	; compare the two.
				ex		de,hl        		; restore hl.
				jr		z,sktyp1     		; addresses are identical.
				jp		colty0

; Display number.
disply:
				ld		bc,displ0   		; display workspace.
				call	num2ch    			; convert accumulator to string.
displ1:
				dec		bc         			; back one character.
				ld		a,(bc)       		; fetch digit.
				or		128          		; insert end marker.
				ld		(bc),a       		; new value.
				ld		hl,displ0    		; display space.
				jp		dmsg3        		; display the string.

displ0:			db 		0,0,0,13+128


; Initialise screen.
initsc:
				ld 		a,(roomtb)   		; whereabouts in the map are we?
       			call	tstsc      			; find displacement.
       			cp		255        			; is it valid?
       			ret		z            		; no, it's rubbish.
       			ld		(scno),a     		; store new room number.
       			ret

; Test screen.
tstsc:
				ld		hl,mapdat-MAPWID	; start of map data, subtract width for negative.
       			ld		b,a          		; store room in b for now.
       			add		a,MAPWID        	; add width in case we're negative.
       			ld		de,0              	; zeroise d.
       			ld		e,a              	; screen into e.
       			add		hl,de           	; add displacement to map data.
       			ld		a,(hl)           	; find room number there.
       			ret

; Screen left.
scrl:
				ld		a,(roomtb)  		; present room table pointer.
       			dec		a          			; room left.
scrl0:
				call	tstsc      			; test screen.
				inc		a           		; is there a screen this way?
       			ret		z           		; no, return to loop.
       			ld		a,b         		; restore room displacement.
       			ld		(roomtb),a   		; new room table position.
scrl1:
				call	initsc      		; set new screen.
				ld		hl,restfl    		; restart screen flag.
				ld		(hl),2      		; set it.
				ret

scrr:
				ld		a,(roomtb)  		; room table pointer.
       			inc		a           		; room right.
       			jr		scrl0

scru:
				ld		a,(roomtb)   		; room table pointer.
				sub		MAPWID      		; room up.
				jr		scrl0

scrd:
				ld		a,(roomtb)   		; room table pointer.
				add		a,MAPWID    		; room down.
       			jr		scrl0

; Jump to new screen.
nwscr:
				ld		hl,mapdat 			; start of map data.
				ld		b,80        		; 80 to search
				ld		c,0					; zero room count 
nwscr0:
				cp		(hl)         		; have we found a match for screen?
				jr		z,nwscr1    		; yes, set new point in map.
				inc		hl          		; next room.
				inc		c           		; count rooms.
				djnz	nwscr0     			; keep looking.
				ret
nwscr1:
				ld		a,c            		; room displacement.
       			ld		(roomtb),a     		; set the map position.
				jr		scrl1          		; draw new room.

; Gravity processing.
grav:
				ld		a,(ix+13)  			; in-air flag.
				and		a             		; are we in the air?
				ret		z           		; no we are not.
				inc		a           		; increment it.
				jp		z,ogrv      		; set to 255, use old gravity.
				ld		(ix+13),a   		; write new setting.
				rra                 		; every other frame.
				jr		nc,grav0     		; don't apply gravity this time.
				ld		a,(ix+14)    		; pixels to move.
				cp		16           		; reached maximum?
				jr		z,grav0      		; yes, continue.
				inc		(ix+14)         	; slow down ascent/speed up fall.
grav0:
				ld		a,(ix+14)    		; get distance to move.
				sra		a               	; divide by 2.
				and		a               	; any movement required?
				ret		z               	; no, not this time.
				cp		128          		; is it up or down?
				jr		nc,gravu     		; it's up.
gravd:
				ld		b,a          		; set pixels to move.
gravd0:
				call	cangd       		; can we go down?
				jr		nz,gravst    		; can't move down, so stop.
				inc		(ix+8)       		; adjust new x coord.
				djnz	gravd0
				ret
gravu:
				neg                 		; flip the sign so it's positive.
				ld		b,a          		; set pixels to move.
gravu0:
				call	cangu          		; can we go up?
				jp		nz,ifalls    		; can't move up, go down next.
				dec 	(ix+8)       		; adjust new x coord.
				djnz 	gravu0
				ret
gravst:
				ld 		a,(ix+14)    		; jump pointer high.
				ld 		(ix+13),0     		; reset falling flag.
				ld 		(ix+14),0     		; store new speed.
				cp 		8             		; was speed the maximum?
evftf:
				jp 		z,evnt15      		; yes, fallen too far.
				ret


; Old gravity processing for compatibility with 4.6 and 4.7.
ogrv:
				ld 		de,0          		; no high byte.
				ld 		e,(ix+14)       	; get index to table.
				ld 		hl,jtab         	; jump table.
				add 	hl,de           	; hl points to jump value.
				ld 		a,(hl)          	; pixels to move.
				cp 		99              	; reached the end?
				jr 		nz,ogrv0        	; no, continue.
				dec 	hl              	; go back to previous value.
				ld 		a,(hl)          	; fetch that from table.
				jr 		ogrv1
ogrv0:
				inc 	(ix+14)         	; point to next table entry.
ogrv1:
				and 	a               	; any movement required?
				ret 	z               	; no, not this time.
				cp 		128             	; is it up or down?
				jr 		nc,ogrvu        	; it's up.
ogrvd:
				ld		b,a         		; set pixels to move.
ogrvd0:
				call	cangd         		; can we go down?
       			jr		nz,ogrvst    		; can't move down, so stop.
       			inc		(ix+8)          	; adjust new x coord.
       			djnz	ogrvd0
       			ret
ogrvu:
				neg               			; flip the sign so it's positive.
       			ld 		b,a     			; set pixels to move.
ogrvu0:
				call	cangu          		; can we go up?
				jr 		nz,ogrv2   			; can't move up, go down next.
				dec 	(ix+8)          	; adjust new x coord.
				djnz 	ogrvu0
				ret
ogrvst:
				ld		de,0        		; no high byte.
				ld		e,(ix+14)   		; get index to table.
				ld		hl,jtab      		; jump table.
				add		hl,de       		; hl points to jump value.
				ld		a,(hl)       		; fetch byte from table.
				cp		99           		; is it the end marker?
				ld		(ix+13),0    		; reset jump flag.
				ld		(ix+14),0    		; reset pointer.
				jp		evftf
ogrv2:
				ld		hl,jtab      		; jump table.
       			ld 		b,0          		; offset into table.
ogrv4:
				ld 		a,(hl)       		; fetch table byte.
				cp 		100           		; hit end or downward move?
				jr 		c,ogrv3      		; yes.
				inc 	hl              	; next byte of table.
				inc 	b               	; next offset.
				jr 		ogrv4        		; keep going until we find crest/end of table.
ogrv3:
				ld		(ix+14),b     		; set next table offset.
				ret

; Initiate fall check.
ifall:
				ld		a,(ix+13)      		; jump pointer flag.
				and		a               	; are we in the air?
				ret		nz              	; if set, we're already in the air.
				ld		a,(ix+9)        	; y coordinate.
				ld		(dispy),a
				ld		h,a
				ld		a,16            	; look down 16 pixels.
				add		a,(ix+8)        	; add x coordinate.
				ld		l,a             	; coords in hl.
				ld 		(dispx),a       	; set up test coordinates.
				call 	tstbl          		; get map address.
				call 	plchk          		; block, platform check.
				ret 	nz              	; it's solid, don't fall.
				inc 	hl              	; look right one cell.
				call 	plchk          		; block, platform check.
				ret 	nz             		; it's solid, don't fall.
				ld 		a,(dispy)       	; y coordinate.
				and 	7               	; position straddling block cells.
				jr 		z,ifalls        	; no more checks needed.
				inc 	hl              	; look to third cell.
				call 	plchk          		; block, platform check.
				ret 	nz              	; it's solid, don't fall.
ifalls:
				inc 	(ix+13)         	; set in air flag.
       			ld 		(ix+14),0       	; initial speed = 0.
       			ret
tfall:
				ld		a,(ix+13)			; jump pointer flag.
				and		a            		; are we in the air?
				ret		nz             		; if set, we're already in the air.
				call	ifall          		; do fall test.
				ld		a,(ix+13)       	; get falling flag.
				and		a               	; is it set?
				ret		z               	; no.
				ld		(ix+13),255     	; we're using the table.
				jr		ogrv2           	; find position in table.

; Get frame data for a particular sprite.
gfrm:
				or		a
				sbc		hl,hl
				ld		l,a					; HL = A
				add		hl,hl				; HL = A * 2
				ld		de,frmlst		
				add		hl,de				; HL = frmlst + (A * 2)
				ret

; Find sprite list for current room.
sprlst:         ld      a,(scno)        	; screen number.
sprls2:         ld      hl,(nmeptr)     	; pointer to enemies.
                ld      b,a             	; loop counter in b register.
                and     a               	; is it the first screen?
                ret     z               	; yes, don't need to search data.

                ld      de,NMESIZ       	; bytes to skip.
sprls1:         ld      a,(hl)          	; fetch type of sprite.
                inc     a               	; is it an end marker?
                jr      z,sprls0        	; yes, end of this room.
                add     hl,de           	; point to next sprite in list.
                jr      sprls1          	; continue until end of room.
sprls0:         inc     hl              	; point to start of next screen.
                djnz    sprls1          	; continue until room found.
                ret

; Clear all but a single player sprite.
nspr:
                ld      b,NUMSPR        	; sprite slots in table.
                ld      ix,sprtab       	; sprite table.
                ld      de,TABSIZ       	; distance to next odd/even entry.
nspr0:
                ld      a,(ix+0)        	; fetch sprite type.
                and     a               	; is it a player?
                jr      z,nspr1         	; yes, keep this one.
                ld      (ix+0),255      	; delete sprite.
                ld      (ix+5),255      	; remove next type.
                add     ix,de           	; next sprite.
                djnz    nspr0           	; one less space in the table.
                ret
nspr1:
                ld      (ix+0),255      	; delete sprite.
                add     ix,de           	; point to next sprite.
                djnz    nspr2           	; one less to do.
                ret
nspr2:
                ld      (ix+0),255      	; delete sprite.
                ld      (ix+5),255      	; remove next type.
                add     ix,de           	; next sprite.
                djnz    nspr2           	; one less space in the table.
                ret

; Two initialisation routines.
; Initialise sprites - copy everything from list to table.
ispr:           ld      b,NUMSPR        ; sprite slots in table.
                ld      ix,sprtab       ; sprite table.
ispr2:          ld      a,(hl)          ; fetch byte.
                cp      255             ; is it an end marker?
                ret     z               ; yes, no more to do.
ispr1:          ld      a,(ix+0)        ; fetch sprite type.
                cp      255             ; is it enabled yet?
                jr      nz,ispr4        ; yes, try another slot.
                ld      a,(ix+5)        ; next type.
                cp      255             ; is it enabled yet?
                jr      z,ispr3         ; no, process this one.
ispr4:          ld      de,TABSIZ       ; distance to next odd/even entry.
                add     ix,de           ; next sprite.
                djnz    ispr1           ; repeat for remaining sprites.
                ret                     ; no more room in table.
ispr3:          call    cpsp            ; initialise a sprite.
                djnz    ispr2           ; one less space in the table.
                ret


; Initialise sprites - but not player, we're keeping the old one.
kspr:
                ld      b,NUMSPR        	; sprite slots in table.
                ld      ix,sprtab       	; sprite table.
kspr2:
                ld      a,(hl)          	; fetch byte.
                cp      255             	; is it an end marker?
                ret     z               	; yes, no more to do.
                and     a               	; is it a player sprite?
                jr      nz,kspr1        	; no, add to table as normal.
                ld      de,NMESIZ       	; distance to next item in list.
                add     hl,de           	; point to next one.
                jr      kspr2
kspr1:
                ld      a,(ix+0)        	; fetch sprite type.
                cp      255             	; is it enabled yet?
                jr      nz,kspr4        	; yes, try another slot.
                ld      a,(ix+5)        	; next type.
                cp      255             	; is it enabled yet?
                jr      z,kspr3         	; no, process this one.
kspr4:
                ld      de,TABSIZ       	; distance to next odd/even entry.
                add     ix,de           	; next sprite.
                djnz    kspr1           	; repeat for remaining sprites.
                ret                     	; no more room in table.
kspr3:
                call    cpsp            	; copy sprite to table.
                djnz    kspr2           	; one less space in the table.
                ret

; Copy sprite from list to table.
cpsp:           ld      a,(hl)          	; fetch byte from table.
                ld      (ix+0),a        	; set up type.
                ld      (ix+PAM1ST),a   	; set up type.
                inc     hl              	; move to next byte.
                ld      a,(hl)          	; fetch byte from table.
                ld      (ix+6),a        	; set up image.
                inc     hl              	; move to next byte.
                ld      a,(hl)          	; fetch byte from table.
                ld      (ix+3),200      	; set initial coordinate off screen.
                ld      (ix+8),a        	; set up coordinate.
                inc     hl              	; move to next byte.
                ld      a,(hl)          	; fetch byte from table.
                ld      (ix+9),a        	; set up coordinate.
                inc     hl              	; move to next byte.
                xor     a               	; zeroes in accumulator.
                ld      (ix+7),a        	; reset frame number.
                ld      (ix+10),a       	; reset direction.
                ld      (ix+13),a       	; reset jump pointer low.
                ld      (ix+14),a       	; reset jump pointer high.
                ld      (ix+16),255     	; reset data pointer to auto-restore.
				ld		(ix+18),15			; set sprite colour to white
				ld		(ix+19),15			; set sprite new colour to white
                push    ix              	; store ix pair.
                push    hl              	; store hl pair.
                push    bc
evis0:          call    evnt09          	; perform event.
                pop     bc
                pop     hl              	; restore hl.
                pop     ix              	; restore ix.
                ld      de,TABSIZ       	; distance to next odd/even entry.
                add     ix,de           	; next sprite.
                ret

; Clear the play area window.
clw:
				ld		a,(ink_colour)
				ld		(@clw_colour),a

				ld		de,@clw_tx
				ld		bc,0
    			ld		a,(wintop)
				call	@setcoord				
				ld		a,(winlft)
				call	@setcoord
				dec		bc
				inc		de
				inc		de
				ld		a,(winwid)       ; width of window.
				call	@setcoord				
				ld		a,(winhgt)       ; height of window.
				call	@setcoord				

				ld		hl,@vdu_clw
				ld		bc,@vdu_clw_end - @vdu_clw
				call	batchvdu

				ld		a,(wintop)      ; get coordinates of window.
				ld		(charx),a       ; put into display position.
				ld		a,(winlft)      ; get coordinates of window.
				ld		(chary),a       ; put into display position.
				ret
@setcoord:
				ld		l,a
				ld		h,16
				mlt		hl
				add		hl,bc
				ex		de,hl
				ld		(hl),e
				inc		hl
				ld		(hl),d
				inc		hl
				ex		de,hl
				ret

@vdu_clw:		db		18,0		; gcol paint
@clw_colour:	db		0			; gcol colour
				db		25, 4	 	; MOVE x,y
@clw_tx:		dw		0
				dw		0
				db		25,$61		; RECTANGLE relative co-ords
				dw		0
				dw		0
@vdu_clw_end:

; TODO
; Effects code.
; Ticker routine is called 25 times per second.
scrly:
				ret
iscrly:
				ret

setink:
				and		15
				ld		(ink_colour),a
				ld		(@vdu_colour+1),a
				ld		hl,@vdu_colour
				ld		bc,2
				call	batchvdu
				ret

@vdu_colour:	db		17,0
ink_colour:		db		0

setpaper:
				and		15
				or		128
				ld		(@vdu_colour+1),a
				ld		hl,@vdu_colour
				ld		bc,2
				call	batchvdu
				ret

@vdu_colour:	db		17,0

setborder:
				ret

setcolour:
				push	af					; save colour value for later
				rrca
				rrca
				rrca						; A = paper with bright bit
				ld		b,a
				push	bc
				call	setpaper			; set the paper colour
				pop		bc
				ld		a,b
				and		8
				ld		b,a					; B = bright bit
				pop		af
				and		7					; A = bottom 3 bits for ink
				or		b					; A = ink with bright bit
				call	setink
				ret

cheat_mode:		db		0

; Re-define control key. A = key number to define
definekey:
				or		a
				sbc		hl,hl
				ld		l,a
				ld		de,keys
				add		hl,de			; HL = keys+A
; wait until no key is pressed
@nokey:
				call	read_key
				jr		c,@nokey
; then wait for a key press
@getkey:
				call	read_key
				jr		nc,@getkey
				ld		(hl),a
				call	keyname
				call	strlen
				call	batchvdu
				ret

sound:
				ret

beep:
				and		127
				ld		h,a
				ld		l,85
				mlt		hl					; H = A / 3

				ld		a,h
				ld		(@dur),a

;				ld		hl,@channel
;				inc		(hl)
;				ld		a,(hl)
;				cp		4
;				jr		nz,@nowrap
;				ld		(hl),1
@nowrap:
				ld		hl,@vdu_sound
				ld		bc,@vdu_sound_end - @vdu_sound
				;rst.lil	$18
				call	batchvdu
				ret
@vdu_sound:
				db		23,0,$85
@channel:		db		1
				db		0					; play sound
				db		64					; volume 64 (out of 127)
				dw		630					; frequency
@dur:			dw		11					; duration
@vdu_sound_end:

@freqtab:
				db		0,255,255,255,255,205,171,146,128,114,102,93,85,79,73,68
				db		64,60,57,54,51,49,47,45,43,41,39,38,37,35,34,33
				db		32,31,30,29,28,28,27,26,26,25,24,24,23,23,22,22
				db		21,21,20,20,20,19,19,19,18,18,18,17,17,17,17,16
				db		16,16,16,15,15,15,15,14,14,14,14,14,13,13,13,13
				db		13,13,12,12,12,12,12,12,12,12,11,11,11,11,11,11
				db		11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9
				db		9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8


;============================================================================================================

KBD_G:			EQU		$AC
KBD_H:			EQU		$AB

check_cheat:
				ld		a,KBD_G
				call	inkey
				jr		nz,@docheat
				ld		a,KBD_H
				call	inkey
				jr		nz,@nocheat
				ret
@docheat:		ld		a,9
				ld		(numlif),a
				ld		(cheat_mode),a
				ret
@nocheat:
				xor		a
				ld		(cheat_mode),a
				ret


;============================================================================================================

; Returns length of zero terminated string at HL in BC
strlen:
				push	af
				push	hl
				xor		a
				ld		bc,0
@loop:
				cp		(hl)
				jr		z,@done
				inc		hl
				inc		bc
				jr		@loop
@done:
				pop		hl
				pop		af
				ret

;============================================================================================================

init_vdp:
				ld		hl,vdu_setup_vdp
				ld		bc,vdu_setup_vdp_end - vdu_setup_vdp
				rst.lil	$18

				ld		hl,palett
				ld		bc,endpalett - palett
				rst.lil	$18

				call	initbatchvdu
				call	batchoff
				ret

vdu_setup_vdp:
				db		22, 21						; MODE 21 (512x384 16 colours single-buffered)
				db		23, 0, $C0, 0				; logical screen scaling off
				db		23, 1, 0					; disable text cursor
				db		23, 16, %01010001, 0		; disable text scrolling
				db		23, 0, $A0, $FF,$FF, 2		; clear all command buffers

				db		23, 0, $85, 0, 8
				db		23, 0, $85, 1, 8
				db		23, 0, $85, 2, 8

				;db		23,0,$85,1,6,1				; ADSR envelope on channel 1
				;dw		5,5							; 5ms attack, 5ms decay
				;db		127							; sustain at 100% volume
				;dw		10							; 10ms release

				db		23,0,$85,1,7,1				; stepped frequency evelope on channel 1
				db		2,5							; 2 phases, repeat+restrict
				dw		2,117,4						; 2ms step length, +117 to freq, over 4 steps
				dw		2,447,3						; 2ms step length, +447 to freq, over 3 steps

				db		31, 27, 22
				db		"Loading..."
vdu_setup_vdp_end:

;============================================================================================================

load_font:
				ld		de,font
				ld		hl,@def_chars
				ld		bc,96*8
@loop:
				push	bc
				ld		b,0
				ld		c,0
				ld		a,(de)
				inc		de

				add		a,a
				rl		b
				rl		b
				add		a,a
				rl		b
				rl		b
				add		a,a
				rl		b
				rl		b
				add		a,a
				rl		b

				add		a,a
				rl		c
				rl		c
				add		a,a
				rl		c
				rl		c
				add		a,a
				rl		c
				rl		c
				add		a,a
				rl		c

				ld		a,b
				add		a,a
				or		b
				ld		b,a

				ld		a,c
				add		a,a
				or		c

				ld		(hl),b
				inc		hl
				ld		(hl),a
				inc		hl
				ld		(hl),b
				inc		hl
				ld		(hl),a
				inc		hl
				pop		bc
				dec		bc
				ld		a,b
				or		c
				jr		nz,@loop

				ld		hl,font_buffer_header
				ld		bc,font_buffer_header_end - font_buffer_header
				rst.lil	$18
					
				ld		hl,@font_data
				ld		bc,256*32
				rst.lil	$18

				ld		hl,@vdu_create_font
				ld		bc,@vdu_create_font_end - @vdu_create_font
				rst.lil	$18
				ret

@font_data:
				ds		32*32
@def_chars:		ds		96*32
				ds		128*32

@vdu_create_font:
				db		23, 0, $95, 1
				dw		$B500
				db		16, 16, 16, 0
				db		23, 0, $95, 0
				dw		$B500
				db		0
@vdu_create_font_end:

font_buffer_header:
				db		23, 0, $A0					; buffered command API code
font_buffer_id1:
				dw		$B500						; 16bit buffer ID $B1xx
				db		2							; command 2 = clear buffer

				db		23, 0, $A0					; buffered command API code
font_buffer_id2:
				dw		$B500						; 16bit buffer ID $xxxx
				db		0							; command 0 = write block to buffer
font_buffer_len:
				dw		256*32						; 16bit buffer length
font_buffer_header_end:

;============================================================================================================

draw_block:
				ld		hl,$B100
				ld		l,a					; HL = $B100 + A
				ld		a,(dispy)
				ld		c,a
				ld		b,16
				mlt		bc					; BC = dispy * 16
				ld		a,(dispx)
				ld		e,a
				ld		d,16
				mlt		de					; DE = dispx * 16
				call	draw_bitmap
				ret

;============================================================================================================

draw_object:
					ld		b,a
					ld		a,(dispx)
					cp		192
					ret		nc
					ld		a,b

					push	af
					ld		hl,vdu_gcol_xor
					ld		bc,3
					call	batchvdu
					pop		af

					ld		hl,$B200
					ld		l,a					; HL = $B200 + A
					ld		a,(dispy)
					ld		c,a
					ld		b,2
					mlt		bc					; BC = dispy * 2
					ld		a,(dispx)
					ld		e,a
					ld		d,2
					mlt		de					; DE = dispx * 2
					call	draw_bitmap

					push	af
					ld		hl,vdu_gcol_paint
					ld		bc,3
					call	batchvdu
@done:
					pop		af
					ret

;============================================================================================================

; A = sprite bitmap number, C = colour
draw_sprite:
					ld		hl,@sprite_colour
					set		7,c
					ld		(hl),c

					ld		hl,$B300
					ld		l,a					; HL = $B300 + A
					ld		a,(dispy)
					ld		c,a
					ld		b,2
					mlt		bc					; BC = dispy * 2
					ld		a,(dispx)
					ld		e,a
					ld		d,2
					mlt		de					; DE = dispx * 2

; plot_sprite: HL = bitmap ID, BC = xpos, DE = ypos, A = colour

					push	ix
					ld		ix,@vdu_draw_bitmap
					ld		(ix+6),l
					ld		(ix+7),h
					ld		(ix+10),c
					ld		(ix+11),b
					ld		(ix+12),e
					ld		(ix+13),d
					lea		hl,ix+0
					ld		bc,@vdu_draw_bitmap_end - @vdu_draw_bitmap
					call	batchvdu
					;rst.lil	$18
					pop		ix
					ret

@vdu_draw_bitmap:
					db		18, 3
@sprite_colour:		db		128+1
					db 		23, 27, $20, 0, 0
					db		25, $EF, 0, 0, 0, 0
					db		18, 0, 128
@vdu_draw_bitmap_end:


;============================================================================================================


; draw_bitmap: HL = bitmap ID, BC = xpos, DE = ypos
draw_bitmap:
					push	ix
					ld		ix,@vdu_draw_bitmap
					ld		(ix+3),l
					ld		(ix+4),h
					ld		(ix+7),c
					ld		(ix+8),b
					ld		(ix+9),e
					ld		(ix+10),d
					lea		hl,ix+0
					ld		bc,@vdu_draw_bitmap_end - @vdu_draw_bitmap
					call	batchvdu
					;rst.lil	$18
					pop		ix
					ret

@vdu_draw_bitmap:
					db 		23, 27, $20, 0, 0
					db		25, $ED, 0, 0, 0, 0
@vdu_draw_bitmap_end:


vdu_gcol_xor:		db		18,3,0
vdu_gcol_paint:		db		18,0,0

;============================================================================================================

; B = number of bitmaps
; HL = bitmap base number
; IY = address of pointers
create_bitmaps:
					ld		ix,(iy+0)
					call	upload_bitmap
					inc		hl
					lea		iy,iy+3
					djnz	create_bitmaps
					ret

;============================================================================================================

; upload_bitmap: upload a bitmap to the VDP
; HL = bitmap number
; IX = address of bitmap data
upload_bitmap:
					push	af
					push	bc
					push	hl
					push	ix

; load the bitmap number into the VDU commands
					ld		a,l
					ld		(bitmap_buffer_id1),a
					ld		(bitmap_buffer_id2),a
					ld		(bitmap_buffer_id3),a
					ld		a,h
					ld		(bitmap_buffer_id1+1),a
					ld		(bitmap_buffer_id2+1),a
					ld		(bitmap_buffer_id3+1),a

; load the bitma format into the VDU commands
					ld		a,(ix+0)
					ld		(bitmap_format),a

; load the width of the bitmap into the VDU commands
					ld		a,(ix+1)
					ld		(bitmap_buffer_w),a
					ld		a,(ix+2)
					ld		(bitmap_buffer_w+1),a

; load the height of the bitmap into the VDU commands
					ld		a,(ix+3)
					ld		(bitmap_buffer_h),a
					ld		a,(ix+4)
					ld		(bitmap_buffer_h+1),a

; put the total number of bytes into the bitmap into BC and save it for later
; need to clear BC first so that the upper byte is set to zero
					ld		bc,0
					ld		c,(ix+5)
					ld		b,(ix+6)
					push	bc

; load the total number of bytes in the bitmap into the VDU commands
					ld		a,c
					ld		(bitmap_buffer_len),a
					ld		a,b
					ld		(bitmap_buffer_len+1),a

; put the address of the bitmap data into HL and save it for later
					lea		hl,ix+7
					push	hl

; send the VDU commands to create the command buffer to the VDP
					ld		hl,bitmap_buffer_header
					ld		bc,bitmap_buffer_header_end - bitmap_buffer_header
					rst.lil	18h

; restore the bitmap size and address from the stack
; and send the bitmap data to the VDP
					pop		hl
					pop		bc
					rst.lil	18h

; send the VDU codes to create the bitmap to the VDP
					ld		hl,bitmap_create
					ld		bc,bitmap_create_end - bitmap_create
					rst.lil	18h

					pop		ix
					pop		hl
					pop		bc
					pop		af
					ret

bitmap_buffer_header:
					db		23, 0, $A0					; buffered command API code
bitmap_buffer_id1:	dw		0							; 16bit buffer ID $B1xx
					db		2							; command 2 = clear buffer

					db		23, 0, $A0					; buffered command API code
bitmap_buffer_id2:	dw		0							; 16bit buffer ID $xxxx
					db		0							; command 0 = write block to buffer
bitmap_buffer_len:	dw		0							; 16bit buffer length
bitmap_buffer_header_end:

bitmap_create:
					db		23, 27, $20					; select bitmap using a 16-bit buffer ID
bitmap_buffer_id3:	dw		0							; buffer ID $xxxx

					db		23, 27, $21					; create bitmap from selected buffer
bitmap_buffer_w:	db		0,0							; 16bit width
bitmap_buffer_h:	db		0,0							; 16bit height
bitmap_format:		db		0							; bitmap format
bitmap_create_end:


;============================================================================================================

keyname:
					push	de
					cpl
					ld		h,a
					ld		l,3
					mlt		hl
					ld		de,keyname_table
					add		hl,de
					ld		hl,(hl)
					pop		de
					ret

; check if a key is down. A = keycode. returns NZ if down, Z if up.
inkey:
					push	bc
					push	de
					push	hl

					cpl

					ld		de,(mos_keymap_addr)
					or		a
					sbc		hl,hl
					ld		l,a
					srl		l
					srl		l
					srl		l
					add		hl,de
					ex		de,hl

					or		a
					sbc		hl,hl
					and		7
					ld		l,a
					ld		bc,@bit_masks
					add		hl,bc

					ld		a,(de)
					and		(hl)

					pop		hl
					pop		de
					pop		bc					
					ret

@bit_masks:			db		1,2,4,8,16,32,64,128

;============================================================================================================

read_key:
					push	bc
					push	hl

					ld		hl,(mos_keymap_addr)
					ld		b,16
					ld		c,0
@loop:
					ld		a,(hl)
					or		a
					jr		nz,@found

					inc		hl
					inc		c
					djnz	@loop

; if we get to here then nothing was pressed
					or		a							; clear the carry flag
					jr		@done						; pop saved registers and return

; we found a key down
@found:
					sla		c
					sla		c
					sla		c							; C = C * 8
@rotate:
					inc		c
					rra
					jr		nc,@rotate
					dec		c
					ld		a,c
					cpl
					scf
@done:
					pop		hl
					pop		bc
					ret

;============================================================================================================

PC_DR:				EQU 	$9E				; GPIO Port C Data Register
PD_DR:				EQU 	$A2				; GPIO Port D Data Register

; read_joy1: A = joypad 1 buttons pressed
;	7	6	5	4	3	2	1	0
;	-	-	F2	F1	U	D	L	R

read_joy1:
					push	bc
					ld		b,0
					in0		a,(PD_DR)
					cpl
					add		a,a
					rl		b				; rotate in fire 2
					add		a,a
					add		a,a
					rl		b				; rotate in fire 1

					in0		a,(PC_DR)
					cpl
					rra
					rra
					rl		b				; rotate in up
					rra
					rra
					rl		b				; rotate in down
					rra
					rra
					rl		b				; rotate in left
					rra
					rra
					rl		b				; rotate in right
					ld		a,b
					pop		bc
					ret

;============================================================================================================

TMR1_CTL:      		equ $83
TMR1_RR_L:     		equ $84
TMR1_RR_H:     		equ $85	

PRT_IRQ_0:    		equ %00000000
IRQ_EN_0:     		equ %00000000
PRT_MODE_0:   		equ %00000000
PRT_EN_0:     		equ %00000000

IRQ_EN_1:     		equ %01000000
PRT_MODE_1:   		equ %00010000
CLK_DIV_256:		equ %00001100
RST_EN_1:     		equ %00000010
PRT_EN_1:     		equ %00000001


init_50hz_timer:
; set the 50hz counter to zero
					xor		a
					ld		(clock),a

; ensure we are running in interrupt mode 2
					im		2

; make sure the PRT is disabled before changing it
					ld 		a, PRT_IRQ_0 | IRQ_EN_0 | PRT_MODE_0 | CLK_DIV_256 | RST_EN_1 | PRT_EN_0
					out0 	(TMR1_CTL),a

; set the PRT reload registers for 50Hz
					ld		hl,1450; 1311
					out0	(TMR1_RR_L),l
					out0	(TMR1_RR_H),h  

; set the interrupt vector via MOS
					ld		e, $0C
					ld		a, $14
					ld		hl, fiftyhz_timer
					rst.lil $08

; now enable the timer
					ld		a, PRT_IRQ_0 | IRQ_EN_1 | PRT_MODE_1 | CLK_DIV_256 | RST_EN_1 | PRT_EN_1
					out0	(TMR1_CTL),a  

					ret

; interrupt routine gets called 50 times per second
; we follow the CPC convention of using alternate registers in interrupts
fiftyhz_timer:
					di
					ex		af,af
					exx
					in0		a,(TMR1_CTL)
					ld		hl,clock
					inc		(hl)
					exx
					ex		af,af
					ei
					reti.l

;============================================================================================================

gfx_present:
					push	af
					push	bc
					push	de
					push	hl
					push	ix
					ld		a,(gfxbatch)
					or		a
					jr		z,@done
					ld		bc,(vdu_buf_count)
					ld		a,b
					or		c
					jr		z,@done
					call	sendbatchvdu
					call	callbatchvdu
@done:
					pop		ix
					pop		hl
					pop		de
					pop		bc
					pop		af
					ret

sendbatchvdu:
					ld		hl,vdu_write_buf_len
					ld		(hl),c
					inc		hl
					ld		(hl),b
					inc		bc
					inc		bc
					inc		bc
					inc		bc
					inc		bc
					inc		bc
					inc		bc
					inc		bc
					ld		hl,vdu_write_buffer
@loop:
@wait_CTS:
					in0		a,($A2)
					tst		a, 8					; Check Port D, bit 3 (CTS)
					jr		nz, @wait_CTS
@TX1:
					in0		a,($C5)					; Get the line status register
					and 	$40						; Check for TX empty
					jr		z, @TX1					; If not set, then TX is not empty so wait until it is

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 1
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 2
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 3
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 4
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 5
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 6
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 7
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 8
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 9
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 10
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 11
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 12
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 13
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 14
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 15
					dec		bc
					ld		a,b
					or		c
					jp		z, @done

					ld		a,(hl)
					inc		hl
					out0	($C0),A					; Write the character to the UART transmit buffer 16

					dec		bc
					ld		a,b
					or		c
					jp		nz, @loop
@done:
					ret

callbatchvdu:
					ld		hl,vdu_call_buffer
					ld		bc,12
					rst.lil	$18
initbatchvdu:
					or		a
					sbc		hl,hl
					ld		(vdu_buf_count),hl
					ld		hl,vdu_buffer
					ld		(vdu_buf_ptr),hl
					ret

batchon:
					ld		a,1
					ld		(gfxbatch),a
					ret

batchoff:
					xor		a
					ld		(gfxbatch),a
					ret

batchvdu:
					ld		a,(gfxbatch)
					or		a
					jr		nz,@dobatch
					rst.lil	$18
					ret
@dobatch:
					push	de
					push	hl
					ld		hl,(vdu_buf_count)
					add		hl,bc
					ld		(vdu_buf_count),hl
					pop		hl
					ld		de,(vdu_buf_ptr)
					ldir
					ld		(vdu_buf_ptr),de
					pop		de
					ret

gfxbatch:			db		0
vdu_buf_count:		dl		0
vdu_buf_ptr:		dl		0
vdu_call_buffer:	db		23, 0, $A0, 1,0, 1,  23, 0, $A0, 1,0, 2
vdu_write_buffer:	db		23, 0, $A0, 1,0, 0
vdu_write_buf_len:	dw		0
vdu_buffer:			ds		16384

;============================================================================================================

sys_timer_addr:		dl		0
mos_vars_addr:		dl		0
mos_keymap_addr:	dl		0

txt_keyname_0:		db		0
txt_keyname_1:		db		0
txt_keyname_2:		db		0
txt_keyname_3:		db		"L SHIFT",0
txt_keyname_4:		db		"L CTRL",0
txt_keyname_5:		db		"L ALT",0
txt_keyname_6:		db		"R SHIFT",0
txt_keyname_7:		db		"R CTRL",0
txt_keyname_8:		db		"R ALT",0
txt_keyname_9:		db		0
txt_keyname_10:		db		0
txt_keyname_11:		db		0
txt_keyname_12:		db		0
txt_keyname_13:		db		0
txt_keyname_14:		db		0
txt_keyname_15:		db		0
txt_keyname_16:		db		"Q",0
txt_keyname_17:		db		"3",0
txt_keyname_18:		db		"4",0
txt_keyname_19:		db		"5",0
txt_keyname_20:		db		"F4",0
txt_keyname_21:		db		"8",0
txt_keyname_22:		db		"F7",0
txt_keyname_23:		db		"MINUS",0
txt_keyname_24:		db		0
txt_keyname_25:		db		"L ARROW",0
txt_keyname_26:		db		"KP 6",0
txt_keyname_27:		db		"KP 7",0
txt_keyname_28:		db		"F11",0
txt_keyname_29:		db		"F12",0
txt_keyname_30:		db		"F10",0
txt_keyname_31:		db		"SCR LK",0
txt_keyname_32:		db		"PRT SC",0
txt_keyname_33:		db		"W",0
txt_keyname_34:		db		"E",0
txt_keyname_35:		db		"T",0
txt_keyname_36:		db		"7",0
txt_keyname_37:		db		"I",0
txt_keyname_38:		db		"9",0
txt_keyname_39:		db		"0",0
txt_keyname_40:		db		0
txt_keyname_41:		db		"D ARROW",0
txt_keyname_42:		db		"KP 8",0
txt_keyname_43:		db		"KP 9",0
txt_keyname_44:		db		"BREAK",0
txt_keyname_45:		db		"GR ACC",0
txt_keyname_46:		db		0
txt_keyname_47:		db		"BAKSPC",0
txt_keyname_48:		db		"1",0
txt_keyname_49:		db		"2",0
txt_keyname_50:		db		"D",0
txt_keyname_51:		db		"R",0
txt_keyname_52:		db		"6",0
txt_keyname_53:		db		"U",0
txt_keyname_54:		db		"O",0
txt_keyname_55:		db		"P",0
txt_keyname_56:		db		"L BRKT",0
txt_keyname_57:		db		"U ARROW",0
txt_keyname_58:		db		"KP PLUS",0
txt_keyname_59:		db		"KP MINUS",0
txt_keyname_60:		db		"KP ENTER",0
txt_keyname_61:		db		"INSERT",0
txt_keyname_62:		db		"HOME",0
txt_keyname_63:		db		"PG UP",0
txt_keyname_64:		db		"CAPS LK",0
txt_keyname_65:		db		"A",0
txt_keyname_66:		db		"X",0
txt_keyname_67:		db		"F",0
txt_keyname_68:		db		"Y",0
txt_keyname_69:		db		"J",0
txt_keyname_70:		db		"K",0
txt_keyname_71:		db		"QUOTE",0
txt_keyname_72:		db		0
txt_keyname_73:		db		"RETURN",0
txt_keyname_74:		db		"KP DIV",0
txt_keyname_75:		db		"KP DEL",0
txt_keyname_76:		db		"KP DOT",0
txt_keyname_77:		db		"NUM LOCK",0
txt_keyname_78:		db		"PG DOWN",0
txt_keyname_79:		db		0
txt_keyname_80:		db		0
txt_keyname_81:		db		"S",0
txt_keyname_82:		db		"C",0
txt_keyname_83:		db		"G",0
txt_keyname_84:		db		"H",0
txt_keyname_85:		db		"N",0
txt_keyname_86:		db		"L",0
txt_keyname_87:		db		"SEMICLN",0
txt_keyname_88:		db		"R BRKT",0
txt_keyname_89:		db		"DELETE",0
txt_keyname_90:		db		0
txt_keyname_91:		db		"KP STAR",0
txt_keyname_92:		db		0
txt_keyname_93:		db		"EQUALS",0
txt_keyname_94:		db		0
txt_keyname_95:		db		0
txt_keyname_96:		db		"TAB",0
txt_keyname_97:		db		"Z",0
txt_keyname_98:		db		"SPACE",0
txt_keyname_99:		db		"V",0
txt_keyname_100:	db		"B",0
txt_keyname_101:	db		"M",0
txt_keyname_102:	db		"COMMA",0
txt_keyname_103:	db		"PERIOD",0
txt_keyname_104:	db		"SLASH",0
txt_keyname_105:	db		"END",0
txt_keyname_106:	db		"KP 0",0
txt_keyname_107:	db		"KP 1",0
txt_keyname_108:	db		"KP 3",0
txt_keyname_109:	db		0
txt_keyname_110:	db		0
txt_keyname_111:	db		0
txt_keyname_112:	db		"ESC",0
txt_keyname_113:	db		"F1",0
txt_keyname_114:	db		"F2",0
txt_keyname_115:	db		"F3",0
txt_keyname_116:	db		"F5",0
txt_keyname_117:	db		"F6",0
txt_keyname_118:	db		"F8",0
txt_keyname_119:	db		"F9",0
txt_keyname_120:	db		0
txt_keyname_121:	db		"R ARROW",0
txt_keyname_122:	db		"KP 4",0
txt_keyname_123:	db		"KP 5",0
txt_keyname_124:	db		"KP 2",0
txt_keyname_125:	db		"L GUI",0
txt_keyname_126:	db		"R GUI",0
txt_keyname_127:	db		"APP",0

keyname_table:
					dl		txt_keyname_0,   txt_keyname_1,   txt_keyname_2,   txt_keyname_3,   txt_keyname_4
					dl		txt_keyname_5,   txt_keyname_6,   txt_keyname_7,   txt_keyname_8,   txt_keyname_9 
					dl		txt_keyname_10,  txt_keyname_11,  txt_keyname_12,  txt_keyname_13,  txt_keyname_14
					dl		txt_keyname_15,  txt_keyname_16,  txt_keyname_17,  txt_keyname_18,  txt_keyname_19
					dl		txt_keyname_20,  txt_keyname_21,  txt_keyname_22,  txt_keyname_23,  txt_keyname_24
					dl		txt_keyname_25,  txt_keyname_26,  txt_keyname_27,  txt_keyname_28,  txt_keyname_29
					dl		txt_keyname_30,  txt_keyname_31,  txt_keyname_32,  txt_keyname_33,  txt_keyname_34 
					dl		txt_keyname_35,  txt_keyname_36,  txt_keyname_37,  txt_keyname_38,  txt_keyname_39
					dl		txt_keyname_40,  txt_keyname_41,  txt_keyname_42,  txt_keyname_43,  txt_keyname_44
					dl		txt_keyname_45,  txt_keyname_46,  txt_keyname_47,  txt_keyname_48,  txt_keyname_49
					dl		txt_keyname_50,  txt_keyname_51,  txt_keyname_52,  txt_keyname_53,  txt_keyname_54
					dl		txt_keyname_55,  txt_keyname_56,  txt_keyname_57,  txt_keyname_58,  txt_keyname_59
					dl		txt_keyname_60,  txt_keyname_61,  txt_keyname_62,  txt_keyname_63,  txt_keyname_64
					dl		txt_keyname_65,  txt_keyname_66,  txt_keyname_67,  txt_keyname_68,  txt_keyname_69
					dl		txt_keyname_70,  txt_keyname_71,  txt_keyname_72,  txt_keyname_73,  txt_keyname_74
					dl		txt_keyname_75,  txt_keyname_76,  txt_keyname_77,  txt_keyname_78,  txt_keyname_79
					dl		txt_keyname_80,  txt_keyname_81,  txt_keyname_82,  txt_keyname_83,  txt_keyname_84 
					dl		txt_keyname_85,  txt_keyname_86,  txt_keyname_87,  txt_keyname_88,  txt_keyname_89
					dl		txt_keyname_90,  txt_keyname_91,  txt_keyname_92,  txt_keyname_93,  txt_keyname_94
					dl		txt_keyname_95,  txt_keyname_96,  txt_keyname_97,  txt_keyname_98,  txt_keyname_99
					dl		txt_keyname_100, txt_keyname_101, txt_keyname_102, txt_keyname_103, txt_keyname_104
					dl		txt_keyname_105, txt_keyname_106, txt_keyname_107, txt_keyname_108, txt_keyname_109
					dl		txt_keyname_110, txt_keyname_111, txt_keyname_112, txt_keyname_113, txt_keyname_114
					dl		txt_keyname_115, txt_keyname_116, txt_keyname_117, txt_keyname_118, txt_keyname_119
					dl		txt_keyname_120, txt_keyname_121, txt_keyname_122, txt_keyname_123, txt_keyname_124
					dl		txt_keyname_125, txt_keyname_126, txt_keyname_127

;============================================================================================================

grbase:			dl 0

joyval:         db 0                    ; joystick reading.
frmno: 			db 0              		; selected frame.
scno:           db 0                    ; present screen number.
numlif:         db 3                    ; number of lives.
nexlev:         db 0                    ; next level flag.
prtmod:         db 0                    ; print mode, 0 = standard, 1 = double-height.

loopa:          db 0                    ; loop counter system variable.
loopb:          db 0                    ; loop counter system variable.
loopc:          db 0                    ; loop counter system variable.

wntopx: 		db 8 * WINDOWTOP
wnlftx: 		db 8 * WINDOWLFT
wnbotx: 		db WINDOWTOP + WINDOWHGT * 8 - 16
wnrgtx: 		db WINDOWLFT + WINDOWWID * 8 - 16
vara:           db 0                    ; general-purpose variable.
varb:           db 0                    ; general-purpose variable.
varc:           db 0                    ; general-purpose variable.
vard:           db 0                    ; general-purpose variable.
vare:           db 0                    ; general-purpose variable.
varf:           db 0                    ; general-purpose variable.
varg:           db 0                    ; general-purpose variable.
varh:           db 0                    ; general-purpose variable.
vari:           db 0                    ; general-purpose variable.
varj:           db 0                    ; general-purpose variable.
vark:           db 0                    ; general-purpose variable.
varl:           db 0                    ; general-purpose variable.
varm:           db 0                    ; general-purpose variable.
varn:           db 0                    ; general-purpose variable.
varo:           db 0                    ; general-purpose variable.
varp:           db 0                    ; general-purpose variable.
varq:           db 0                    ; general-purpose variable.
varr:           db 0                    ; general-purpose variable.
vars:           db 0                    ; general-purpose variable.
vart:           db 0                    ; general-purpose variable.
varu:           db 0                    ; general-purpose variable.
varv:           db 0                    ; general-purpose variable.
varw:           db 0                    ; general-purpose variable.
varz:           db 0                    ; general-purpose variable.
contrl:			db 0              		; control, 0 = keyboard, 1 = Kempston, 2 = Sinclair, 3 = Mouse.

varrnd:			db 255            		; last random number.
varobj:         db 254                  ; last object number.
varopt:			db 255            		; last option chosen from menu.
varblk:			db 255            		; block type.

seed:			db 0             		; seed for random numbers.
sndtyp:         db 0
restfl:         db 0                    ; restart screen flag.
deadf:          db 0                    ; dead flag.
gamwon:         db 0                    ; game won flag.
dispx:          db 0                    ; cursor x position.
dispy:          db 0                    ; cursor y position.
                db 0                    ; padding
charx:          db 0                    ; cursor x position.
chary:          db 0                    ; cursor y position.
clock:			db 0              		; frame counter in 50ths of a second.
numob:          db NUMOBJ               ; number of objects in game.
roomtb:         db 34                   ; room number.

score:          db "000000"             ; player's score.
hiscor:			db "000000"       		; high score.
bonus: 			db "000000"       		; bonus.

; Make sure pointers are arranged in the same order as the data itself.
frmptr:         dl frmlst               ; sprite frames.
blkptr:         dl chgfx                ; block graphics.
;colptr:         dl bcol                 ; address of char colours.
proptr:         dl bprop                ; address of char properties.
scrptr:         dl scdat                ; address of screens.
nmeptr:         dl nmedat               ; enemy start positions.

; Don't change the order of these four.  Menu routine relies on winlft following wintop.
wintop:			db WINDOWTOP      		; top of window.
winlft:         db WINDOWLFT      		; left edge.
winhgt:         db WINDOWHGT      		; window height.
winwid:         db WINDOWWID      		; window width.

combyt:			db 0					; byte type compressed.
comcnt:			db 0					; compression counter.

; Sprite table.
; ix+0  = type.
; ix+1  = sprite image number.
; ix+2  = frame.
; ix+3  = x coord.
; ix+4  = y coord.

; ix+5  = new type.
; ix+6  = new image number.
; ix+7  = new frame.
; ix+8  = new x coord.
; ix+9  = new y coord.

; ix+10 = direction.
; ix+11 = parameter 1.
; ix+12 = parameter 2.
; ix+13 = jump pointer low.
; ix+14 = jump pointer high.
; ix+15 = data pointer low.
; ix+16 = data pointer high.
; ix+17 = data pointer top
; ix+18 = sprite colour
; ix+19 = new sprite colour

sprtab:
			ds	NUMSPR * TABSIZ 
ssprit:		db 255,255,255,255,255,255,255,0,192,120,0,0,0,255,255,255,255,255,15


MAP:		ds 24*32                ; main attributes map. Stores tile attributes

; =======================================================================================
