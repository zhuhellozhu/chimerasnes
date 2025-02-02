#include "chisnes.h"
#include "memmap.h"
#include "ppu.h"
#include "dsp.h"
#include "cpuexec.h"
#include "apu.h"
#include "dma.h"
#include "fxemu.h"
#include "fxinst.h"
#include "gfx.h"
#include "soundux.h"
#include "cheats.h"
#include "sa1.h"
#include "snesapu.h"
#include "spc7110.h"
#include "obc1.h"
#include "srtc.h"
#include "bsx.h"

MainLoopPtr MainLoop;
SCPUState   CPU;
SICPU       ICPU;

SAPU      APU;
SIAPU     IAPU;
SEXTState EXT;

SSettings Settings;
SDSP0     DSP0;
SDSP1     DSP1;
SDSP2     DSP2;
SDSP3     DSP3;
SDSP4     DSP4;
SSA1      SA1;
SRTCData  RTCData;
SOBC1     OBC1;
FXRegs_s  FXRegs;
FXInfo_s  SuperFX;
SBSX      BSX;

void    (*SetSETA)(uint32_t, uint8_t);
uint8_t (*GetSETA)(uint32_t);

SnesModel  M1SNES = {1, 3, 2};
SnesModel  M2SNES = {2, 4, 3};
SnesModel* Model = &M1SNES;

CMemory Memory;

SPPU PPU;
InternalPPU IPPU;

SDMA DMA[8];

uint8_t* HDMAMemPointers[8];
uint8_t* HDMABasePointers[8];

SBG BG;
SGFX GFX;
SLineData LineData[240];
SLineMatrixData LineMatrixData[240];

uint8_t Mode7Depths[2];

NormalTileRenderer  DrawTilePtr = NULL;
ClippedTileRenderer DrawClippedTilePtr = NULL;
NormalTileRenderer  DrawHiResTilePtr = NULL;
ClippedTileRenderer DrawHiResClippedTilePtr = NULL;
LargePixelRenderer  DrawLargePixelPtr = NULL;

uint32_t odd_high[4][16];
uint32_t odd_low[4][16];
uint32_t even_high[4][16];
uint32_t even_low[4][16];

bool finishedFrame = false;

int8_t   FilterTaps[8];
int16_t  Loop[FIRBUF];
uint16_t SignExtend[2] = {0x0000, 0xff00};

int32_t HDMA_ModeByteCounts[8] = {1, 2, 2, 4, 4, 4, 2, 4};

uint8_t brightness_cap[64];

uint8_t mul_brightness[16][32] =
{{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}, {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
}, {
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04
}, {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06
}, {
	0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
	0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08
}, {
	0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05,
	0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a
}, {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
	0x06, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c
}, {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
	0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e
}, {
	0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08,
	0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11
}, {
	0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09,
	0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x11, 0x12, 0x13
}, {
	0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x0a,
	0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15
}, {
	0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b,
	0x0c, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17
}, {
	0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c,
	0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x16, 0x16, 0x17, 0x18, 0x19
}, {
	0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d,
	0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1a, 0x1b
}, {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
}, {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
}};

uint8_t BitShifts[8][4] =
{
	{2, 2, 2, 2}, /* 0 */
	{4, 4, 2, 0}, /* 1 */
	{4, 4, 0, 0}, /* 2 */
	{8, 4, 0, 0}, /* 3 */
	{8, 2, 0, 0}, /* 4 */
	{4, 2, 0, 0}, /* 5 */
	{4, 0, 0, 0}, /* 6 */
	{8, 0, 0, 0}  /* 7 */
};

uint8_t TileShifts[8][4] =
{
	{4, 4, 4, 4}, /* 0 */
	{5, 5, 4, 0}, /* 1 */
	{5, 5, 0, 0}, /* 2 */
	{6, 5, 0, 0}, /* 3 */
	{6, 4, 0, 0}, /* 4 */
	{5, 4, 0, 0}, /* 5 */
	{5, 0, 0, 0}, /* 6 */
	{6, 0, 0, 0}  /* 7 */
};

uint8_t PaletteShifts[8][4] =
{
	{2, 2, 2, 2}, /* 0 */
	{4, 4, 2, 0}, /* 1 */
	{4, 4, 0, 0}, /* 2 */
	{0, 4, 0, 0}, /* 3 */
	{0, 2, 0, 0}, /* 4 */
	{4, 2, 0, 0}, /* 5 */
	{4, 0, 0, 0}, /* 6 */
	{0, 0, 0, 0}  /* 7 */
};

