// work.cpp : Defines the entry point for the applicati
//
#define DEBUG 1
#include <stdlib.h>
#include "MyTimer.h"
#include "titris.h"
#include "ai.h"
#include "compat.h"
#include "sound.h"
#include <iostream>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH    640  // size of screen
#define SCREEN_HEIGHT   480
#define MAX_COLORS      256  // maximum colors


//tempstuff
FILE * debuginfo;

SDL_Window* window = nullptr;
SDL_Surface* surface = nullptr; // dd primary surface
SDL_Renderer* renderer = nullptr;
std::unordered_map<int, bool> keys;

int Gi = 0;
int Gx = 0;
int mousex=0;
int mousey=0;
int mousedown=0;
int rowsFilled[4];
int gDying;
int gCount;
int predict=0;

enum STATES
{
	MENU,PLAYING,PAUSED,LEVELSTART,LEVELEND,GAMESTART,GAMEEND,GAMEMENU,PLAYERDIED,HIGHSCORES
}state =GAMESTART;

STAT stats;
HIGHSCOREENTRY		highScores[HIGHSCOREENTRIES];
MyTimer timer;
FOCAL focal;
COLINFO colinfo;

TITRI titris[MAX_TITRI];
BLOCK blocks[MAX_BLOCKS];
BLOCK_PTR gameSurf[ROWS][COLS];

SDL_Surface*  background = nullptr;
SDL_Surface*  blockSurf[8];
SDL_Surface*  menuItems[6];
SDL_Surface*  hsSurf[4];
int numMenuItems =6;

char gBuffer[255]; //Global buffer, used for high score names
char gNumChars=0; //global count for the above buffer
bool setScore=false;

int delay=100;
int delayBack=100;
TTF_Font* font;

struct RGB {
  uint8_t r, g, b;
  RGB(int ri, int gi, int bi) : r(ri), g(gi), b(bi) {
  }
};

void Draw_Text_GDI(const char* message, int x, int y, const RGB& rgb, SDL_Surface* surface) {

  SDL_Color color = {rgb.r, rgb.g, rgb.b, 255};
  SDL_Surface* surfaceMessage =
    TTF_RenderText_Solid(font, message, color);

  SDL_Rect rect = {x, y, 100, 100};
  SDL_BlitSurface(surfaceMessage, nullptr, surface, &rect);

  SDL_FreeSurface(surfaceMessage);
}


void setState(STATES theState)
{
	char buffer[255];
	state = theState;
	switch(theState)
	{
		case GAMEMENU:
			loadImageBackground("textures/menu.bmp");
			break;
		case PLAYING:
			sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
			loadImageBackground(buffer);
			break;
		case HIGHSCORES:
			loadImageBackground("textures/hsback.bmp");
			break;
		default: break;
	}
}
void outputBlox(FILE * file)
{
	int i=0;
	while(i<MAX_BLOCKS)
	{
		fprintf(file,"x:%d y:%d valid:%d img:%d\n",blocks[i].x,blocks[i].y,blocks[i].valid,blocks[i].blockImgIndex);
		i++;
	}
}
void resetStats()
{
	stats.score=0;
	stats.lives=3;
	stats.blocks=0;
	stats.lines=0;
	stats.level=1;
	loadLevel("lvl/l1/");
}

int getNextAvailTitrisIndex()
{
	int cnt=0;
	int i=0;
	while(cnt<MAX_TITRI)
	{
		if(titris[cnt].valid==0)
		{
			if(makeTitri(&titris[cnt],rand()%7)==-1)
			{
				return -1;
			}
			break;
		}
		cnt++;
	}

	return cnt;

}
inline void doUsersMove()
{
	int dir=M_NOMOVE;
	if(keys[SDLK_RIGHT])
	{
		keys[SDLK_RIGHT]=false;
		dir=M_RIGHT;
	}
	if(keys[SDLK_LEFT])
	{
		keys[SDLK_LEFT]=false;
		dir=M_LEFT;
	}
	moveTitri(&titris[focal.cur],dir);
	dir=ROTNO;
	if(keys[SDLK_UP])
	{
		keys[SDLK_UP]=false;
		dir=ROTC;
	}
	if(keys[SDLK_DOWN])
	{
		keys[SDLK_DOWN]=false;
		dir=ROTCC;
	}
	rotateTitri(&titris[focal.cur],dir);	
}

int numAvailBlocks()
{
	int index=0;
	int count=0;
	while(index<MAX_BLOCKS)
	{
		if(blocks[index].valid==1)
			count++;
		index++;
	}
	return count;
}


/*
	getAvailBlockIndex()
	return the index of the next available block, and returns
	-1 if their is no available blocks.
*/
int getAvailBlockIndex()
{
	int index=0;
	while(index<MAX_BLOCKS)
	{
		if(blocks[index].valid==0)
			break;
		index++;
	}
	if(index==MAX_BLOCKS)
		index=ERR;
	return index;
}

int checkBlockLocation(BLOCK *block)
{

	if(block->x<0||block->x>=COLS)
	{
		colinfo.cause=HIT_WALL;
		return 0;
	}
	if(block->y>=ROWS)
	{
		colinfo.cause=HIT_BLOCKBOTTOM;
		colinfo.yLoc=20;
		return 0;
	}
	if(gameSurf[block->y][block->x]!=NULL)
	{
		colinfo.cause=HIT_BLOCKBOTTOM;
		colinfo.yLoc=block->y;
		colinfo.xLoc=block->x;
		return 0;
	}

	return 1;
}
/*
	CheckNewLocation(TITRI *tit) 
	returns 1 if the move was ok. 
*/
int checkNewLocation(TITRI *tit)
{
	int index=0;
	while(index<4)
	{
		if(!checkBlockLocation(tit->blocks[index]))
			return 0;
		index++;
	}
	return 1;
}

