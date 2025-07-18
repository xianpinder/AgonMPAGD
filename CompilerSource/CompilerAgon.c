
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* Definitions. */

#define LF						0x0a
#define CR						0x0d

enum
{
	EVENT_SPRITE_0,
	EVENT_SPRITE_1,
	EVENT_SPRITE_2,
	EVENT_SPRITE_3,
	EVENT_SPRITE_4,
	EVENT_SPRITE_5,
	EVENT_SPRITE_6,
	EVENT_SPRITE_7,
	EVENT_SPRITE_8,
	EVENT_INITIALISE_SPRITE,
	EVENT_MAIN_LOOP_1,
	EVENT_MAIN_LOOP_2,
	EVENT_INTRO_MENU,
	EVENT_GAME_INITIALISATION,
	EVENT_RESTART_SCREEN,
	EVENT_FELL_TOO_FAR,
	EVENT_KILL_PLAYER,
	EVENT_LOST_GAME,
	EVENT_COMPLETED_GAME,
	EVENT_NEW_HIGH_SCORE,
	EVENT_COLLECTED_BLOCK,
	NUM_EVENTS												/* number of separate events to compile. */
};

#define MAX_EVENT_SIZE			500000						/* maximum size of compiled event routines. */
#define NUM_NESTING_LEVELS		12
#define NUM_REPEAT_LEVELS		3
#define SNDSIZ					41							/* size of each AY sound. */

#define NO_ARGUMENT				255
#define SPRITE_PARAMETER		0
#define NUMERIC					1

/* Game engine labels. */

#define PAM1ST					5							/* ix+5 is first sprite parameter. */
#define X						8							/* ix+8 is x coordinate of sprite. */
#define Y						9							/* ix+9 is y coordinate of sprite. */
#define GLOBALS					( VAR_EDGET - SPR_X + X )	/* global variables. */
#define IDIFF					( SPR_TYP - PAM1ST )		/* diff between type and first sprite param. */
#define ASMLABEL_DUMMY			1366


/* Here's the vocabulary. */

enum
{
	INS_IF = 1,
	INS_WHILE,
	INS_SPRITEUP,
	INS_SPRITEDOWN,
	INS_SPRITELEFT,
	INS_SPRITERIGHT,
	INS_ENDIF,
	INS_ENDWHILE,
	INS_CANGOUP,
	INS_CANGODOWN,
	INS_CANGOLEFT,
	INS_CANGORIGHT,
	INS_LADDERABOVE,
	INS_LADDERBELOW,
	INS_DEADLY,
	INS_CUSTOM,
	INS_TO,
	INS_FROM,
	INS_BY,

	FIRST_PARAMETER,
	SPR_TYP = FIRST_PARAMETER,
	SPR_IMG,
	SPR_FRM,
	SPR_X,
	SPR_Y,
	SPR_DIR,
	SPR_PMA,
	SPR_PMB,
	SPR_AIRBORNE,
	SPR_SPEED,

	FIRST_VARIABLE,
	VAR_EDGET = FIRST_VARIABLE,
	VAR_EDGEL,
	VAR_EDGEB,
	VAR_EDGER,
	VAR_SCREEN,
	VAR_LIV,
	VAR_A,
	VAR_B,
	VAR_C,
	VAR_D,
	VAR_E,
	VAR_F,
	VAR_G,
	VAR_H,
	VAR_I,
	VAR_J,
	VAR_K,
	VAR_L,
	VAR_M,
	VAR_N,
	VAR_O,
	VAR_P,
	VAR_Q,
	VAR_R,
	VAR_S,
	VAR_T,
	VAR_U,
	VAR_V,
	VAR_W,
	VAR_Z,
	VAR_CONTROL,
	VAR_LINE,
	VAR_COLUMN,
	VAR_CLOCK,
	VAR_RND,
	VAR_OBJ,
	VAR_OPT,
	VAR_BLOCK,
	LAST_PARAMETER = VAR_BLOCK,

	INS_GOT,
	INS_KEY,
	INS_DEFINEKEY,
	INS_COLLISION,
	INS_NUM,												/* number follows this marker. */
	OPE_EQU,												/* operators. */
	OPE_GRTEQU,
	OPE_GRT,
	OPE_NOT,
	OPE_LESEQU,
	OPE_LES,
	INS_LET,
	INS_ANIM,
	INS_ANIMBACK,
	INS_PUTBLOCK,
	INS_DIG,
	INS_NEXTLEVEL,
	INS_RESTART,
	INS_SPAWN,
	INS_REMOVE,
	INS_GETRANDOM,
	INS_RANDOMIZE,
	INS_ELSE,
	INS_DISPLAYHIGH,
	INS_DISPLAYSCORE,
	INS_DISPLAYBONUS,
	INS_SCORE,
	INS_BONUS,
	INS_ADDBONUS,
	INS_ZEROBONUS,
	INS_SOUND,
	INS_BEEP,
	INS_CRASH,
	INS_CLS,
	INS_BORDER,
	INS_COLOUR,
	INS_PAPER,
	INS_INK,
	INS_CLUT,
	INS_DELAY,
	INS_PRINT,
	INS_PRINTMODE,
	INS_AT,
	INS_CHR,
	INS_MENU,
	INS_INVENTORY,
	INS_KILL,
	INS_ADD,
	INS_SUB,
	INS_DISPLAY,
	INS_SCREENUP,
	INS_SCREENDOWN,
	INS_SCREENLEFT,
	INS_SCREENRIGHT,
	INS_WAITKEY,
	INS_JUMP,
	INS_FALL,
	INS_TABLEJUMP,
	INS_TABLEFALL,
	INS_OTHER,
	INS_SPAWNED,
	INS_ORIGINAL,
	INS_ENDGAME,
	INS_GET,
	INS_PUT,
	INS_REMOVEOBJ,
	INS_DETECTOBJ,
	INS_ASM,
	INS_EXIT,
	INS_REPEAT,
	INS_ENDREPEAT,
	INS_MULTIPLY,
	INS_DIVIDE,
	INS_SPRITEINK,
	INS_TRAIL,
	INS_LASER,
	INS_STAR,
	INS_EXPLODE,
	INS_REDRAW,
	INS_SILENCE,
	INS_CLW,
	INS_PALETTE,
	INS_GETBLOCK,
	INS_PLOT,
	INS_UNDOSPRITEMOVE,
	INS_READ,
	INS_DATA,
	INS_RESTORE,
	INS_TICKER,
	INS_USER,
	INS_DEFINEPARTICLE,
	INS_PARTICLEUP,
	INS_PARTICLEDOWN,
	INS_PARTICLELEFT,
	INS_PARTICLERIGHT,
	INS_DECAYPARTICLE,
	INS_NEWPARTICLE,
	INS_MESSAGE,
	INS_STOPFALL,
	INS_GETBLOCKS,
	INS_DOUBLEDIGITS,
	INS_TRIPLEDIGITS,
	INS_CLOCK,
	INS_MACHINE,
	INS_CALL,
	
	CMP_EVENT,
	CMP_DEFINEBLOCK,
	CMP_DEFINEWINDOW,
	CMP_DEFINESPRITE,
	CMP_DEFINESCREEN,
	CMP_SPRITEPOSITION,
	CMP_DEFINEOBJECT,
	CMP_MAP,
	CMP_STARTSCREEN,
	CMP_WIDTH,
	CMP_ENDMAP,
	CMP_DEFINEPALETTE,
	CMP_DEFINEMESSAGES,
	CMP_DEFINEFONT,
	CMP_DEFINEJUMP,
	CMP_DEFINECONTROLS,

	CON_LEFT,
	CON_RIGHT,
	CON_UP,
	CON_DOWN,
	CON_FIRE,
	CON_FIRE2,
	CON_FIRE3,
	CON_OPTION1,
	CON_OPTION2,
	CON_OPTION3,
	CON_OPTION4,
	CON_BULLET,
	CON_KEYBOARD,
	CON_KEMPSTON,
	CON_SINCLAIR,
	CON_BPROP0,
	CON_BPROP1,
	CON_BPROP2,
	CON_BPROP3,
	CON_BPROP4,
	CON_BPROP5,
	CON_BPROP6,
	CON_BPROP7,
	CON_BPROP8,
	CON_FAST,
	CON_MEDIUM,
	CON_SLOW,
	CON_VERYSLOW,
	CON_BLOCKWIDTH,
	CON_SPRITEWIDTH,
	CON_SPECTRUM,
	CON_CPC,
	CON_TIMEX,
	CON_ATOM,
	CON_DRAGON,
	CON_NEXT,
	CON_BBC,
	CON_ELECTRON,
	CON_MSX,

	CON_PLAYER,
	CON_TYPE1,
	CON_TYPE2,
	CON_TYPE3,
	CON_TYPE4,
	CON_TYPE5,
	CON_TYPE6,
	CON_TYPE7,
	CON_TYPE8,
	CON_TYPE9,
	CON_TYPE10,
	CON_TYPE11,
	CON_TYPE12,
	CON_TYPE13,
	CON_TYPE14,
	CON_TYPE15,
	CON_TYPE16,
	CON_TYPE17,
	CON_TYPE18,
	CON_TYPE19,
	CON_TYPE20,
	LAST_CONSTANT,
	FINAL_INSTRUCTION = LAST_CONSTANT,
	INS_STR
};

/* Machine formats. */
enum
{
	FORMAT_SPECTRUM,
	FORMAT_CPC,
	FORMAT_TIMEX,
	FORMAT_ATOM,
	FORMAT_DRAGON,
	FORMAT_NEXT,
	FORMAT_BBC,
	FORMAT_ELECTRON,
	FORMAT_MSX,
	FORMAT_AGON
};


/****************************************************************************************************************/
/* Function prototypes.                                                                                         */
/****************************************************************************************************************/

void StartEvent( unsigned short int nEvent );
void BuildFile( void );
void EndEvent( void );
void EndDummyEvent( void );
void CreateMessages( void );
void CreateBlocks( void );
void CreateSprites( void );
void CreateScreens( void );
void CreatePositions( void );
void CreateObjects( void );
void CreatePalette( void );
void CreateFont( void );
void CreateHopTable( void );
void CreateKeyTable( void );
void CreateDigCode( void );
unsigned short int NextKeyword( void );
void CountLines( unsigned char *cSrc );
unsigned short int GetNum( short int nBits );
void Compile( unsigned short int nInstruction );
void ResetIf( void );
void CR_If( void );
void CR_While( void );
void CR_SpriteUp( void );
void CR_SpriteDown( void );
void CR_SpriteLeft( void );
void CR_SpriteRight( void );
void CR_EndIf( void );
void CR_EndWhile( void );
void CR_CanGoUp( void );
void CR_CanGoDown( void );
void CR_CanGoLeft( void );
void CR_CanGoRight( void );
void CR_LadderAbove( void );
void CR_LadderBelow( void );
void CR_Deadly( void );
void CR_Custom( void );
void CR_To( void );
void CR_By( void );
void CR_ClS( void );
void CR_Got( void );
void CR_Key( void );
void CR_DefineKey( void );
void CR_Collision( void );
void CR_Anim( void );
void CR_AnimBack( void );
void CR_PutBlock( void );
void CR_Dig( void );
void CR_NextLevel( void );
void CR_Restart( void );
void CR_Spawn( void );
void CR_Remove( void );
void CR_GetRandom( void );
void CR_Randomize( void );
void CR_DisplayHighScore( void );
void CR_DisplayScore( void );
void CR_DisplayBonus( void );
void CR_Score( void );
void CR_Bonus( void );
void CR_AddBonus( void );
void CR_ZeroBonus( void );
void CR_Sound( void );
void CR_Beep( void );
void CR_Crash( void );
void CR_Border( void );
void CR_Colour( void );
void CR_Paper( void );
void CR_Ink( void );
void CR_Clut( void );
void CR_Delay( void );
void CR_Print( void );
void CR_PrintMode( void );
void CR_At( void );
void CR_Chr( void );
void CR_Menu( void );
void CR_Inventory( void );
void CR_Kill( void );
void CR_AddSubtract( void );
void CR_Display( void );
void CR_ScreenUp( void );
void CR_ScreenDown( void );
void CR_ScreenLeft( void );
void CR_ScreenRight( void );
void CR_WaitKey( void );
void CR_Jump( void );
void CR_Fall( void );
void CR_TableJump( void );
void CR_TableFall( void );
void CR_Other( void );
void CR_Spawned( void );
void CR_Original( void );
void CR_EndGame( void );
void CR_Get( void );
void CR_Put( void );
void CR_RemoveObject( void );
void CR_DetectObject( void );
void CR_Asm( void );
void CR_Exit( void );
void CR_Repeat( void );
void CR_EndRepeat( void );
void CR_Multiply( void );
void CR_Divide( void );
void CR_SpriteInk( void );
void CR_Trail( void );
void CR_Laser( void );
void CR_Star( void );
void CR_Explode( void );
void CR_Redraw( void );
void CR_Silence( void );
void CR_ClW( void );
void CR_Palette( void );
void CR_GetBlock( void );
void CR_Read( void );
void CR_Data( void );
void CR_Restore( void );
void CR_Plot( void );
void CR_UndoSpriteMove( void );
void CR_Ticker( void );
void CR_User( void );
void CR_DefineParticle( void );
void CR_ParticleUp( void );
void CR_ParticleDown( void );
void CR_ParticleLeft( void );
void CR_ParticleRight( void );
void CR_ParticleTimer( void );
void CR_StartParticle( void );
void CR_Message( void );
void CR_StopFall( void );
void CR_GetBlocks( void );
void CR_ValidateMachine( void );
void CR_Call( void );
void CR_Event( void );
void CR_DefineBlock( void );
void CR_DefineWindow( void );
void CR_DefineSprite( void );
void CR_DefineScreen( void );
void CR_SpritePosition( void );
void CR_DefineObject( void );
void CR_Map( void );
void CR_DefinePalette( void );
void CR_DefineMessages( void );
void CR_DefineFont( void );
void CR_DefineJump( void );
void CR_DefineControls( void );
unsigned char ConvertKey( short int nNum );

char SpriteEvent( void );
void CompileShift( short int nArg );
unsigned short int CompileArgument( void );
unsigned short int CompileKnownArgument( short int nArg );
unsigned short int NumberOnly( void );
void CR_Operator( unsigned short int nOperator );
void CR_Else( void );
void CR_Arg( void );
void CR_Pam( unsigned short int nParam );
void CR_ArgA( short int nNum );
void CR_ArgB( short int nNum );
void CR_PamA( short int nNum );
void CR_PamB( short int nNum );
void CR_PamC( short int nNum );
void CR_StackIf( void );
short int Joystick( short int nArg );
void CompileCondition( void );
void WriteJPNZ( void );
void WriteNumber( unsigned short int nInteger );
void WriteText( const char *cChar );
void WriteInstruction( const char *cCommand );
void WriteInstructionAndLabel( const char *cCommand );
void WriteInstructionArg( const char *cCommand, unsigned short int nNum );
void WriteLabel( unsigned short int nWhere );
void NewLine( void );
void Error( const char *cMsg );

/* Constants. */

