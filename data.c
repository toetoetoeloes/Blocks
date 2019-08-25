#include <windows.h>

// Block frames are encoded as 16-bit unsigned integers representing 4x4 arrays with
// the MSB at [0,0] and the LSB at [3,3].

/*
	0000
	0110
	0110
	0000
*/

static const WORD wBlockO[] = { 0x0660 };

/*
	0000 0010
	1111 0010
	0000 0010
	0000 0010
*/

static const WORD wBlockI[] = { 0x0f00, 0x2222 };

/*
	0000 0010
	0011 0011
	0110 0001
	0000 0000
*/

static const WORD wBlockS[] = { 0x0360, 0x2310 };

/*
	0000 0001
	0110 0011
	0011 0010
	0000 0000
*/

static const WORD wBlockZ[] = { 0x0630, 0x2640 };

/*
	0000 0010 0001 0110
	0111 0010 0111 0010
	0100 0011 0000 0010
	0000 0000 0000 0000
*/

static const WORD wBlockL[] = { 0x0740, 0x2230, 0x1700, 0x6220 };

/*
	0000 0011 0100 0010
	0111 0010 0111 0010
	0001 0010 0000 0110
	0000 0000 0000 0000
*/

static const WORD wBlockJ[] = { 0x0710, 0x3220, 0x4700, 0x2260 };

/*
	0000 0010 0010 0010
	0111 0011 0111 0110
	0010 0010 0000 0010
	0000 0000 0000 0000
*/

static const WORD wBlockT[] = { 0x0720, 0x2320, 0x2700, 0x2620 };


// g_pBlocks can be indexed with the BLOCKTYPE enumerator.

const WORD *g_pBlocks[7] = {
	wBlockO,
	wBlockI,
	wBlockS,
	wBlockZ,
	wBlockL,
	wBlockJ,
	wBlockT
};


// g_pFrameCounts can be indexed with the BLOCKTYPE enumerator.

const int g_pFrameCounts[7] = { 1, 2, 2, 2, 4, 4, 4 };