int makeTitri(TITRI * tit, int type)
{
	int zeroI,oneI,twoI,threeI;
	/*
	if(predict)
	{
		type =rand()%5;
		if(type==1)
			type=TLINE;
		else if(type==2)
			type=TSQUARE;
		else if(type==0)
			type=TT;
		else if(type==3)
			type=TNORML;
		else
			type=TREVL;
	}
	*/
	
	int flag=0;
	if((zeroI=getAvailBlockIndex())==-1)
		flag=1;
	else
		blocks[zeroI].valid=1;
	if((oneI=getAvailBlockIndex())==-1)
		flag=1;
	else
		blocks[oneI].valid=1;
	if((twoI=getAvailBlockIndex())==-1)
		flag=1;
	else
		blocks[twoI].valid=1;
	if((threeI=getAvailBlockIndex())==-1)
		flag=1;
	else 
		blocks[threeI].valid=1;
	if(flag)
	{
		if(DEBUG)
		{
			if(debuginfo)
				fprintf(debuginfo,"OUT OF BLOX/n");
		}
		return ERR;
	}
	
	tit->blocks[0] = &blocks[zeroI];
	tit->blocks[1] = &blocks[oneI];
	tit->blocks[2] = &blocks[twoI];
	tit->blocks[3] = &blocks[threeI];

	for(int i =0;i<4;i++)
	{
		tit->blocks[i]->blockImgIndex=type;
	}

	tit->valid=1;
	tit->titType=type;

	switch(type)
	{

		case TT:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-1;
			blocks[oneI].x=6;
			blocks[oneI].y=-1;
			blocks[twoI].x=7;
			blocks[twoI].y=-1;
			blocks[threeI].x=6;
			blocks[threeI].y=-2;
			break;
		case TSQUARE:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-1;
			blocks[oneI].x=6;
			blocks[oneI].y=-1;
			blocks[twoI].x=5;
			blocks[twoI].y=-2;
			blocks[threeI].x=6;
			blocks[threeI].y=-2;
			break;
		case TLINE:
			blocks[zeroI].x=6;
			blocks[zeroI].y=-4;
			blocks[oneI].x=6;
			blocks[oneI].y=-3;
			blocks[twoI].x=6;
			blocks[twoI].y=-2;
			blocks[threeI].x=6;
			blocks[threeI].y=-1;
			break;
		case TUPSTEP:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-1;
			blocks[oneI].x=6;
			blocks[oneI].y=-1;
			blocks[twoI].x=6;
			blocks[twoI].y=-2;
			blocks[threeI].x=7;
			blocks[threeI].y=-2;
			break;
		case TDNSTEP:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-2;
			blocks[oneI].x=6;
			blocks[oneI].y=-2;
			blocks[twoI].x=6;
			blocks[twoI].y=-1;
			blocks[threeI].x=7;
			blocks[threeI].y=-1;
			break;
		case TNORML:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-1;
			blocks[oneI].x=6;
			blocks[oneI].y=-1;
			blocks[twoI].x=7;
			blocks[twoI].y=-1;
			blocks[threeI].x=7;
			blocks[threeI].y=-2;
			break;
		case TREVL:
			blocks[zeroI].x=5;
			blocks[zeroI].y=-2;
			blocks[oneI].x=5;
			blocks[oneI].y=-1;
			blocks[twoI].x=6;
			blocks[twoI].y=-1;
			blocks[threeI].x=7;
			blocks[threeI].y=-1;
			break;
		default: break;
	}
	return 1;
}
int titrisInit()
{
	int i =0;
	int temp;

	gDying=0;
	delay = 100;
	while(i<MAX_BLOCKS)
	{
		memset(&blocks[i],0,sizeof(BLOCK));
		i++;
	}
	i=0;
	while(i<MAX_TITRI)
	{
		temp = rand()%7;
		memset(&titris[i],0,sizeof(TITRI));
		makeTitri(&titris[i],temp);
		i++;
	}
	focal.cur=0;
	focal.next=1;
	titris[focal.next].valid=2;
	i=0;
	
	while(i<ROWS)
	{
		int j=0;
		while(j<COLS)
		{
			gameSurf[i][j]=NULL;
			j++;
		}
		i++;
	}
	return 1;
}
int drawBlock(BLOCK * aBlock)
{	
	int x=aBlock->x*16+240;
	int y=aBlock->y*16+50;

	if(aBlock->y<0)
          return 1;
	DDraw_Draw_Surface(blockSurf[aBlock->blockImgIndex],x,y,16,16,surface,0);
	return 1;
}
int drawTitri(TITRI * aTit)
{
	for(int i =0;i<4;i++)
	{
		if(aTit->blocks[i]->y>=0)
			drawBlock(aTit->blocks[i]);
	}
	return 1;
}
int rotateTitri(TITRI * tit,int dir)
{
	BLOCK backUps[4];
	blockcpy4(backUps,tit->blocks);
	memset(&colinfo,0,sizeof(COLINFO));
	if(dir==ROTNO)
		return 1;
	if(tit->titType==TSQUARE)
		return 1;
	if(dir==ROTC)
	{
		switch(tit->titType)
		{
			//choose third block to be center
			case(TLINE):
			{
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					tit->blocks[0]->y+=2;
					tit->blocks[1]->y+=1;
					tit->blocks[3]->y--;
					tit->blocks[0]->x=tit->blocks[2]->x;
					tit->blocks[1]->x=tit->blocks[2]->x;
					tit->blocks[3]->x=tit->blocks[2]->x;
				}
				else
				{
					tit->blocks[0]->x+=2;
					tit->blocks[1]->x+=1;
					tit->blocks[3]->x--;
					tit->blocks[0]->y=tit->blocks[2]->y;
					tit->blocks[1]->y=tit->blocks[2]->y;
					tit->blocks[3]->y=tit->blocks[2]->y;
				}
			}
			break;
			case(TUPSTEP):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					/*
						looks like:
								   23
								  01
					*/
					if(tit->blocks[0]->y>tit->blocks[2]->y)
					{
						tit->blocks[0]->x=tit->blocks[2]->x;
						tit->blocks[0]->y=tit->blocks[2]->y;
						tit->blocks[2]->x++;
						tit->blocks[2]->y++;
						tit->blocks[3]->y+=2;
					}
					/*
						Looks like: 10
					               32
					*/
					else
					{
						tit->blocks[3]->y-=2;
						tit->blocks[2]->x--;
						tit->blocks[2]->y--;
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
					}
				}
				else
				{
					if(tit->blocks[0]->y<tit->blocks[3]->y)
					{
						tit->blocks[3]->x-=2;
						tit->blocks[2]->x--;
						tit->blocks[2]->y++;
						tit->blocks[0]->y++;
						tit->blocks[0]->x++;
					}
					else
					{
						tit->blocks[3]->x+=2;
						tit->blocks[2]->x++;
						tit->blocks[2]->y--;
						tit->blocks[0]->x--;
						tit->blocks[0]->y--;
					}
				}
			break;
			case(TDNSTEP):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y>tit->blocks[2]->y)
					{
						tit->blocks[3]->x++;
						tit->blocks[3]->y--;
						tit->blocks[1]->y--;
						tit->blocks[1]->x--;
						tit->blocks[0]->x-=2;
					}
					else
					{
						tit->blocks[3]->x--;
						tit->blocks[3]->y++;
						tit->blocks[1]->x++;
						tit->blocks[1]->y++;
						tit->blocks[0]->x+=2;
					}
				}
				else
				{
					if(tit->blocks[0]->y<tit->blocks[3]->y)
					{
						tit->blocks[0]->y+=2;
						tit->blocks[1]->x--;
						tit->blocks[1]->y++;
						tit->blocks[3]->y--;
						tit->blocks[3]->x--;
					}
					else
					{
						tit->blocks[0]->y-=2;
						tit->blocks[1]->x++;
						tit->blocks[1]->y--;
						tit->blocks[3]->y++;
						tit->blocks[3]->x++;
					}
				}
			break;
			case(TNORML):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y > tit->blocks[3]->y)
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x++;
						tit->blocks[2]->y++;
						tit->blocks[2]->x--;
						tit->blocks[3]->y+=2;
					}
					else
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
						tit->blocks[2]->y--;
						tit->blocks[2]->x++;
						tit->blocks[3]->y-=2;
					}
				}	
				else
				{
					if(tit->blocks[0]->y < tit->blocks[3]->y)
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x++;
						tit->blocks[2]->y--;
						tit->blocks[2]->x--;
						tit->blocks[3]->x-=2;
					}
					else
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x--;
						tit->blocks[2]->y++;
						tit->blocks[2]->x++;
						tit->blocks[3]->x+=2;
					}
				}
				break;
			case(TREVL):
				if(tit->blocks[1]->y==tit->blocks[2]->y)
				{
					if(tit->blocks[1]->y >tit->blocks[0]->y)
					{
						tit->blocks[3]->y++;
						tit->blocks[3]->x--;
						tit->blocks[1]->x++;
						tit->blocks[1]->y--;
						tit->blocks[0]->x+=2;
					}
					else
					{
						tit->blocks[3]->y--;
						tit->blocks[3]->x++;
						tit->blocks[1]->x--;
						tit->blocks[1]->y++;
						tit->blocks[0]->x-=2;
					}
				}	
				else
				{
					if(tit->blocks[2]->y > tit->blocks[0]->y)
					{
						tit->blocks[3]->x--;
						tit->blocks[3]->y--;
						tit->blocks[1]->x++;
						tit->blocks[1]->y++;
						tit->blocks[0]->y+=2;
					}
					else
					{
						tit->blocks[3]->x++;
						tit->blocks[3]->y++;
						tit->blocks[1]->x--;
						tit->blocks[1]->y--;
						tit->blocks[0]->y-=2;
					}
				}
				break;
			case(TT):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y >tit->blocks[3]->y)
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x++;
						tit->blocks[2]->y++;
						tit->blocks[2]->x--;
						tit->blocks[3]->x++;
						tit->blocks[3]->y++;
					}
					else
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
						tit->blocks[2]->y--;
						tit->blocks[2]->x++;
						tit->blocks[3]->x--;
						tit->blocks[3]->y--;
					}
				}	
				else
				{
					if(tit->blocks[0]->y < tit->blocks[3]->y)
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x++;
						tit->blocks[2]->y--;
						tit->blocks[2]->x--;
						tit->blocks[3]->x--;
						tit->blocks[3]->y++;
					}
					else
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x--;
						tit->blocks[2]->y++;
						tit->blocks[2]->x++;
						tit->blocks[3]->x++;
						tit->blocks[3]->y--;
					}
				}
			break;

			default: return -1;
		}
	}
	else
	{
		switch(tit->titType)
		{
			case(TLINE):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					tit->blocks[0]->y+=2;
					tit->blocks[1]->y+=1;
					tit->blocks[3]->y--;
					tit->blocks[0]->x=tit->blocks[2]->x;
					tit->blocks[1]->x=tit->blocks[2]->x;
					tit->blocks[3]->x=tit->blocks[2]->x;
				}
				else
				{
					tit->blocks[0]->x+=2;
					tit->blocks[1]->x+=1;
					tit->blocks[3]->x--;
					tit->blocks[0]->y=tit->blocks[2]->y;
					tit->blocks[1]->y=tit->blocks[2]->y;
					tit->blocks[3]->y=tit->blocks[2]->y;
				}
				break;
			case(TUPSTEP):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y>tit->blocks[2]->y)
					{
						tit->blocks[0]->x++;
						tit->blocks[0]->y++;
						tit->blocks[2]->x--;
						tit->blocks[2]->y++;
						tit->blocks[3]->x-=2;
					}
					else
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x--;
						tit->blocks[2]->x++;
						tit->blocks[2]->y--;
						tit->blocks[3]->x+=2;
					}
				}
				else
				{
					if(tit->blocks[0]->y<tit->blocks[3]->y)
					{
						tit->blocks[3]->y-=2;
						tit->blocks[2]->x--;
						tit->blocks[2]->y--;
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
					}
					else
					{
						tit->blocks[3]->y+=2;
						tit->blocks[2]->x++;
						tit->blocks[2]->y++;
						tit->blocks[0]->y--;
						tit->blocks[0]->x++;
					}
				}
			break;
			case(TDNSTEP):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y>tit->blocks[2]->y)
					{
						tit->blocks[3]->x++;
						tit->blocks[3]->y++;
						tit->blocks[1]->x++;
						tit->blocks[1]->y--;
						tit->blocks[0]->y-=2;
					}
					else
					{
						tit->blocks[3]->y--;
						tit->blocks[3]->x--;
						tit->blocks[1]->x--;
						tit->blocks[1]->y++;
						tit->blocks[0]->y+=2;
					}
				}
				else
				{
					if(tit->blocks[0]->y<tit->blocks[3]->y)
					{
						tit->blocks[0]->x-=2;
						tit->blocks[1]->x--;
						tit->blocks[1]->y--;
						tit->blocks[3]->y--;
						tit->blocks[3]->x++;
					}
					else
					{
						tit->blocks[0]->x+=2;
						tit->blocks[1]->x++;
						tit->blocks[1]->y++;
						tit->blocks[3]->y++;
						tit->blocks[3]->x--;
					}
				}
			break;
			case(TNORML):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y >tit->blocks[3]->y)
					{
						tit->blocks[2]->y--;
						tit->blocks[2]->x--;
						tit->blocks[3]->x-=2;
						tit->blocks[0]->x++;
						tit->blocks[0]->y++;
					}
					else
					{
						tit->blocks[2]->y++;
						tit->blocks[2]->x++;
						tit->blocks[3]->x+=2;
						tit->blocks[0]->x--;
						tit->blocks[0]->y--;
					}
				}	
				else
				{
					if(tit->blocks[0]->y < tit->blocks[3]->y)
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
						tit->blocks[2]->y--;
						tit->blocks[2]->x++;
						tit->blocks[3]->y-=2;
					}
					else
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x++;
						tit->blocks[2]->y++;
						tit->blocks[2]->x--;
						tit->blocks[3]->y+=2;
					}
				}
				break;
			case(TREVL):
				if(tit->blocks[1]->y==tit->blocks[2]->y)
				{
					if(tit->blocks[1]->y >tit->blocks[0]->y)
					{
						tit->blocks[3]->y--;
						tit->blocks[3]->x--;
						tit->blocks[1]->x++;
						tit->blocks[1]->y++;
						tit->blocks[0]->y+=2;
					}
					else
					{
						tit->blocks[3]->y++;
						tit->blocks[3]->x++;
						tit->blocks[1]->x--;
						tit->blocks[1]->y--;
						tit->blocks[0]->y-=2;
					}
				}	
				else
				{
					if(tit->blocks[2]->y > tit->blocks[0]->y)
					{
						tit->blocks[3]->x++;
						tit->blocks[3]->y--;
						tit->blocks[1]->x--;
						tit->blocks[1]->y++;
						tit->blocks[0]->x-=2;
					}
					else
					{
						tit->blocks[3]->x--;
						tit->blocks[3]->y++;
						tit->blocks[1]->x++;
						tit->blocks[1]->y--;
						tit->blocks[0]->x+=2;
					}
				}
				break;
			case(TT):
				if(tit->blocks[0]->y==tit->blocks[1]->y)
				{
					if(tit->blocks[0]->y >tit->blocks[3]->y)
					{
						tit->blocks[2]->y--;
						tit->blocks[2]->x--;
						tit->blocks[3]->y++;
						tit->blocks[3]->x--;
						tit->blocks[0]->x++;
						tit->blocks[0]->y++;
					}
					else
					{
						tit->blocks[2]->y++;
						tit->blocks[2]->x++;
						tit->blocks[3]->y--;
						tit->blocks[3]->x++;
						tit->blocks[0]->x--;
						tit->blocks[0]->y--;
					}
				}	
				else
				{
					if(tit->blocks[0]->y < tit->blocks[3]->y)
					{
						tit->blocks[0]->y++;
						tit->blocks[0]->x--;
						tit->blocks[2]->y--;
						tit->blocks[2]->x++;
						tit->blocks[3]->x--;
						tit->blocks[3]->y--;
					}
					else
					{
						tit->blocks[0]->y--;
						tit->blocks[0]->x++;
						tit->blocks[2]->y++;
						tit->blocks[2]->x--;
						tit->blocks[3]->x++;
						tit->blocks[3]->y++;
					}
				}
				break;
			default: break;
		}
	}
	if(!checkNewLocation(tit))
	{
		int i =0;
		while(i<4)
		{
			*(tit->blocks[i])=backUps[i];
			i++;
		}
		if(colinfo.cause==HIT_WALL)
			return 1;
		else return 0;
	}
	return 1;

}
void blockcpy4(BLOCK dest[4],BLOCK * src[4])
{
	int i=0;
	while(i<4)
	{
		dest[i]=*src[i];
		i++;
	}
}