const char *keywrd =
{
	/* Some keywords. */

	"IF."				// if.
	"WHILE."			// while.
	"SPRITEUP."			// move sprite up.
	"SPRITEDOWN."		// move sprite down.
	"SPRITELEFT."		// move sprite left.
	"SPRITERIGHT."		// move sprite right.
	"ENDIF."			// endif.
	"ENDWHILE."			// endwhile.
	"CANGOUP."			// sprite can go up test.
	"CANGODOWN."		// sprite can go down test.
	"CANGOLEFT."		// sprite can go left test.
	"CANGORIGHT."		// sprite can go right test.
	"LADDERABOVE."		// ladder above test.
	"LADDERBELOW."		// ladder below test.
	"DEADLY."			// check if touching deadly block.
	"CUSTOM."    		// check if touching custom block.
	"TO."           	// variable to increment.
	"FROM."          	// variable to decrement.
	"BY."	          	// multiply or divide by.

	/* Sprite variables. */

	"TYPE."				// first parameter.
	"IMAGE."			// image number.
	"FRAME."			// frame number.
	"Y."				// vertical position.
	"X."				// horizontal position.
	"DIRECTION."		// user sprite parameter.
	"SETTINGA."			// sprite parameter a.
	"SETTINGB."			// sprite parameter b.
	"AIRBORNE."			// sprite airborne flag.
	"JUMPSPEED."		// sprite jump/fall speed.

	/* Global variables.  There's a table of labels for these lower down. */

	"TOPEDGE."			// screen edge.
	"LEFTEDGE."			// screen edge.
	"BOTTOMEDGE."		// screen edge.
	"RIGHTEDGE."		// screen edge.
	"SCREEN."			// screen number.
	"LIVES."			// lives.
	"A."				// variable.
	"B."				// variable.
	"C."				// variable.
	"D."				// variable.
	"E."				// variable.
	"F."				// variable.
	"G."				// variable.
	"H."				// variable.
	"I."				// variable.
	"J."				// variable.
	"K."				// variable.
	"L."				// variable.
	"M."				// variable.
	"N."				// variable.
	"O."				// variable.
	"P."				// variable.
	"Q."				// variable.
	"R."				// variable.
	"S."				// variable.
	"T."				// variable.
	"U."				// variable.
	"V."				// variable.
	"W."				// variable.
	"Z."				// variable.
	"CONTROL."			// control.
	"LINE."				// x coordinate.
	"COLUMN."			// y coordinate.
	"CLOCK."			// clock.
	"RND."				// last random number variable.
	"OBJ."				// last object variable.
	"OPT."				// menu option variable.
	"BLOCK."			// block type variable.
	"GOT."				// function.
	"KEY."				// function.

	/* Commands. */

	"DEFINEKEY."		// define key.
	"COLLISION."		// collision with sprite.
	" ."				// number to follow.
	"=."				// equals, ignored.
	">=."				// greater than or equal to, ignored.
	">."				// greater than, ignored.
	"<>."				// not equal to, ignored.
	"<=."				// less than or equal to, ignored.
	"<."				// less than, ignored.
	"LET."				// x=y.
	"ANIMATE."			// animate sprite.
	"ANIMBACK."			// animate sprite backwards.
	"PUTBLOCK."			// put block on screen.
	"DIG."				// dig.
	"NEXTLEVEL."		// next level.
	"RESTART."			// restart game.
	"SPAWN."			// spawn new sprite.
	"REMOVE."			// remove this sprite.
	"GETRANDOM."		// variable.
	"RANDOMIZE."		// randomize.
	"ELSE."				// else.
	"SHOWHIGH."			// show highscore.
	"SHOWSCORE."		// show score.
	"SHOWBONUS."		// show bonus.
	"SCORE."			// increase score.
	"BONUS."			// increase bonus.
	"ADDBONUS."			// add bonus to score.
	"ZEROBONUS."		// add bonus to score.
	"SOUND."			// play sound.
	"BEEP."				// play beeper sound.
	"CRASH."			// play white noise sound.
	"CLS."				// clear screen.
	"BORDER."			// set border.
	"COLOUR."			// set all attributes.
	"PAPER."			// set PAPER attributes.
	"INK."				// set INK attributes.
	"CLUT."				// set CLUT attributes.
	"DELAY."			// pause for a while.
	"PRINT."			// display message.
	"PRINTMODE."		// changes text mode, 0 or 1.
	"AT."				// coordinates.
	"CHR."				// show character.
	"MENU."				// menu in a box.
	"INV."				// inventory menu.
	"KILL."				// kill the player.
	"ADD."				// add to variable.
	"SUBTRACT."			// subtract from variable.
	"DISPLAY."			// display number.
	"SCREENUP."			// up a screen.
	"SCREENDOWN."		// down a screen.
	"SCREENLEFT."		// left a screen.
	"SCREENRIGHT."		// right a screen.
	"WAITKEY."			// wait for keypress.
	"JUMP."				// jump.
	"FALL."				// fall.
	"TABLEJUMP."		// jump.
	"TABLEFALL."		// fall.
	"OTHER."			// select second collision sprite.
	"SPAWNED."			// select spawned sprite.
	"ENDSPRITE."		// select original sprite.
	"ENDGAME."			// end game with victory.
	"GET."				// get object.
	"PUT."				// drop object.
	"REMOVEOBJ."		// remove object.
	"DETECTOBJ."		// detect object.
	"ASM."				// encode.
	"EXIT."				// exit.
	"REPEAT."			// repeat.
	"ENDREPEAT."		// endrepeat.
	"MULTIPLY."			// multiply.
	"DIVIDE."			// divide.
	"SPRITEINK."		// set sprite ink.
	"TRAIL."			// leave a trail.
	"LASER."			// shoot a laser.
	"STAR."				// starfield.
	"EXPLODE."			// start a shrapnel explosion.
	"REDRAW."			// redraw the play area.
	"SILENCE."			// silence AY channels.
	"CLW."				// clear play area window.
	"PALETTE."			// set palette entry.
	"GETBLOCK."			// get block at screen position.
	"PLOT."				// plot pixel.
	"UNDOSPRITEMOVE."	// undo last sprite movement.
	"READ."				// read data.
	"DATA."				// block of data.
	"RESTORE."			// restore to start of list.
	"TICKER."			// ticker message.
	"USER."				// user routine.
	"DEFINEPARTICLE."	// define the user particle behaviour.
	"PARTICLEUP."		// move up.
	"PARTICLEDOWN."		// move down.
	"PARTICLELEFT."		// move left.
	"PARTICLERIGHT."	// move right.
	"PARTICLEDECAY."	// decrement timer and remove.
	"NEWPARTICLE."		// start a new user particle.
	"MESSAGE."			// display a message.
	"STOPFALL."			// stop falling.
	"GETBLOCKS."		// get collectable blocks.
	"DOUBLEDIGITS."		// show as double digits.
	"TRIPLEDIGITS."		// show as triple digits.
	"SECONDS."			// show as timer.
	"MACHINE."			// machine for which we're compiling.
	"CALL."				// call an external routine.

	/* compiler keywords. */
	"EVENT."			// change event.
	"DEFINEBLOCK."		// define block.
	"DEFINEWINDOW."		// define window.
	"DEFINESPRITE."		// define sprite.
	"DEFINESCREEN."		// define screen.
	"SPRITEPOSITION."	// define sprite position.
	"DEFINEOBJECT."		// define object.
	"MAP."				// set up map.
	"STARTSCREEN."		// start screen.
	"WIDTH."			// map width.
	"ENDMAP."			// end of map.
	"DEFINEPALETTE."	// define palette.
	"DEFINEMESSAGES."	// define messages.
	"DEFINEFONT."		// define font.
	"DEFINEJUMP."		// define jump table.
	"DEFINECONTROLS."	// define key table.

	/* Constants. */
	"LEFT."				// left constant (keys).
	"RIGHT."			// right constant (keys).
	"UP."				// up constant (keys).
	"DOWN."				// down constant (keys).
	"FIRE2."			// fire2 constant (keys).
	"FIRE3."			// fire3 constant (keys).
	"FIRE."				// fire constant (keys).
	"OPTION1."			// option constant (keys).
	"OPTION2."			// option constant (keys).
	"OPTION3."			// option constant (keys).
	"OPTION4."			// option constant (keys).
	"BULLET."			// collision bullet.
	"KEYBOARD."			// control option.
	"KEMPSTON."			// control option.
	"SINCLAIR."			// control option.
	"EMPTYBLOCK."		// empty space.
	"PLATFORMBLOCK."	// platform.
	"WALLBLOCK."		// wall.
	"LADDERBLOCK."		// ladder.
	"FODDERBLOCK."		// fodder.
	"DEADLYBLOCK."		// deadly.
	"CUSTOMBLOCK."		// custom.
	"WATERBLOCK."		// water.
	"COLLECTABLE."		// collectable.
	"FAST."				// animation speed.
	"MEDIUM."			// animation speed.
	"SLOW."				// animation speed.
	"VERYSLOW."			// animation speed.
	"BLOCKWIDTH."		// width of blocks.
	"SPRITEWIDTH."		// width of sprites.
	"ZX."				// machine format.
	"CPC."				// machine format.
	"TIMEX."			// machine format.
	"ATOM."				// machine format.
	"DRAGON."			// machine format.
	"NEXT."				// machine format.
	"BBC."				// machine format.
	"ELECTRON."			// machine format.
	"MSX."				// machine format.
	"PLAYER."			// player.
	"SPRITETYPE1."		// sprite type 1.
	"SPRITETYPE2."		// sprite type 2.
	"SPRITETYPE3."		// sprite type 3.
	"SPRITETYPE4."		// sprite type 4.
	"SPRITETYPE5."		// sprite type 5.
	"SPRITETYPE6."		// sprite type 6.
	"SPRITETYPE7."		// sprite type 7.
	"SPRITETYPE8."		// sprite type 8.
	"INITSPRITE."		// initialise sprite.
	"MAINLOOP1."		// main loop 1.
	"MAINLOOP2."		// main loop 2.
	"INTROMENU."		// main menu.
	"GAMEINIT."			// game initialisation.
	"RESTARTSCREEN."	// restart a screen.
	"FELLTOOFAR."		// sprite fell too far.
	"KILLPLAYER."		// kill player.
	"LOSTGAME."			// game over.
	"COMPLETEDGAME."	// won game.
	"NEWHIGHSCORE."		// new high score.
	"COLLECTBLOCK."		// collected block.
};

const short int nConstantsTable[] =
{
	0, 1, 2, 3, 5, 6, 4,		// keys left, right, up, down, fire, fire2, fire3.
	7, 8, 9, 10,				// keys option1, option2, option3, option4.
	10,							// laser bullet.
	0, 1, 2,					// keyboard and joystick controls.
	0, 1, 2, 3, 4,				// block types.
	5, 6, 7, 8,
	0, 1, 3, 7,					// animation speeds.
	8, 16,						// block and sprite widths.
	0, 1, 2, 3, 4, 5, 6, 7, 8,	// machine formats.
	EVENT_SPRITE_0,				// events.
	EVENT_SPRITE_1,
	EVENT_SPRITE_2,
	EVENT_SPRITE_3,
	EVENT_SPRITE_4,
	EVENT_SPRITE_5,
	EVENT_SPRITE_6,
	EVENT_SPRITE_7,
	EVENT_SPRITE_8,
	EVENT_INITIALISE_SPRITE,
	EVENT_MAIN_LOOP_1,
	EVENT_MAIN_LOOP_2,
	EVENT_INTRO_MENU,
	EVENT_GAME_INITIALISATION,
	EVENT_RESTART_SCREEN,
	EVENT_FELL_TOO_FAR,
	EVENT_KILL_PLAYER,
	EVENT_LOST_GAME,
	EVENT_COMPLETED_GAME,
	EVENT_NEW_HIGH_SCORE,
	EVENT_COLLECTED_BLOCK,
};

const unsigned char cVariables[][ 7 ] =
{
	"wntopx",			// top edge.
	"wnlftx",			// left edge.
	"wnbotx",			// bottom edge.
	"wnrgtx",			// right edge.
	"scno",				// screen number.
	"numlif",			// lives.
	"vara",				// variable.
	"varb",				// variable.
	"varc",				// variable.
	"vard",				// variable.
	"vare",				// variable.
	"varf",				// variable.
	"varg",				// variable.
	"varh",				// variable.
	"vari",				// variable.
	"varj",				// variable.
	"vark",				// variable.
	"varl",				// variable.
	"varm",				// variable.
	"varn",				// variable.
	"varo",				// variable.
	"varp",				// variable.
	"varq",				// variable.
	"varr",				// variable.
	"vars",				// variable.
	"vart",				// variable.
	"varu",				// variable.
	"varv",				// variable.
	"varw",				// variable.
	"varz",				// variable.
	"contrl",			// keyboard/Kempston/Sinclair controls.
	"charx",			// x coordinate.
	"chary",			// y coordinate.
	"clock",			// last clock reading.
	"varrnd",			// last random number variable.
	"varobj",			// last object variable.
	"varopt",			// last option chosen.
	"varblk"			// block variable.
};

unsigned char cDefaultPalette[] =
{
	0, 2, 20, 19, 128, 227, 200, 146, 0, 2, 20, 19, 128, 227, 200, 146,
	0, 35, 60, 63, 224, 227, 252, 255, 0, 35, 60, 63, 224, 227, 252, 255,
	0, 33, 39, 162, 243, 64, 128, 244, 0, 33, 39, 162, 243, 64, 128, 244,
	0, 44, 80, 120, 108, 109, 146, 219, 0, 44, 80, 120, 108, 109, 146, 219
};

unsigned char cDefaultFont[ 768 ];

/* Hop/jump table. */
unsigned char cDefaultHop[ 25 ] =
{
	248,250,252,254,254,255,255,255,0,0,0,1,1,1,2,2,4,6,8,8,8,99
};

unsigned char cDefaultKeys[ 11 ] =
{
	35,27,29,28,16,31,1,36,28,20,12
};

const unsigned char cKeyOrder[ 11 ] =
{
	3,2,1,0,4,5,6,7,8,9,10
};

/* Variables. */

unsigned long int nErrors = 0;
//unsigned short int nSourceLength = 0;
size_t lSize;									/* source file length. */
//unsigned long int lSize2;
unsigned short int nLine;
unsigned int nAddress = 0;							/* compilation start address. */
unsigned int nCurrent;								/* current compilation address. */
unsigned short int nDebug = 0;								/* debug output. */
unsigned short int nAdventure = 0;							/* adventure mode. */
unsigned short int nAssembler = 0;							/* assembler output. */
unsigned short int nAY = 0;									/* AY sound output. */
unsigned char *cBufPos;
unsigned char *cBuff;
unsigned char *cObjt;
unsigned char *cStart;
unsigned char *cErrPos;
unsigned short int nIfBuff[ NUM_NESTING_LEVELS ][ 2 ];		/* nested IF addresses. */
unsigned short int nNumIfs;									/* number of IFs. */
unsigned short int nReptBuff[ NUM_REPEAT_LEVELS ];			/* nested REPEAT addresses. */
unsigned short int nNumRepts;								/* number of REPEATs. */
unsigned short int nWhileBuff[ NUM_NESTING_LEVELS ][ 3 ];	/* nested WHILE addresses. */
unsigned short int nNumWhiles;								/* number of WHILEs. */
unsigned short int nGravity;								/* gravity call flag. */
unsigned short int nIfSet;
unsigned short int nPamType;								/* parameter type. */
unsigned short int nPamNum;									/* parameter number. */
unsigned short int nLastOperator;							/* last operator. */
unsigned short int nLastCondition;							/* IF or WHILE. */
unsigned short int nOpType;									/* operation type, eg add or subtract. */
unsigned short int nIncDec = 0;								/* non-zero when only inc or dec needed. */
unsigned short int nNextLabel;								/* label to write. */
short int nEvent;									/* event number passed to compiler */
unsigned short int nAnswerWantedHere;						/* where to put the result of add, sub, mul or div. */
char cSingleEvent;											/* whether we're building one event or rebuilding the lot. */
char cConstant;												/* non-zero when dealing with a constant. */
unsigned short int nConstant;								/* last constant. */
unsigned short int nMessageNumber = 0;						/* number of text messages in the game. */
unsigned short int nScreen = 0;								/* number of screens. */
unsigned short int nPositions = 0;							/* number of sprite positions. */
unsigned short int nObjects = 0;							/* number of objects. */
unsigned short int nParticle = 0;							/* non-zero when user has written custom routine. */
unsigned short int nReadingControls = 0;					/* Non-zero when reading controls in a WHILE loop. */
unsigned char cData = 0;									/* non-zero when we've encountered a data statement. */
unsigned char cDataRequired = 0;							/* non-zero when we need to find data. */
unsigned char cWindow = 0;									/* non-zero when window defined. */
unsigned char cPalette = 0;
unsigned short int nList[ NUM_EVENTS ];						/* number of data elements in each event. */
short int nWinTop = 0;										/* window position. */
short int nWinLeft = 0;
short int nWinHeight = 0;									/* window dimensions. */
short int nWinWidth = 0;
short int nStartScreen = -1;								/* starting screen. */
unsigned char cMapWid = 0;									/* width of map. */
short int nStartOffset = 0;									/* starting screen offset. */
short int nUseFont = 0;										/* use custom font when non-zero. */
short int nUseHopTable = 0;									/* use jump table when non-zero. */
short int nDig = 0;											/* append dig code when non-zero. */

FILE *pObject;												/* output file. */
//FILE *pEngine;												/* engine source file. */
FILE *pDebug;												/* debugger source file. */
FILE *pWorkMsg;												/* message work file. */
FILE *pWorkBlk;												/* block work file. */
FILE *pWorkSpr;												/* sprite work file. */
FILE *pWorkScr;												/* screen layout work file. */
FILE *pWorkNme;												/* sprite position work file. */
FILE *pWorkObj;												/* objects work file. */

/* Functions */
int main( int argc, const char* argv[] )
{
	//short int nChr = 0;
	//short int nTmp;
	int nArgs;
	FILE *pSource;
	char szDebugFilename[ 13 ] = { "debugzx.asm" };
	//const char *szEngineFilename = "engineagon.asm";
	char szSourceFilename[ 128 ];
	char szObjectFilename[ 128 ];
	char szWorkFile1Name[ 128 ];
	char szWorkFile2Name[ 128 ];
	char szWorkFile3Name[ 128 ];
	char szWorkFile4Name[ 128 ];
	char szWorkFile5Name[ 128 ];
	char szWorkFile6Name[ 128 ];
	char cTemp;
	char* cChar;

	cChar = &cTemp;

	puts( "AGD Compiler for Agon version 0.7.10" );
	puts( "(C) Jonathan Cauldwell May 2020" );
	puts( "AgonLight/Console8 port v0.1 by Christian Pinder" );

	if ( argc >= 2 && argc <= 6 )
	{
		cSingleEvent = 0;
		nEvent = -1;
		nMessageNumber = 0;
	}
	else
	{
		fputs( "Usage: Compiler ProjectName\n-a\tAdventure mode\n-y\tEnable AY sound effects\neg: CompilerAgon TEST", stderr );
		// invalid number of command line arguments
		exit ( 1 );
	}

	nArgs = argc - 2;
	while ( nArgs-- > 0 )
	{
		if ( argv[ nArgs + 2 ][ 0 ] == '-' || argv[ nArgs + 2 ][ 0 ] == '/' )
		{
			switch( argv[ nArgs + 2 ][ 1 ] )
			{
				case 'a':
				case 'A':
					nAdventure = 1;
					break;
				case 'y':
				case 'Y':
					nAY = 1;
					break;
				default:
					fputs( "Unrecognised switch", stderr );
					exit ( 1 );
					break;
			}
		}
	}

	/* Open target files. */
	sprintf( szObjectFilename, "%s.asm", argv[ 1 ] );
	pObject = fopen( szObjectFilename, "wb" );

	if ( !pObject )
	{
        fprintf( stderr, "Unable to create target file: %s\n", szObjectFilename );
		exit ( 1 );
	}

	sprintf( szWorkFile1Name, "%s.txt", argv[ 1 ] );
	pWorkMsg = fopen( szWorkFile1Name, "wb" );
	if ( !pWorkMsg )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile1Name );
		exit ( 1 );
	}

	sprintf( szWorkFile2Name, "%s.blk", argv[ 1 ] );
	pWorkBlk = fopen( szWorkFile2Name, "wb" );
	if ( !pWorkBlk )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile2Name );
		exit ( 1 );
	}

	sprintf( szWorkFile3Name, "%s.spr", argv[ 1 ] );
	pWorkSpr = fopen( szWorkFile3Name, "wb" );
	if ( !pWorkSpr )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile3Name );
		exit ( 1 );
	}

	sprintf( szWorkFile4Name, "%s.scl", argv[ 1 ] );
	pWorkScr = fopen( szWorkFile4Name, "wb" );
	if ( !pWorkScr )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile4Name );
		exit ( 1 );
	}

	sprintf( szWorkFile5Name, "%s.nme", argv[ 1 ] );
	pWorkNme = fopen( szWorkFile5Name, "wb" );
	if ( !pWorkNme )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile5Name );
		exit ( 1 );
	}

	sprintf( szWorkFile6Name, "%s.ojt", argv[ 1 ] );
	pWorkObj = fopen( szWorkFile6Name, "wb" );
	if ( !pWorkObj )
	{
       	fprintf( stderr, "Unable to create work file: %s\n", szWorkFile6Name );
		exit ( 1 );
	}

	fprintf( pObject, "       INCLUDE \"engine.asm\"\n\n" );

	//fprintf( pObject, "\n       jp start\n" );
	//fprintf( pObject, "\n\n" );

	/* Find the engine. */
	//pEngine = fopen( szEngineFilename, "r" );
	//if ( !pEngine )
	//{
	//	fprintf(stderr, "Cannot find %s\n", szEngineFilename);
	//	exit ( 1 );
	//}

	/* Allocate buffer for the target code. */
	cObjt = ( unsigned char* )malloc( MAX_EVENT_SIZE );
	cStart = cObjt;
	if ( !cObjt )
	{
		fputs( "Out of memory\n", stderr );
		exit ( 1 );
	}

	/* Process single file. */
	sprintf( szSourceFilename, "%s.agd", argv[ 1 ] );
	printf( "Sourcename: %s\n", szSourceFilename );

	/* Open source file. */
	pSource = fopen( szSourceFilename, "r" );
	lSize = fread( cBuff, 1, lSize, pSource );

	if ( pSource )
	{
		/* Establish its size. */
		fseek( pSource, 0, SEEK_END );
		lSize = ftell( pSource );
		rewind( pSource );

		/* Allocate buffer for the script source code. */
		cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );
		if ( !cBuff )
		{
			fputs( "Out of memory\n", stderr );
			exit ( 1 );
		}

		/* Read source file into the buffer. */
		lSize = fread( cBuff, 1, lSize, pSource );

		/* Compile our target */
		cErrPos = cBufPos = cBuff;							/* start of buffer */
		nLine = 1;											/* line number */

		BuildFile();

		/* Close source file and free up the memory. */
		fclose( pSource );
		free( cBuff );

		/* user particle routine not defined, put a ret here. */
		if ( nParticle == 0 )
		{
			WriteInstructionAndLabel( "ptcusr: ret" );
		}

		if ( cWindow == 0 )
		{
			fputs( "DEFINEWINDOW missing\n", stderr );
			exit ( 1 );
		}

		fwrite( cStart, 1, nCurrent - nAddress, pObject );	/* write output to file. */
	}

	/* output textfile messages to assembly. */
	fclose( pWorkMsg );
	pWorkMsg = fopen( szWorkFile1Name, "rb" );
	if ( !pWorkMsg )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile1Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkMsg, 0, SEEK_END );
	lSize = ftell( pWorkMsg );
	rewind( pWorkMsg );

	/* Allocate buffer for the work file text. */
	cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

	if ( !cBuff )
	{
		fputs( "Out of memory\n", stderr );
		exit ( 1 );
	}

	cBufPos = cBuff;										/* start of buffer */

	/* Read text file into the buffer. */
	lSize = fread( cBuff, 1, lSize, pWorkMsg );

	CreateMessages();

	fwrite( cStart, 1, nCurrent - nAddress, pObject );
	free( cBuff );

	/* Now process the screen layouts. */
	fclose( pWorkScr );
	pWorkScr = fopen( szWorkFile4Name, "rb" );
	if ( !pWorkScr )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile4Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkScr, 0, SEEK_END );
	lSize = ftell( pWorkScr );
	rewind( pWorkScr );

	if ( lSize > 0 )
	{
		/* Allocate buffer for the work file text. */
		cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

		if ( !cBuff )
		{
			fputs( "Out of memory\n", stderr );
			exit ( 1 );
		}

		cBufPos = cBuff;									/* start of buffer */

		/* Read data file into the buffer. */
		lSize = fread( cBuff, 1, lSize, pWorkScr );

		CreateScreens();
		fwrite( cStart, 1, nCurrent - nAddress, pObject );
		free( cBuff );
	}

	/* Now process the blocks. */
	fclose( pWorkBlk );
	pWorkBlk = fopen( szWorkFile2Name, "rb" );
	if ( !pWorkBlk )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile2Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkBlk, 0, SEEK_END );
	lSize = ftell( pWorkBlk );
	rewind( pWorkBlk );

	if ( lSize > 0 )
	{
		/* Allocate buffer for the work file text. */
		cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

		if ( !cBuff )
		{
			fputs( "Out of memory\n", stderr );
			exit ( 1 );
		}

		cBufPos = cBuff;									/* start of buffer */

		/* Read data file into the buffer. */
		lSize = fread( cBuff, 1, lSize, pWorkBlk );

		CreateBlocks();
		fwrite( cStart, 1, nCurrent - nAddress, pObject );
		free( cBuff );
	}

	/* Now process the sprites. */
	fclose( pWorkSpr );
	pWorkSpr = fopen( szWorkFile3Name, "rb" );
	if ( !pWorkSpr )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile3Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkSpr, 0, SEEK_END );
	lSize = ftell( pWorkSpr );
	rewind( pWorkSpr );

	if ( lSize > 0 )
	{
		/* Allocate buffer for the work file text. */
		cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

		if ( !cBuff )
		{
			fputs( "Out of memory\n", stderr );
			exit ( 1 );
		}

		cBufPos = cBuff;									/* start of buffer */

		/* Read data file into the buffer. */
		lSize = fread( cBuff, 1, lSize, pWorkSpr );

		CreateSprites();
		fwrite( cStart, 1, nCurrent - nAddress, pObject );
		free( cBuff );
	}

	/* Now process the sprite positions. */
	fclose( pWorkNme );
	pWorkNme = fopen( szWorkFile5Name, "rb" );
	if ( !pWorkNme )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile5Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkNme, 0, SEEK_END );
	lSize = ftell( pWorkNme );
	rewind( pWorkNme );

	if ( lSize > 0 )
	{
		/* Allocate buffer for the work file text. */
		cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

		if ( !cBuff )
		{
			fputs( "Out of memory\n", stderr );
			exit ( 1 );
		}

		cBufPos = cBuff;									/* start of buffer */

		/* Read data file into the buffer. */
		lSize = fread( cBuff, 1, lSize, pWorkNme );

		CreatePositions();
		fwrite( cStart, 1, nCurrent - nAddress, pObject );
		free( cBuff );
	}

	/* generate assembly data for objects. */
	fclose( pWorkObj );
	pWorkObj = fopen( szWorkFile6Name, "rb" );
	if ( !pWorkObj )
	{
       	fprintf( stderr, "Unable to read work file: %s\n", szWorkFile6Name );
		exit ( 1 );
	}

	/* Establish its size. */
	fseek( pWorkObj, 0, SEEK_END );
	lSize = ftell( pWorkObj );
	rewind( pWorkObj );

	/* Allocate buffer for the work file text. */
	cBuff = ( unsigned char* )malloc( sizeof( char )*lSize );

	if ( !cBuff )
	{
		fputs( "Out of memory\n", stderr );
		exit ( 1 );
	}

	cBufPos = cBuff;										/* start of buffer */

	/* Read file into the buffer. */
	lSize = fread( cBuff, 1, lSize, pWorkObj );

	CreateObjects();
	CreatePalette();
	CreateFont();
	CreateHopTable();
	CreateKeyTable();
	CreateDigCode();

	fwrite( cStart, 1, nCurrent - nAddress, pObject );
	free( cBuff );

