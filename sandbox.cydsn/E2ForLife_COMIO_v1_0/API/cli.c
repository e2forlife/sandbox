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

#if (`$INCLUDE_CLI` == 1)

#include <cytypes.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
	
#include "`$INSTANCE_NAME`.h"

#include "`$FreeRTOS`.h"
#include "`$FreeRTOS`_task.h"
#include "`$FreeRTOS`_queue.h"
	
/* ------------------------------------------------------------------------ */
	
const `$INSTANCE_NAME`_CLI_COMMAND `$INSTANCE_NAME`_CommandTable[] =
{
	{ "help", `$INSTANCE_NAME`_CliHelp, "List available commands and descriptions" },
	{ "cls",  `$INSTANCE_NAME`_CliClearScreen, "Clear Display." },
	/* -------------------------------------------------------------------- */
	/* `#START USER_COMMAND_TABLE` */
	
	
	/* `#END` */
	/* -------------------------------------------------------------------- */
	{ "", NULL,"End of Command Table"}	
};
	
/* ------------------------------------------------------------------------ */

int `$INSTANCE_NAME`_CLIrefresh;
uint8 `$INSTANCE_NAME`_CLIinitVar;

/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_SystemMsg(const char *str, uint8 level)
{
	switch(level) {
		case `$INSTANCE_NAME`_NOTE:
			`$INSTANCE_NAME`_PrintStringColor("\r\n[NOTE]",15,0);
			`$INSTANCE_NAME`_PrintString(": ");
			`$INSTANCE_NAME`_PrintStringColor(str,2,0);
			break;
		case `$INSTANCE_NAME`_WARN:
			`$INSTANCE_NAME`_PrintStringColor("\r\n[WARNING]",15,0);
			`$INSTANCE_NAME`_PrintString(": ");
			`$INSTANCE_NAME`_PrintStringColor(str,3,0);
			break;
		case `$INSTANCE_NAME`_ERROR:
			`$INSTANCE_NAME`_PrintStringColor("\r\n[ERROR]",15,0);
			`$INSTANCE_NAME`_PrintString(": ");
			`$INSTANCE_NAME`_PrintStringColor(str,1,0);
			break;
		case `$INSTANCE_NAME`_FATAL:
			`$INSTANCE_NAME`_PrintStringColor("\r\n[FATAL]",15,0);
			`$INSTANCE_NAME`_PrintString(": ");
			`$INSTANCE_NAME`_PrintStringColor(str,9,0);
			break;
		default:
			`$INSTANCE_NAME`_PrintStringColor("\r\n[????]",15,0);
			`$INSTANCE_NAME`_PrintString(": ");
			`$INSTANCE_NAME`_PrintStringColor(str,5,0);
			break;
	}
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_CliHelp( int argc, char **argv )
{
	int idx;
	char bfr[51];
	
	`$INSTANCE_NAME`_PrintString("\x1b[1;1H\x1b[2J");
	
	idx = 0;
	while ( strlen(`$INSTANCE_NAME`_CommandTable[idx].name) != 0) {
		if ( strlen(`$INSTANCE_NAME`_CommandTable[idx].desc) > 0 ) {
			sprintf(bfr,"\r\n[%10s]",`$INSTANCE_NAME`_CommandTable[idx].name);
			`$INSTANCE_NAME`_PrintStringColor(bfr,15,0);
			`$INSTANCE_NAME`_PrintString(" : ");
			`$INSTANCE_NAME`_PrintStringColor(`$INSTANCE_NAME`_CommandTable[idx].desc, ((idx&0x01)?10:2),1);
		}
		++idx;
	}
	`$INSTANCE_NAME`_PrintString("\r\n\n");
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_CliClearScreen( int argc, char **argv )
{
	argc = argc;
	argv = argv;
	
	`$INSTANCE_NAME`_PrintString("\x1b[1;1H\x1b[2J");
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_CliShowPrompt( char *lineBuffer )
{
	int idx;
	char outBuffer[11];
	
	`$INSTANCE_NAME`_PrintString("\r\n");
	`$INSTANCE_NAME`_PrintStringColor("`$UserMessageString`",`$MSG_FG_COLOR`,`$MSG_BG_COLOR`);
	`$INSTANCE_NAME`_PrintStringColor("`$UserPromptString`",`$PROMPT_FG_COLOR`,`$PROMPT_BG_COLOR`);
	`$INSTANCE_NAME`_PrintString(" : ");
	`$INSTANCE_NAME`_SetColor(`$INPUT_FG_COLOR`,`$INPUT_BG_COLOR`);
	for(idx=0;idx<`$MAX_CLI_INPUT_BUFFER`;++idx) {
		`$INSTANCE_NAME`_PutChar(' ');
	}
	sprintf(outBuffer,"\x1b[%dD",`$MAX_CLI_INPUT_BUFFER`);
	`$INSTANCE_NAME`_PrintString( outBuffer );
	`$INSTANCE_NAME`_PrintString( lineBuffer);
}
/* ------------------------------------------------------------------------ */
int `$INSTANCE_NAME`_CliGetArguments( char *buffer, int *argc, char **argv )
{
	int idx;
	cystatus result;
	
	result = CYRET_STARTED;
	idx = 0;
	*argc = 0;
	while ( (buffer[idx] != 0) && (result == CYRET_STARTED) ) {
		/*
		 * when a space is detected, replace it withe a NULL to
		 * seperate the string from the line. When a trailing space is
		 * at the end of the line, just ignore that argument. Set the
		 * parse status to started to singal that data needs to be
		 * stored for the next argument.
		 */
		if ( isspace( (int) buffer[idx]) ) {
			while (isspace( (int) buffer[idx]) ) {
				buffer[idx] = 0;
				++idx;
			}
			argv[*argc] = &buffer[idx];
			*argc = *argc + 1;
		}
		/* 
		 * The end of a command can be the end of the buffer, or, a
		 * semicolon can be used for the creation of compound
		 * statements. When a semi is found, return to the processing
		 * loop to execute the command.
		 */
		else if (buffer[idx] == ';') {
			/* process the command */
			buffer[idx] = 0;
			result = CYRET_FINISHED;
		}
		++idx;
	}
	
	return idx;
}
/* ------------------------------------------------------------------------ */
/**
 * \brief Process a token from the line buffer as a read or write operation
 * \param *buffer pointer to the buffer (with the read operator removed)
 * \param read identifier for read operation or write operations
 *
 * process token will pull the next token (has to be a label) from the input
 * buffer, and process the line to completion.  If there is a syntax error
 * the buffer is dumped and the user is notified. otherwise, the processor
 * function is called.
 */
cystatus `$INSTANCE_NAME`_CliProcessCommand(const `$INSTANCE_NAME`_CLI_COMMAND *tbl, int argc, char **argv)
{
	int idx;
	`$INSTANCE_NAME`_CLIfunc fn;
	
	static char outBuffer[`$MAX_CLI_OUTPUT_BUFFER`];
	
	cystatus result;
	
	result = CYRET_UNKNOWN;
	fn = NULL;
	if (argc > 0) {
		/* look for the processed command */
		idx = 0;
		while ( strlen(tbl[idx].name ) > 0) {
			if ( strcmp(tbl[idx].name, argv[0]) == 0 ) {
				fn = tbl[idx].fn;
				if (fn != NULL) {
					result = fn(argc,argv);
				}
				else {
					result = CYRET_INVALID_OBJECT;
					sprintf(outBuffer,"\"%s\" has not yet been implemented.",argv[0]);
					`$INSTANCE_NAME`_SystemMsg(outBuffer,`$INSTANCE_NAME`_WARN);
				}
			}
			++idx;
		}
		
		if (result == CYRET_UNKNOWN) {
			sprintf(outBuffer,"Unknown Command \"%s\"",argv[0]);
			`$INSTANCE_NAME`_SystemMsg(outBuffer, `$INSTANCE_NAME`_ERROR);
		}
	}
	return result;
}
/* ======================================================================== */
	
void `$INSTANCE_NAME`_vCliTask( void *pvParameters )
{
	/* Buffer to hold received user input on the line */
	static char lineBuffer[`$MAX_CLI_INPUT_BUFFER`];
	
 	`$INSTANCE_NAME`_CLI_COMMAND *CommandTable;
	int idx;
	char lookahead;
	char ch;
	char *argv[25];
	int argc;
	
	
	/*
	 * Grab the adadress of the command table from the OS parameters
	 * passed to the task, and assign them to the local data used in the CLI
	 */
	CommandTable = (`$INSTANCE_NAME`_CLI_COMMAND*) pvParameters;

	/*
	 * User CLI initialization code for Performing any operations to setup
	 * special hardware or other item prior to the connection validation.
	 */
	/* `#START USER_CLI_INITIALIZATION_BEFORE_CONNECT` */
	
	/* `#END` */

	/*
	 * CLI Initialization:
	 * Wait for user input to confirm that the CLI has connected with a
	 * terminal.. essentially, since the USB port is open and connected
	 * the second it attaches, we must wait for user input to validate the
	 * connection, and make sure that the prompts are visible.
	 */
	`$INSTANCE_NAME`_GetChar();
	lineBuffer[0] = 0;
	/*
	 * The connection has been validated, this merge region allows the
	 * definition of functions that are performed once the connection
	 * is connected and valid. For Example, this might be a great place to
	 * add a welcome string, logon, or some other thing such as this.
	 */
	/* `#START USER_CLI_INITIALIZATION_AFTER_CONNECT` */
	
	/* `#END` */
	
	for(;;) {
		/*
		 * Wait for user input.
		 */
		`$INSTANCE_NAME`_CliShowPrompt(lineBuffer);
		
		/* Read the input line from the user with blocking functions */
		`$INSTANCE_NAME`_GetString( lineBuffer );
		
		/*
		 * Strip arguments from the line buffer, and handle compound
		 * statements by looping through the input buffer until the NULL
		 * is encountered.
		 */
		idx = 0;
		while ( lineBuffer[ idx ] != 0) {
			idx += `$INSTANCE_NAME`_CliGetArguments(&lineBuffer[idx],&argc,argv);
			if (argc > 0) {
				`$INSTANCE_NAME`_CliProcessCommand(CommandTable,argc,argv);
			}
		}
		
		/* Erase the contents of the line buffer at the end of the command */
		memset((void*)&lineBuffer[0],0,idx);
	}
}
/* ------------------------------------------------------------------------ */

#endif
/* ------------------------------------------------------------------------ */
/* [] END OF FILE */
