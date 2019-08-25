#ifndef _DEFS_H_
#define _DEFS_H_

// Target Windows 10

#define UNICODE
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#define _CRT_RAND_S
#include <windows.h>
#include <uxtheme.h>
#include "resource.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <strsafe.h>

#define LOCAL static

// Includes terminating null character.
#define MAX_STRING 256

#define ROWS 20
#define COLS 10

typedef enum {
    BLOCKO,
    BLOCKI,
    BLOCKS,
    BLOCKZ,
    BLOCKL,
    BLOCKJ,
    BLOCKT
} BLOCKTYPE;

typedef enum {
    GAMESTATE_PLAY,
    GAMESTATE_ELIMINATEFULLROWS
} GAMESTATE;

typedef enum {
    COLORTABLE_MONOCHROME,
    COLORTABLE_BICOLOR,
    COLORTABLE_MULTICOLOR
} COLORTABLEENTRIES;

typedef struct {
    WCHAR szName[MAX_STRING];
    int nLevel;
    int nScore;
} HIGHSCORE;

extern HINSTANCE g_hInstance;
extern HWND g_hwndMain;
extern HIGHSCORE g_hsHighScores[10];
extern WCHAR g_szNewHighScoreName[MAX_STRING];
extern const WORD *g_pBlocks[7];
extern const int g_pFrameCounts[7];
extern int g_nBlockStatistics[7];

void ShowNewHighScoreDialog (void);
void ShowHighScoresDialog (void);
void ShowStatisticsDialog (void);
void ShowAboutDialog (void);

int RandInt (int iMin, int iMax);

#endif /* _DEFS_H_ */