#if 0
	/* Find the engine. */
	pEngine = fopen( szEngineFilename, "r" );
	if ( !pEngine )
	{
		fputs( "Cannot find enginezx.asm\n", stderr );
		exit ( 1 );
	}

	/* Copy the engine to the target file. */
	lSize = fread( cChar, 1, 1, pEngine );			/* read first character of engine source. */

	fprintf( pObject, "\n\n" );

	while ( lSize > 0 )
	{
		fwrite( cChar, 1, 1, pObject );				/* write code to output file. */
		lSize = fread( cChar, 1, 1, pEngine );		/* read next byte of source. */
	}
#endif

	fprintf( pObject, "\n\n" );
	if ( nAY > 0 )
	{
		fprintf( pObject, "plsnd: call plsnd1\n" );
		fprintf( pObject, "       call plsnd2\n" );
		fprintf( pObject, "       call plsnd3\n" );
		fprintf( pObject, "       jp w8912\n" );
	}
	else
	{
		fprintf( pObject, "plsnd: ret\n" );
	}

	//if ( nDebug == 0 )
	//{
	//	fprintf( pObject, "DEBUG  equ mloop\n" );
	//}
	//else
	//{
	//	fprintf( pObject, "DEBUG  equ dbloop\n" );
	//}

	if ( nAdventure > 0 )
	{
		fprintf( pObject, "wbloc:  ld hl,(pbptr)\n" );
		fprintf( pObject, "        push af\n" );
		fprintf( pObject, "        ld a,(scno)\n" );
		fprintf( pObject, "        ld (hl),a\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        ld a,(dispx)\n");
		fprintf( pObject, "        ld (hl),a\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        ld a,(dispy)\n");
		fprintf( pObject, "        ld (hl),a\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        pop af\n" );
		fprintf( pObject, "        ld (hl),a\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        ld (pbptr),hl\n" );
		fprintf( pObject, "        ret\n" );

		fprintf( pObject, "ibloc:  ld hl,eop\n" );
		fprintf( pObject, "        ld (pbptr),hl\n" );
		fprintf( pObject, "        jp initsc\n" );

		fprintf( pObject, "rbloc:  ld hl,eop\n" );
		fprintf( pObject, "rbloc2: ld bc,4\n" );
		fprintf( pObject, "       ld de,(pbptr)\n" );
		fprintf( pObject, "       and a\n" );
		fprintf( pObject, "rbloc1: push hl\n" );
		fprintf( pObject, "       sbc hl,de\n" );
		fprintf( pObject, "       pop hl\n" );
		fprintf( pObject, "       ret nc\n" );
		fprintf( pObject, "       ld a,(scno)\n" );
		fprintf( pObject, "       cp (hl)\n" );
		fprintf( pObject, "       jr z,rbloc0\n" );
		fprintf( pObject, "       add hl,bc\n" );
		fprintf( pObject, "        jr rbloc1\n" );
		fprintf( pObject, "rbloc0: inc hl\n" );
		fprintf( pObject, "        ld a,(hl)\n" );
		fprintf( pObject, "        ld (dispx),a\n");
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        ld a,(hl)\n" );
		fprintf( pObject, "        ld (dispy),a\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        ld a,(hl)\n" );
		fprintf( pObject, "        inc hl\n" );
		fprintf( pObject, "        push hl\n" );
		fprintf( pObject, "        call pattr\n" );
		fprintf( pObject, "        pop hl\n" );
		fprintf( pObject, "        jr rbloc2\n" );
		fprintf( pObject, "pbptr:  dl eop\n" );
	}
	else
	{
		fprintf( pObject, "ibloc:  jp initsc\n");
		fprintf( pObject, "rbloc:  ret\n");
	}

	fprintf( pObject, "eop:\n");

	/* Close target file and free up the memory. */
	fclose( pObject );
	free( cStart );

	/* Delete workfiles. */
	fclose( pWorkMsg );
	fclose( pWorkBlk );
	fclose( pWorkSpr );
	fclose( pWorkScr );
	fclose( pWorkNme );
	fclose( pWorkObj );
	remove( szWorkFile1Name );
	remove( szWorkFile2Name );
	remove( szWorkFile3Name );
	remove( szWorkFile4Name );
	remove( szWorkFile5Name );
	remove( szWorkFile6Name );

	printf( "Output: %s\n", szObjectFilename );

	return ( nErrors );
}

/* Sets up the label at the start of each event. */
void StartEvent( unsigned short int nEvent )
{
	unsigned short int nCount;
	char cLine[ 14 ];
	char *cChar = cLine;

	*cChar = 0;

	/* reset compilation address. */
	nCurrent = nAddress;
	nOpType = 0;
//	nRepeatAddress = ASMLABEL_DUMMY;
	nNextLabel = 0;
	nNumRepts = 0;
	for ( nCount = 0; nCount < NUM_REPEAT_LEVELS; nCount++ )
	{
		nReptBuff[ nCount ] = ASMLABEL_DUMMY;
	}

	cObjt = cStart + ( nCurrent - nAddress );
	if ( nEvent < 99 )
	{
		sprintf( cLine, "\nevnt%02d:", nEvent );		/* don't write label for dummy event. */
	}

	while ( *cChar )
	{
		*cObjt = *cChar++;
		cObjt++;
		nCurrent++;
	}

	/* Reset the IF address stack. */
	nNumIfs = 0;
	nIfSet = 0;
	nNumWhiles = 0;
	nGravity = 0;
	ResetIf();

	/* Reset number of DATA statement elements. */
	//fprintf(stderr, "nEvent = %d\n", nEvent);
	//assert (nEvent < 21);
	if (nEvent < NUM_EVENTS)
		nList[ nEvent ] = 0;
	cData = 0;
	cDataRequired = 0;

	for ( nCount = 0; nCount < NUM_NESTING_LEVELS; nCount++ )
	{
		nIfBuff[ nCount ][ 0 ] = 0;
		nIfBuff[ nCount ][ 1 ] = 0;
		nWhileBuff[ nCount ][ 0 ] = 0;
		nWhileBuff[ nCount ][ 1 ] = 0;
		nWhileBuff[ nCount ][ 2 ] = 0;
	}
}

/* Build our object file */
void BuildFile( void )
{
//	unsigned short int nCount;
	unsigned short int nKeyword;

//	for ( nCount = 0; nCount < NUM_NESTING_LEVELS; nCount++ )
//	{
//		nIfBuff[ nCount ][ 0 ] = 0;
//		nIfBuff[ nCount ][ 1 ] = 0;
//		nWhileBuff[ nCount ][ 0 ] = 0;
//		nWhileBuff[ nCount ][ 1 ] = 0;
//		nWhileBuff[ nCount ][ 2 ] = 0;
//	}

	do
	{
		nKeyword = NextKeyword();
		if ( nKeyword < FINAL_INSTRUCTION &&
			 nKeyword > 0 )
		{
			Compile( nKeyword );
		}
	}
	while ( cBufPos < ( cBuff + lSize ) );

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();											/* always put a ret at the end. */
	}
}

void EndEvent( void )
{
	if ( nGravity > 0 )										/* did we use jump or fall? */
	{
		WriteInstruction( "jp grav" );						/* finish with call to gravity routine. */
	}
	else
	{
		WriteInstruction( "ret" );							/* always put a ret at the end. */
	}

	if ( nNumIfs > 0 )
	{
		Error( "Missing ENDIF" );
	}

	if ( nNumRepts > 0 )
	{
		Error( "Missing ENDREPEAT" );
	}

	if ( nNumWhiles > 0 )
	{
		Error( "Missing ENDWHILE" );
	}

	if ( cDataRequired != 0 && cData == 0 )
	{
		Error( "Missing DATA" );
	}

	fwrite( cStart, 1, nCurrent - nAddress, pObject );		/* write output to file. */
	nEvent = -1;
	nCurrent = nAddress;
}

void EndDummyEvent( void )
{
	fwrite( cStart, 1, nCurrent - nAddress, pObject );		/* write output to file. */
	nEvent = -1;
	nCurrent = nAddress;
}

void CreateMessages( void )
{
	short int nStart = 1;

	/* Set up source address. */
	unsigned char *cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nmsgdat:" );

	while ( ( cSrc - cBuff ) < lSize )
	{
		while ( *cSrc < 128 )								/* end marker? */
		{
			if ( *cSrc == 13 || *cSrc == 39 || *cSrc == '"')
			{
				if ( nStart )
				{
					WriteText( "\n       db " );			/* single defb */
				}
				else
				{
					WriteText( "\"," );						/* end quote and comma */
				}
				WriteNumber( *cSrc++ );						/* write as numeric outside quotes */
				nStart = 1;
			}
			else
			{
				if ( nStart )
				{
					WriteText( "\n       db \"" );			/* start of text message */
					nStart = 0;
				}
				*cObjt = *cSrc++;
				cObjt++;
				nCurrent++;
			}
		}

		if ( nStart )
		{
			WriteText( "\n       db " );					/* single defb */
		}
		else
		{
			WriteText( "\"," );								/* end quote and comma */
		}

		WriteNumber( *cSrc++ );								/* write last char with terminator bit */
		nStart = 1;
	}

	/* Number of messages */
	WriteText( "\nnummsg: db " );
	WriteNumber( nMessageNumber );
}

void xCreateBlocks( void )
{
	short int nData;
	unsigned char *cSrc;									/* source pointer. */
	short int nCounter = 0;
	char cType[ 256 ];
	short int nAttr[ 256 ];

	/* Set up source address. */
	cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nchgfx:" );

	do
	{
		WriteText( "\n       db " );						/* start of text message */
		nData = 0;
		cType[ nCounter ] = *cSrc++;						/* store type in array */
		while ( nData++ < 7 )
		{
			WriteNumber( *cSrc++ );							/* write byte of data */
			WriteText( "," );								/* put a comma */
		}

		WriteNumber( *cSrc++ );								/* write final byte of graphic data */
		nAttr[ nCounter ] = *cSrc++;						/* store attribute in array */
		nCounter++;
	}
	while ( ( cSrc - cBuff ) < lSize );

	/* Now do the block attributes. */
	WriteText( "\nbcol:" );
	nData = 0;
	while ( nData < nCounter )
	{
		WriteText( "\n       db " );
		WriteNumber( nAttr[ nData++ ] );
	}

	/* Now do the block properties. */
	WriteText( "\nbprop:" );
	nData = 0;
	while ( nData < nCounter )
	{
		WriteText( "\n       db " );
		WriteNumber( cType[ nData++ ] );
	}
}


// cDefaultPalette
// convert GGGRRRBB to AABBGGRR
int getAgonColour (int zxcol)
{
	int bgr = cDefaultPalette[zxcol];

	int abgr = 128 + 64; // both bits of AA set
	abgr |= (bgr >> 3) & 3;		// red
	abgr |= (bgr >> 4) & 12;
	abgr |= (bgr << 4) & 48;
	return abgr;
}

void CreateBlocks(void)
{
	int nCounter = 0;
	char cType[256];
	short int nAttr[256];
	char txt[100];

	// Set up source address.
	unsigned char *cSrc = cBufPos;

	// reset compilation address.
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + (nCurrent - nAddress);
	WriteText("\nchgfx:");

	do
	{
		cType[nCounter] = *cSrc++;				// store type in array

		sprintf (txt, "\nblock%d:", nCounter);
		WriteText(txt);

		WriteText("\n       db 1");				// type 1 bitmap ARGB2222
		WriteText("\n       dw 16,16");			// 16x16 size
		WriteText("\n       dw 256");			// 16 * 16

		int attrib = cSrc[8];
		int bright = (attrib & 64) != 0;
		int paper = (attrib >> 3) & 7;
		int ink = attrib & 7;
		if (bright)
		{
			paper += 8;
			ink += 8;
		}
		paper = getAgonColour(paper);
		ink = getAgonColour(ink);

		// we need to convert a 8x8 monochrome bitmap into a 16x16 colour bitmap
		for (int row = 0; row < 8; row++)
		{
			int zxbyte = *cSrc++;
			for (int i = 0; i < 2; i++)
			{
				int bw = zxbyte;
				WriteText("\n       db ");						/* start of text message */
				for (int col = 0; col < 8; col++)
				{
					int bit = bw >> 7;
					int agcol = bit == 0 ? paper : ink;
					bw = (bw << 1) & 255;
					WriteNumber(agcol);
					WriteText(",");
					WriteNumber(agcol);							/* write byte of data */
					if (col != 7)
						WriteText(",");						/* put a comma */
				}
			}
		}
		WriteText("\n");						// blank line separator

		nAttr[nCounter] = *cSrc++;						/* store attribute in array */
		nCounter++;
	} while ((cSrc - cBuff) < lSize);

	// Now do the block pointers.
	WriteText("\nblkptrs:");
	for (int nData = 0; nData < nCounter; nData++)
	{
		sprintf(txt, "\n       dl block%d", nData);
		WriteText(txt);
	}

	// Now do the block properties.
	WriteText("\nbprop:");
	for (int nData = 0; nData < nCounter; nData++)
	{
		WriteText("\n       db ");
		WriteNumber(cType[nData]);
	}

	// Now store the number of blocks in the game.
	WriteText("\nnumbl:  db ");
	WriteNumber(nCounter);
}



void CreateSprites(void)
{
	int nFrame;
	int cFrames[256];
	char txt[200];

	// Set up source address
	unsigned char *cSrc = cBufPos;

	// reset compilation address
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + (nCurrent - nAddress);
	WriteText("\nsprgfx:");

	int sprbmp = 0;
	int nCounter = 0;
	do
	{
		int frameCount = *cSrc++;
		cFrames[nCounter++] = frameCount;	// store num frames in array

		for (nFrame = 0; nFrame < frameCount; nFrame++)
		{
			sprintf(txt, "\nsprbmp%d:", sprbmp++);
			WriteText(txt);

			WriteText("\n       db 1");				// type 1 bitmap ARGB2222
			WriteText("\n       dw 32,32");			// 32x32 size
			WriteText("\n       dw 1024");			// 32 * 32

			int paper = 0;		// transparent black
			int ink = 255;		// solid white

			// we need to convert a 16x16 monochrome bitmap into a 32x32 colour bitmap
			for (int row = 0; row < 16; row++)
			{
				int zxbyte[2];
				zxbyte[0] = *cSrc++;
				zxbyte[1] = *cSrc++;
				for (int i = 0; i < 2; i++)
				{
					WriteText("\n       db ");
					for (int byte = 0; byte < 2; byte++)
					{
						int bw = zxbyte[byte];
						for (int col = 0; col < 8; col++)
						{
							int bit = bw >> 7;
							int agcol = bit == 0 ? paper : ink;
							bw = (bw << 1) & 255;
							WriteNumber(agcol);
							WriteText(",");
							WriteNumber(agcol);
							if ((byte * 8) + col != 15)
								WriteText(",");	
						}
					}
				}
			}
			WriteText("\n");						// blank line separator
		}
	} while ((cSrc - cBuff) < lSize);

	/* Now do the frame list. */
	WriteText("\nfrmlst:");
	
	nFrame = 0;
	for (int nData = 0; nData < nCounter; nData++)
	{
		WriteText("\n       db ");
		//WriteInstruction("db ");
		WriteNumber(nFrame);
		WriteText(",");
		WriteNumber(cFrames[nData]);
		nFrame += cFrames[nData];
	}

	WriteText(",");
	WriteNumber(nFrame);
	WriteText(",0");

	// Now do the sprite bmp pointers.
	WriteText("\nsprbmpptrs:");
	for (int nData = 0; nData < nFrame; nData++)
	{
		sprintf(txt, "\n       dl sprbmp%d", nData);
		WriteText(txt);
	}

	// Now store the number of sprite bmps in the game.
	WriteText("\nnumsprbmp:  db ");
	WriteNumber(nFrame);

}