int moveTitri(TITRI * tit,int direction)
{
	BLOCK backUps[4];
	int result=-1;
	blockcpy4(backUps,tit->blocks);
	memset(&colinfo,0,sizeof(COLINFO));

	//Remember to Check collision with walls and other titis
	switch(direction)
	{
	case M_LEFT:
		tit->blocks[0]->x--;
		tit->blocks[1]->x--;
		tit->blocks[2]->x--;
		tit->blocks[3]->x--;
	break;
	case M_DOWN:
		tit->blocks[0]->y++;
		tit->blocks[1]->y++;
		tit->blocks[2]->y++;
		tit->blocks[3]->y++;
	break;
	case M_RIGHT:
		tit->blocks[0]->x++;
		tit->blocks[1]->x++;
		tit->blocks[2]->x++;
		tit->blocks[3]->x++;
	break;
	default:break;
	}
	if(!checkNewLocation(tit))
	{
		int i =0;
		while(i<4)
		{
			*(tit->blocks[i])=backUps[i];
			i++;
		}
		if(colinfo.cause==HIT_WALL)
			return 1;
		else return 0;
	}
	return 1;

}
int loadLevel(const char * str)
{
	
	char buf[255];
	for(int i =0;i<8;i++)
	{
 		if(blockSurf[i])
		{
                  SDL_FreeSurface(blockSurf[i]);
                  blockSurf[i]=NULL;
		}
	}

	strcpy(buf,str);
	strcat(buf,"block1.bmp");
	blockSurf[0] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block2.bmp");
	blockSurf[1] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block3.bmp");
	blockSurf[2] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block4.bmp");
	blockSurf[3] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block5.bmp");
	blockSurf[4] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block6.bmp");
	blockSurf[5] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"block7.bmp");
	blockSurf[6] = SDL_LoadBMP(buf);

	memset(buf,0,255);
	strcpy(buf,str);
	strcat(buf,"dying.bmp");
	blockSurf[7] = SDL_LoadBMP(buf);
	return 1;
}

