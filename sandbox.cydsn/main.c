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


void vMainTask( void *pvParameters);

cystatus SetBrightness( int argc, char **argv );

int main()
{
	CyGlobalIntEnable;
	
	/* Initialize the OS vectors - make it ready to create the task, below */
    

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	COMIO_Start();
	CLI_Start();
	
	xTaskCreate( vMainTask, "MAIN Task", 200, NULL, 3, NULL );
	FreeRTOS_Start();
	
    for(;;);
}

void vMainTask( void *pvParameters)
{
	int blink;
	char ch;
	
	Brightness_Start();
	
	CLI_RegisterCommand(SetBrightness,"led","Set LED Brightness");
	
	blink = 70;	
	for(;;) {
		
//		Brightness_WritePulse0( blink );
		blink = (blink == 250)?70:(blink+10);
		
		vTaskDelay( 1000/portTICK_PERIOD_MS);
	}
}

cystatus SetBrightness( int argc, char **argv )
{
	int arg;
	uint8 usage;
	int value;
	char out[25];
	
	usage = 0;
	for (arg=1;arg<argc;++arg) {
		if (strcmp(argv[arg],"-h") == 0) {
			usage = 0xFF;
		}
		else if ( isdigit((int)argv[arg][0]) ) {
			sscanf(argv[arg],"%d",&value);
			Brightness_WritePulse0( value );
		}
	}
	
	if (usage != 0) {
		sprintf(out,"\r\n\n%s",argv[0]);
		COMIO_PrintString(out);
		COMIO_PrintString(" {value}\r\n\n");
	}
	
	value = Brightness_ReadPulse0();
	COMIO_PrintString("Brightness : ");
	sprintf(out,"%d\r\n\n",value);
	COMIO_PrintString(out);
}

/* [] END OF FILE */
