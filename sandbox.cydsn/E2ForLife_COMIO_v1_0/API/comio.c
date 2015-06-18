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

/* Local Receive FIFO */
uint8 `$INSTANCE_NAME`_RxFifo[ `$INSTANCE_NAME`_RX_SIZE ];
uint16 `$INSTANCE_NAME`_RxWritePtr;
uint16 `$INSTANCE_NAME`_RxReadPtr;
uint16 `$INSTANCE_NAME`_RxLength;

/* Local Transmit FIFO */
uint8 `$INSTANCE_NAME`_TxFifo[ `$INSTANCE_NAME`_TX_SIZE ];
uint16 `$INSTANCE_NAME`_TxWritePtr;
uint16 `$INSTANCE_NAME`_TxReadPtr;
uint16 `$INSTANCE_NAME`_TxLength;

/* ======================================================================== */
void `$INSTANCE_NAME`_Start( void )
{
	/* Initialize USB Buffers */
	`$INSTANCE_NAME`_RxWritePtr = 0;
	`$INSTANCE_NAME`_RxReadPtr = 0;
	`$INSTANCE_NAME`_RxLength = 0;
	`$INSTANCE_NAME`_TxWritePtr = 0;
	`$INSTANCE_NAME`_TxReadPtr = 0;
	`$INSTANCE_NAME`_TxLength = 0;
	
    /* Enable Global Interrupts */
    CyGlobalIntEnable;                        

    /* Start USBFS Operation with 3V operation */
    `$COM_INSTANCE`_Start(0u, `$COM_INSTANCE`_3V_OPERATION);

	/* Check for enumeration after initialization */
	`$INSTANCE_NAME`_Idle();
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
					`$INSTANCE_NAME`_RxFifo[ `$INSTANCE_NAME`_RxWritePtr++ ] = buffer[idx];
					`$INSTANCE_NAME`_RxLength++;
					if ( `$INSTANCE_NAME`_RxWritePtr >= `$INSTANCE_NAME`_RX_SIZE ) {
						`$INSTANCE_NAME`_RxWritePtr = 0;
					}
					/* handle overflow */
					if (`$INSTANCE_NAME`_RxLength > `$INSTANCE_NAME`_RX_SIZE) {
						`$INSTANCE_NAME`_RxLength = `$INSTANCE_NAME`_RX_SIZE;
						++`$INSTANCE_NAME`_RxReadPtr;
						if (`$INSTANCE_NAME`_RxReadPtr > `$INSTANCE_NAME`_RX_SIZE) {
							`$INSTANCE_NAME`_RxReadPtr = 0;
						}
					}
				}
			}
		}
		/*
		 * detrmine if there is data to be sent from the buffer to the host,
		 * and send a block of data from the FIFO up to the endpoint limit.
		 */
		if (`$INSTANCE_NAME`_TxLength > 0) {
#if (`$INSTANCE_NAME`_BLOCKING_SEND == `$INSTANCE_NAME`_YES)
			/* Wait till component is ready to send more data to the PC */			
//            while(`$COM_INSTANCE`_CDCIsReady() == 0u);
#else
			if (`$COM_INSTANCE`_CDCIsReady() == 0) return;
#endif
			count = 0;
			while ( (count < `$INSTANCE_NAME`_BUFFER_LEN) && (`$INSTANCE_NAME`_TxLength > 0) ) {
				buffer[count++] = `$INSTANCE_NAME`_TxFifo[`$INSTANCE_NAME`_TxReadPtr++];
				if (`$INSTANCE_NAME`_TxReadPtr >= `$INSTANCE_NAME`_TX_SIZE) {
					`$INSTANCE_NAME`_TxReadPtr = 0;
				}
				--`$INSTANCE_NAME`_TxLength;
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
	while (`$INSTANCE_NAME`_RxLength == 0) {
		`$INSTANCE_NAME`_Idle();
	}
	
	value = (char) `$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr++];
	if (`$INSTANCE_NAME`_RxReadPtr >= `$INSTANCE_NAME`_RX_SIZE) {
		`$INSTANCE_NAME`_RxReadPtr = 0;
	}
	--`$INSTANCE_NAME`_RxLength;
	
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
	`$INSTANCE_NAME`_TxFifo[`$INSTANCE_NAME`_TxWritePtr++] = ch;
	if (`$INSTANCE_NAME`_TxWritePtr >= `$INSTANCE_NAME`_TX_SIZE) {
		`$INSTANCE_NAME`_TxWritePtr = 0;
	}
	++`$INSTANCE_NAME`_TxLength;
	/* Overflow handler */
	if (`$INSTANCE_NAME`_TxLength > `$INSTANCE_NAME`_TX_SIZE) {
		result = CYRET_CANCELED;
		`$INSTANCE_NAME`_TxLength = `$INSTANCE_NAME`_TX_SIZE;
		++`$INSTANCE_NAME`_TxReadPtr;
		if (`$INSTANCE_NAME`_TxReadPtr >= `$INSTANCE_NAME`_TX_SIZE) {
			`$INSTANCE_NAME`_TxReadPtr = 0;
		}
	}
	/*
	 * The transmot data has been placed in to the Tx buffer, so no, try to
	 * send some (or all) of it.
	 */
	`$INSTANCE_NAME`_Idle();
	
	return result;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_PutString( const char *str )
{
	int idx;
	cystatus result;
	
	`$INSTANCE_NAME`_Idle();

	result = CYRET_SUCCESS;
	idx = 0;
	while ( (str[idx] != 0) && (result == CYRET_SUCCESS) ) {
		/* insert character in to the send fifo */
		`$INSTANCE_NAME`_TxFifo[`$INSTANCE_NAME`_TxWritePtr++] = str[idx++];
		if (`$INSTANCE_NAME`_TxWritePtr >= `$INSTANCE_NAME`_TX_SIZE) {
			`$INSTANCE_NAME`_TxWritePtr = 0;
		}
		++`$INSTANCE_NAME`_TxLength;
		/* Overflow handler */
		if (`$INSTANCE_NAME`_TxLength > `$INSTANCE_NAME`_TX_SIZE) {
			result = CYRET_CANCELED;
			`$INSTANCE_NAME`_TxLength = `$INSTANCE_NAME`_TX_SIZE;
			++`$INSTANCE_NAME`_TxReadPtr;
			if (`$INSTANCE_NAME`_TxReadPtr >= `$INSTANCE_NAME`_TX_SIZE) {
				`$INSTANCE_NAME`_TxReadPtr = 0;
			}
		}
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
	return `$INSTANCE_NAME`_PutString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_ClearLine(uint8 mode)
{
	char buffer[15];
	
	sprintf(buffer,"\x1b[%dK",mode);
	return `$INSTANCE_NAME`_PutString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_SetCursor(uint8 row, uint8 col)
{
	char buffer[21];
	
	sprintf(buffer,"\x1b[%d;%dH",row,col);
	return `$INSTANCE_NAME`_PutString(buffer);
}
/* ------------------------------------------------------------------------- */
cystatus `$INSTANCE_NAME`_PutStringColor(const char *str, uint8 fg, uint8 bg)
{
	cystatus result;
	int idx;
	
	result = `$INSTANCE_NAME`_SetColor(fg,bg);
	idx = 0;
	while ( (str[idx] != 0) && (result == CYRET_SUCCESS) ) {
		if ( ( (str[idx] == '[') || (str[idx] == ']') || (str[idx] == '(') || (str[idx] == ')') ) && (bg!=4) ) {
			result = `$INSTANCE_NAME`_SetColor(4,bg);
			result = `$INSTANCE_NAME`_PutChar(str[idx]);
			result = `$INSTANCE_NAME`_SetColor(fg,bg);
		}
		else {
			result = `$INSTANCE_NAME`_PutChar( str[idx] );
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
	int idx;
	
	/* Process queue'd I/O over the USB port */
	`$INSTANCE_NAME`_Idle();
	
	result = CYRET_STARTED;
	idx = strlen( str );
	while (
#if (`$INSTANCE_NAME`_BLOCKING_GETS == `$INSTANCE_NAME`_NO)
			(`$INSTANCE_NAME`_RxLength > 0) && 
#endif
			(`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] != '\r') && (`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] != '\n') ) {
		ch = `$INSTANCE_NAME`_GetChar();
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
	
	if ( (`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] == '\r') || (`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] == '\n') ) {
		do {
			`$INSTANCE_NAME`_GetChar(); /* Remove the EOL character from buffer */
		}
		while( (`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] == '\r') || (`$INSTANCE_NAME`_RxFifo[`$INSTANCE_NAME`_RxReadPtr] == '\n') );
		result = CYRET_FINISHED;
	}
	return result;
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearFifo( void )
{
	`$INSTANCE_NAME`_TxReadPtr = 0;
	`$INSTANCE_NAME`_TxWritePtr = 0;
	`$INSTANCE_NAME`_TxLength = 0;
	`$INSTANCE_NAME`_RxReadPtr = 0;
	`$INSTANCE_NAME`_RxWritePtr = 0;
	`$INSTANCE_NAME`_RxLength = 0;
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearTxBuffer( void )
{
	`$INSTANCE_NAME`_TxReadPtr = 0;
	`$INSTANCE_NAME`_TxWritePtr = 0;
	`$INSTANCE_NAME`_TxLength = 0;
}
/* ------------------------------------------------------------------------- */
void `$INSTANCE_NAME`_ClearRxBuffer( void )
{
	`$INSTANCE_NAME`_RxReadPtr = 0;
	`$INSTANCE_NAME`_RxWritePtr = 0;
	`$INSTANCE_NAME`_RxLength = 0;
}
/* ------------------------------------------------------------------------- */
/* [] END OF FILE */