int Game_Init()
{
   FILE * hs;
   /*
     Set the computer AI to off
   */
   predict=0;
   debuginfo=fopen("debug.txt","w");

   /*
		Open up the high scores file
   */
   memset(highScores,0,sizeof(highScores));
   hs= fopen("tet.hsd","r");
   if(hs)
   {
     for(int i=0;i<HIGHSCOREENTRIES;i++)
     {
       fscanf(hs,"%s %d",highScores[i].name,&highScores[i].score);
     }
     fclose(hs);
   }
   else
   {
     for(int i=0;i<HIGHSCOREENTRIES;i++)
     {
       strcpy(highScores[i].name, "AAA");
     }
   }

   int rendererFlags = SDL_RENDERER_SOFTWARE, windowFlags = 0;

   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
     printf("Couldn't initialize SDL: %s\n", SDL_GetError());
     exit(1);
   }
   
   window = SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                             SCREEN_HEIGHT, windowFlags);
   
   if (!window) {
     printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_GetError());
     exit(1);
   }
   
   SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
   
   renderer = SDL_CreateRenderer(window, -1, rendererFlags);
   
   if (!renderer) {
     printf("Failed to create renderer: %s\n", SDL_GetError());
     exit(1);
   }

   if (TTF_Init()==-1) {
     std::cerr << "Failed to TTF: " << SDL_GetError();
     return -1;
   }
   font = TTF_OpenFont("Vera.ttf", 14);
   loadLevel("lvl/l1/");
	
   menuItems[0] = SDL_LoadBMP("textures/start.bmp");
   menuItems[1] = SDL_LoadBMP("textures/startHi.bmp");
   menuItems[2] = SDL_LoadBMP("textures/hs.bmp");
   menuItems[3] = SDL_LoadBMP("textures/hsHi.bmp");
   menuItems[4] = SDL_LoadBMP("textures/exit.bmp");
   menuItems[5] = SDL_LoadBMP("textures/exitHi.bmp");
   hsSurf[0] = SDL_LoadBMP("textures/ok.bmp");
   hsSurf[1] = SDL_LoadBMP("textures/okHi.bmp");
   hsSurf[2] = SDL_LoadBMP("textures/reset.bmp");
   hsSurf[3] = SDL_LoadBMP("textures/resetHi.bmp");

   loadImageBackground("textures/startup.bmp");
   return 0;
}

