/* ======================================================================== */
/*
 * Copyright (c) 2015, E2ForLife.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the E2ForLife.com nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL E2FORLIFE.COM BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ======================================================================== */
#include <project.h>
#include <stdio.h>
#include <stdlib.h>

#define MAP_ROW    ( 1 )
#define MAP_COL    ( 1 )

const uint8 map[256] =
{
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
	 1,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,  2,  0,  1,
	 1,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  1,
	 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};

#define TILE_EMPTY      (  0 )
#define TILE_WALL       (  1 )
#define TILE_GRASS      (  2 )
#define TILE_FOREST     (  3 )
#define TILE_SWAMP      (  4 )
#define TILE_WATER      (  5 )
#define TILE_SEA        (  6 )
#define TILE_DESERT     (  7 )
#define TILE_HILLS      (  8 )
#define TILE_MOUNTAIN   (  9 )
#define TILE_SNOW       ( 10 )
#define TILE_LAVA       ( 11 )
#define TILE_ACID       ( 12 )

void drawMap( uint8 *m )
{
	int pos;
	int col;
	int row;
	
	COMIO_Position(MAP_ROW,MAP_COL);
	col = MAP_COL;
	row = MAP_ROW;
	
	for(pos=0;pos<256;++pos) {
		switch(m[pos]) {
			case TILE_EMPTY:
				COMIO_PrintStringColor("  ",0,0);
				break;
			case TILE_WALL:
				COMIO_PrintStringColor("  ",15,15);
				break;
			case TILE_GRASS:
				COMIO_PrintStringColor("\xB0\xB0",2,0);
				break;
			case TILE_FOREST:
				COMIO_PrintStringColor("\xB2\xB2",2,0);
				break;
			case TILE_SWAMP:
				COMIO_PrintStringColor("\xB0\xB0",5,0);
				break;
			case TILE_WATER:
				COMIO_PrintStringColor("\xB0\xB0",12,4);
				break;
			case TILE_SEA:
				COMIO_PrintStringColor("\xB1\xB1",12,4);
				break;
			case TILE_DESERT:
				COMIO_PrintStringColor("\xB0\xB0",3,0);
				break;
			case TILE_HILLS:
				COMIO_PrintStringColor("\xB1\xB2",7,2);
				break;
			case TILE_MOUNTAIN:
				COMIO_PrintStringColor("^^",15,8);
				break;
			case TILE_SNOW:
				COMIO_PrintStringColor("\xB1\xB1",14,6);
				break;
			case TILE_LAVA:
				COMIO_PrintStringColor("\xF7\xF7",11,9);
				break;
			case TILE_ACID:
				COMIO_PrintStringColor("##",10,2);
				break;
			default:
				break;
		}
		++col;
		if (col >= (MAP_COL+16) ) {
			++row;
			col = MAP_COL;
			COMIO_Position(row,col);
		}
	}
}

int main()
{
	cystatus result;
	uint16 scan;
	char buffer[51];
	
    CyGlobalIntEnable;

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	COMIO_Start();
	PWM_Start();
	
	/* wait for a character from the serial stream */
	COMIO_GetChar();
	COMIO_PrintStringColor("\r\n[Set your terminal for CP833 translation]\r\n",15,0);
	COMIO_PrintStringColor("\r\nPlease press a key to continue...",2,0);
	COMIO_GetChar();
	COMIO_PrintStringColor("Building Maps...",15,4);
	for(scan=0;scan<256;++scan) {
		COMIO_PrintStringColor(".",15,4);
		CyDelay(10);
	}
	COMIO_Position(1,1);
	COMIO_PrintString("\x1b[2J");
	for(scan=32;scan<256;++scan) {
		sprintf(buffer,"\r\n%3d - %2X ( %c )",scan,scan,scan);
		COMIO_PrintStringColor(buffer,2,0);
		CyDelay(10);
	}
	
    for(;;)
    {
        /* Place your application code here. */
		COMIO_Idle();
		
		scan = COMIO_ScanKey();
		if (scan != 0 )	{
			if (scan&COMIO_KEY_CTRL) {
				switch(scan&0x00FF) {
					case COMIO_KEY_UP:
						COMIO_PrintStringColor("\r\nUP\r\n",10,0);
						break;
					case COMIO_KEY_DOWN:
						COMIO_PrintStringColor("\r\nDOWN\r\n",11,0);
						break;
					case COMIO_KEY_LEFT:
						COMIO_PrintStringColor("\r\nLEFT\r\n",12,0);
						break;
					case COMIO_KEY_RIGHT:
						COMIO_PrintStringColor("\r\nRIGHT\r\n",13,0);
						break;
					default:
						COMIO_PrintStringColor("\r\n[ESC]",14,0);
						COMIO_PutChar(scan&0x00FF);
						COMIO_PrintString("\r\n");
						break;
				}
			}
			else {
				COMIO_PutChar(scan);
				COMIO_PrintString("\xB0\xB1\xB2\r\n");
			}
		}
    }
}

/* [] END OF FILE */
