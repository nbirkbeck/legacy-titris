#ifndef _TITRIS_H_
#define _TITRIS_H_

#define MAX_BLOCKS 200
#define MAX_TITRI   2
#define ERR -1
#define ROWS 20
#define COLS 10

#define ONELINEINC 10
#define TWOLINEINC 20
#define THREELINEINC 40
#define TETRISINC    75
#define HIT_WALL 0
#define HIT_BLOCKSIDE 1
#define HIT_BLOCKBOTTOM 2

#define HIGHSCOREENTRIES 10 

//Here are the possible ways to move a titri
#define M_RIGHT	0
#define M_LEFT  1
#define M_DOWN  2
#define M_NOMOVE  3
//These are the types of titris
#define TLINE 0
#define TSQUARE 1
#define TUPSTEP 2
#define TDNSTEP 3
#define TT 4
#define TNORML 5
#define TREVL 6

//These are the rotation directives
#define ROTC 0
#define ROTCC 1
#define ROTNO 2
/*
	Drawing and block variables
*/
typedef struct tagBLOCK
{
	int x;
	int y;
	int valid;
	int blockImgIndex;
}BLOCK, *BLOCK_PTR;

typedef struct tagCOLINFO
{
	int cause;
	int yLoc;
	int xLoc;
}COLINFO;

typedef struct tagTITRI
{
	BLOCK* blocks[4];
	int valid;
	int titType;
	int speed;
}TITRI;

typedef struct tagFOCAL
{
	int next;
	int cur;
}FOCAL;

typedef struct tagSTAT
{
	int score;
	int lives;
	int blocks;
	int lines;
	int level;
}STAT;


typedef struct tagHIGHSCOREENTRY
{
	char name[255];
	int score;
}HIGHSCOREENTRY;

int loadLevel(const char * str);
void surfRemoveRow(int row);
void saveHighScores();
int makeTitri(TITRI * tit, int type);
int loadImageBackground(const char * filename);
int moveTitri(TITRI * tit,int direction);
int rotateTitri(TITRI * tit,int dir);
void blockcpy4(BLOCK dest[4],BLOCK * src[4]);
inline int checkNewBlockLocation(BLOCK *block);

#endif