int loadImageBackground(const char * filename)
{
  if(background)
  {
    SDL_FreeSurface(background);
    background=NULL;
  }
  background = SDL_LoadBMP(filename);
  return 1;
}

int drawBackground(int trans=0)
{
  DDraw_Draw_Surface(background,0,0,640,480,surface,trans);
  return 1;
}

int fillSurfRow()
{
  int temp=ROWS-1;
  int bounds = ROWS-2;
  int j=0;
  BLOCK block;
  block.blockImgIndex=rand()%7;
  while(bounds>=-1)
  {
		
    block.y=temp;
    j=0;
    while(j<COLS)
    {
      block.x=j;
      drawBlock(&block);
      j++;
    }
		
		
    temp--;
    if(temp==bounds)
    {
      //while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
      Sleep(10);
      bounds--;
      temp=ROWS-1;
    }
  }
  return 1;
}

int scanSurfForLines(int *rowsFilled)
{
  int i,j;
  int numRowsFilled=0;
  i=0;
  for(i=0;i<ROWS;i++)
  {
    int flag=1;
		
    for(j=0;j<COLS;j++)
    {
      if(gameSurf[i][j]==NULL)
      {
        flag=0;
      }
    }
    if(flag)
    {
      rowsFilled[numRowsFilled]=i;
      numRowsFilled++;
    }
  }
  return numRowsFilled;
}