uint8_t PaletteMasks[8][4] =
{
	{7, 7, 7, 7}, /* 0 */
	{7, 7, 7, 0}, /* 1 */
	{7, 7, 0, 0}, /* 2 */
	{0, 7, 0, 0}, /* 3 */
	{0, 7, 0, 0}, /* 4 */
	{7, 7, 0, 0}, /* 5 */
	{7, 0, 0, 0}, /* 6 */
	{0, 0, 0, 0}  /* 7 */
};

uint8_t Depths[8][4] =
{
	{TILE_2BIT, TILE_2BIT, TILE_2BIT, TILE_2BIT}, /* 0 */
	{TILE_4BIT, TILE_4BIT, TILE_2BIT, 0},         /* 1 */
	{TILE_4BIT, TILE_4BIT, 0,         0},         /* 2 */
	{TILE_8BIT, TILE_4BIT, 0,         0},         /* 3 */
	{TILE_8BIT, TILE_2BIT, 0,         0},         /* 4 */
	{TILE_4BIT, TILE_2BIT, 0,         0},         /* 5 */
	{TILE_4BIT, 0,         0,         0},         /* 6 */
	{0,         0,         0,         0}          /* 7 */
};

uint16_t DirectColourMaps[8][256];

uint8_t OpLengthsM0X0[256] =
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
	/* 00 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 10 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 20 */ 3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 30 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 40 */ 1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 50 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
	/* 60 */ 1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 70 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 80 */ 2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* a0 */ 3, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* b0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* c0 */ 3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* d0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* e0 */ 3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* f0 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4
};

uint8_t OpLengthsM0X1[256] =
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
	/* 00 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 10 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 20 */ 3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 30 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 40 */ 1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 50 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
	/* 60 */ 1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 70 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 80 */ 2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* a0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* b0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* c0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* d0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* e0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* f0 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4
};

uint8_t OpLengthsM1X0[256] =
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
	/* 00 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 10 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 20 */ 3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 30 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 40 */ 1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 50 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
	/* 60 */ 1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 70 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 80 */ 2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* a0 */ 3, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* b0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* c0 */ 3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* d0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* e0 */ 3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* f0 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4
};

uint8_t OpLengthsM1X1[256] =
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e, f */
	/* 00 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 10 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 20 */ 3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 30 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 40 */ 1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 50 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
	/* 60 */ 1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 70 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* 80 */ 2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* 90 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* a0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* b0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* c0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* d0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	/* e0 */ 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	/* f0 */ 2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4
};

uint8_t APUCycleLengths[256] = /* Raw SPC700 instruction cycle lengths */
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e,  f */
	/* 00 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6,  8,
	/* 10 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4,  6,
	/* 20 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5,  4,
	/* 30 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3,  8,
	/* 40 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6,  6,
	/* 50 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4,  3,
	/* 60 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5,  5,
	/* 70 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  6,
	/* 80 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4,  5,
	/* 90 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12, 5,
	/* a0 */ 3, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4,  4,
	/* b0 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  4,
	/* c0 */ 3, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4,  9,
	/* d0 */ 2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 4, 5, 2, 2, 6,  3,
	/* e0 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4,  3,
	/* f0 */ 2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4,  3
};

uint8_t APUCycles[256] = /* Actual data used by CPU emulation, will be scaled by APUReset routine to be relative to the 65c816 instruction lengths. */
{
	/*       0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, b, c, d, e,  f */
	/* 00 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 6,  8,
	/* 10 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 4,  6,
	/* 20 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 4, 5,  4,
	/* 30 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 6, 5, 2, 2, 3,  8,
	/* 40 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 6,  6,
	/* 50 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 4, 5, 2, 2, 4,  3,
	/* 60 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 4, 5,  5,
	/* 70 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  6,
	/* 80 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 6, 5, 4, 5, 2, 4,  5,
	/* 90 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 12, 5,
	/* a0 */ 3, 8, 4, 5, 3, 4, 3, 6, 2, 6, 4, 4, 5, 2, 4,  4,
	/* b0 */ 2, 8, 4, 5, 4, 5, 5, 6, 5, 5, 5, 5, 2, 2, 3,  4,
	/* c0 */ 3, 8, 4, 5, 4, 5, 4, 7, 2, 5, 6, 4, 5, 2, 4,  9,
	/* d0 */ 2, 8, 4, 5, 5, 6, 6, 7, 4, 5, 4, 5, 2, 2, 6,  3,
	/* e0 */ 2, 8, 4, 5, 3, 4, 3, 6, 2, 4, 5, 3, 4, 3, 4,  3,
	/* f0 */ 2, 8, 4, 5, 4, 5, 5, 6, 3, 4, 5, 4, 2, 2, 4,  3
};