void CreateScreens( void )
{
	short int nThisScreen = 0;
	short int nBytes = 0;										/* bytes to write. */
	//short int nByteCount;
	short int nColumn = 0;
	short int nCount = 0;
	short int nByte = 0;
	short int nFirstByte = -1;
	short int nScreenSize = 0;
	unsigned char *cSrc;										/* source pointer. */

	/* Set up source address. */
	cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nscdat:" );

	/* write compressed screen sizes */
	nColumn = 99;

	while ( nThisScreen++ < nScreen )
	{
		nScreenSize = 0;
		nBytes = nWinWidth * nWinHeight;

		while ( nBytes > 0 )
		{
			nCount = 0;
			nFirstByte = *cSrc;									/* fetch first byte. */

			do
			{
				nByte = *++cSrc;
				nCount++;										/* count the bytes. */
				nBytes--;
			}
			while ( nByte == nFirstByte && nCount < 256 && nBytes > 0 );

			if ( nCount > 3 )
			{
				nScreenSize += 3;
			}
			else
			{
				while ( nCount-- > 0 )
				{
					nScreenSize++;
				}
			}
		}

		if ( nColumn > 24 )
		{
			WriteInstruction( "dw " );
			nColumn = 0;
		}
		else
		{
			WriteText( "," );
			nColumn++;
		}

		WriteNumber( nScreenSize );
	}

	/* Restore source address. */
	cSrc = cBufPos;

	/* Now do the compression. */
	nThisScreen = 0;

	while ( nThisScreen++ < nScreen )
	{
		nBytes = nWinWidth * nWinHeight;
		nColumn = 99;											/* fresh defb line for each screen. */

		while ( nBytes > 0 )
		{
			nCount = 0;
			nFirstByte = *cSrc;									/* fetch first byte. */

			do
			{
				nCount++;										/* count the bytes. */
				nBytes--;
				nByte = *++cSrc;
			}
			while ( nByte == nFirstByte && nCount < 256 && nBytes > 0 );

			if ( nColumn > 32 )
			{
				nColumn = 0;
				WriteInstruction( "db " );
			}
			else
			{
				WriteText( "," );
			}

			if ( nCount > 3 )
			{
				WriteNumber( 255 );
				WriteText( "," );
				WriteNumber( nFirstByte );
				WriteText( "," );
				WriteNumber( nCount & 255 );
				nColumn += 3;
			}
			else
			{
				while ( nCount-- > 0 )
				{
					WriteNumber( nFirstByte );
					nColumn++;
					if ( nCount > 0 )
					{
						WriteText( "," );
					}
				}
			}
		}
	}

	/* Now store the number of screens in the game. */
	WriteText( "\nnumsc:  db " );
	WriteNumber( nScreen );
}

void CreateScreens2( void )
{
	short int nThisScreen = 0;
	short int nBytes = nWinWidth * nWinHeight;				/* bytes to write. */
	short int nColumn = 0;
	short int nCount = 0;
	unsigned char *cSrc;									/* source pointer. */

	/* Set up source address. */
	cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nscdat:" );
	nCount = 0;

	while ( nThisScreen < nScreen )
	{
		WriteInstruction( "dw " );
		WriteNumber( nBytes );
		nCount = 0;
		nColumn = 33;
		while ( nCount < nBytes )
		{
			if ( nColumn >= nWinWidth )
			{
				WriteInstruction( "db " );
				nColumn = 0;
			}
			else
			{
				WriteText( "," );
			}

			WriteNumber( *cSrc++ );							/* write byte of data */
			nColumn++;
			nCount++;
		}
		nThisScreen++;
	}

	/* Now store the number of screens in the game. */
	WriteText( "\nnumsc:  db " );
	WriteNumber( nScreen );
}

void CreatePositions( void )
{
	short int nThisScreen = 0;
	short int nCount;
	short int nData;
	short int nPos = 0;
	char cScrn;
	unsigned char *cSrc;									/* source pointer. */

	/* Set up source address. */
	cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nnmedat: db " );
	cScrn = *cSrc++;										/* get screen. */

	while ( nPos < nPositions && nThisScreen < nScreen )
	{
		while ( cScrn > nThisScreen )						/* write null entries for screens with no sprites. */
		{
			WriteNumber( 255 );
			nThisScreen++;
			WriteText( "," );
		}

		for ( nCount = 0; nCount < 4; nCount++ )
		{
			nData = *cSrc++;
			WriteNumber( nData );
			WriteText( "," );
		}

		nPos++;												/* one more position defined. */

		if ( nPos < nPositions )
		{
			cScrn = *cSrc++;								/* get screen. */
			if ( cScrn != nThisScreen )
			{
				WriteNumber( 255 );							/* end marker for screen. */
				WriteInstruction( "db " );
//				WriteText( "\n" );
				nThisScreen++;
			}
		}
		else
		{
			WriteNumber( 255 );								/* end marker for whole game. */
			nThisScreen = nScreen;
		}
	}
}