void surfRemoveRows(int * srcRows,int cnt)
{
  int i;

  if(!cnt)
    return;
  for(i=0;i<cnt;i++)
  {
    int j=0;
    while(j<COLS)
    {
      gameSurf[srcRows[i]][j]->valid=0;
      memset(gameSurf[srcRows[i]][j],0,sizeof(BLOCK));
      j++;
    }
  }
  i=0;
  while(i<cnt)
  {
    surfRemoveRow(srcRows[i]);
    i++;
  }
}

void surfRemoveRow(int row)
{
  int i=row-1;
  while(i>=0)
  {
    int j=0;
    while(j<COLS)
    {
      gameSurf[i+1][j]=gameSurf[i][j];
      gameSurf[i][j]=NULL;
      if(gameSurf[i+1][j])
        gameSurf[i+1][j]->y=i+1;
      j++;
    }
    i--;
  }
}

void surfSetRowsDying(int * srcRows,int cnt)
{
  int i=0;
	
  for(i=0;i<cnt;i++)
  {
    int j=0;
    while(j<COLS)
    {
      //This shouldnt be necessary
      if(gameSurf[srcRows[i]][j])
        gameSurf[srcRows[i]][j]->blockImgIndex=7;
      j++;
    }
  }
}

int simulateGravity()
{
	int flag =1;
	
	if(!moveTitri(&titris[focal.cur],M_DOWN))
	{
		if(colinfo.cause==HIT_BLOCKBOTTOM)
		{
			gCount=0;
			PlaySound("sounds/hit.wav");
			if(gDying!=0)
			{
				gDying=0;
				surfRemoveRows(rowsFilled,gCount);
			}
			
			memset(rowsFilled,0,sizeof(rowsFilled));

			if(colinfo.yLoc<=1)
			{
				//Player died
				flag=0;
				state=PLAYERDIED;
			}
			if(flag)
			{
				
				if(DEBUG)
				{
					/*
					fprintf(debuginfo,"OLD:cur:%d, next:%d\n",focal.cur,focal.next);
					fprintf(debuginfo,"OLD:gCount:%d, gDying:%d\n",gCount, gDying);
					*/
				}

				for(int i =0;i<4;i++)
				{
					if(titris[focal.cur].blocks[i]->y>=0)
					{
						gameSurf[titris[focal.cur].blocks[i]->y][titris[focal.cur].blocks[i]->x]=
								titris[focal.cur].blocks[i];
					}
				}
				stats.blocks+=4;
				titris[focal.cur].valid=0;
				titris[focal.cur].blocks[0]=NULL;
				titris[focal.cur].blocks[1]=NULL;
				titris[focal.cur].blocks[2]=NULL;
				titris[focal.cur].blocks[3]=NULL;
			
				gCount=scanSurfForLines(rowsFilled);
				
				if(gCount>0)
				{
					surfSetRowsDying(rowsFilled,gCount);
					if(predict)
					{
						gDying=0;
						surfRemoveRows(rowsFilled,gCount);
					}
					else
						gDying=-20; //WIll Blink for 20frames, so 2/3s.
					/* 
						here is where we update the score
					*/
					
					
					switch(gCount)
					{
					case 1:
						stats.score+=ONELINEINC;
						stats.lines+=1;
						PlaySound("Sounds/line.wav");
						break;
					case 2:
						stats.score+=TWOLINEINC;
						stats.lines+=2;
						PlaySound("Sounds/line.wav");
						break;
					case 3:
						stats.score+=THREELINEINC;
						stats.lines+=3;
						PlaySound("Sounds/line.wav");
						break;
					case 4:
						stats.score+=TETRISINC;
						stats.lines+=4;
						PlaySound("Sounds/tetris.wav");
						break;
					default: break;
					}
					char buffer[255];
					switch(stats.level)
					{
						case 1:
							
							if(stats.lines>=20)
							{
								delayBack=100;
								stats.level++;
								loadLevel("lvl/l2/");
								sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								loadImageBackground(buffer);
							}
							break;
						case 2:
							
							if(stats.lines>=100)
							{
								delayBack=85;
								stats.level++;
								loadLevel("lvl/l3/");
								sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								loadImageBackground(buffer);
								
							}
							break;

						case 3:
							if(stats.lines>=400)
							{
								stats.level++;
								loadLevel("lvl/l4/");
								sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								loadImageBackground(buffer);
								delayBack=80;
							}
							break;
						case 4:
							if(stats.lines>=175)
							{
								stats.level++;
								loadLevel("lvl/l5/");
								sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								loadImageBackground(buffer);
								delayBack=70;
							}
							break;
						case 5:
							if(stats.lines>=250)
							{
								stats.level++;
								loadLevel("lvl/l6/");
								sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								loadImageBackground(buffer);
								delayBack=50;
							}
							break;
						case 6:
							if(stats.lines>=500)
							{
								stats.level++;
								//sprintf(buffer,"lvl/l%d/back.bmp",stats.level);
								//loadImageBackground(buffer);
								delayBack=25;
							}
							break;
						default:
							delayBack--;
					}
					
				}
				

				if(predict)
					AIfindDestination(&titris[focal.next]);

				focal.cur = focal.next;
				
				if((focal.next = getNextAvailTitrisIndex())==-1)
					;
				titris[focal.next].valid=2;
				if(focal.cur==focal.next)
					;
				if(DEBUG)
				{
					/*
						Debuging outline function
					int array[COLS];
					getSurfOutline(array);
					fprintf(debuginfo,"Surface outline\n");
					for(int i=0;i<COLS;i++)
						fprintf(debuginfo,"%d ",array[i]);
					
					fprintf(debuginfo,"/n");
					*/
					/*
					fprintf(debuginfo,"cur:%d, next:%d\n",focal.cur,focal.next);
					fprintf(debuginfo,"gCount:%d, gDying:%d\n",gCount, gDying);
					fprintf(debuginfo,"lines:%d\n",stats.lines);
					fprintf(debuginfo,"level;%d\n",stats.level);
					int temp= numAvailBlocks();
					fprintf(debuginfo,"NumavailBlox:%d\n",temp);
					*/
				}
			}		
		}
	}
	return flag;
}
int updateGameState()
{
	int flag=1;
	if(!predict)
		doUsersMove();
	else
	{
		delay--;
		if(!(delay%1))
		{
			AIdoMove();
			flag=simulateGravity();
			return flag;
		}
		if(delay<0)
			delay=20;
	}
	if(gDying<-1)
	{
		gDying++;
	}
	if(gDying==-1)
	{
		gDying=0;
		surfRemoveRows(rowsFilled,gCount);
	}
	/*
		IF the user is playing, and pressing space,
		pull the block down faster
	*/
	if(keys[SDLK_SPACE] && !predict)
	{
		flag=simulateGravity();
	}

	delay--;
	
	/*
		This is the actual gravity that is inflicted on a tetri,
		but it only happens when delay is counted below 0
	*/
	if(delay<0)
	{
		delay=delayBack;
		if(flag)
			flag=simulateGravity();
		
	}
	return flag;
}
int drawGameSurf()
{
	drawTitri(&titris[focal.cur]);
	for(int i=0;i<ROWS;i++)
	{
		for(int j=0;j<COLS;j++)
		{
			if(gameSurf[i][j]!=NULL)
				drawBlock(gameSurf[i][j]);
		}
	}
	return 1;

}
void insertingNewScore()
{
	//User hasnt finished putting in name, so just output what they have so far
	if(gBuffer[gNumChars-1]!='\r')
	{
		gBuffer[gNumChars]='\0';
		Draw_Text_GDI("You have new High Score, Enter Name:",200,275,RGB(200,100,200),surface);
		Draw_Text_GDI(gBuffer,200,285,RGB(200,100,200),surface);
	}
	else
	{
		/*
			Need to get users name here somehow
		*/
		gBuffer[gNumChars-1]='\0';

		highScores[9].score = stats.score;
		strcpy(highScores[9].name,gBuffer);
		memset(gBuffer,0,sizeof(gBuffer));
		gNumChars=0;
		setScore=false;
		int i=0;
		int j=0;
		for(i=0;i<HIGHSCOREENTRIES-1;i++)
		{
			int largI;
			j=i;
			largI=j;
			while(j<HIGHSCOREENTRIES)
			{
				if(highScores[largI].score<=highScores[j].score)
				{
					largI=j;
				}
				j++;
			}
			HIGHSCOREENTRY temp;
			temp.score=highScores[i].score;
			strcpy(temp.name,highScores[i].name);
			highScores[i].score=highScores[largI].score;
			strcpy(highScores[i].name,highScores[largI].name);
			highScores[largI].score=temp.score;
			strcpy(highScores[largI].name,temp.name);
		}
		/*
			This may be a bad place to do this, but anyhow
			Couldnt do it anywhere else, or info would be gone for here
		*/
		saveHighScores();
		//resetStats();
	}
}
void saveHighScores()
{
	FILE *hs = fopen("tet.hsd","w");
	int i;
	for(i=0;i<HIGHSCOREENTRIES;i++)
		fprintf(hs,"%s %d\n",highScores[i].name,highScores[i].score);
	fclose(hs);
}


