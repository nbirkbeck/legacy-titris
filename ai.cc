#include "ai.h"
#include "titris.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

extern FOCAL focal;
extern COLINFO colinfo;
extern TITRI titris[MAX_TITRI];
extern BLOCK blocks[MAX_BLOCKS];
extern BLOCK_PTR gameSurf[ROWS][COLS];
TITRI* cur;
int surfOutline[COLS];
int gFlags;
FILE* debug = fopen("debugAI.txt", "w");
#define mAx(a, b) ((a) > (b) ? a : b)

BLOCK destBlocks[4];
/*
        getSurfOutline:
        Takes a pointer to an array, that is already allocated, and
        fills it up with the outline of the blox on the surface.  So
        the index is the column number, and the value is the depth
*/
void AIgetSurfOutline(int* array) {
  int i = 0;
  int j = 0;
  for (j = 0; j < COLS; j++) {
    i = 0;
    while (i < ROWS) {
      if (gameSurf[i][j] != NULL)
        break;
      i++;
    }

    array[j] = i;
  }
}
void AIdoMove() {

  if (cur == NULL)
    return;
  /*
  if(cur->blocks[1]->y+4<=destBlocks[1].y)
          moveTitri(cur,M_DOWN);
  else if(cur->blocks[2]->y+4<=destBlocks[2].y)
          moveTitri(cur,M_DOWN);
  */
  switch (cur->titType) {
  case TLINE: {
    int leftmostI = 0;
    if (destBlocks[0].y == destBlocks[1].y) {
      if (cur->blocks[0]->y != cur->blocks[1]->y)
        rotateTitri(cur, ROTCC);
    }

    for (int i = 1; i < 4; i++) {
      if (cur->blocks[i]->x < cur->blocks[leftmostI]->x)
        leftmostI = i;
    }
    // Move right one
    if (cur->blocks[leftmostI]->x < destBlocks[0].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[leftmostI]->x > destBlocks[0].x)
      moveTitri(cur, M_LEFT);
    else
      ;
  } break;
  case TSQUARE: {
    int leftmostI = 0;
    for (int i = 1; i < 4; i++) {
      if (cur->blocks[i]->x < cur->blocks[leftmostI]->x)
        leftmostI = i;
    }
    // Move right one
    if (cur->blocks[leftmostI]->x < destBlocks[0].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[leftmostI]->x > destBlocks[0].x)
      moveTitri(cur, M_LEFT);
    else
      ;
  } break;
  case TREVL: {
    switch (gFlags) {
    case FACEUP:
      break;
    case FACERIGHT:
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
      break;
    case FACEDOWN:
      rotateTitri(cur, ROTC);
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
      break;
    case FACELEFT:
      rotateTitri(cur, ROTCC);
      gFlags = DONOTHING;
      break;
    default:
      break;
    }
    if (cur->blocks[2]->x < destBlocks[2].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[2]->x > destBlocks[2].x)
      moveTitri(cur, M_LEFT);
  } break;

  case TNORML:
  case TT: {
    switch (gFlags) {
    case FACEUP:
      break;
    case FACERIGHT:
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
      break;
    case FACEDOWN:
      rotateTitri(cur, ROTC);
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
      break;
    case FACELEFT:
      rotateTitri(cur, ROTCC);
      gFlags = DONOTHING;
      break;
    default:
      break;
    }
    if (cur->blocks[1]->x < destBlocks[1].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[1]->x > destBlocks[1].x)
      moveTitri(cur, M_LEFT);
  } break;
  case TUPSTEP:
    switch (gFlags) {
    case FACEUP:
      gFlags = DONOTHING;
      break;
    case FACERIGHT:
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
    default:
      break;
    }
    if (cur->blocks[1]->x < destBlocks[1].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[1]->x > destBlocks[1].x)
      moveTitri(cur, M_LEFT);
    break;
  case TDNSTEP:
    switch (gFlags) {
    case FACEUP:
      gFlags = DONOTHING;
      break;
    case FACERIGHT:
      rotateTitri(cur, ROTC);
      gFlags = DONOTHING;
    default:
      break;
    }
    if (cur->blocks[2]->x < destBlocks[2].x)
      moveTitri(cur, M_RIGHT);
    else if (cur->blocks[2]->x > destBlocks[2].x)
      moveTitri(cur, M_LEFT);
    break;
  default:
    break;
  }
}