void CreateObjects( void )
{
	char txt[200];

	// Set up source address.
	unsigned char *cSrc = cBufPos;

	/* reset compilation address. */
	nCurrent = nAddress;
	nNextLabel = 0;

	cObjt = cStart + ( nCurrent - nAddress );
	WriteText( "\nNUMOBJ: equ " );
	WriteNumber( nObjects );

	if (nObjects == 0)
	{
		WriteText("\nobjdta: dw 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
		WriteInstruction("db 0,254,0,0,254,0,0");
		return;
	}
	WriteText( "\nobjdta:" );

	for (int nCount = 0; nCount < nObjects; nCount++ )
	{
		int cAttr = *cSrc++;								/* get attribute. */
		int cScrn = *cSrc++;								/* get screen. */
		int cX = *cSrc++;									/* get x. */
		int cY = *cSrc++;									/* get y. */
		cSrc += 32;
		WriteInstruction( "db " );
		WriteNumber(nCount);
		WriteText( "," );
		WriteNumber( cScrn );
		WriteText( "," );
		WriteNumber( cX );
		WriteText( "," );
		WriteNumber( cY );
		WriteText( "," );
		WriteNumber( cScrn );
		WriteText( "," );
		WriteNumber( cX );
		WriteText( "," );
		WriteNumber( cY );
	}

	// Set up source address
	cSrc = cBufPos;

	for (int nCount = 0; nCount < nObjects; nCount++)
	{
		int cAttr = *cSrc++;								// get attribute
		cSrc += 3;											// skip screen, y, x

		sprintf(txt, "\nobjbmp%d:", nCount);
		WriteText(txt);

		WriteText("\n       db 1");				// type 1 bitmap ARGB2222
		WriteText("\n       dw 32,32");			// 32x32 size
		WriteText("\n       dw 1024");			// 32 * 32

		int attrib = cAttr;
		int bright = (attrib & 64) != 0;
		int paper = (attrib >> 3) & 7;
		int ink = attrib & 7;
		if (bright)
		{
			paper += 8;
			ink += 8;
		}
		paper = getAgonColour(paper);
		ink = getAgonColour(ink);

		// we need to convert a 16x16 monochrome bitmap into a 32x32 colour bitmap
		for (int row = 0; row < 16; row++)
		{
			int zxbyte[2];
			zxbyte[0] = *cSrc++;
			zxbyte[1] = *cSrc++;
			for (int i = 0; i < 2; i++)
			{
				WriteText("\n       db ");						/* start of text message */
				for (int byte = 0; byte < 2; byte++)
				{
					int bw = zxbyte[byte];
					for (int col = 0; col < 8; col++)
					{
						int bit = bw >> 7;
						int agcol = bit == 0 ? paper : ink;
						bw = (bw << 1) & 255;
						WriteNumber(agcol);
						WriteText(",");
						WriteNumber(agcol);							/* write byte of data */
						if ((byte * 8) + col != 15)
							WriteText(",");						/* put a comma */
					}
				}
			}
		}
		WriteText("\n");						// blank line separator
	}

	// Now do the object bitmap pointers
	WriteText("\nobjbmpptrs:");
	for (int nData = 0; nData < nObjects; nData++)
	{
		sprintf(txt, "\n       dl objbmp%d", nData);
		WriteText(txt);
	}
}

void CreatePalette()
{
	int levels[4] = {0,85,170,255};
	char txt[200];
//	19, l, p, r, g, b

	WriteText ("\npalett:");

	for (int i = 0; i < 16; i++)
	{
		int grb = cDefaultPalette[i];	// GGGRRRBB

		int r = levels[(grb >> 3) & 3];
		int g = levels[(grb >> 6) & 3];
		int b = levels[grb & 3];

		sprintf (txt, "\n    db 19,%d,255,%d,%d,%d", i, r, g, b);
		WriteText (txt);
	}
	WriteText("\nendpalett:\n");
}

void CreateFont( void )
{
	short int nChar = 0;
	short int nByte;

	if ( nUseFont > 0 )
	{
		WriteText( "\nfont:" );
		for ( nChar = 0; nChar < 96; nChar++ )
		{
			WriteInstruction( "db " );
			for ( nByte = 0; nByte < 8; nByte++ )
			{
				WriteNumber( cDefaultFont[ nChar * 8 + nByte ] );
				if ( nByte < 7 )
				{
					WriteText( "," );
				}
			}
		}
	}
	else
	{
		WriteText( "\nfont:   equ 15616" );
	}
}

void CreateHopTable( void )
{
	short int nChar = 0;

	WriteText( "\njtab:" );
	nChar = 0;
	WriteInstruction( "db " );

	if ( nUseHopTable > 0 )
	{
		while ( cDefaultHop[ nChar ] != 99 )
		{
			WriteNumber( cDefaultHop[ nChar++ ] );
			WriteText( "," );
		}
	}

	WriteNumber( 99 );
}

void CreateKeyTable( void )
{
	short int nKey;

	WriteText( "\nkeys:   db " );

	for ( nKey = 0; nKey < 10; nKey++ )
	{
		WriteNumber( cDefaultKeys[ nKey ] );
		WriteText( "," );
	}

	WriteNumber( cDefaultKeys[ nKey ] );
}

void CreateDigCode( void )
{
	if ( nDig > 0 )
	{
		WriteText( "\ndig:    and 3" );
		WriteInstruction( "jr z,digl" );
		WriteInstruction( "dec a" );
		WriteInstruction( "jr z,digr" );
		WriteInstruction( "dec a" );
		WriteInstruction( "jr z,digu" );

		WriteInstruction( "ld h,(ix+9)" );
		WriteInstruction( "ld a,16" );
		WriteInstruction( "add a,(ix+8)" );
		WriteInstruction( "ld l,a" );
		WriteInstruction( "jr digv" );

		WriteText( "\ndigu:   ld a,(ix+8)" );
		WriteInstruction( "ld h,(ix+9)" );
		WriteInstruction( "sub 2" );
		WriteInstruction( "ld l,a" );
		WriteText( "\ndigv:   ld a,l" );
		WriteInstruction("ld (dispx),a");
		WriteInstruction("ld a,h");
		WriteInstruction("ld (dispy),a");
		WriteInstruction( "call tstbl" );
		WriteInstruction( "call fdchk" );
		WriteInstruction( "inc hl" );
		WriteInstruction( "call fdchk" );
		WriteInstruction( "ld a,(dispy)" );
		WriteInstruction( "and 7" );
		WriteInstruction( "ret z" );
		WriteInstruction( "inc hl" );
		WriteInstruction( "jp fdchk" );

		WriteText( "\ndigl:   ld l,(ix+8)" );
		WriteInstruction( "ld a,(ix+9)" );
		WriteInstruction( "sub 2" );
		WriteInstruction( "ld h,a" );
		WriteText( "\ndigh:   ld a,l" );
		WriteInstruction( "ld (dispx),a");
		WriteInstruction( "ld a,h");
		WriteInstruction( "ld (dispy),a");
		WriteInstruction( "ld a,l" );
		WriteInstruction( "and 7" );
		WriteInstruction( "ld a,3" );
		WriteInstruction( "jr nz,digh1" );
		WriteInstruction( "dec a" );
		WriteText( "\ndigh1:  ld b,a" );
		WriteInstruction( "call tstbl" );
		WriteText( "\ndigh0:  push bc" );
		WriteInstruction( "call fdchk" );
		WriteInstruction( "ld de,32" );
		WriteInstruction( "add hl,de" );
		WriteInstruction( "pop bc" );
		WriteInstruction( "djnz digh0" );
		WriteInstruction( "ret" );
		WriteText( "\ndigr:   ld l,(ix+8)" );
		WriteInstruction( "ld a,(ix+9)" );
		WriteInstruction( "add a,16" );
		WriteInstruction( "ld h,a" );
		WriteInstruction( "jr digh" );
	}
}

/* Return next keyword number */
unsigned short int NextKeyword( void )
{
	unsigned short int nFound;
	unsigned short int nSrc = 0;
	unsigned short int nResWord = INS_IF;					/* ID of reserved word we're checking. */
	unsigned short int nWord = 0;
	unsigned short int nLength = 0;							/* length of literal string. */
	const char *cRes;								/* reserved word pointer. */
	unsigned char *cSrcSt;									/* source pointer, word start. */
	unsigned char *cSrc;									/* source pointer. */
	unsigned char cText;
	unsigned char cEnd = 0;

	/* Set up source address. */
	cSrc = cBufPos;
	nFound = 0;

	/* Skip spaces, newlines, comments etc. */
	do
	{
		if ( *cSrc == ';' )									/* comment marker? */
		{
			do												/* skip to next line */
			{
				cSrc++;
			}
			while ( *cSrc != LF && *cSrc != CR );
//			nLine++;
		}
		else
		{
			if ( isdigit( *cSrc ) ||						/* valid character? */
				 isalpha( *cSrc ) ||
				 *cSrc == '$' ||
				 ( *cSrc >= '<' && *cSrc <= '>' ) )
			{
				nFound++;									/* flag as legitimate. */
			}
			else											/* treat as space. */
			{
				CountLines( cSrc );

				if ( *cSrc == '"' )							/* found a string? */
				{
					nLength = 0;
					cSrc++;									/* skip to first letter */
					while( *cSrc != '"' || cEnd > 0 )
					{
						CountLines( cSrc );					/* line feed inside quotes */
						cText = *cSrc;						/* this is the character to write */
						cSrc++;
						if ( ( cSrc - cBuff ) >= lSize )
						{
							cText |= 128;					/* terminate the end of the string */
							cEnd++;
						}
						if ( cEnd > 0 )
						{
							cText |= 128;					/* terminate the end of the string */
						}
						if ( *cSrc == '"' )
						{
							if ( *( cSrc + 1 ) == '"' )		/* double quote, so skip second and continue */
							{
								fwrite( &cText, 1, 1, pWorkMsg );
								cText = '"';
								cSrc += 2;
							}
							else
							{
								cText |= 128;				/* single quote so terminate the string */
							}
						}
//						if ( *cSrc == '"' || cEnd > 0 )
//						{
//							cText |= 128;					/* terminate the end of the string */
//						}
						fwrite( &cText, 1, 1, pWorkMsg );	/* write character to workfile. */
						nLength++;
					}
					if ( nLength > 0 )
					{
						nFound++;
						cBufPos = ++cSrc;
						return( INS_STR );
					}
				}
				else
				{
					if ( *cSrc == '\'' )					/* found an ASCII character? */
					{
						if ( *( cSrc + 2 ) == '\'' &&		/* single char within single quote? */
							 ( 2 + cSrc - cBuff ) < lSize )
						{
							cBufPos = cSrc;
							return( INS_NUM );
						}
					}
				}
				cSrc++;										/* skip character */
			}
		}
	}
	while ( !nFound && ( ( cSrc - cBuff ) < lSize ) );

	if ( !nFound )											/* end of file */
	{
		cBufPos = cBuff + lSize + 1;
		return( 0 );
	}

	if ( isdigit( *cSrc ) ||								/* encountered a number. */
		 *cSrc == '$' )
	{
		cBufPos = cSrc;
		return( INS_NUM );
	}

	/* Point to the reserved words and off we go */
	cSrcSt = cSrc;
	cRes = keywrd;
	nFound = 0;

	do
	{
		if ( toupper( *cSrc ) == *cRes )
		{
			cSrc++;
			cRes++;
		}
		else
		{
			if ( *cRes == '.' &&							/* end marker for reserved word? */
//				 ( *cSrc <= '<' || *cSrc >= '>' ) &&
				 ( toupper( *cSrc ) < 'A' ||				/* no more of source word? */
				   toupper( *cSrc ) > 'Z' ||
				   cSrc >= ( cBuff + lSize ) ) )			/* EOF before NL/CR. */
			{
				nFound++;									/* great, matched a reserved word. */
			}
			else
			{
				nResWord++;									/* keep tally of words we've skipped. */
				if ( nResWord < FINAL_INSTRUCTION )
				{
					while ( *cRes++ != '.' );				/* find next reserved word. */
					cSrc = cSrcSt;							/* back to start of source word. */
				}
				else
				{
					while ( isalpha( *cSrc ) ||				/* find end of unrecognised word. */
						 ( *cSrc >= '<' && *cSrc <= '>' ) )
					{
						cSrc++;
					}
				}
			}
		}
	}
	while ( !nFound && nResWord < FINAL_INSTRUCTION );

	if ( !nFound )
	{
		Error( "Unrecognised instruction" );
	}

	cBufPos = cSrc;

	/* Treat constants as numbers */
	if ( nResWord >= CON_LEFT &&
		 nResWord < LAST_CONSTANT )
	{
		cConstant++;
		nConstant = nConstantsTable[ nResWord - CON_LEFT ];
		nResWord = INS_NUM;
	}

	return ( nResWord );
}

void CountLines( unsigned char *cSrc )
{
	if ( *cSrc == CR )
	{
		cSrc++;
	}
	if ( *cSrc == LF )
	{
		nLine++;											/* count lines */
		cErrPos = cBufPos;									/* store position for error reporting. */
	}
}

/* Return number. */
unsigned short int GetNum( short int nBits )
{
	unsigned long int lNum = 0;
	unsigned short int nNum = 0;
	unsigned char *cSrc;									/* source pointer. */

	if ( cConstant > 0 )									/* dealing with a constant? */
	{
		cConstant = 0;										/* expect a number next time. */
		return ( nConstant );
	}

	cSrc = cBufPos;

	if ( isdigit( *cSrc ) )									/* plain old numeric */
	{
		while( isdigit( *cSrc ) && cSrc < ( cBuff + lSize ) )
		{
			lNum = 10 * lNum + ( *cSrc - '0' );
			cSrc++;
		}
	}
	else
	{
		if ( *cSrc == '$' )									/* hexadecimal */
		{
			cSrc++;											/* skip to digits */
			while( isdigit( *cSrc ) ||
				 ( toupper( *cSrc ) >= 'A' && toupper( *cSrc ) <= 'F' ) )
			{
				lNum <<= 4;
				if ( isdigit( *cSrc ) )
				{
					lNum += ( *cSrc - '0' );
				}
				else
				{
					lNum += ( toupper( *cSrc ) - 55 );
				}
				cSrc++;
			}
		}
		else												/* ASCII char within single quotes */
		{
			lNum = *( ++cSrc );
			cSrc += 2;
		}
	}

	if ( nBits == 8 && lNum > 255 )
	{
		Error( "Number too big, must be between 0 and 255" );
	}
	else
	{
		if ( lNum > 65535 )
		{
			Error( "Number too big, must be between 0 and 65535" );
		}
	}

	nNum = ( short int )lNum;

	if ( nBits == 8 )
	{
		nNum &= 0xff;
	}

	cBufPos = cSrc;

	return ( nNum );
}


/* Parsed an instruction, this routine deals with it. */

void Compile( unsigned short int nInstruction )
{
	switch( nInstruction )
	{
		case INS_IF:
			CR_If();
			break;
		case INS_WHILE:
			CR_While();
			break;
		case INS_SPRITEUP:
			CR_SpriteUp();
			break;
		case INS_SPRITEDOWN:
			CR_SpriteDown();
			break;
		case INS_SPRITELEFT:
			CR_SpriteLeft();
			break;
		case INS_SPRITERIGHT:
			CR_SpriteRight();
			break;
		case INS_ENDIF:
			CR_EndIf();
			break;
		case INS_ENDWHILE:
			CR_EndWhile();
			break;
		case INS_CANGOUP:
			CR_CanGoUp();
			break;
		case INS_CANGODOWN:
			CR_CanGoDown();
			break;
		case INS_CANGOLEFT:
			CR_CanGoLeft();
			break;
		case INS_CANGORIGHT:
			CR_CanGoRight();
			break;
		case INS_LADDERABOVE:
			CR_LadderAbove();
			break;
		case INS_LADDERBELOW:
			CR_LadderBelow();
			break;
		case INS_DEADLY:
			CR_Deadly();
			break;
		case INS_CUSTOM:
			CR_Custom();
			break;
		case INS_TO:
		case INS_FROM:
			CR_To();
			break;
		case INS_BY:
			CR_By();
			break;
		case INS_NUM:
			CR_Arg();
			break;
		case OPE_EQU:
		case OPE_GRT:
		case OPE_GRTEQU:
		case OPE_NOT:
		case OPE_LESEQU:
		case OPE_LES:
			CR_Operator( nInstruction );
			break;
		case INS_LET:
			ResetIf();
			break;
		case INS_ELSE:
			CR_Else();
			break;
		case VAR_EDGET:
		case VAR_EDGEB:
		case VAR_EDGEL:
		case VAR_EDGER:
		case VAR_SCREEN:
		case VAR_LIV:
		case VAR_A:
		case VAR_B:
		case VAR_C:
		case VAR_D:
		case VAR_E:
		case VAR_F:
		case VAR_G:
		case VAR_H:
		case VAR_I:
		case VAR_J:
		case VAR_K:
		case VAR_L:
		case VAR_M:
		case VAR_N:
		case VAR_O:
		case VAR_P:
		case VAR_Q:
		case VAR_R:
		case VAR_S:
		case VAR_T:
		case VAR_U:
		case VAR_V:
		case VAR_W:
		case VAR_Z:
		case VAR_CONTROL:
		case VAR_LINE:
		case VAR_COLUMN:
		case VAR_CLOCK:
		case VAR_RND:
		case VAR_OBJ:
		case VAR_OPT:
		case VAR_BLOCK:
		case SPR_TYP:
		case SPR_IMG:
		case SPR_FRM:
		case SPR_X:
		case SPR_Y:
		case SPR_DIR:
		case SPR_PMA:
		case SPR_PMB:
		case SPR_AIRBORNE:
		case SPR_SPEED:
			CR_Pam( nInstruction );
			break;
		case INS_GOT:
			CR_Got();
			break;
		case INS_KEY:
			CR_Key();
			break;
		case INS_DEFINEKEY:
			CR_DefineKey();
			break;
		case INS_COLLISION:
			CR_Collision();
			break;
		case INS_ANIM:
			CR_Anim();
			break;
		case INS_ANIMBACK:
			CR_AnimBack();
			break;
		case INS_PUTBLOCK:
			CR_PutBlock();
			break;
		case INS_DIG:
			CR_Dig();
			break;
		case INS_NEXTLEVEL:
			CR_NextLevel();
			break;
		case INS_RESTART:
			CR_Restart();
			break;
		case INS_SPAWN:
			CR_Spawn();
			break;
		case INS_REMOVE:
			CR_Remove();
			break;
		case INS_GETRANDOM:
			CR_GetRandom();
			break;
		case INS_RANDOMIZE:
			CR_Randomize();
			break;
		case INS_DISPLAYHIGH:
			CR_DisplayHighScore();
			break;
		case INS_DISPLAYSCORE:
			CR_DisplayScore();
			break;
		case INS_DISPLAYBONUS:
			CR_DisplayBonus();
			break;
		case INS_SCORE:
			CR_Score();
			break;
		case INS_BONUS:
			CR_Bonus();
			break;
		case INS_ADDBONUS:
			CR_AddBonus();
			break;
		case INS_ZEROBONUS:
			CR_ZeroBonus();
			break;
		case INS_SOUND:
			CR_Sound();
			break;
		case INS_BEEP:
			CR_Beep();
			break;
		case INS_CRASH:
			CR_Crash();
			break;
		case INS_CLS:
			CR_ClS();
			break;
		case INS_BORDER:
			CR_Border();
			break;
		case INS_COLOUR:
			CR_Colour();
			break;
		case INS_PAPER:
			CR_Paper();
			break;
		case INS_INK:
			CR_Ink();
			break;
		case INS_CLUT:
			CR_Clut();
			break;
		case INS_DELAY:
			CR_Delay();
			break;
		case INS_PRINTMODE:
			CR_PrintMode();
			break;
		case INS_PRINT:
			CR_Print();
			break;
		case INS_AT:
			CR_At();
			break;
		case INS_CHR:
			CR_Chr();
			break;
		case INS_MENU:
			CR_Menu();
			break;
		case INS_INVENTORY:
			CR_Inventory();
			break;
		case INS_KILL:
			CR_Kill();
			break;
		case INS_ADD:
			nOpType = 129;									/* code for ADD A,C (needed by CR_To). */
			CR_AddSubtract();
			break;
		case INS_SUB:
			nOpType = 145;									/* code for SUB C (needed by CR_To). */
			CR_AddSubtract();
			break;
		case INS_DISPLAY:
			CR_Display();
			break;
		case INS_SCREENUP:
			CR_ScreenUp();
			break;
		case INS_SCREENDOWN:
			CR_ScreenDown();
			break;
		case INS_SCREENLEFT:
			CR_ScreenLeft();
			break;
		case INS_SCREENRIGHT:
			CR_ScreenRight();
			break;
		case INS_WAITKEY:
			CR_WaitKey();
			break;
		case INS_JUMP:
			CR_Jump();
			break;
		case INS_FALL:
			CR_Fall();
			break;
		case INS_TABLEJUMP:
			CR_TableJump();
			break;
		case INS_TABLEFALL:
			CR_TableFall();
			break;
		case INS_OTHER:
			CR_Other();
			break;
		case INS_SPAWNED:
			CR_Spawned();
			break;
		case INS_ORIGINAL:
			CR_Original();
			break;
		case INS_ENDGAME:
			CR_EndGame();
			break;
		case INS_GET:
			CR_Get();
			break;
		case INS_PUT:
			CR_Put();
			break;
		case INS_REMOVEOBJ:
			CR_RemoveObject();
			break;
		case INS_DETECTOBJ:
			CR_DetectObject();
			break;
		case INS_ASM:
			CR_Asm();
			break;
		case INS_EXIT:
			CR_Exit();
			break;
		case INS_REPEAT:
			CR_Repeat();
			break;
		case INS_ENDREPEAT:
			CR_EndRepeat();
			break;
		case INS_MULTIPLY:
			CR_Multiply();
			break;
		case INS_DIVIDE:
			CR_Divide();
			break;
		case INS_SPRITEINK:
			CR_SpriteInk();
			break;
		case INS_TRAIL:
			CR_Trail();
			break;
		case INS_LASER:
			CR_Laser();
			break;
		case INS_STAR:
			CR_Star();
			break;
		case INS_EXPLODE:
			CR_Explode();
			break;
		case INS_REDRAW:
			CR_Redraw();
			break;
		case INS_SILENCE:
			CR_Silence();
			break;
		case INS_CLW:
			CR_ClW();
			break;
		case INS_PALETTE:
			CR_Palette();
			break;
		case INS_GETBLOCK:
			CR_GetBlock();
			break;
		case INS_PLOT:
			CR_Plot();
			break;
		case INS_UNDOSPRITEMOVE:
			CR_UndoSpriteMove();
			break;
		case INS_READ:
			CR_Read();
			break;
		case INS_DATA:
			CR_Data();
			break;
		case INS_RESTORE:
			CR_Restore();
			break;
		case INS_TICKER:
			CR_Ticker();
			break;
		case INS_USER:
			CR_User();
			break;
		case INS_DEFINEPARTICLE:
			CR_DefineParticle();
			break;
		case INS_PARTICLEUP:
			CR_ParticleUp();
			break;
		case INS_PARTICLEDOWN:
			CR_ParticleDown();
			break;
		case INS_PARTICLELEFT:
			CR_ParticleLeft();
			break;
		case INS_PARTICLERIGHT:
			CR_ParticleRight();
			break;
		case INS_DECAYPARTICLE:
			CR_ParticleTimer();
			break;
		case INS_NEWPARTICLE:
			CR_StartParticle();
			break;
		case INS_MESSAGE:
			CR_Message();
			break;
		case INS_STOPFALL:
			CR_StopFall();
			break;
		case INS_GETBLOCKS:
			CR_GetBlocks();
			break;
		case INS_MACHINE:
			CR_ValidateMachine();
			break;
		case INS_CALL:
			CR_Call();
			break;
		case CMP_EVENT:
			CR_Event();
			break;
		case CMP_DEFINEBLOCK:
			CR_DefineBlock();
			break;
		case CMP_DEFINEWINDOW:
			CR_DefineWindow();
			break;
		case CMP_DEFINESPRITE:
			CR_DefineSprite();
			break;
		case CMP_DEFINESCREEN:
			CR_DefineScreen();
			break;
		case CMP_SPRITEPOSITION:
			CR_SpritePosition();
			break;
		case CMP_DEFINEOBJECT:
			CR_DefineObject();
			break;
		case CMP_MAP:
			CR_Map();
			break;
		case CMP_DEFINEPALETTE:
			CR_DefinePalette();
			break;
		case CMP_DEFINEMESSAGES:
			CR_DefineMessages();
			break;
		case CMP_DEFINEFONT:
			CR_DefineFont();
			break;
		case CMP_DEFINEJUMP:
			CR_DefineJump();
			break;
		case CMP_DEFINECONTROLS:
			CR_DefineControls();
			break;
		default:
			printf( "Instruction %d not handled\n", nInstruction );
			break;
	}
}

void ResetIf( void )
{
	nIfSet = 0;
	nPamType = NO_ARGUMENT;
	nPamNum = 255;
}

/****************************************************************************************************************/
/* Individual compilation routines.                                                                             */
/****************************************************************************************************************/

void CR_If( void )
{
	nLastOperator = OPE_EQU;
	nLastCondition = INS_IF;
	ResetIf();
	nIfSet = 1;
}

void CR_While( void )
{
	nLastOperator = OPE_EQU;
	nLastCondition = INS_WHILE;
	ResetIf();
	nIfSet = 1;
	nNextLabel = nCurrent;									/* label for unconditional jump to start of while loop. */
	nWhileBuff[ nNumWhiles ][ 2 ] = nCurrent;				/* remember label for ENDWHILE. */
	nReadingControls = 0;									/* haven't read the joystick/keyboard in this loop yet. */
}

void CR_SpriteUp( void )
{
	WriteInstruction( "dec (ix+8)" );
	WriteInstruction( "dec (ix+8)" );
}

void CR_SpriteDown( void )
{
	WriteInstruction( "inc (ix+8)" );
	WriteInstruction( "inc (ix+8)" );
}

void CR_SpriteLeft( void )
{
	WriteInstruction( "dec (ix+9)" );
	WriteInstruction( "dec (ix+9)" );
}

void CR_SpriteRight( void )
{
	WriteInstruction( "inc (ix+9)" );
	WriteInstruction( "inc (ix+9)" );
}

void CR_EndIf( void )
{
	unsigned short int nAddr1;
	unsigned short int nAddr2;

	if ( nNumIfs > 0 )
	{
		nAddr2 = nCurrent;									/* where we are is ENDIF address. */

		if ( nIfBuff[ --nNumIfs ][ 1 ] > 0 )				/* is there a second jump address to write? */
		{
			nAddr1 = nIfBuff[ nNumIfs ][ 1 ];				/* where to put label after conditional jump. */
			nCurrent = nAddr1;								/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful IF. */
			nCurrent = nIfBuff[ nNumIfs ][ 0 ];				/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful IF. */
			nIfBuff[ nNumIfs ][ 1 ] = 0;					/* done with this now, don't rewrite address later. */
		}
		else
		{
			nAddr1 = nIfBuff[ nNumIfs ][ 0 ];				/* where to put label after conditional jump. */
//			nAddr2 = nCurrent;								/* where we are is ENDIF address. */
			nCurrent = nAddr1;								/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful IF. */
		}

		nNextLabel = nCurrent;
		nCurrent = nAddr2;
		nNextLabel = nAddr2;
	}
	else
	{
		Error( "ENDIF without IF" );
	}
}

void CR_EndWhile( void )
{
	unsigned short int nAddr1;
	unsigned short int nAddr2;
	//unsigned short int nAddr3;

	if ( nNumWhiles > 0 )
	{
		if ( nReadingControls > 0 )							/* are we reading the joystick in this loop? */
		{
			WriteInstruction( "call joykey" );				/* user might be writing a sub-game! */
			nReadingControls = 0;							/* not foolproof, but it's close enough. */
		}

		WriteInstruction( "jp " );
		WriteLabel( nWhileBuff[ --nNumWhiles ][ 2 ] );		/* unconditional loop back to start of while. */

		nAddr2 = nCurrent;									/* where we are is ENDIF address. */

//		if ( nWhileBuff[ --nNumWhiles ][ 1 ] > 0 )			/* is there a second jump address to write? */
		if ( nWhileBuff[ nNumWhiles ][ 1 ] > 0 )			/* is there a second jump address to write? */
		{
			nAddr1 = nWhileBuff[ nNumWhiles ][ 1 ];			/* where to put label after conditional jump. */
			nCurrent = nAddr1;								/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful WHILE. */
			nCurrent = nWhileBuff[ nNumWhiles ][ 0 ];		/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful WHILE. */
			nWhileBuff[ nNumWhiles ][ 1 ] = 0;				/* done with this now, don't rewrite address later. */
		}
		else
		{
			nAddr1 = nWhileBuff[ nNumWhiles ][ 0 ];			/* where to put label after conditional jump. */
			nCurrent = nAddr1;								/* go back to end of original condition. */
			WriteLabel( nAddr2 );							/* set jump address for unsuccessful WHILE. */
		}

		nNextLabel = nCurrent;
		nCurrent = nAddr2;
		nNextLabel = nAddr2;
//		WriteInstruction( "jp " );
//		WriteLabel( nWhileBuff[ nNumWhiles ][ 2 ] );		/* unconditional loop back to start of while. */
		nWhileBuff[ nNumWhiles ][ 2 ] = 0;
	}
	else
	{
		Error( "ENDWHILE without WHILE" );
	}
}

void CR_CanGoUp( void )
{
	WriteInstruction( "call cangu" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_CanGoDown( void )
{
	WriteInstruction( "call cangd" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_CanGoLeft( void )
{
	WriteInstruction( "call cangl" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_CanGoRight( void )
{
	WriteInstruction( "call cangr" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_LadderAbove( void )
{
	WriteInstruction( "call laddu" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_LadderBelow( void )
{
	WriteInstruction( "call laddd" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_Deadly( void )
{
	WriteInstruction( "ld b,DEADLY" );
	WriteInstruction( "call tded" );
	WriteInstruction( "cp b" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_Custom( void )
{
	WriteInstruction( "ld b,CUSTOM" );
	WriteInstruction( "call tded" );
	WriteInstruction( "cp b" );
	WriteInstruction( "jp nz,      " );
	CompileCondition();
	ResetIf();
}

void CR_To( void )
{
	if ( nOpType > 0 )
	{
		nAnswerWantedHere = CompileArgument();
		if ( nOpType == 129 )
		{
			if ( nIncDec > 0 )
			{
				WriteInstruction( "inc a" );
			}
			else
			{
				WriteInstruction( "add a,c" );
			}
		}
		else
		{
			if ( nIncDec > 0 )
			{
				WriteInstruction( "dec a" );
			}
			else
			{
				WriteInstruction( "sub c" );
			}
		}
		if ( nAnswerWantedHere >= FIRST_PARAMETER &&
			 nAnswerWantedHere <= LAST_PARAMETER )
		{
			CR_PamC( nAnswerWantedHere );					/* put accumulator in variable or sprite parameter. */
		}
		nIncDec = nOpType = 0;
	}
	else
	{
		Error( "ADD or SUBTRACT missing" );
	}
}

void CR_By( void )
{
	char cGenericCalculation = 0;
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;

	if ( nOpType > 0 )
	{
		if ( nArg1 == INS_NUM )								/* it's a number, check if we can do a shift instead. */
		{
			nArg2 = GetNum( 8 );
			switch( nArg2 )
			{
				case 1:										/* multiply/divide by 1 has no effect. */
					break;

				case 3:
				case 5:
				case 6:
				case 10:
					if ( nOpType == INS_MULTIPLY )
					{
						CompileShift( nArg2 );				/* compile a shift instead. */
					}
					else
					{
						cGenericCalculation++;
					}
					break;
				case 2:
				case 4:
				case 8:
				case 16:
				case 32:
				case 64:
				case 128:
					CompileShift( nArg2 );					/* compile a shift instead. */
					break;
				default:
					cGenericCalculation++;
			}
		}
		else												/* not a number, don't know what this will be. */
		{
			cGenericCalculation++;
		}

		if ( cGenericCalculation > 0 )
		{
			WriteInstruction( "ld d,a" );
			if ( nArg1 == INS_NUM )
			{
				WriteInstruction( "ld a," );
				WriteNumber( nArg2 );
			}
			else
			{
				CompileKnownArgument( nArg1 );
			}

			if ( nOpType == INS_MULTIPLY )
			{
				WriteInstruction( "ld h,a" );
				WriteInstruction( "call imul" );			/* hl = h * d. */
				WriteInstruction( "ld a,l" );
			}
			else
			{
				WriteInstruction( "ld e,a" );
				WriteInstruction( "call idiv" );			/* d = d/e, remainder in a. */
				WriteInstruction( "ld a,d" );
			}
			if ( nAnswerWantedHere >= FIRST_PARAMETER &&
				 nAnswerWantedHere <= LAST_PARAMETER )
			{
				CR_PamC( nAnswerWantedHere );				/* put accumulator in variable or sprite parameter. */
			}
		}
	}
	else
	{
		Error( "MULTIPLY or DIVIDE missing" );
	}

	nOpType = 0;
}

/* We can optimise the multiplication or division by using shifts. */
void CompileShift( short int nArg )
{
	if ( nOpType == INS_MULTIPLY )
	{
		switch ( nArg )
		{
			case 2:
				WriteInstruction( "add a,a" );
				break;
			case 3:
				WriteInstruction( "ld d,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,d" );
				break;
			case 4:
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				break;
			case 5:
				WriteInstruction( "ld d,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,d" );
				break;
			case 6:
				WriteInstruction( "add a,a" );
				WriteInstruction( "ld d,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,d" );
				break;
			case 8:
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				break;
			case 10:
				WriteInstruction( "add a,a" );
				WriteInstruction( "ld d,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,d" );
				break;
			case 16:
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				WriteInstruction( "add a,a" );
				break;
			case 32:
				WriteInstruction( "rrca" );
				WriteInstruction( "rrca" );
				WriteInstruction( "rrca" );
				WriteInstruction( "and 224" );
				break;
			case 64:
				WriteInstruction( "rrca" );
				WriteInstruction( "rrca" );
				WriteInstruction( "and 192" );
				break;
			case 128:
				WriteInstruction( "rrca" );
				WriteInstruction( "and 128" );
				break;
		}
	}
	else
	{
		switch ( nArg )
		{
			case 2:
				WriteInstruction( "srl a" );
				break;
			case 4:
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "and 63" );
				break;
			case 8:
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "and 31" );
				break;
			case 16:
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "rra" );
				WriteInstruction( "and 15" );
				break;
			case 32:
				WriteInstruction( "rlca" );
				WriteInstruction( "rlca" );
				WriteInstruction( "rlca" );
				WriteInstruction( "and 7" );
				break;
			case 64:
				WriteInstruction( "rlca" );
				WriteInstruction( "rlca" );
				WriteInstruction( "and 3" );
				break;
			case 128:
				WriteInstruction( "rlca" );
				WriteInstruction( "and 1" );
				break;
		}
	}

	if ( nAnswerWantedHere >= FIRST_PARAMETER &&
		 nAnswerWantedHere <= LAST_PARAMETER )
	{
		CR_PamC( nAnswerWantedHere );				/* put accumulator in variable or sprite parameter. */
	}
}

void CR_Got( void )
{
	CompileArgument();
	WriteInstruction( "call gotob" );
	WriteInstruction( "jp c,      " );
	CompileCondition();
	ResetIf();
}

void CR_Key( void )
{
	unsigned short int nArg = NextKeyword();
	unsigned short int nArg2;
	char szInstruction[ 160 ];

	if ( nArg == INS_NUM )										/* good, it's a number. */
	{
		nArg = GetNum( 8 );
		if ( nArg < 7 )
		{
			nArg2 = Joystick( nArg );
			WriteInstruction( "ld a,(joyval)" );
			WriteInstruction( "and " );
			WriteNumber( 1 << nArg2 );
			WriteInstruction( "jp z,      " );
			nReadingControls++;									/* set flag to read joystick in WHILE loop. */
		}
		else
		{
			sprintf( szInstruction, "ld a,(keys+%d)", nArg );	/* get key from table. */
			WriteInstruction( szInstruction );
			WriteInstruction( "call ktest" );					/* test it. */
			WriteInstruction( "jp c,      " );
		}
	}
	else														/* cripes, we need to do this longhand. */
	{
		CompileKnownArgument( nArg );							/* puts argument into accumulator. */
		WriteInstruction( "ld hl,keys" );						/* keys. */
		WriteInstruction( "ld de,0");
		WriteInstruction( "ld e,a" );							/* key number in de. */
		WriteInstruction( "add hl,de" );						/* point to key. */
		WriteInstruction( "ld a,(hl)" );						/* get key. */
		WriteInstruction( "call ktest" );						/* test it now. */
		WriteInstruction( "jp c,      " );
	}

	CompileCondition();
	ResetIf();
}

void CR_DefineKey( void )
{
	char szInstruction[ 150 ];
	unsigned short int nNum = NumberOnly();

	sprintf(szInstruction, "ld a,%d", Joystick(nNum));
	WriteInstruction(szInstruction);
	WriteInstruction( "call definekey" );
}

void CR_Collision( void )
{
	unsigned short int nArg = NextKeyword();

	/* Literal number */
	if ( nArg == INS_NUM )
	{
		nArg = GetNum( 8 );
		if ( nArg == 10 )										/* it's a laser bullet. */
		{
			WriteInstruction( "call lcol" );
			WriteInstruction( "jp nc,      " );
		}
		else													/* it's a sprite type. */
		{
			WriteInstruction( "ld b," );
			WriteNumber( nArg );								/* sprite type to find. */
			WriteInstruction( "call sktyp" );
			WriteInstruction( "jp nc,      " );
		}
	}
	else
	{
		CompileKnownArgument( nArg );							/* puts argument into accumulator */
		WriteInstruction( "ld b,a" );
		WriteInstruction( "call sktyp" );
		WriteInstruction( "jp nc,      " );
	}

	CompileCondition();
	ResetIf();
}

void CR_Anim( void )
{
	unsigned short int nArg;
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;

	/* Store source address so we don't skip first instruction after messages. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* first argument is numeric. */
	{
		nArg = GetNum( 8 );									/* store first argument. */
	}
	else
	{
		cBufPos = cSrc;										/* restore source address so we don't miss the next line. */
		nLine = nCurrentLine;
		nArg = 0;
	}

	if ( nArg == 0 )
	{
		WriteInstruction( "xor a" );
	}
	else
	{
		WriteInstruction( "ld a," );
		WriteNumber( nArg );								/* first argument in c register. */
	}

	WriteInstruction( "call animsp" );
}

void CR_AnimBack( void )
{
	unsigned short int nArg;
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;

	/* Store source address so we don't skip first instruction after messages. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* first argument is numeric. */
	{
		nArg = GetNum( 8 );									/* store first argument. */
	}
	else
	{
		cBufPos = cSrc;										/* restore source address so we don't miss the next line. */
		nLine = nCurrentLine;
		nArg = 0;
	}

	if ( nArg == 0 )
	{
		WriteInstruction( "xor a" );
	}
	else
	{
		WriteInstruction( "ld a," );
		WriteNumber( nArg );								/* first argument in c register. */
	}

	WriteInstruction( "call animbk" );
}

void CR_PutBlock( void )
{
	WriteInstruction( "ld a,(charx)" );
	WriteInstruction( "ld (dispx),a" );
	WriteInstruction( "ld a,(chary)");
	WriteInstruction( "ld (dispy),a");
	CompileArgument();

	if ( nAdventure > 0 )
	{
		WriteInstruction( "call wbloc" );
	}

	WriteInstruction( "call pattr" );
	WriteInstruction( "ld a,(dispx)" );
	WriteInstruction( "ld (charx),a" );
	WriteInstruction( "ld a,(dispy)");
	WriteInstruction( "ld (chary),a");
}

void CR_Dig( void )
{
	CompileArgument();
	WriteInstruction( "call dig" );
	nDig = 1;
}

void CR_NextLevel( void )
{
	WriteInstruction( "ld hl,nexlev" );
	WriteInstruction( "ld (hl),1" );
}

void CR_Restart( void )
{
	WriteInstruction( "ld hl,restfl" );
	WriteInstruction( "ld (hl),1" );						/* set to one to restart level. */
}

void CR_Spawn( void )
{
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		nArg1 = GetNum( 8 );								/* store first argument. */
		nArg2 = NextKeyword();								/* get second argument. */
		if ( nArg2 == INS_NUM )								/* second argument is numeric too. */
		{
			nArg2 = 256 * GetNum( 8 ) + nArg1;
			WriteInstruction( "ld bc," );
			WriteNumber( nArg2 );							/* pass both parameters as 16-bit argument. */
		}
		else
		{
			WriteInstruction( "ld c," );
			WriteNumber( nArg1 );							/* first argument in c register. */
			CompileKnownArgument( nArg2 );					/* puts argument into accumulator. */
			WriteInstruction( "ld b,a" );					/* put that into b. */
		}
	}
	else
	{
		CompileKnownArgument( nArg1 );						/* puts first argument into accumulator. */
		WriteInstruction( "ld c,a" );						/* copy into c register. */
		CompileArgument();									/* puts second argument into accumulator. */
		WriteInstruction( "ld b,a" );						/* put that into b. */
	}

	WriteInstruction( "call spawn" );
}

void CR_Remove( void )
{
	WriteInstruction( "ld (ix+5),255" );
}

void CR_GetRandom( void )
{
	CompileArgument();										/* maximum number in accumulator. */
	WriteInstruction( "ld d,a" );							/* multiplication parameter. */
	WriteInstruction( "call random" );						/* random number 0 - 255. */
	WriteInstruction( "ld h,a" );							/* second multiplication parameter. */
	WriteInstruction( "call imul" );						/* multiply together. */
	WriteInstruction( "ld a,h" );							/* put result in accumulator. */
	WriteInstruction( "ld (varrnd),a" );					/* write to random variable. */
}

void CR_Randomize( void )
{
	CompileArgument();										/* maximum number in accumulator. */
	WriteInstruction( "ld (seed),a" );						/* write to random seed. */
}

void CR_DisplayHighScore( void )
{
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;
	unsigned short int nArg;

	/* Store source address so we don't skip next instruction. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* numeric argument, digits to display. */
	{
		nArg = GetNum( 8 );									/* store first argument. */
		if ( nArg < 1 || nArg > 6 )
		{
			Error( "High score display must be 1 to 6 digits" );
		}
	}
	else
	{
		cBufPos = cSrc;										/* restore source address so we don't miss the next line. */
		nLine = nCurrentLine;
		nArg = 6;
	}

	if ( nArg == 6 )
	{
		WriteInstruction( "ld hl,hiscor" );
		WriteInstruction( "ld b,6" );
	}
	else
	{
		WriteInstruction( "ld hl,hiscor+" );
		WriteNumber( 6 - nArg );
		WriteInstruction( "ld b," );
		WriteNumber( nArg );
	}

	WriteInstruction( "call dscor" );
}

void CR_DisplayScore( void )
{
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;
	unsigned short int nArg;

	/* Store source address so we don't skip next instruction. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* numeric argument, digits to display. */
	{
		nArg = GetNum( 8 );									/* store first argument. */
		if ( nArg < 1 || nArg > 6 )
		{
			Error( "Score display must be 1 to 6 digits" );
		}
	}
	else
	{
		cBufPos = cSrc;										/* restore source address so we don't miss the next line. */
		nLine = nCurrentLine;
		nArg = 6;
	}

	if ( nArg == 6 )
	{
		WriteInstruction( "ld hl,score" );
		WriteInstruction( "ld b,6" );
	}
	else
	{
		WriteInstruction( "ld hl,score+" );
		WriteNumber( 6 - nArg );
		WriteInstruction( "ld b," );
		WriteNumber( nArg );
	}

	WriteInstruction( "call dscor" );
}

void CR_DisplayBonus( void )
{
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;
	unsigned short int nArg;

	/* Store source address so we don't skip next instruction. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* numeric argument, digits to display. */
	{
		nArg = GetNum( 8 );									/* store first argument. */
		if ( nArg < 1 || nArg > 6 )
		{
			Error( "Bonus display must be 1 to 6 digits" );
		}
	}
	else
	{
		cBufPos = cSrc;										/* restore source address so we don't miss the next line. */
		nLine = nCurrentLine;
		nArg = 6;
	}

	if ( nArg == 6 )
	{
		WriteInstruction( "ld hl,bonus" );
		WriteInstruction( "ld b,6" );
	}
	else
	{
		WriteInstruction( "ld hl,bonus+" );
		WriteNumber( 6 - nArg );
		WriteInstruction( "ld b," );
		WriteNumber( nArg );
	}

	WriteInstruction( "call dscor" );
}

void CR_Score( void )
{
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* literal number, could be 16 bits. */
	{
		nArg = GetNum( 16 );
		WriteInstruction( "ld hl," );
		WriteNumber( nArg );
	}
	else													/* work out 8-bit argument to add. */
	{
		CompileKnownArgument( nArg );						/* puts argument into accumulator. */
		WriteInstruction( "ld hl,0" );						/* no high bytes. */
		WriteInstruction( "ld l,a" );						/* low byte of parameter in l. */
	}

	WriteInstruction( "call addsc" );
}

void CR_Bonus( void )
{
	WriteInstruction( "call swpsb" );						/* swap bonus into score. */
	CR_Score();												/* score the points. */
	WriteInstruction( "call swpsb" );						/* swap back again. */
}

void CR_AddBonus( void )
{
	WriteInstruction( "call addbo" );						/* add bonus to score. */
}

void CR_ZeroBonus( void )
{
	WriteInstruction( "ld hl,bonus" );
	WriteInstruction( "ld bc,5" );
	WriteInstruction( "ld (hl),48" );
	WriteInstruction( "ld de,bonus+1" );
	WriteInstruction( "ldir" );
}

void CR_Sound( void )
{
	CompileArgument();
	WriteInstruction( "call sound" );
}

void CR_Beep( void )
{
	CompileArgument();
	WriteInstruction( "call beep" );
}

void CR_Crash( void )
{
	CompileArgument();
	WriteInstruction( "call crash" );
}

void CR_ClS( void )
{
	WriteInstruction( "call cls" );
}

void CR_Border( void )
{
	CompileArgument();
	WriteInstruction( "call setborder" );						/* address of ROM BORDER routine. */
}

void CR_Colour( void )
{
	CompileArgument();
	WriteInstruction( "call setcolour" );						/* set the permanent attributes. */
}

void CR_Paper( void )
{
	CompileArgument();
	WriteInstruction( "call setpaper");						/* set the permanent attributes. */
}

void CR_Ink( void )
{
	CompileArgument();
	WriteInstruction("call setink");						/* set the permanent attributes. */
}

void CR_Clut( void )
{
	CompileArgument();
	WriteInstruction("call setclut");						/* set the permanent attributes. */
}

void CR_Delay( void )
{
	unsigned short int nArg = NextKeyword();

	WriteInstruction( "push ix" );							/* DELAY command causes ix to be corrupted. */

	if ( nArg == INS_NUM )									/* literal number. */
	{
		nArg = GetNum( 8 );
		WriteInstruction( "ld b," );
		WriteNumber( nArg );
	}
	else													/* work out 8-bit argument to add. */
	{
		CompileKnownArgument( nArg );						/* puts argument into accumulator. */
		WriteInstruction( "ld b,a" );
	}

	WriteInstruction( "call delay" );
	WriteInstruction( "pop ix" );
}

void CR_Print( void )
{
	CompileArgument();
	WriteInstruction( "call dmsg" );
}

void CR_PrintMode( void )
{
	CompileArgument();
	WriteInstruction( "ld (prtmod),a" );					/* set print mode. */
}

void CR_At( void )
{
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		nArg1 = GetNum( 8 );								/* store first argument. */
		nArg2 = NextKeyword();								/* get second argument. */
		if ( nArg2 == INS_NUM )								/* second argument is numeric too. */
		{
			nArg2 = 256 * GetNum( 8 ) + nArg1;
			WriteInstruction( "ld hl," );
			WriteNumber( nArg2 );							/* pass both parameters as 16-bit argument. */
		}
		else
		{
			WriteInstruction( "ld l," );
			WriteNumber( nArg1 );
			CompileKnownArgument( nArg2 );					/* puts argument into accumulator. */
			WriteInstruction( "ld h,a" );					/* put that into h. */
		}
	}
	else
	{
		CompileKnownArgument( nArg1 );						/* puts first argument into accumulator. */
		WriteInstruction( "ld l,a" );						/* copy into c register. */
		CompileArgument();									/* puts second argument into accumulator. */
		WriteInstruction( "ld h,a" );						/* put that into b. */
	}

	WriteInstruction( "ld a,l" );
	WriteInstruction( "ld (charx),a");
	WriteInstruction( "ld a,h");
	WriteInstruction( "ld (chary),a");

//	CompileArgument();
//	WriteInstruction( "ld (charx),a" );
//	CompileArgument();
//	WriteInstruction( "ld (chary),a" );
}

void CR_Chr( void )
{
	CompileArgument();
	WriteInstruction( "call achar" );
}

void CR_Menu( void )
{
	CompileArgument();
	WriteInstruction( "call mmenu" );
}

void CR_Inventory( void )
{
	CompileArgument();
	WriteInstruction( "call minve" );
}

void CR_Kill( void )
{
	WriteInstruction( "ld hl,deadf" );						/* player dead flag. */
	WriteInstruction( "ld (hl),1" );						/* set to non-zero. */
}

void CR_AddSubtract( void )
{
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* literal number. */
	{
		nArg = GetNum( 8 );
		if ( nArg == 1 )
		{
			nIncDec++;										/* increment/decrement will suffice. */
		}
		else
		{
			WriteInstruction( "ld c," );					/* put number to add/subtract into c register. */
			WriteNumber( nArg );
		}
	}
	else													/* work out 8-bit argument to add. */
	{
		CompileKnownArgument( nArg );						/* puts argument into accumulator. */
		WriteInstruction( "ld c,a" );
	}
}

void CR_Display( void )
{
	unsigned char *cSrc;									/* source pointer. */
	unsigned short int nArg;

	cSrc = cBufPos;											/* store position in buffer. */
	nArg = NextKeyword();

	switch( nArg )
	{
		case INS_DOUBLEDIGITS:
			CompileArgument();
			WriteInstruction( "ld bc,displ0" );
			WriteInstruction( "call num2dd" );
			WriteInstruction( "call displ1" );
			break;
		case INS_TRIPLEDIGITS:
			CompileArgument();
			WriteInstruction( "ld bc,displ0" );
			WriteInstruction( "call num2td" );
			WriteInstruction( "call displ1" );
			break;
		case INS_CLOCK:
			CompileArgument();
			WriteInstruction( "ld d,a" );
			WriteInstruction( "ld e,60" );
			WriteInstruction( "call idiv" );			/* d = d/e, remainder in a. */
			WriteInstruction( "push af" );
			WriteInstruction( "ld a,d" );
			WriteInstruction( "call disply" );
			WriteInstruction( "ld hl,chary" );
			WriteInstruction( "inc (hl)" );
			WriteInstruction( "pop af" );
			WriteInstruction( "ld bc,displ0" );
			WriteInstruction( "call num2dd" );
			WriteInstruction( "call displ1" );
			break;
		default:
			cBufPos = cSrc;									/* restore buffer position and compile standard DISPLAY command. */
			CompileArgument();
			WriteInstruction( "call disply" );
			break;
	}
}

void CR_ScreenUp( void )
{
	WriteInstruction( "call scru" );
}

void CR_ScreenDown( void )
{
	WriteInstruction( "call scrd" );
}

void CR_ScreenLeft( void )
{
	WriteInstruction( "call scrl" );
}

void CR_ScreenRight( void )
{
	WriteInstruction( "call scrr" );
}

void CR_WaitKey( void )
{
	WriteInstruction( "call prskey" );
}

void CR_Jump( void )
{
	CompileArgument();
	WriteInstruction( "call jump" );
	nGravity++;
}

void CR_Fall( void )
{
	WriteInstruction( "call ifall" );
	nGravity++;
}

void CR_TableJump( void )
{
	WriteInstruction( "call hop" );
	nGravity++;
	nUseHopTable++;
}

void CR_TableFall( void )
{
	WriteInstruction( "call tfall" );
	nGravity++;
	nUseHopTable++;
}

void CR_Other( void )
{
	WriteInstruction( "ld ix,(skptr)" );
}

void CR_Spawned( void )
{
	WriteInstruction( "ld ix,(spptr)" );
}

void CR_Original( void )
{
	WriteInstruction( "ld ix,(ogptr)" );
}

void CR_EndGame( void )
{
	WriteInstruction( "ld hl,gamwon" );
	WriteInstruction( "ld (hl),1" );
}

void CR_Get( void )
{
	CompileArgument();
	WriteInstruction( "call getob" );
}

void CR_Put( void )
{
	CompileArgument();
	WriteInstruction( "ld (dispy),a" );							/* horizontal first. */
	CompileArgument();
	WriteInstruction( "ld (dispx),a" );					/* store coordinates. */
	CompileArgument();										/* put object number in accumulator. */
	WriteInstruction( "call drpob" );
}

void CR_RemoveObject( void )
{
	CompileArgument();
	WriteInstruction( "call remob" );
}

void CR_DetectObject( void )
{
	WriteInstruction( "call skobj" );
	WriteInstruction( "ld (varobj),a" );
}

void CR_Asm( void )											/* this is undocumented as it's dangerous! */
{
	unsigned short int nNum = NumberOnly();
	WriteInstruction( "db " );

	WriteNumber( nNum );									/* write opcode straight to code. */
}

void CR_Exit( void )
{
	WriteInstruction( "ret" );								/* finish event. */
}

void CR_Repeat( void )
{
	char szInstruction[ 130 ];

	if ( nNumRepts < NUM_REPEAT_LEVELS )
	{
		CompileArgument();
		sprintf( szInstruction, "ld (loop%c),a", nNumRepts + 'a' );
		WriteInstruction( szInstruction );
		nReptBuff[ nNumRepts ] = nCurrent;					/* store current address. */
		nNextLabel = nCurrent;
		nNumRepts++;
	}
	else
	{
		Error( "Too many REPEATs" );
		CompileArgument();
	}
}

void CR_EndRepeat( void )
{
	char szInstruction[ 120 ];

	if ( nNumRepts > 0 )
	{
		nNumRepts--;
		sprintf( szInstruction, "ld hl,loop%c", nNumRepts + 'a' );
		WriteInstruction( szInstruction );
		WriteInstruction( "dec (hl)" );
		WriteInstruction( "jp nz," );
		WriteLabel( nReptBuff[ nNumRepts ] );
		nReptBuff[ nNumRepts ] = ASMLABEL_DUMMY;
	}
	else
	{
		Error( "ENDREPEAT without REPEAT" );
	}

}

void CR_Multiply( void )
{
	nOpType = INS_MULTIPLY;									/* remember instruction is multiply (needed by CR_By). */
	nAnswerWantedHere = CompileArgument();					/* stores the place where we want the answer. */
}

void CR_Divide( void )
{
	nOpType = INS_DIVIDE;									/* remember it's a divide (needed by CR_By). */
	nAnswerWantedHere = CompileArgument();					/* stores the place where we want the answer. */
}

void CR_SpriteInk( void )
{
	CompileArgument();
	WriteInstruction( "and 7" );
	WriteInstruction( "ld c,a" );
	WriteInstruction( "call cspr" );
}

void CR_Trail( void )
{
	WriteInstruction( "call vapour" );
}

void CR_Laser( void )
{
	CompileArgument();										/* 0 or 1 for direction. */
	WriteInstruction( "call shoot" );
}

void CR_Star( void )
{
	CompileArgument();										/* direction 0 - 3. */
	WriteInstruction( "ld c,a" );
	WriteInstruction( "call qrand" );
	WriteInstruction( "and 3" );
	WriteInstruction( "call z,star" );
}

void CR_Explode( void )
{
	CompileArgument();										/* number of particles required. */
	WriteInstruction( "call explod" );
}

void CR_Redraw( void )
{
	WriteInstruction( "call redraw" );
}

void CR_Silence( void )
{
	WriteInstruction( "call silenc" );
}

void CR_ClW( void )
{
	WriteInstruction( "call clw" );
	WriteInstruction( "call inishr" );						/* clear the shrapnel table. */
}

void CR_Palette( void )
{
	CompileArgument();
	WriteInstruction("ld c,a");
	CompileArgument();								
	WriteInstruction( "call palette" );
}

void CR_GetBlock( void )
{
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		nArg1 = GetNum( 8 );								/* store first argument. */
		nArg2 = NextKeyword();								/* get second argument. */
		if ( nArg2 == INS_NUM )								/* second argument is numeric too. */
		{
			nArg2 = 256 * nArg1 + GetNum( 8 );
			WriteInstruction( "ld hl," );
			WriteNumber( nArg2 );							/* pass both parameters as 16-bit argument. */
		}
		else
		{
			WriteInstruction( "ld h," );
			WriteNumber( nArg1 );							/* first argument in c register. */
			CompileKnownArgument( nArg2 );					/* puts argument into accumulator. */
			WriteInstruction( "ld l,a" );					/* put that into b. */
		}
	}
	else
	{
		CompileKnownArgument( nArg1 );						/* puts first argument into accumulator. */
		WriteInstruction( "ld h,a" );						/* copy into c register. */
		CompileArgument();									/* puts second argument into accumulator. */
		WriteInstruction( "ld l,a" );						/* put that into b. */
	}

	WriteInstruction( "ld a,l");					/* set the test coordinates. */
	WriteInstruction( "ld (dispx),a" );					/* set the test coordinates. */
	WriteInstruction( "ld a,h");					/* set the test coordinates. */
	WriteInstruction( "ld (dispy),a");					/* set the test coordinates. */
	WriteInstruction( "call tstbl" );						/* get block there. */
	WriteInstruction( "ld (varblk),a" );					/* write to block variable. */
}

void CR_Read( void )
{
	char szInstruction[ 120 ];

	cDataRequired = 1;										/* need to find data at the end. */
	sprintf( szInstruction, "call read%02d", nEvent );
	WriteInstruction( szInstruction );

	nAnswerWantedHere = NextKeyword();
	if ( nAnswerWantedHere >= FIRST_PARAMETER &&
		 nAnswerWantedHere <= LAST_PARAMETER )
	{
		CR_PamC( nAnswerWantedHere );						/* put accumulator in variable or sprite parameter. */
	}
}

void CR_Data( void )
{
	short int nDataNums = 0;
	unsigned short int nArg = 0;
	unsigned short int nValue = 0;
	unsigned char *cSrc;									/* source pointer. */
	char szInstruction[ 220 ];
	unsigned short int nList = 0;

	do
	{
		cSrc = cBufPos;										/* store position in buffer. */
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nValue = GetNum( 16 );							/* get the value. */
			if ( nDataNums == 0 )
			{
				if ( nList == 0 )
				{
					WriteInstruction( "ret" );				/* make sure we don't drop into data. */
				}
				if ( nList == 0 && cData == 0 )
				{
					sprintf( szInstruction, "rptr%02d: dl rdat%02d", nEvent, nEvent );
					WriteInstructionAndLabel( szInstruction );
					sprintf( szInstruction, "rdat%02d: db %d", nEvent, nValue );
					WriteInstructionAndLabel( szInstruction );
				}
				else
				{
					sprintf( szInstruction, "db %d", nValue );
					WriteInstruction( szInstruction );
				}
			}
			else
			{
				sprintf( szInstruction, ",%d", nValue );
				WriteText( szInstruction );
			}

			if ( ++nDataNums > 10 )
			{
				nDataNums = 0;
			}

			nList++;										/* tally number in list. */
		}

		if ( nArg != INS_NUM && nArg != INS_DATA )			/* another data statement could follow. */
		{
			cBufPos = cSrc;									/* go back to previous position. */
		}
	}
	while ( ( ( cBufPos - cBuff ) < lSize ) && ( nArg == INS_NUM || nArg == INS_DATA ) );

	if ( cData == 0 )
	{
		/* Now we set up a read routine. */
		if ( SpriteEvent() )
		{
			sprintf( szInstruction, "read%02d: ld hl,(ix+15)", nEvent );
			WriteInstructionAndLabel( szInstruction );
			//
			// WriteInstruction( "ld h,(ix+16)" );
		}
		else
		{
			sprintf( szInstruction, "read%02d: ld hl,(rptr%02d)", nEvent, nEvent );
			WriteInstructionAndLabel( szInstruction );
		}
		sprintf( szInstruction, "ld de,rdat%02d+%d", nEvent, nList );
		WriteInstruction( szInstruction );
		WriteInstruction( "scf" );
		WriteInstruction( "ex de,hl" );
		WriteInstruction( "sbc hl,de" );
		WriteInstruction( "ex de,hl" );
		WriteInstruction( "jr nc,$+6" );
		sprintf( szInstruction, "ld hl,rdat%02d", nEvent );
		WriteInstruction( szInstruction );
		WriteInstruction( "ld a,(hl)" );
		WriteInstruction( "inc hl" );

		if ( SpriteEvent() )
		{
			WriteInstruction( "ld (ix+15),hl" );
			//WriteInstruction( "ld (ix+16),h" );
		}
		else
		{
			sprintf( szInstruction, "ld (rptr%02d),hl", nEvent );
			WriteInstruction( szInstruction );
		}

		WriteInstruction( "ret" );
	}

	cData = 1;												/* flag that we've found data. */

	if ( nDataNums == 0 )
	{
		Error( "No data found" );
	}
}

void CR_Restore( void )
{
	char szInstruction[ 150 ];

	cDataRequired = 1;										/* need to find data at the end. */

	if ( SpriteEvent() )
	{
		WriteInstruction( "ld (ix+16),255" );				/* set data pointer to beyond range. */
	}
	else
	{
		sprintf( szInstruction, "ld h,255");
		WriteInstruction( szInstruction );
		sprintf( szInstruction, "ld (rptr%02d),hl", nEvent );
		WriteInstruction( szInstruction );
	}
}

void CR_DefineParticle( void )
{
	if ( nParticle != 0 )
	{
		Error( "User particle already defined" );
	}
	else
	{
		nParticle++;
		WriteInstruction( "ret" );							/* make sure we don't drop through from elsewhere. */
		WriteInstructionAndLabel( "ptcusr:" );
	}
}

void CR_ParticleUp( void )
{
	WriteInstruction( "dec (ix+3)" );
}

void CR_ParticleDown( void )
{
	WriteInstruction( "inc (ix+3)" );
}

void CR_ParticleLeft( void )
{
	WriteInstruction( "dec (ix+5)" );
}

void CR_ParticleRight( void )
{
	WriteInstruction( "inc (ix+5)" );
}

void CR_ParticleTimer( void )
{
	WriteInstruction( "dec (ix+1)" );						/* decrement shrapnel timer. */
	WriteInstruction( "jp z,trailk" );						/* reached zero, kill it off. */
}

void CR_StartParticle( void )
{
	CompileArgument();										/* palette register to write. */
	WriteInstruction( "push ix" );
	WriteInstruction( "call ptusr" );
	WriteInstruction( "pop ix" );
}

void CR_Message( void )
{
	CompileArgument();										/* message number to display. */
	WriteInstruction( "call dmsg" );
}

void CR_StopFall( void )
{
	WriteInstruction( "call gravst" );
}

void CR_GetBlocks( void )
{
	WriteInstruction( "call getcol" );
}

void CR_ValidateMachine( void )
{
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* first argument is numeric. */
	{
		nArg = GetNum( 8 );									/* get the argument. */
		if ( nArg != FORMAT_SPECTRUM )						/* make sure this is the correct machine. */
		{
			Error( "Source is for a different machine, ceasing compilation" );
			cBufPos = cBuff + lSize;
		} 
	}
	else
	{
		Error( "Invalid argument for MACHINE" );
	}
}

void CR_Call( void )
{
	char szInstruction[ 120 ];
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )									/* first argument is numeric. */
	{
		nArg = GetNum( 16 );								/* get the address. */
		sprintf( szInstruction, "call %d", nArg );			/* compile a call instruction to this address. */
		WriteInstruction( szInstruction );
	}
	else
	{
		Error( "CALL must be followed by address of routine" );
	}
}

void CR_Plot( void )
{
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		nArg1 = GetNum( 8 );								/* store first argument. */
		nArg2 = NextKeyword();								/* get second argument. */
		if ( nArg2 == INS_NUM )								/* second argument is numeric too. */
		{
			nArg2 = 256 * nArg1 + GetNum( 8 );
			WriteInstruction( "ld hl," );
			WriteNumber( nArg2 );							/* pass both parameters as 16-bit argument. */
		}
		else
		{
			WriteInstruction( "ld h," );
			WriteNumber( nArg1 );							/* first argument in c register. */
			CompileKnownArgument( nArg2 );					/* puts argument into accumulator. */
			WriteInstruction( "ld l,a" );					/* put that into b. */
		}
	}
	else
	{
		CompileKnownArgument( nArg1 );						/* puts first argument into accumulator. */
		WriteInstruction( "ld h,a" );						/* copy into c register. */
		CompileArgument();									/* puts second argument into accumulator. */
		WriteInstruction( "ld l,a" );						/* put that into b. */
	}

	WriteInstruction( "ld a,l" );					/* set the test coordinates. */
	WriteInstruction( "ld (dispx),a");					/* set the test coordinates. */
	WriteInstruction( "ld a,h");					/* set the test coordinates. */
	WriteInstruction( "ld (dispy),h");					/* set the test coordinates. */
	WriteInstruction( "call plot0" );						/* plot the pixel. */
}

void CR_UndoSpriteMove( void )
{
	WriteInstruction( "ld a,(ix+3)" );
	WriteInstruction( "ld (ix+8),a" );
	WriteInstruction( "ld a,(ix+4)" );
	WriteInstruction( "ld (ix+9),a" );
}

void CR_Ticker( void )
{
	unsigned short int nArg1 = NextKeyword();
	unsigned short int nArg2;
	unsigned char *cSrc;									/* source pointer. */

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		cSrc = cBufPos;										/* store position in buffer. */
		nArg1 = GetNum( 8 );								/* store first argument. */
		if ( nArg1 == 0 )
		{
			WriteInstruction( "ld hl,scrly" );
			WriteInstruction( "ld (hl),201" );
		}
		else
		{
			nArg2 = NextKeyword();							/* get second argument. */
			if ( nArg2 == INS_STR )							/* second argument should be a string. */
			{
				nArg2 = 256 * nArg1 + nMessageNumber++;
				WriteInstruction( "ld bc," );
				WriteNumber( nArg2 );						/* pass both parameters as 16-bit argument. */
				WriteInstruction( "call iscrly" );
			}
			else
			{
				if ( nArg2 == INS_NUM )						/* if not a string, must be a message number. */
				{
//					nArg2 = 256 * GetNum( 8 ) + nArg1;
					nArg2 = 256 * nArg1 + GetNum( 8 );
					WriteInstruction( "ld bc," );
					WriteNumber( nArg2 );					/* pass both parameters as 16-bit argument. */
					WriteInstruction( "call iscrly" );
				}
				else
				{
//					Error( "Invalid argument for TICKER" );
					cBufPos = cSrc;
					CompileArgument();									/* puts first argument into accumulator. */
					WriteInstruction( "ld b,a" );						/* copy into b register. */
					CompileArgument();									/* puts second argument into accumulator. */
					WriteInstruction( "ld c,a" );						/* put that into c. */
					WriteInstruction( "call iscrly" );
				}
			}
		}
	}
	else
	{
		CompileKnownArgument( nArg1 );						/* puts first argument into accumulator. */
		WriteInstruction( "ld b,a" );						/* copy into b register. */
		CompileArgument();									/* puts second argument into accumulator. */
		WriteInstruction( "ld c,a" );						/* put that into c. */
		WriteInstruction( "call iscrly" );
	}
}

void CR_User( void )
{
	unsigned short int nArg;
	unsigned char *cSrc;									/* source pointer. */

	cSrc = cBufPos;											/* store position in buffer. */
	nArg = NextKeyword();

	if ( nArg == INS_NUM ||
		 nArg == INS_STR ||
		 ( nArg >= FIRST_PARAMETER && nArg <= LAST_PARAMETER ) )
	{
		CompileKnownArgument( nArg );						/* puts argument into accumulator. */
	}
	else
	{
		cBufPos = cSrc;										/* no argument, restore position. */
	}

	WriteInstruction( "call user" );
}

void CR_Event( void )
{
	unsigned short int nArg1 = NextKeyword();

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();											/* always put a ret at the end. */
	}

	if ( nArg1 == INS_NUM )									/* first argument is numeric. */
	{
		nArg1 = GetNum( 8 );								/* store first argument. */

		if ( nArg1 >= 0 && nArg1 < NUM_EVENTS )
		{
			nEvent = nArg1;
			StartEvent( nEvent );							/* write event label and header. */
		}
		else
		{
			Error( "Invalid event" );
		}
	}
	else
	{
		Error( "Invalid event" );
	}
}

void CR_DefineBlock( void )
{
	unsigned short int nArg;
	char cChar;
	short int nDatum = 0;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();											/* always put a ret at the end. */
		nEvent = -1;
	}

	do
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			cChar = ( char )nArg;
			fwrite( &cChar, 1, 1, pWorkBlk );				/* write character to blocks workfile. */
			nDatum++;
		}
		else
		{
			Error( "Missing data for DEFINEBLOCK" );
			nDatum = 10;
		}
	}
	while ( nDatum < 10 );
}

void CR_DefineWindow( void )
{
	char szInstruction[ 180 ];
	unsigned short int nArg;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();											/* always put a ret at the end. */
		nEvent = -1;
	}

	if ( cWindow == 0 )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			nWinTop = nArg;
//			sprintf( szInstruction, "wintop defb %d", nArg );
			sprintf( szInstruction, "WINDOWTOP: equ %d", nArg );
			WriteInstructionAndLabel( szInstruction );
		}
		else
		{
			Error( "Invalid top edge for DEFINEWINDOW" );
		}

		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			nWinLeft = nArg;