int drawHighScoreBoard()
{
	char buf[255];
	sprintf(buf,"       Name             Score",stats.score);
	Draw_Text_GDI(buf,200,150,RGB(80,4,3),surface);
        
	for(int i=0;i<10;i++)
	{
		sprintf(buf,"%20s  %15d",highScores[i].name, highScores[i].score);
		Draw_Text_GDI(buf,200,160+10*i,RGB{100,50,7},surface);
	}
	if(!setScore)
	{
		if(mousex>192&&mousex<320)
		{
                  DDraw_Draw_Surface(hsSurf[1],192,300,128,32,surface,1);
                  if(mousedown)
                  {
                    mousedown=0;
                    setState(GAMEMENU);
                  }
		}
		else
                  DDraw_Draw_Surface(hsSurf[0],192,300,128,32,surface,1);


		if(mousex<468&&mousex>320)
		{
                  DDraw_Draw_Surface(hsSurf[3],320,300,128,32,surface,1);
			if(mousedown)
			{
				mousedown=0;
				for(int i=0;i<HIGHSCOREENTRIES;i++)
				{
					strcpy(highScores[i].name,"____");
					highScores[i].score=0;
				
				}
				/*
					Save the reset scores to disk
				*/
				saveHighScores();
			}
		}
		else
                  DDraw_Draw_Surface(hsSurf[2],320,300,128,32,surface,1);
	}
	
	return 1;
}
void drawScoreBoard()
{
	char buf[255];
	sprintf(buf,"SCORE:%d",stats.score);
    Draw_Text_GDI(buf,20,160,RGB(170,160,200),surface);
	sprintf(buf,"Lives:%d",stats.lives);
	Draw_Text_GDI(buf,20,170,RGB(170,160,200),surface);
	sprintf(buf,"blox:%d",stats.blocks);
	Draw_Text_GDI(buf,20,180,RGB(170,160,200),surface);
	sprintf(buf,"lines:%d",stats.lines);
	Draw_Text_GDI(buf,20,190,RGB(170,160,200),surface);
	sprintf(buf,"level:%d",stats.level);
	Draw_Text_GDI(buf,20,200,RGB(170,160,200),surface);
}
int drawMenu()
{
	
	if(mousey>100&&mousey<132)
	{
          DDraw_Draw_Surface(menuItems[1],192,100,256,32,surface,1);
		if(mousedown)
		{
			mousedown=0;
			titrisInit();
			resetStats();
			
			setState(PLAYING);
			
		}
	}
	else
	{
          DDraw_Draw_Surface(menuItems[0],192,100,256,32,surface,1);
	}

	if(mousey>132&&mousey<164)
	{
          DDraw_Draw_Surface(menuItems[3],192,132,256,32,surface,1);
		if(mousedown)
		{
			mousedown=0;
			setState(HIGHSCORES);
			return 1;
			
		}
	}
	else
	{
          DDraw_Draw_Surface(menuItems[2],192,132,256,32,surface,1);
	}

	if(mousey>164&&mousey<192)
	{
          DDraw_Draw_Surface(menuItems[5],192,164,256,32,surface,1);
          if(mousedown)
          {
            exit(1);
          }
	}
	else
	{
          DDraw_Draw_Surface(menuItems[4],192,164,256,32,surface,1);
	}

	if(mousedown)
	{
		
		mousedown=0;
		//setState(PLAYING);
	}
	return 1;
}
int drawNextTitri()
{
	BLOCK ** aBlock=titris[focal.next].blocks;
	int i=0;
	while(i<4)
	{
		int x=aBlock[i]->x*16+408;
		int y=aBlock[i]->y*16+122;
		DDraw_Draw_Surface(blockSurf[aBlock[i]->blockImgIndex],x,y,16,16,surface,0);
		i++;
	}
	return 1;
}
int Game_Main()
{
	timer.start();
	switch (state)
	{
	case GAMESTART:
	{
		titrisInit();
		for(int it=0;it<100;it++)
		{
			drawBackground();
			///while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
		}
		setState(GAMEMENU);

		//state=GAMEMENU;
		//loadImageBackground("menu.bmp");
		resetStats();
		
	}
	break;

	case HIGHSCORES:
	{
		drawBackground();
		drawHighScoreBoard();
		if(setScore)
			insertingNewScore();
		//while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
	}
	break;
	case GAMEEND:
	{	
		int i;
		for(i=0;i<HIGHSCOREENTRIES;i++)
			if(stats.score > highScores[i].score)
				break;
		if(i!=HIGHSCOREENTRIES)
			setScore=true;
		saveHighScores();
		titrisInit();
		//resetStats();
		setState(HIGHSCORES);
		
	}
	break;

	case PLAYERDIED:
	{
		drawBackground();
		fillSurfRow();
		stats.lives--;
		if(stats.lives>0)
		{
			titrisInit();
			setState(PLAYING);
		
		}else setState(GAMEEND);
	}
	break;
	case GAMEMENU:
		
		drawBackground(1);
		drawMenu();
		//while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
		break;
	case LEVELSTART:
		titrisInit();
		Sleep(2000);
		state=PLAYING;
	break;
	case LEVELEND:
	break;
	case PLAYING:

          drawBackground();
		drawScoreBoard();
		/*Update game state will only fail when the player dies*/
		if(updateGameState())
		{
			drawGameSurf();
			drawNextTitri();
			//while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
				
		}
	break;
	case PAUSED:
	break;
	}
	timer.wait(20);	
	return 1;
}

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
	fclose(debuginfo);
	return(1);

}

