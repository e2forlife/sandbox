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
#include <cytypes.h>
#include <cylib.h>
#include <stdio.h>

#include "`$INSTANCE_NAME`.h"
#include "`$COM_INSTANCE`.h"

#if (`$INSTANCE_NAME`_USB_MODE == `$INSTANCE_NAME`_YES)
#include "`$COM_INSTANCE`_cdc.h"
#endif

/* ======================================================================== */
#define `$INSTANCE_NAME`_QMAX_IDX      ( 2 )
#define `$INSTANCE_NAME`_QSIZE_IDX     ( 0 )
#define `$INSTANCE_NAME`_QREAD_IDX     ( 4 )
#define `$INSTANCE_NAME`_QWRITE_IDX    ( 6 )
#define `$INSTANCE_NAME`_QBASE_IDX     ( 8 )

#define `$INSTANCE_NAME`_QReadPtr(q)   ( *((uint16*)&q[ `$INSTANCE_NAME`_QREAD_IDX ]) )
#define `$INSTANCE_NAME`_QWritePtr(q)  ( *((uint16*)&q[ `$INSTANCE_NAME`_QWRITE_IDX ]) )
#define `$INSTANCE_NAME`_QSize(q)      ( *((uint16*)&q[ `$INSTANCE_NAME`_QSIZE_IDX ]))
#define `$INSTANCE_NAME`_QMax(q)       ( *((uint16*)&q[ `$INSTANCE_NAME`_QMAX_IDX]))
#define `$INSTANCE_NAME`_QData(q,i)    ( *((uint8*)&q[ i + `$INSTANCE_NAME`_QBASE_IDX]))

/* Local Receive FIFO */
uint8 `$INSTANCE_NAME`_RxQ[ `$INSTANCE_NAME`_RX_SIZE + 8];

/* Local Transmit FIFO */
uint8 `$INSTANCE_NAME`_TxQ[ `$INSTANCE_NAME`_TX_SIZE + 8 ];

uint8 `$INSTANCE_NAME`_initVar;