//			sprintf( szInstruction, "winlft defb %d", nArg );
			sprintf( szInstruction, "WINDOWLFT: equ %d", nArg );
			WriteInstructionAndLabel( szInstruction );
		}
		else
		{
			Error( "Invalid left edge for DEFINEWINDOW" );
		}

		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			nWinHeight = nArg;
//			sprintf( szInstruction, "winhgt defb %d", nArg );
			sprintf( szInstruction, "WINDOWHGT: equ %d", nArg );
			WriteInstructionAndLabel( szInstruction );
		}
		else
		{
			Error( "Invalid height for DEFINEWINDOW" );
		}

		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			nWinWidth = nArg;
//			sprintf( szInstruction, "winwid defb %d", nArg );
			sprintf( szInstruction, "WINDOWWID: equ %d ;", nArg );
			WriteInstructionAndLabel( szInstruction );
		}
		else
		{
			Error( "Invalid width for DEFINEWINDOW" );
		}

		cWindow++;
		fwrite( cStart, 1, nCurrent - nAddress, pObject );	/* write output to file. */
	}
	else
	{
		Error( "Window already defined" );
	}

	if ( nWinTop + nWinHeight > 24 )
	{
		Error( "Window extends beyond bottom of screen" );
	}

	if ( nWinLeft + nWinWidth > 32 )
	{
		Error( "Window extends beyond right edge of screen" );
	}
}