int HandleInput();

int main(int ac, char* av[]) {
  // initialize game here
  Game_Init();

  // enter main event loop
  while (true) {
    surface = SDL_GetWindowSurface(window);

    HandleInput();

    Game_Main();

    SDL_UpdateWindowSurface(window);
  }

  Game_Shutdown();

  return 0;
}

int HandleKeyPress(SDL_Keysym keysym) {
  int wparam = keysym.sym;
  //User is typing in his/her name, so just fill global buffer
  if(state==HIGHSCORES&&setScore)
  {
    //Backspace
    if(wparam==8)
    {
      if(gNumChars>=1)
        gNumChars--;
    }
    else
      gBuffer[gNumChars++]=wparam;
  }
  if(wparam=='p'||wparam=='P')
  {
    predict=!predict;
    AIfindDestination(&titris[focal.cur]);
  }

  if(wparam==27)
  {
    setState(GAMEMENU);
  }
  return 0;
}

int HandleInput() {
  int wparam = 0;
  SDL_Event event;
  while (SDL_PollEvent(&event))
    switch (event.type) {
    case SDL_MOUSEMOTION:
      mousex = event.motion.x;
      mousey = event.motion.y;
      break;
    case SDL_MOUSEBUTTONDOWN:
      mousedown = 1;
      break;
    case SDL_MOUSEBUTTONUP:
      mousedown = 0;
      break;
    case SDL_QUIT:
      exit(0);
      break;

    case SDL_KEYDOWN: {
      SDL_Keysym keysym = event.key.keysym;
      if (!keys[keysym.sym]) {
        HandleKeyPress(keysym);
      }
      keys[keysym.sym] = true;
    } break;
    case SDL_KEYUP: {
      SDL_Keysym keysym = event.key.keysym;
      keys[keysym.sym] = false;
    } break;
    }

  return 0;
}