/* ======================================================================== */
void `$INSTANCE_NAME`_Start( void )
{
	if (`$INSTANCE_NAME`_initVar != 1) {
		`$INSTANCE_NAME`_Init();
	}
	
	/* Check for enumeration after initialization */
	`$INSTANCE_NAME`_Enable();
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_Init( void )
{
	/* Initialize USB Buffers */
	memset((void*)`$INSTANCE_NAME`_RxQ, 0, `$INSTANCE_NAME`_RX_SIZE + 8);
	memset((void*)`$INSTANCE_NAME`_TxQ, 0, `$INSTANCE_NAME`_TX_SIZE + 8);
	`$INSTANCE_NAME`_QMax(`$INSTANCE_NAME`_RxQ) = `$INSTANCE_NAME`_RX_SIZE;
	`$INSTANCE_NAME`_QMax(`$INSTANCE_NAME`_TxQ) = `$INSTANCE_NAME`_TX_SIZE;
	
	
    /* Enable Global Interrupts */
    CyGlobalIntEnable;                        

	if (`$COM_INSTANCE`_initVar == 0) {
    	/* Start USBFS Operation with 3V operation */
		#if (CYDEV_VDDIO1_MV < 5000)
    		`$COM_INSTANCE`_Start(0u, `$COM_INSTANCE`_3V_OPERATION);
		#else
			`$COM_INSTANCE`_Start(0u, `$COM_INSTANCE`_5V_OPERATION);
		#endif
	}
	
	#if ( (`$AutoSpawn_Task` == 1)&&(`$vCliTask` == 1)&&(`$INCLUDE_CLI`==1) )
		/* Initialize and start the CLI task thread */
		
	#endif
	
	`$INSTANCE_NAME`_initVar = 1;
}	
/* ======================================================================== */
void `$INSTANCE_NAME`_QWrite(uint8* q, uint8 value)
{
	/*
	 * first check for overflow on received data and adjust the Q so that
	 * there is room for the new byte.
	 */
	if (`$INSTANCE_NAME`_QSize(q) >= `$INSTANCE_NAME`_QMax(q) ) {
		`$INSTANCE_NAME`_QSize(q) = `$INSTANCE_NAME`_QSize(q) - 1;
		`$INSTANCE_NAME`_QReadPtr(q) = `$INSTANCE_NAME`_QReadPtr(q) + 1;
		if (`$INSTANCE_NAME`_QReadPtr(q) >= `$INSTANCE_NAME`_QMax(q) ) {
			`$INSTANCE_NAME`_QReadPtr(q) = 0;
		}
	}
	
	`$INSTANCE_NAME`_QData(q,`$INSTANCE_NAME`_QWritePtr(q)) = value;
	`$INSTANCE_NAME`_QWritePtr(q) = `$INSTANCE_NAME`_QWritePtr(q) + 1;
	if (`$INSTANCE_NAME`_QWritePtr(q) >= `$INSTANCE_NAME`_QMax(q) ) {
		`$INSTANCE_NAME`_QWritePtr(q) = 0;
	}	
}
/* ------------------------------------------------------------------------ */
uint8 `$INSTANCE_NAME`_QRead(uint8* q)
{
	uint8 value;
	
	value = 0xFF;
	if (`$INSTANCE_NAME`_QSize(q) > 0) {
		value = `$INSTANCE_NAME`_QData(q, `$INSTANCE_NAME`_QReadPtr(q));
		`$INSTANCE_NAME`_QReadPtr(q) = `$INSTANCE_NAME`_QReadPtr(q) + 1;
		if (`$INSTANCE_NAME`_QReadPtr(q) >= `$INSTANCE_NAME`_QMax(q) ) {
			`$INSTANCE_NAME`_QReadPtr(q) = 0;
		}
		`$INSTANCE_NAME`_QSize(q) = `$INSTANCE_NAME`_QSize(q) - 1;
	}
	return value;
}
/* ------------------------------------------------------------------------ */
uint8 `$INSTANCE_NAME`_QPeek( uint8* q )
{
	uint8 value;
	
	if (`$INSTANCE_NAME`_QSize(q) > 0) {
		value = `$INSTANCE_NAME`_QData(q,`$INSTANCE_NAME`_QReadPtr(q));
	}
	else {
		value = 0;
	}
	return value;
}
/* ======================================================================== */
void `$INSTANCE_NAME`_Idle( void )
{
    uint16 count;
    uint8 buffer[`$INSTANCE_NAME`_BUFFER_LEN];
	uint16 idx;
	
	/* Handle enumeration of USB port */
    if(`$COM_INSTANCE`_IsConfigurationChanged() != 0u) /* Host could send double SET_INTERFACE request */
    {
        if(`$COM_INSTANCE`_GetConfiguration() != 0u)   /* Init IN endpoints when device configured */
        {
            /* Enumeration is done, enable OUT endpoint for receive data from Host */
            `$COM_INSTANCE`_CDC_Init();
        }
    }
	/* Service the USB CDC */
    if(`$COM_INSTANCE`_GetConfiguration() != 0u)
    {
        if(`$COM_INSTANCE`_DataIsReady() != 0u)               /* Check for input data from PC */
        {   
            count = `$COM_INSTANCE`_GetAll(buffer);           /* Read received data and re-enable OUT endpoint */
            if(count != 0u)
            {
				/* insert data in to Receive FIFO */
				for(idx=0;idx<count;++idx) {
					`$INSTANCE_NAME`_QWrite(`$INSTANCE_NAME`_RxQ, buffer[idx]);
				}
			}
		}
		/*
		 * detrmine if there is data to be sent from the buffer to the host,
		 * and send a block of data from the FIFO up to the endpoint limit.
		 */
		if (`$INSTANCE_NAME`_QSize(`$INSTANCE_NAME`_TxQ ) > 0) {
#if (`$INSTANCE_NAME`_BLOCKING_SEND == `$INSTANCE_NAME`_YES)
			/* Wait till component is ready to send more data to the PC */			
            while(`$COM_INSTANCE`_CDCIsReady() == 0u);
#else
			if (`$COM_INSTANCE`_CDCIsReady() == 0) return;
#endif
			count = 0;
			while ( (count < `$INSTANCE_NAME`_BUFFER_LEN) && (`$INSTANCE_NAME`_QSize(`$INSTANCE_NAME`_TxQ) > 0) ) {
				buffer[count++] = `$INSTANCE_NAME`_QRead(`$INSTANCE_NAME`_TxQ);
			}
			/* Send data back to host */
            `$COM_INSTANCE`_PutData(buffer, count);
            /* If the last sent packet is exactly maximum packet size, 
             *  it shall be followed by a zero-length packet to assure the
             *  end of segment is properly identified by the terminal.
             */
            if(count == `$INSTANCE_NAME`_BUFFER_LEN){
				/* Wait till component is ready to send more data to the PC */
                while(`$COM_INSTANCE`_CDCIsReady() == 0u); 
                `$COM_INSTANCE`_PutData(NULL, 0u);         /* Send zero-length packet to PC */
            }
        }  
	}
}
/* ------------------------------------------------------------------------ */
char `$INSTANCE_NAME`_GetChar( void )
{
	char value;
	
	/* wait for data to become available */
	`$INSTANCE_NAME`_Idle();
	while (`$INSTANCE_NAME`_QSize(`$INSTANCE_NAME`_RxQ) == 0) {
		`$INSTANCE_NAME`_Idle();
	}
	
	value = (char) `$INSTANCE_NAME`_QRead(`$INSTANCE_NAME`_RxQ);
	
	return value;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_PutChar( char ch )
{
	cystatus result;
	
	/* set default, and process USB CDC port */
	result = CYRET_SUCCESS;
	`$INSTANCE_NAME`_Idle();
	/* insert character in to the send fifo */
	`$INSTANCE_NAME`_QWrite(`$INSTANCE_NAME`_TxQ, ch);
	/*
	 * The transmot data has been placed in to the Tx buffer, so no, try to
	 * send some (or all) of it.
	 */
	`$INSTANCE_NAME`_Idle();
	
	return result;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_PrintString( const char *str )
{
	int idx;
	cystatus result;
	
	`$INSTANCE_NAME`_Idle();

	result = CYRET_SUCCESS;
	idx = 0;
	while ( (str[idx] != 0) && (result == CYRET_SUCCESS) ) {
		/* insert character in to the send fifo */
		`$INSTANCE_NAME`_QWrite(`$INSTANCE_NAME`_TxQ,str[idx++]);		
	}
	/*
	 * The transmot data has been placed in to the Tx buffer, so no, try to
	 * send some (or all) of it.
	 */
	`$INSTANCE_NAME`_Idle();
	return result;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_SetColor( uint8 fg, uint8 bg )
{
	char buffer[21];
	
	fg = (fg > 7) ?	(90 + (fg&0x07)):(fg + 30);
	bg = (bg > 7) ? (100+(bg&7)):(bg + 40);
	sprintf(buffer,"\x1b[%d;%dm",fg,bg);
	return `$INSTANCE_NAME`_PrintString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_ClearLine(uint8 mode)
{
	char buffer[15];
	
	sprintf(buffer,"\x1b[%dK",mode);
	return `$INSTANCE_NAME`_PrintString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_Position(uint8 row, uint8 col)
{
	char buffer[21];
	
	sprintf(buffer,"\x1b[%d;%dH",row+1,col+1);
	return `$INSTANCE_NAME`_PrintString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_PrintStringColor(const char *str, uint8 fg, uint8 bg)
{
	cystatus result;
	int idx;
	
	result = `$INSTANCE_NAME`_SetColor(fg,bg);
	idx = 0;
	while ( (str[idx] != 0) && (result == CYRET_SUCCESS) ) {
		if ( ( (str[idx] == '[') || (str[idx] == ']') || (str[idx] == '(') || (str[idx] == ')') ) && (bg!=4) ) {
			result = `$INSTANCE_NAME`_SetColor(4,bg);
			`$INSTANCE_NAME`_QWrite( `$INSTANCE_NAME`_TxQ, str[idx] );
			result = `$INSTANCE_NAME`_SetColor(fg,bg);
		}
		else {
			`$INSTANCE_NAME`_QWrite( `$INSTANCE_NAME`_TxQ, str[idx] );
		}
		++idx;
	}
	`$INSTANCE_NAME`_SetColor(7,0);
	
	return result;
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_GetString(char *str)
{
	cystatus result;
	char ch;
	char lookahead;
	int idx;
	
	/* Process queue'd I/O over the USB port */
	`$INSTANCE_NAME`_Idle();
	
	result = CYRET_STARTED;
	idx = strlen( str );
	lookahead = (char) `$INSTANCE_NAME`_QPeek(`$INSTANCE_NAME`_RxQ);
	
	while (
#if (`$INSTANCE_NAME`_BLOCKING_GETS == `$INSTANCE_NAME`_NO)
			(`$INSTANCE_NAME`_QSize(`$INSTANCE_NAME`_RxQ) > 0) && 
#endif
			(lookahead != '\r') && (lookahead != '\n') ) {
		ch = `$INSTANCE_NAME`_GetChar();
		lookahead = (char)`$INSTANCE_NAME`_QPeek(`$INSTANCE_NAME`_RxQ);
		if ( (ch == '\b') || (ch == 127) ) {
			str[idx] = 0;
			if (idx>0) {
				idx--;
			}
		}
		else {
			str[idx++] = ch;
		}
		str[idx] = 0;
	}
	
	if ( (lookahead == '\r') || (lookahead == '\n') ) {
		do {
			`$INSTANCE_NAME`_GetChar(); /* Remove the EOL character from buffer */
			lookahead = `$INSTANCE_NAME`_QPeek(`$INSTANCE_NAME`_RxQ);
		}
		while( (lookahead == '\r') || (lookahead == '\n') );
		result = CYRET_FINISHED;
	}
	return result;
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearFifo( void )
{
	`$INSTANCE_NAME`_ClearTxBuffer();
	`$INSTANCE_NAME`_ClearRxBuffer();
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearTxBuffer( void )
{
	memset((void*)`$INSTANCE_NAME`_TxQ,0, 8);
	`$INSTANCE_NAME`_QMax(`$INSTANCE_NAME`_TxQ) = `$INSTANCE_NAME`_TX_SIZE;
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearRxBuffer( void )
{
	memset((void*)`$INSTANCE_NAME`_RxQ,0, 8);
	`$INSTANCE_NAME`_QMax(`$INSTANCE_NAME`_RxQ) = `$INSTANCE_NAME`_RX_SIZE;
}
/* ------------------------------------------------------------------------- */
/*
 * scan key, and process escape sequences for command keys.
 */
uint16 `$INSTANCE_NAME`_ScanKey( void )
{
	uint16 result;
	char ch;
	
	`$INSTANCE_NAME`_Idle();
	result = 0;
	if (`$INSTANCE_NAME`_QSize(`$INSTANCE_NAME`_RxQ) > 0) {
		ch = `$INSTANCE_NAME`_GetChar();
		if (ch == '\x1b') {
			ch = `$INSTANCE_NAME`_GetChar(); /* wait for bracket */
			if (ch == '[') {
				do {
					ch = `$INSTANCE_NAME`_GetChar(); /* Get command */
				}
				while ( !isalpha((int)ch) );
				result = (uint16) ch;
				result |= `$INSTANCE_NAME`_KEY_CTRL;
			}
		}
		else {
			result = ch;
		}
	}
	return result;
}
/* ------------------------------------------------------------------------- */
/* [] END OF FILE */