void CR_DefineSprite( void )
{
	unsigned short int nArg;
	char cChar;
	short int nDatum = 0;
	short int nFrames = 0;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();											/* always put a ret at the end. */
		nEvent = -1;
	}

	nArg = NextKeyword();
	if ( nArg == INS_NUM )
	{
		nFrames = GetNum( 8 );
		fwrite( &nFrames, 1, 1, pWorkSpr );						/* write character to sprites workfile. */
	}
	else
	{
		Error( "Number of frames undefined for DEFINESPRITE" );
	}

	while ( nFrames-- > 0 )
	{
		nDatum = 0;
		do
		{
			nArg = NextKeyword();
			if ( nArg == INS_NUM )
			{
				nArg = GetNum( 8 );
				cChar = ( char )nArg;
				fwrite( &cChar, 1, 1, pWorkSpr );				/* write character to sprites workfile. */
				nDatum++;
			}
			else
			{
				Error( "Missing data for DEFINESPRITE" );
				nDatum = 32;
			}
		}
		while ( nDatum < 32 );
	}
}

void CR_DefineScreen( void )
{
	unsigned short int nArg;
	char cChar;
	short int nBytes = nWinWidth * nWinHeight;
	char szMsg[ 41 ];

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();												/* always put a ret at the end. */
		nEvent = -1;
	}

	if ( cWindow == 0 )
	{
		Error( "Window must be defined before screen layouts" );
	}

	while ( nBytes > 0 )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			cChar = ( char )nArg;
			fwrite( &cChar, 1, 1, pWorkScr );					/* write character to screen workfile. */
			nBytes--;
		}
		else
		{
			sprintf( szMsg, "Missing DEFINESCREEN data for screen %d", nScreen );
			Error( szMsg );
			nBytes = 0;
		}
	}

	nScreen++;
}

void CR_SpritePosition( void )
{
	unsigned short int nArg;
	short int nCount = 0;
	char cChar;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();												/* always put a ret at the end. */
		nEvent = -1;
	}

	cChar = ( char )( nScreen - 1 );
	fwrite( &cChar, 1, 1, pWorkNme );

	for( nCount = 0; nCount < 4; nCount++ )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nArg = GetNum( 8 );
			cChar = ( char )nArg;
			fwrite( &cChar, 1, 1, pWorkNme );					/* write character to screen workfile. */
		}
		else
		{
			Error( "Missing SPRITEPOSITION data" );
		}
	}

	nPositions++;
}

void CR_DefineObject( void )
{
	unsigned short int nArg;
	short int nDatum = 0;
	unsigned char cChar;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();												/* always put a ret at the end. */
		nEvent = -1;
	}

	do
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			cChar = ( char )GetNum( 8 );
			fwrite( &cChar, 1, 1, pWorkObj );					/* write character to objects workfile. */
			nDatum++;
		}
		else
		{
			Error( "Missing data for DEFINEOBJECT" );
			nDatum = 36;
		}
	}
	while ( nDatum < 36 );

	nObjects++;
}

void CR_Map( void )
{
	unsigned short int nArg;
	unsigned short int nScreen = 0;
	short int nCol = 0;
	short int nDatum = 0;
	short int nDone = 0;

	if ( nEvent >= 0 && nEvent < NUM_EVENTS )
	{
		EndEvent();												/* always put a ret at the end. */
	}

	StartEvent( 99 );											/* use dummy event as map doesn't use a workfile. */

	nArg = NextKeyword();
	if ( nArg == CMP_WIDTH )									/* first argument is width, WIDTH clause optional */
	{
		nArg = NextKeyword();
	}

	if ( nArg == INS_NUM )
	{
		cMapWid = ( char )GetNum( 8 );
		WriteText( "\nMAPWID: equ " );
		WriteNumber( cMapWid );

		/* seal off the upper edge of the map. */
		WriteInstruction( "db " );
		nDone = cMapWid - 1;
		while ( nDone > 0 )
		{
			WriteText( "255," );
			nDone--;
		}
		WriteText( "255" );
	}
	else
	{
		Error( "Map WIDTH not defined" );
	}

	WriteText( "\nmapdat:" );

	nArg = NextKeyword();
	if ( nArg == CMP_STARTSCREEN )								/* first argument is width, WIDTH clause optional */
	{
		nArg = NextKeyword();
	}

	if ( nArg == INS_NUM )
	{
		nStartScreen = GetNum( 8 );
	}
	else
	{
		Error( "Invalid screen number for STARTSCREEN" );
	}

	do
	{
		nArg = NextKeyword();
		switch( nArg )
		{
			case INS_NUM:
				nScreen = GetNum( 8 );
				if ( nScreen == nStartScreen )
				{
					nStartOffset = nDatum;
				}
				if ( nCol == 0 )
				{
					WriteInstruction( "db " );
				}
				else
				{
					WriteText( "," );
				}
				WriteNumber( nScreen );
				nDatum++;
				nCol++;
				if ( nCol >= cMapWid )
				{
					nCol = 0;
				}
				break;
			case CMP_ENDMAP:
				nDone++;
				break;
		}
	}
	while ( nDone == 0 );

	/* Now write block of 255 bytes to seal the map lower edge */
	WriteInstruction( "db " );
	nDone = cMapWid - 1;

	while ( nDone > 0 )
	{
		WriteText( "255," );
		nDone--;
	}

	WriteText( "255" );

	WriteText( "\nstmap:  db " );
	WriteNumber( nStartOffset );
	EndDummyEvent();											/* write output to target file. */
}