/*
        AIfindDestination(TITRI *titri)
        Should be called whenever a new titri is about to be dropped.
        This will set up the state of the dest object so that whenever the
        AIdoMove() function is called, the info will be valid
*/
void AIfindDestination(TITRI* titri) {
  gFlags = DONOTHING;

  // Initialize the AI variables
  int deepFR = -1;
  int deepFD = -1;
  int deepFU = -1;
  int deepFL = -1;
  int j = 0;

  cur = titri;

  for (int ii = 0; ii < 4; ii++) {
    memset(&destBlocks[ii], 0, sizeof(BLOCK));
    ;
  }

  AIgetSurfOutline(surfOutline);

  switch (cur->titType) {
  case TLINE:
    /*
    {
            OLD CODE THAT ALWAYS PREFER HORIZONTAL TO VERTICAL
    int flag=-1;
    int j=0;
    int y=0;
    while(j<(COLS-3))
    {
    
            if((surfOutline[j]==surfOutline[j+1])&&(surfOutline[j+1]==surfOutline[j+2])&&(surfOutline[j+2]==surfOutline[j+3]))
            {
                    if(flag=-1)
                    {
                            flag=j;
                            y=surfOutline[j];
                    }
                    else if(y<surfOutline[j])
                    {
                            flag=j;
                            y=surfOutline[j];
                    }
            }
            j++;
    }
    //Can we lay it down horizontally?
    if(flag!=-1)
    {
            FILE * out = fopen("ai.txt","w");
            fprintf(out,"Yes its horizontal");
            fclose(out);
            destBlocks[0].x=flag;
            destBlocks[1].x=flag+1;
            destBlocks[2].x=flag+2;
            destBlocks[3].x=flag+3;
            destBlocks[0].y=y;
            destBlocks[1].y=y;
            destBlocks[2].y=y;
            destBlocks[3].y=y;
    }
    else
    {
            int deepestI=0;
            j=0;
            while(j<COLS)
            {
                    if(surfOutline[j]>surfOutline[deepestI])
                            deepestI=j;
                    j++;
            }
            destBlocks[0].x=deepestI;
            destBlocks[1].x=deepestI;
            destBlocks[2].x=deepestI;
            destBlocks[3].x=deepestI;
            destBlocks[0].y=surfOutline[deepestI]-3;
            destBlocks[1].y=surfOutline[deepestI]-2;
            destBlocks[2].y=surfOutline[deepestI]-1;
            destBlocks[3].y=surfOutline[deepestI];
    }
}
break;
*/
    {
      j = 0;
      while (j < (COLS - 3)) {

        if ((surfOutline[j] == surfOutline[j + 1]) &&
            (surfOutline[j + 1] == surfOutline[j + 2]) &&
            (surfOutline[j + 2] == surfOutline[j + 3])) {
          if (deepFR == -1)
            deepFR = j;

          else if (surfOutline[deepFR] < surfOutline[j])
            deepFR = j;
        }
        j++;
      }
      j = 0;
      while (j < COLS) {
        if (surfOutline[j] > surfOutline[deepFU])
          deepFU = j;
        j++;
      }
      // set it vertical by default
      gFlags = FACEUP;
      destBlocks[0].x = deepFU;
      destBlocks[1].x = deepFU;
      destBlocks[2].x = deepFU;
      destBlocks[3].x = deepFU;
      destBlocks[0].y = surfOutline[deepFU] - 3;
      destBlocks[1].y = surfOutline[deepFU] - 2;
      destBlocks[2].y = surfOutline[deepFU] - 1;
      destBlocks[3].y = surfOutline[deepFU];
      if (deepFR != -1) {
        // Choose the horizontal only when it is at most 3 less
        if (surfOutline[deepFR] + 3 > surfOutline[deepFU]) {
          destBlocks[0].y = surfOutline[deepFR];
          destBlocks[1].y = surfOutline[deepFR];
          destBlocks[2].y = surfOutline[deepFR];
          destBlocks[3].y = surfOutline[deepFR];
          destBlocks[0].x = deepFR;
          destBlocks[1].x = deepFR + 1;
          destBlocks[2].x = deepFR + 2;
          destBlocks[3].x = deepFR + 3;
        }
      }
    }
    break;

  case TSQUARE: {

    int deepest = -1;
    int j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] == surfOutline[j + 1]) {
        if (deepest != -1) {
          if (surfOutline[j] > surfOutline[deepest])
            deepest = j;

        } else
          deepest = j;
      }
      j++;
    }

    // ere are no two heights the same

    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] + 1 == surfOutline[j + 1]) {
        if (deepFR == -1)
          deepFR = j;
        else if (surfOutline[deepFR] < surfOutline[j])
          deepFR = j;
      } else if (surfOutline[j] == surfOutline[j + 1] + 1) {
        if (deepFR == -1)
          deepFR = j;
        else if (surfOutline[deepFR] < surfOutline[j])
          deepFR = j;
      }
      j++;
    }

    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] < surfOutline[j + 1]) {
        if (deepFD == -1) {
          deepFD = j;
        } else if (surfOutline[deepFD] < surfOutline[j])
          deepFD = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] > surfOutline[j + 1]) {
        if (deepFL == -1) {
          deepFL = j + 1;
        } else if (surfOutline[deepFL] < surfOutline[j + 1])
          deepFL = j + 1;
      }
      j++;
    }
    /*
            will steal a tetris spot here
    */
    if (deepest == -1 && deepFR == -1) {
      if (deepFL == -1)
        deepest = deepFD;

      else if (deepFD == -1)
        deepest = deepFL - 1;
    }
    if (deepest == -1)
      deepest = deepFR;
    else if (surfOutline[deepest] + 5 <= surfOutline[deepFR])
      deepest = deepFR;
    else if (surfOutline[deepest] + 6 <= surfOutline[deepFL])
      deepest = deepFL - 1;
    else if (surfOutline[deepest] + 6 <= surfOutline[deepFD])
      deepest = deepFD;

    gFlags = FACEUP;
    destBlocks[0].x = deepest;
    destBlocks[0].y = surfOutline[deepest];
    destBlocks[1].x = deepest + 1;
    destBlocks[1].y = surfOutline[deepest];
    destBlocks[2].x = deepest;
    destBlocks[2].y = destBlocks[0].y - 1;
    destBlocks[3].x = deepest + 1;
    destBlocks[3].y = destBlocks[2].y;

  } break;
  case TT: {
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2])) {
        if (deepFU == -1)
          deepFU = j;
        else if (surfOutline[deepFU] < surfOutline[j])
          deepFU = j;
      }
      j++;
    }
    j = 0;
    // Look for face down case
    while (j < (COLS - 1)) {
      if ((surfOutline[j] + 1 == surfOutline[j + 1]) &&
          (surfOutline[j] == surfOutline[j + 2])) {
        if (deepFD == -1)
          deepFD = j + 1;
        else if (surfOutline[j + 1] > surfOutline[deepFD])
          deepFD = j + 1;
      }
      j++;
    }
    // Look for face left
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] + 1 == surfOutline[j + 1]) {
        if (deepFL == -1) {
          deepFL = j + 1;
        } else {
          if (surfOutline[j + 1] > surfOutline[deepFL])
            deepFL = j + 1;
        }
      }
      j++;
    }

    // look for face right
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] - 1 == surfOutline[j + 1]) {
        if (deepFR == -1)
          deepFR = j;
        else if (surfOutline[j] > surfOutline[deepFR])
          deepFR = j;
      }
      j++;
    }
    int max1, max2;
    int themax;
    max1 = mAx(surfOutline[deepFU], surfOutline[deepFD]);
    max2 = mAx(surfOutline[deepFR], surfOutline[deepFL]);
    themax = mAx(max1, max2);

    FILE* file = fopen("debugTT.txt", "w");
    fprintf(file, "deepFU%d,deepFD%d,deepFR%d,deepFL%d\n", deepFU, deepFD,
            deepFR, deepFL);
    fclose(file);
    if (themax == surfOutline[deepFU]) {
      gFlags = FACEUP;
      destBlocks[1].x = deepFU + 1;
      destBlocks[1].y = surfOutline[deepFU];
    } else if (themax == surfOutline[deepFD]) {
      gFlags = FACEDOWN;
      destBlocks[1].x = deepFD;
      destBlocks[1].y = surfOutline[deepFD] - 1;
    }

    else if (themax == surfOutline[deepFR]) {
      gFlags = FACERIGHT;
      destBlocks[1].x = deepFR;
      destBlocks[1].y = surfOutline[deepFR] - 1;
    }

    else if (themax == surfOutline[deepFL]) {
      gFlags = FACELEFT;
      destBlocks[1].x = deepFL;
      destBlocks[1].y = surfOutline[deepFL] - 1;
    }
  } break;
  case TNORML: {
    int deepFLS = -1;
    int deepFRS = -1;
    int max1 = -1;
    int max2 = -1;
    int themax = -1;
    j = 0;

    while (j < (COLS - 2)) {
      if ((surfOutline[j] == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2])) {
        if (deepFU == -1)
          deepFU = j;
        else if (surfOutline[deepFU] < surfOutline[j])
          deepFU = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] - 1 == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2])) {
        if (deepFD == -1)
          deepFD = j;
        else if (surfOutline[deepFD] < surfOutline[j])
          deepFD = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] == surfOutline[j + 1]) {
        if (deepFR == -1)
          deepFR = j;
        else if (surfOutline[deepFR] < surfOutline[j])
          deepFR = j;
      }
      if (surfOutline[j] > surfOutline[j + 1]) {
        if (deepFRS == -1)
          deepFRS = j;
        else if (surfOutline[deepFRS] < surfOutline[j])
          deepFRS = j;
      }
      if (surfOutline[j] == surfOutline[j + 1] - 2) {
        if (deepFL == -1) {
          deepFL = j + 1;
        } else if (surfOutline[deepFL] < surfOutline[j + 1]) {
          deepFL = j + 1;
        }
      }
      if (surfOutline[j] < surfOutline[j + 1] - 2) {
        if (deepFLS == -1)
          deepFLS = j;
        else if (surfOutline[deepFLS] < surfOutline[j])
          deepFLS = j;
      }
      j++;
    }
    j = 0;

    if (deepFU != -1 && deepFD != -1)
      max1 = mAx(surfOutline[deepFU], surfOutline[deepFD]);
    else if (deepFR == -1 && deepFL == -1)
      max1 = -1;
    else if (deepFU == -1)
      max1 = surfOutline[deepFD];
    else
      max1 = surfOutline[deepFU];

    if (deepFR != -1 && deepFL != -1)
      max2 = mAx(surfOutline[deepFR], surfOutline[deepFL]);
    else if (deepFR == -1 && deepFL == -1)
      max2 = -1;
    else if (deepFR == -1)
      max2 = surfOutline[deepFL];
    else
      max2 = surfOutline[deepFR];

    themax = mAx(max1, max2);

    if (themax == surfOutline[deepFU]) {
      gFlags = FACEUP;
      destBlocks[1].x = deepFU + 1;
      destBlocks[1].y = surfOutline[deepFU];
    } else if (themax == surfOutline[deepFD]) {
      gFlags = FACEDOWN;
      destBlocks[1].x = deepFD + 1;
      destBlocks[1].y = surfOutline[deepFD] - 1;
    }

    else if (themax == surfOutline[deepFR]) {
      gFlags = FACERIGHT;
      destBlocks[1].x = deepFR;
      destBlocks[1].y = surfOutline[deepFR] - 1;
    }

    else if (themax == surfOutline[deepFL]) {
      gFlags = FACELEFT;
      destBlocks[1].x = deepFL;
      destBlocks[1].y = surfOutline[deepFL] - 2;
    }

    //	In this case, all the perfect fits dont exist, must find
    //	someplace else for the titri

    else if (themax == -1) {
      j = 0;
      deepFR = -1;
      while (j < (COLS - 1)) {
        if (surfOutline[j] + 1 == surfOutline[j + 1]) {
          if (deepFR == -1)
            deepFR = j;
          else if (surfOutline[deepFR] < surfOutline[j])
            deepFR = j;
        } else if (surfOutline[j] == surfOutline[j + 1] + 1) {
          if (deepFR == -1)
            deepFR = j;
          else if (surfOutline[deepFR] < surfOutline[j])
            deepFR = j;
        }
        j++;
      }
      if (deepFR != -1) {
        gFlags = FACERIGHT;
        destBlocks[1].x = deepFR;
        destBlocks[1].y = surfOutline[deepFR] - 1;
      }
      // else No where good to put block. and code here
    }
    /*try to avoid towwering up, not working
    if(themax!=-1)
    {
            if(deepFLS!=-1)
            {
                    if((themax+5)<surfOutline[deepFLS])
                    {
                            gFlags=FACELEFT;
                            destBlocks[1].x=deepFLS+1;
                            destBlocks[1].y=surfOutline[deepFLS]+1;
                    }
                    
            }
            else if(deepFRS!=-1)
            {
                    if(themax+5<surfOutline[deepFRS])
                    {
                            gFlags=FACERIGHT;
                            destBlocks[1].x=deepFRS;
                            destBlocks[1].y=surfOutline[deepFRS]-1;
                    }
            }
    }*/
  } break;
  case TREVL: {
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2])) {
        if (deepFU == -1)
          deepFU = j;
        else if (surfOutline[deepFU] < surfOutline[j])
          deepFU = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2] - 1)) {
        if (deepFD == -1)
          deepFD = j + 2;
        else if (surfOutline[deepFD] < surfOutline[j + 2])
          deepFD = j + 2;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] == surfOutline[j + 1]) {
        if (deepFL == -1)
          deepFL = j;
        else if (surfOutline[deepFL] < surfOutline[j])
          deepFL = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if (surfOutline[j] == surfOutline[j + 1] + 2) {
        if (deepFR == -1) {
          deepFR = j;
        } else if (surfOutline[deepFR] < surfOutline[j]) {
          deepFR = j;
        }
      }
      j++;
    }
    int max1 = -1;
    int max2 = -1;
    int themax;
    if (deepFU != -1 && deepFD != -1)
      max1 = mAx(surfOutline[deepFU], surfOutline[deepFD]);
    else if (deepFR == -1 && deepFL == -1)
      max1 = -1;
    else if (deepFU == -1)
      max1 = surfOutline[deepFD];
    else
      max1 = surfOutline[deepFU];

    if (deepFR != -1 && deepFL != -1)
      max2 = mAx(surfOutline[deepFR], surfOutline[deepFL]);
    else if (deepFR == -1 && deepFL == -1)
      max2 = -1;
    else if (deepFR == -1)
      max2 = surfOutline[deepFL];
    else
      max2 = surfOutline[deepFR];
    themax = mAx(max1, max2);

    if (themax == surfOutline[deepFU]) {
      gFlags = FACEUP;
      destBlocks[2].x = deepFU + 1;
      destBlocks[2].y = surfOutline[deepFU];
    } else if (themax == surfOutline[deepFD]) {
      gFlags = FACEDOWN;
      destBlocks[2].x = deepFD - 1;
      destBlocks[2].y = surfOutline[deepFD] - 1;
    }

    else if (themax == surfOutline[deepFR]) {
      gFlags = FACERIGHT;
      destBlocks[2].x = deepFR;
      destBlocks[2].y = surfOutline[deepFR] - 1;
    }

    else if (themax == surfOutline[deepFL]) {
      gFlags = FACELEFT;
      destBlocks[2].x = deepFL + 1;
      destBlocks[2].y = surfOutline[deepFL] - 1;

    } else if (themax == -1) {
      j = 0;
      deepFL = -1;
      while (j < (COLS - 1)) {
        if (surfOutline[j] + 1 == surfOutline[j + 1]) {
          if (deepFL == -1)
            deepFL = j;
          else if (surfOutline[deepFL] < surfOutline[j])
            deepFL = j;
        } else if (surfOutline[j] == surfOutline[j + 1] + 1) {
          if (deepFL == -1)
            deepFL = j;
          else if (surfOutline[deepFL] < surfOutline[j])
            deepFL = j;
        }
        j++;
      }
      if (deepFL != -1) {
        gFlags = FACELEFT;
        destBlocks[2].x = deepFL + 1;
        destBlocks[2].y = surfOutline[deepFL] - 1;
      }
      // else add code here, nowhere good for block
    }
  } break;
  case TUPSTEP: {
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == (surfOutline[j + 2] + 1))) {
        if (deepFU == -1)
          deepFU = j;
        else if (surfOutline[deepFU] < surfOutline[j])
          deepFU = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if ((surfOutline[j] + 1 == surfOutline[j + 1])) {
        if (deepFR == -1)
          deepFR = j + 1;
        else if (surfOutline[deepFR] < surfOutline[j + 1])
          deepFR = j + 1;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if ((surfOutline[j] == surfOutline[j + 1])) {
        if (deepFD == -1)
          deepFD = j;
        else if (surfOutline[deepFD] < surfOutline[j])
          deepFD = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if ((surfOutline[j] < surfOutline[j + 1])) {
        if (deepFL == -1)
          deepFL = j;
        else if (surfOutline[deepFL] < surfOutline[j])
          deepFL = j;
      } else if ((surfOutline[j] > surfOutline[j + 1])) {
        if (deepFL == -1)
          deepFL = j;
        else if (surfOutline[deepFL] < surfOutline[j])
          deepFL = j;
      }
      j++;
    }

    if (deepFU != -1 && deepFR != -1) {
      int maX = mAx(surfOutline[deepFU], surfOutline[deepFR]);
      if (maX == surfOutline[deepFU]) {
        gFlags = FACEUP;
        destBlocks[1].x = deepFU + 1;
        destBlocks[1].y = surfOutline[deepFU];
      } else {
        gFlags = FACERIGHT;
        destBlocks[1].x = deepFR - 1;
        destBlocks[1].y = surfOutline[deepFR] - 1;
      }
      // This avoids a towering up in one spot
      if (deepFD != -1)
        if (maX + 3 <= surfOutline[deepFD]) {
          gFlags = FACERIGHT;
          destBlocks[1].x = deepFD;
          destBlocks[1].y = surfOutline[deepFD] - 1;
        }
      if (deepFL != -1)
        if (maX + 6 <= surfOutline[deepFU]) {
          gFlags = FACERIGHT;
          destBlocks[1].x = deepFL;
          destBlocks[1].y = surfOutline[deepFL];
        }
    } else if ((deepFR == -1) && (deepFU == -1)) {
      if (deepFD != -1) {
        gFlags = FACERIGHT;
        destBlocks[1].x = deepFD;
        destBlocks[1].y = surfOutline[deepFD] - 1;
      }
      // else no two spot either..shit
      else {
        j = 0;
        while (j < (COLS - 1)) {
          if ((surfOutline[j] + 1 > surfOutline[j + 1])) {
            if (deepFR == -1)
              deepFR = j;
            else if (surfOutline[deepFR] < surfOutline[j])
              deepFR = j;
          }
          j++;
        }
        if (deepFR != -1) {
          gFlags = FACERIGHT;
          destBlocks[1].x = deepFR;
          destBlocks[1].y = surfOutline[deepFR + 1];
        } else {
          FILE* file = fopen("upstep.txt", "w");
          fprintf(file, "Upstep dont know where to go\n");
          fclose(file);
        }
      }
    } else if (deepFU == -1) {
      gFlags = FACERIGHT;
      destBlocks[1].x = deepFR - 1;
      destBlocks[1].y = surfOutline[deepFR] - 1;
      if (deepFD != -1)
        if (surfOutline[deepFR] + 3 <= surfOutline[deepFD]) {
          destBlocks[1].x = deepFD;
          destBlocks[1].y = surfOutline[deepFD] - 1;
        }
      if (deepFL != -1)
        if (surfOutline[deepFR] + 6 <= surfOutline[deepFU]) {
          gFlags = FACERIGHT;
          destBlocks[1].x = deepFL;
          destBlocks[1].y = surfOutline[deepFL];
        }
    } else {
      gFlags = FACEUP;
      destBlocks[1].x = deepFU + 1;
      destBlocks[1].y = surfOutline[deepFU];
    }
  } break;
  case TDNSTEP: {
    j = 0;
    while (j < (COLS - 2)) {
      if ((surfOutline[j] + 1 == surfOutline[j + 1]) &&
          (surfOutline[j + 1] == surfOutline[j + 2])) {
        if (deepFU == -1)
          deepFU = j;
        else if (surfOutline[deepFU] < surfOutline[j])
          deepFU = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if ((surfOutline[j] == surfOutline[j + 1] + 1)) {
        if (deepFR == -1)
          deepFR = j;
        else if (surfOutline[deepFR] < surfOutline[j])
          deepFR = j;
      }
      j++;
    }
    j = 0;
    while (j < (COLS - 1)) {
      if ((surfOutline[j] == surfOutline[j + 1])) {
        if (deepFD == -1)
          deepFD = j;
        else if (surfOutline[deepFD] < surfOutline[j])
          deepFD = j;
      }
      j++;
    }
    if (deepFU != -1 && deepFR != -1) {
      int maX = mAx(surfOutline[deepFU], surfOutline[deepFR]);
      if (maX == surfOutline[deepFU]) {
        gFlags = FACEUP;
        destBlocks[2].x = deepFU + 1;
        destBlocks[2].y = surfOutline[deepFU];
      } else {
        gFlags = FACERIGHT;
        destBlocks[2].x = deepFR;
        destBlocks[2].y = surfOutline[deepFR] - 1;
      }
      // Try to avoid towering up of the blox
      if (deepFD != -1)
        if (maX + 3 <= surfOutline[deepFD]) {
          gFlags = FACERIGHT;
          destBlocks[2].x = deepFD;
          destBlocks[2].y = surfOutline[deepFD];
        }

    } else if (deepFU == -1 && deepFR == -1) {
      if (deepFD != -1) {
        gFlags = FACERIGHT;
        destBlocks[2].x = deepFD;
        destBlocks[2].y = surfOutline[deepFD];
      } else {
        j = 0;
        while (j < (COLS - 1)) {
          if ((surfOutline[j] < surfOutline[j + 1] + 1)) {
            if (deepFR == -1)
              deepFR = j;
            else if (surfOutline[deepFR] < surfOutline[j])
              deepFR = j;
          }
          j++;
        }
        if (deepFR != -1) {
          gFlags = FACERIGHT;
          destBlocks[2].x = deepFR;
          destBlocks[2].y = surfOutline[deepFR + 1];
        }
      }
    } else if (deepFU == -1) {
      gFlags = FACERIGHT;
      destBlocks[2].x = deepFR;
      destBlocks[2].y = surfOutline[deepFR] - 1;
      // Try to avoid towering up of the blox
      if (deepFD != -1)
        if (surfOutline[deepFR] + 3 <= surfOutline[deepFD]) {
          FILE* file = fopen("exists.txt", "w");
          fprintf(file, "yes, its in\n");
          fclose(file);
          destBlocks[2].x = deepFD;
          destBlocks[2].y = surfOutline[deepFD];
        }

    } else // deepFR==-1
    {
      gFlags = FACEUP;
      destBlocks[2].x = deepFU + 1;
      destBlocks[2].y = surfOutline[deepFU];
    }
  }
    if (gFlags == DONOTHING)
      fprintf(debug, "donothing, TTYPE%d\n", cur->titType);
  default:
    break;
  }
}
