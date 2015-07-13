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
#include "`$COM_INSTANCE`_cdc.h"

#include "`$FreeRTOS`.h"
#include "`$FreeRTOS`_task.h"
#include "`$FreeRTOS`_queue.h"
#include "`$FreeRTOS`_semphr.h"

#if (`$INCLUDE_CLI`==1)

	extern `$INSTANCE_NAME`_CLI_COMMAND *`$INSTANCE_NAME`_CommandTable;

#endif

/* ======================================================================== */

xQueueHandle `$INSTANCE_NAME`_RxQ;
xQueueHandle `$INSTANCE_NAME`_TxQ;

xSemaphoreHandle `$INSTANCE_NAME`_UsbDevice;

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
	/*  Initialize the USB COM port */
	
	if (`$COM_INSTANCE`_initVar == 0) {
    	/* Start USBFS Operation with 3V operation */
		#if (CYDEV_VDDIO1_MV < 5000)
    		`$COM_INSTANCE`_Start(0u, `$COM_INSTANCE`_3V_OPERATION);
		#else
			`$COM_INSTANCE`_Start(0u, `$COM_INSTANCE`_5V_OPERATION);
		#endif
	}
	
	/* Initialize USB Buffers */
	`$INSTANCE_NAME`_RxQ = xQueueCreate( `$RX_SIZE`, 1 );
	`$INSTANCE_NAME`_TxQ = xQueueCreate( `$TX_SIZE`, 1 );
	`$INSTANCE_NAME`_UsbDevice = xSemaphoreCreateMutex();
	
	if ( (`$INSTANCE_NAME`_RxQ != NULL) && (`$INSTANCE_NAME`_TxQ != NULL) ) {
		
		xTaskCreate( `$INSTANCE_NAME`_ReaderTask,"`$INSTANCE_NAME` USB Read/Control Task", 400, NULL, `$USB_PRIORITY`, NULL);
		xTaskCreate( `$INSTANCE_NAME`_WriterTask,"`$INSTANCE_NAME` USB Output Task", 400, NULL, `$USB_PRIORITY`, NULL);

		#if (`$INCLUDE_CLI`==1)
			
			/* Initialize and start the CLI task thread */
			xTaskCreate( `$INSTANCE_NAME`_vCliTask, "`$INSTANCE_NAME` CLI Task", 600, (void*)&`$INSTANCE_NAME`_CommandTable[0],`$CLI_PRIORITY`,NULL);
		
		#endif
	
		`$INSTANCE_NAME`_initVar = 1;
	}
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_Enable( void )
{
	if (`$INSTANCE_NAME`_initVar != 0) {
		/*
		 * COMIO was initialized, and now is bing enabled for use.
		 * Enter user extension enables within the merge region below.
		 */
		/* `#START COMIO_ENABLE` */
		
		/* `#END` */
	}
}
/* ======================================================================== */
void `$INSTANCE_NAME`_ReaderTask( void *pvParameters )
{
    uint16 count;
    uint8 buffer[`$INSTANCE_NAME`_BUFFER_LEN];
	uint16 idx;
	
	for (;;) {
		xSemaphoreTake( `$INSTANCE_NAME`_UsbDevice, portMAX_DELAY );
		{
			/* Handle enumeration of USB port */
	    	if(`$COM_INSTANCE`_IsConfigurationChanged() != 0u) /* Host could send double SET_INTERFACE request */
	    	{
	        	if(`$COM_INSTANCE`_GetConfiguration() != 0u)   /* Init IN endpoints when device configured */
	        	{
	            	/* Enumeration is done, enable OUT endpoint for receive data from Host */
	            	`$COM_INSTANCE`_CDC_Init();
	        	}
	    	}
		}
		xSemaphoreGive( `$INSTANCE_NAME`_UsbDevice );
		
		/* Service the USB CDC */
		xSemaphoreTake( `$INSTANCE_NAME`_UsbDevice, portMAX_DELAY);
		{
		    if(`$COM_INSTANCE`_GetConfiguration() != 0u)
		    {
		        if(`$COM_INSTANCE`_DataIsReady() != 0u)               /* Check for input data from PC */
		        {   
		            count = `$COM_INSTANCE`_GetAll(buffer);           /* Read received data and re-enable OUT endpoint */
		            if(count != 0u)
		            {
						/* insert data in to Receive FIFO */
						for(idx=0;idx<count;++idx) {
							xQueueSend( `$INSTANCE_NAME`_RxQ, (void*)&buffer[idx],portMAX_DELAY);
							#if (`$DEBUG_USB_RX_ECHO` == 1 )
								xQueueSend( `$INSTANCE_NAME`_TxQ, (void*)&buffer[idx],portMAX_DELAY);
							#endif
						}
					}
				}
			}
		}
		xSemaphoreGive( `$INSTANCE_NAME`_UsbDevice);
		
		vTaskDelay(`$USB_SCAN_RATE`/portTICK_PERIOD_MS);
	}
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_WriterTask( void *pvParameters )
{
    uint16 count;
    uint8 buffer[`$INSTANCE_NAME`_BUFFER_LEN];
	uint16 idx;
	portBASE_TYPE xStatus;
	
	for (;;) {
		/* Service the USB CDC */
		xSemaphoreTake( `$INSTANCE_NAME`_UsbDevice, portMAX_DELAY);
		{
		    if(`$COM_INSTANCE`_GetConfiguration() != 0u)
		    {
				/*
				 * build a buffer of data to send back over the USB COM port.
				 */
				count = uxQueueMessagesWaiting( `$INSTANCE_NAME`_TxQ );
				count = (count > `$INSTANCE_NAME`_BUFFER_LEN)? `$INSTANCE_NAME`_BUFFER_LEN:count;
				
				/* Wait till component is ready to send more data to the PC */			
	            if ( (`$COM_INSTANCE`_CDCIsReady() != 0u) && (count > 0) ) {
					taskENTER_CRITICAL();
					for (idx = 0; idx < count; ++idx) {
						xStatus = xQueueReceive( `$INSTANCE_NAME`_TxQ,&buffer[idx], `$USB_SCAN_RATE`/portTICK_RATE_MS);
					}
					/* Send data back to host */
	    	        `$COM_INSTANCE`_PutData(buffer, count);
					
					/* If the last sent packet is exactly maximum packet size, 
	            	 *  it shall be followed by a zero-length packet to assure the
	             	*  end of segment is properly identified by the terminal.
	             	*/
	            	if(count == `$INSTANCE_NAME`_BUFFER_LEN){
						/* Wait till component is ready to send more data to the PC */
	                	while(`$COM_INSTANCE`_CDCIsReady() == 0u) {
							vTaskDelay( `$USB_SCAN_RATE` / portTICK_RATE_MS );
						}
	                	`$COM_INSTANCE`_PutData(NULL, 0u);         /* Send zero-length packet to PC */
	            	} 
					taskEXIT_CRITICAL();
				}
			}
		}
		xSemaphoreGive( `$INSTANCE_NAME`_UsbDevice );
		
		vTaskDelay(`$USB_SCAN_RATE`/portTICK_PERIOD_MS);
	}
}

/* ======================================================================== */
/* ------------------------------------------------------------------------ */
char `$INSTANCE_NAME`_GetChar( void )
{
	char value;
	portBASE_TYPE xStatus;
	
	/* wait for data to become available */
	xStatus = xQueueReceive( `$INSTANCE_NAME`_RxQ, &value, portMAX_DELAY);
	
	if (xStatus != pdPASS) {
		value = 255;
	}
	
	return value;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_PutChar( char ch )
{
	
	portBASE_TYPE xStatus;
	
	xStatus = xQueueSend( `$INSTANCE_NAME`_TxQ, &ch, portMAX_DELAY);
	
	return (xStatus == pdPASS)?CYRET_SUCCESS:CYRET_MEMORY;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_PrintString( const char *str )
{
	int idx;
	cystatus result;
	
	result = CYRET_SUCCESS;
	idx = 0;
	while ( (str[idx] != 0) && (result == CYRET_SUCCESS) ) {
		/* insert character in to the send fifo */
		result = `$INSTANCE_NAME`_PutChar(str[idx++]);		
	}
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
			`$INSTANCE_NAME`_PutChar( str[idx] );
			result = `$INSTANCE_NAME`_SetColor(fg,bg);
		}
		else {
			`$INSTANCE_NAME`_PutChar( str[idx] );
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
	portBASE_TYPE xStatus;
	
	result = CYRET_STARTED;
	idx = 0;
	xStatus = xQueuePeek( `$INSTANCE_NAME`_RxQ, &lookahead, portMAX_DELAY);
	
	/* 
	 * While there are no EOL character read, read the data fro mthe queue,
	 * and take a peek at the next data in the buffer.
	 */
	while ( (lookahead != '\r') && (lookahead != '\n') ) {
		ch = `$INSTANCE_NAME`_GetChar();
		xStatus = xQueuePeek( `$INSTANCE_NAME`_RxQ, &lookahead, portMAX_DELAY);
		if ( (ch == '\b') || (ch == 127) ) {
			str[idx] = 0;
			if (idx>0) {
				idx--;
			}
			`$INSTANCE_NAME`_PrintString("\b \b");
		}
		else {
			str[idx++] = ch;
			`$INSTANCE_NAME`_PutChar( ch );
		}
		str[idx] = 0;
	}
	
	/*
	 * Remove the lingering EOL characters from the queue to prepare reading
	 * of the next string.
	 */
	if ( (lookahead == '\r') || (lookahead == '\n') ) {
		do {
			`$INSTANCE_NAME`_GetChar(); /* Remove the EOL character from buffer */
			xStatus = xQueuePeek( `$INSTANCE_NAME`_RxQ, &lookahead, portMAX_DELAY);
		}
		while( (lookahead == '\r') || (lookahead == '\n') );
		result = CYRET_FINISHED;
	}
	return result;
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
				result |= `$INSTANCE_NAME`_KEY_CSI;
			}
		}
		else if ( ch < 32 ) {
			/*
			 * In situations where the control key was pressed (CTRL-C)
			 * 
			 */
			result = (ch+32) | `$INSTANCE_NAME`_KEY_CTRL;
		}
		else if ( ((ch>='A')&&(ch <='Z'))||((ch>='!')&&(ch<='+'))||(ch=='^')||(ch=='_')||(ch==':')||(ch=='<')||(ch=='>')||(ch=='?') ) {
			result = ch | `$INSTANCE_NAME`_KEY_SHFT;
		}
		else {
			result = ch;
		}
	}
	return result;
}
/* ------------------------------------------------------------------------- */
/* [] END OF FILE */