void CR_DefinePalette( void )
{
	unsigned short int nArg;

	while ( cPalette < 64 )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			cDefaultPalette[ cPalette ] = ( unsigned char )GetNum( 8 );
		}
		else
		{
			Error( "DEFINEPALETTE requires 64 RGB definitions" );
			cPalette = 64;
		}

		cPalette++;
	}
}

void CR_DefineMessages( void )
{
	unsigned short int nArg;
	unsigned char *cSrc;									/* source pointer. */
	short int nCurrentLine = nLine;

	/* Store source address so we don't skip first instruction after messages. */
	cSrc = cBufPos;
	nArg = NextKeyword();

	if ( nMessageNumber > 0 )
	{
		Error( "MESSAGES must be defined before events" );
	}

	while ( nArg == INS_STR )						/* go through until we find something that isn't a string. */
	{
		cSrc = cBufPos;
		nCurrentLine = nLine;
		CR_ArgA( nMessageNumber++ );				/* number of this message. */
		nArg = NextKeyword();
	}

	cBufPos = cSrc;									/* restore source address so we don't miss the next line. */
	nLine = nCurrentLine;
}

void CR_DefineFont( void )
{
	unsigned short int nArg;
	short int nByte = 0;

	nUseFont = 1;

	while ( nByte < 768 )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			cDefaultFont[ nByte++ ] = ( unsigned char )GetNum( 8 );
		}
		else
		{
			Error( "DEFINEFONT missing data" );
			nByte = 768;
			nUseFont = 0;
		}
	}
}

void CR_DefineJump( void )
{
	unsigned short int nArg;
	unsigned short int nNum = 0;
	short int nByte = 0;

	while ( nNum != 99 )
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nNum = ( unsigned char )GetNum( 8 );
			cDefaultHop[ nByte ] = nNum;
			if ( nByte < 25 )
			{
				nByte++;
			}
			else
			{
				Error( "DEFINEJUMP table too big" );
				nNum = 99;
			}
		}
		else
		{
			Error( "DEFINEJUMP missing 99 end marker" );
			nNum = 99;
		}
	}

	cDefaultHop[ 24 ] = 99;
}

void CR_DefineControls( void )
{
	unsigned char *cSrc;									/* source pointer. */
	unsigned short int nArg;
	unsigned short int nNum = 0;
	short int nByte = 0;
	short int nCount = 0;
	short int nCurrentLine = nLine;

	/* Store source address so we don't skip first instruction after messages. */
	cSrc = cBufPos;

	do
	{
		nArg = NextKeyword();
		if ( nArg == INS_NUM )
		{
			nNum = ( unsigned char )GetNum( 8 );
			if ( nCount < 11 )
			{
				cDefaultKeys[ cKeyOrder[ nCount++ ] ] = ConvertKey( nNum );
			}
		}
	}
	while ( nArg == INS_NUM );

	cBufPos = cSrc;									/* restore source address so we don't miss the next line. */
	nLine = nCurrentLine;
}


const unsigned char keycnv[128] = {
	// 0-7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 8,9 (backspace, tab)
	0xD0, 0x9F,
	// 10-26
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 27 (esc)
	0x8F,
	// 28-31
	0x00, 0x00, 0x00, 0x00,
	// 32 (space)
	0x9D,
	// 33-43
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 44-47 (comma minus period forwardslash)
	0x99, 0xC4, 0x98, 0x97,
	// 48-57 (0-9)
	0xD8, 0xCF, 0xCE, 0xEE, 0xED, 0xEC, 0xCB, 0xDB, 0xEA, 0xD9,
	// 58-64 (colon semicolon < equals > ? @)
	0xB7, 0xA8, 0x00, 0xA2, 0x00, 0x00, 0x00,
	// 65-90 (A-Z)
	0xBE, 0x9B, 0xAD, 0xCD, 0xDD, 0xBC, 0xAC, 0xAB, 0xDA, 0xBA, 0xB9, 0xA9, 0x9A,
	0xAA, 0xC9, 0xC8, 0xEF, 0xCC, 0xAE, 0xDC, 0xCA, 0x9C, 0xDE, 0xBD, 0xBB, 0x9E,
	// 91-96 ([ \ ] ^ _ `)
	0xC7, 0x00, 0xA7, 0x00, 0x00, 0xD2,
	// 97-122 (a-z)
	0xBE, 0x9B, 0xAD, 0xCD, 0xDD, 0xBC, 0xAC, 0xAB, 0xDA, 0xBA, 0xB9, 0xA9, 0x9A,
	0xAA, 0xC9, 0xC8, 0xEF, 0xCC, 0xAE, 0xDC, 0xCA, 0x9C, 0xDE, 0xBD, 0xBB, 0x9E,
	// 123-126 ( { | } ~ )
	0x00, 0x00, 0x00, 0x00,
	// 127 (del)
	0xA6
};

unsigned char ConvertKey( short int nNum )
{
	if ((nNum < 0) || (nNum > 127))
		return 0;

	return keycnv[nNum];
}

char SpriteEvent( void )
{
	char cSpriteEvent = 0;

	if ( nEvent <= EVENT_INITIALISE_SPRITE ||
		 nEvent == EVENT_FELL_TOO_FAR )
	{
		cSpriteEvent = 1;
	}

	return ( cSpriteEvent );
}

/****************************************************************************************************************/
/* Command requires a number, variable or sprite parameter as an argument.                                      */
/****************************************************************************************************************/
unsigned short int CompileArgument( void )
{
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )
	{
		CR_ArgA( GetNum( 8 ) );
	}
	else
	{
		if ( nArg >= FIRST_PARAMETER &&
			 nArg <= LAST_PARAMETER )
		{
			CR_PamA( nArg );
		}
		else
		{
			if ( nArg == INS_STR )							/* it was a string argument. */
			{
				CR_ArgA( nMessageNumber++ );				/* number of this message. */
			}
			else
			{
				Error( "Not a number or variable" );
			}
		}
	}

	return ( nArg );
}

/****************************************************************************************************************/
/* Command requires a number, variable or sprite parameter as an argument.                                      */
/****************************************************************************************************************/
unsigned short int CompileKnownArgument( short int nArg )
{
	if ( nArg == INS_NUM )
	{
		CR_ArgA( GetNum( 8 ) );
	}
	else
	{
		if ( nArg >= FIRST_PARAMETER &&
			 nArg <= LAST_PARAMETER )
		{
			CR_PamA( nArg );
		}
		else
		{
			if ( nArg == INS_STR )							/* it was a string argument. */
			{
				CR_ArgA( nMessageNumber++ );				/* number of this message. */
			}
			else
			{
				Error( "Not a number or variable" );
			}
		}
	}

	return ( nArg );
}

unsigned short int NumberOnly( void )
{
	unsigned short int nArg = NextKeyword();

	if ( nArg == INS_NUM )
	{
		nArg = GetNum( 8 );
	}
	else
	{
		Error( "Only a number will do" );
		nArg = 0;
	}

	return ( nArg );
}

void CR_Operator( unsigned short int nOperator )
{
	nLastOperator = nOperator;
}

void CR_Else( void )
{
	unsigned short int nAddr1;
	unsigned short int nAddr2;
	unsigned short int nAddr3;

	if ( nNumIfs > 0 )
	{
		WriteInstruction( "jp " );							/* jump over the ELSE to the ENDIF. */
		nAddr2 = nCurrent;									/* store where we are. */
		nAddr1 = nIfBuff[ nNumIfs - 1 ][ 0 ];				/* original conditional jump. */
		nIfBuff[ nNumIfs - 1 ][ 0 ] = nAddr2;				/* store ELSE address so we can write it later. */
		nCurrent = nAddr2;
		WriteLabel( nAddr2 );								/* set jump address before ELSE. */

		nAddr3 = nCurrent;									/* where to resume after the ELSE. */
		nCurrent = nAddr1;
		WriteLabel( nAddr3 );								/* set jump address before ELSE. */
		nCurrent = nAddr3;
		nNextLabel = nCurrent;

		/* special case for < operator with ELSE. */
		if ( nIfBuff[ nNumIfs - 1 ][ 1 ] > 0 )				/* is there a second jump address to write? */
		{
			nCurrent = nIfBuff[ nNumIfs - 1 ][ 1 ];			/* place in source where label is required. */
			WriteLabel( nAddr3 );							/* write the label. */
			nCurrent = nAddr3;								/* place where we're writing the ELSE code. */
			nIfBuff[ nNumIfs - 1 ][ 1 ] = 0;				/* clear second label so ENDIF won't overwrite it. */
		}

		ResetIf();											/* no longer in an IF clause. */
	}
	else
	{
		Error( "ELSE without IF" );
	}
}

/****************************************************************************************************************/
/* We've hit a loose numeric value, so it's an argument for something.                                          */
/* We need to establish how it fits in to the code.                                                             */
/****************************************************************************************************************/
void CR_Arg( void )
{
	if ( nPamType == 255 )									/* this is the first argument we've found. */
	{
		nPamType = NUMERIC;									/* set flag to say we've found a number. */
		nPamNum = GetNum( 8 );
	}
	else													/* this is the second argument. */
	{
		if ( nIfSet > 0 )									/* we're in an IF or WHILE. */
		{
			CR_ArgA( GetNum( 8 ) );							/* compile code to set up this argument. */

			if ( nPamType == NUMERIC )
			{
				CR_ArgB( nPamNum );							/* compile second argument: numeric. */
				CR_StackIf();
			}

			if ( nPamType == SPRITE_PARAMETER )
			{
				CR_PamB( nPamNum );							/* compile second argument: variable or sprite parameter. */
				CR_StackIf();
			}
		}
		else												/* not a comparison, so we're setting a sprite parameter. */
		{
			if ( nPamType == SPRITE_PARAMETER )
			{
				CR_ArgA( GetNum( 8 ) );						/* compile second argument: variable or sprite parameter. */
				CR_PamC( nPamNum );							/* compile code to set variable or sprite parameter. */
			}
			else											/* trying to assign a number to another number. */
			{
				GetNum( 16 );								/* ignore the number. */
			}
			ResetIf();
		}
	}
}

/****************************************************************************************************************/
/* We've hit a loose variable or sprite parameter, so it's an argument for something.                           */
/* We need to establish how it fits in to the code.                                                             */
/****************************************************************************************************************/
void CR_Pam( unsigned short int nParam )
{
	if ( nPamType == 255 )									/* this is the first argument we've found. */
	{
		nPamType = SPRITE_PARAMETER;
		nPamNum = nParam;
	}
	else													/* this is the second argument. */
	{
		if ( nIfSet > 0 )									/* we're in an IF. */
		{
			CR_PamA( nParam );								/* compile second argument: variable or sprite parameter. */
			if ( nPamType == SPRITE_PARAMETER )
			{
				CR_PamB( nPamNum );							/* compare with first argument. */
			}
			else
			{
				CR_ArgB( nPamNum );							/* compare with first argument. */
			}
			CompileCondition();
			ResetIf();
		}
		else												/* not an IF, we must be assigning a value. */
		{
			if ( nPamType == SPRITE_PARAMETER )
			{
				CR_PamA( nParam );							/* set up the value. */
				CR_PamC( nPamNum );
			}
			else
			{
				ResetIf();
			}
		}
	}
}


/****************************************************************************************************************/
/* CR_ArgA, CR_PamA compile code to put the number or parameter in the accumulator.                             */
/* CR_ArgB, CR_PamB compile code to compare the number or parameter with the number already in the accumulator. */
/****************************************************************************************************************/
void CR_ArgA( short int nNum )
{
	if ( nNum == 0 )
	{
		WriteInstruction( "xor a" );
	}
	else
	{
		WriteInstruction( "ld a," );
		WriteNumber( nNum );
	}
}

void CR_ArgB( short int nNum )
{
	WriteInstruction( "cp " );
	WriteNumber( nNum );
	WriteJPNZ();											/* write conditional jump at end of if. */
}

void CR_PamA( short int nNum )
{
	char cVar[ 14 ];

	if ( nNum >= FIRST_VARIABLE )							/* load accumulator with global variable. */
	{
		sprintf( cVar, "ld a,(%s)", cVariables[ nNum - FIRST_VARIABLE ] );
		WriteInstruction( cVar );
	}
	else													/* load accumulator with sprite parameter. */
	{
		WriteInstructionArg( "ld a,(ix+?)", nNum - IDIFF );
	}
}

void CR_PamB( short int nNum )
{
	char cVar[ 13 ];

	if ( nNum >= FIRST_VARIABLE )							/* compare accumulator with global variable. */
	{
		sprintf( cVar, "ld hl,%s", cVariables[ nNum - FIRST_VARIABLE ] );
		WriteInstruction( cVar );
		WriteInstruction( "cp (hl)" );
	}
	else													/* compare accumulator with sprite parameter. */
	{
		WriteInstructionArg( "cp (ix+?)", nNum - IDIFF );
	}

	WriteJPNZ();											/* write conditional jump at end of if. */
}

void CR_PamC( short int nNum )
{
	char cVar[ 14 ];

	if ( nNum >= FIRST_VARIABLE )							/* compare accumulator with global variable. */
	{
		sprintf( cVar, "ld (%s),a", cVariables[ nNum - FIRST_VARIABLE ] );
		WriteInstruction( cVar );

		if ( nNum == VAR_SCREEN )							/* is this code changing the screen? */
		{
			WriteInstruction( "call nwscr" );				/* address of routine to display the new screen. */
		}
	}
	else													/* compare accumulator with sprite parameter. */
	{
		WriteInstructionArg( "ld (ix+?),a", nNum - IDIFF );
	}
}


void CR_StackIf( void )
{
	CompileCondition();
	ResetIf();
}

/****************************************************************************************************************/
/* Converts up/down/left/right to Kempston joystick right/left/down/up.                                         */
/****************************************************************************************************************/
short int Joystick( short int nArg )
{
	short int nArg2;

	switch( nArg )										/* conversion to Kempston bit order. */
	{
		case 0:
			nArg2 = 1;
			break;
		case 1:
			nArg2 = 0;
			break;
		case 2:
			nArg2 = 3;
			break;
		case 3:
			nArg2 = 2;
			break;
		default:
			nArg2 = nArg;
			break;
	}

	return ( nArg2 );
}

/****************************************************************************************************************/
/* We don't yet know where we want to jump following the condition, so remember address where label will be     */
/* written when we have that address.                                                                           */
/****************************************************************************************************************/
void CompileCondition( void )
{
	if ( nLastCondition == INS_IF )
	{
		if ( nNumIfs < NUM_NESTING_LEVELS )
		{
			nIfBuff[ nNumIfs ][ 0 ] = nCurrent - 6;			/* minus 6 for label after conditional jump. */
			nNumIfs++;
		}
		else
		{
			fputs( "Too many IFs\n", stderr );
		}
	}
	else
	{
		if ( nNumWhiles < NUM_NESTING_LEVELS )
		{
			nWhileBuff[ nNumWhiles ][ 0 ] = nCurrent - 6;	/* minus 6 for label after conditional jump. */
			nNumWhiles++;
		}
		else
		{
			fputs( "Too many WHILEs\n", stderr );
		}
	}
}

/****************************************************************************************************************/
/* Writes the conditional jump at the end of an IF.                                                             */
/****************************************************************************************************************/
void WriteJPNZ( void )
{
	if ( nLastCondition == INS_IF )
	{
		nIfBuff[ nNumIfs ][ 1 ] = 0;
	}
	else
	{
		nWhileBuff[ nNumWhiles ][ 1 ] = 0;
	}

	switch ( nLastOperator )
	{
		case OPE_NOT:
			WriteInstruction( "jp z,xxxxxx" );
			break;
		case OPE_GRTEQU:
			WriteInstruction( "jr z,$+5" );					/* test succeeded, skip jp nc instruction */
			WriteInstruction( "jp nc,xxxxxx" );
//			nIfBuff[ nNumIfs ][ 1 ] = nCurrent - 6;			/* minus 6 for label after conditional jump. */
//			WriteInstruction( "jp z,xxxxxx" );
			break;
		case OPE_GRT:
			WriteInstruction( "jp nc,xxxxxx" );
			break;
		case OPE_LESEQU:
			WriteInstruction( "jp c,xxxxxx" );
			break;
		case OPE_LES:
			WriteInstruction( "jp c,xxxxxx" );
			if ( nLastCondition == INS_IF )
			{
				nIfBuff[ nNumIfs ][ 1 ] = nCurrent - 6;		/* minus 6 for label after conditional jump. */
			}
			else
			{
				nWhileBuff[ nNumWhiles ][ 1 ] = nCurrent - 6;
			}
				nIfBuff[ nNumIfs ][ 1 ] = nCurrent - 6;		/* minus 6 for label after conditional jump. */
			WriteInstruction( "jp z,xxxxxx" );
			break;
		case OPE_EQU:
		default:
			WriteInstruction( "jp nz,xxxxxx" );
			break;
	}
}

void WriteNumber( unsigned short int nInteger )
{
	char cNum[ 6 ];
	char *cChar = cNum;

	sprintf( cNum, "%d", nInteger );
	cObjt = cStart + ( nCurrent - nAddress );

	while ( *cChar )
	{
		*cObjt = *cChar++;
		cObjt++;
		nCurrent++;
	}
}

void WriteText( const char *cChar )
{
	while ( *cChar )
	{
		*cObjt = *cChar++;
		cObjt++;
		nCurrent++;
	}
}

void WriteInstruction( const char *cCommand )
{
	NewLine();
	cObjt = cStart + ( nCurrent - nAddress );

	while ( *cCommand )
	{
		*cObjt = *cCommand++;
		cObjt++;
		nCurrent++;
	}
}

void WriteInstructionAndLabel( const char *cCommand )
{
	short int nChar = 0;
	unsigned char cLine[ 3 ] = "\n";

	cObjt = cStart + ( nCurrent - nAddress );

	while ( cLine[ nChar ] )
	{
		*cObjt = cLine[ nChar++ ];
		cObjt++;
		nCurrent++;
	}

	while ( *cCommand )
	{
		*cObjt = *cCommand++;
		cObjt++;
		nCurrent++;
	}
}

void WriteInstructionArg( const char *cCommand, unsigned short int nNum )
{
	NewLine();
	cObjt = cStart + ( nCurrent - nAddress );

	while ( *cCommand )
	{
		if ( *cCommand == '?' )
		{
			WriteNumber( nNum );
			cCommand++;
		}
		else
		{
			*cObjt = *cCommand++;
			cObjt++;
			nCurrent++;
		}
	}
}

void WriteLabel( unsigned short int nWhere )
{
	char cLabel[ 7 ];
	char *cChar = cLabel;

	sprintf( cLabel, "%c%05d", nEvent + 'a', nWhere >> 2 );
	cObjt = cStart + ( nCurrent - nAddress );

	while ( *cChar )
	{
		*cObjt = *cChar++;
		cObjt++;
		nCurrent++;
	}
}

void NewLine( void )
{
	char cLine[ 10 ] = "\n        ";
	char *cChar = cLine;

	cObjt = cStart + ( nCurrent - nAddress );

	if ( nNextLabel > 0 )
	{
		sprintf( cLine, "\n%c%05d: ", nEvent + 'a', nNextLabel >> 2 );
		nNextLabel = 0;
	}
	else
	{
		strcpy( cLine, "\n        " );
	}

	while ( *cChar )
	{
		*cObjt = *cChar++;
		cObjt++;
		nCurrent++;
	}
}

void Error( const char *cMsg )
{
	unsigned char *cErr;

	fprintf( stderr, "%s on line %d:\n", cMsg, nLine );
	cErr = cErrPos + 1;

	while ( *cErr != LF )
	{
		if ( *cErr != CR )
		{
			fprintf( stderr, "%c", *cErr++ );
		}
	}

	fprintf( stderr, "\n" );
	nErrors++;
}
