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

#include "`$INSTANCE_NAME`.h"



/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Buffer to hold received user input on the line */
char `$INSTANCE_NAME`_CLIlineBuffer[256];
char `$INSTANCE_NAME`_CLIoutBuffer[128];

int `$INSTANCE_NAME`_CLIrefresh;

/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_SystemmMsg(const char *str, uint8 level)
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
void `$INSTANCE_NAME`_CliHelp( const `$INSTANCE_NAME`_CLI_COMMAND *tbl )
{
	int idx;
	char bfr[51];
	
	`$INSTANCE_NAME`_PrintString("\x1b[1;1H\x1b[2J");
	
	idx = 0;
	while ( strlen(tbl[idx].name) != 0) {
		if ( strlen(tbl[idx].desc) > 0 ) {
			sprintf(bfr,"\r\n[%10s]",tbl[idx].name);
			`$INSTANCE_NAME`_PrintStringColor(bfr,15,0);
			`$INSTANCE_NAME`_PrintString(" : ");
			`$INSTANCE_NAME`_PrintStringColor(tbl[idx].desc, ((idx&0x01)?10:2),1);
		}
		++idx;
	}
	`$INSTANCE_NAME`_PrintString("\r\n\n");
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_CliClearScreen( void )
{
	`$INSTANCE_NAME`_PrintString("\x1b[1;1H\x1b[2J");
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
	cystatus result;
	
	result = CYRET_UNKNOWN;
	fn = NULL;
	if (argc > 0) {
		/* look for the processed command */
		idx = 0;
		if (strcmp("HELP", argv[0]) == 0) {
			result = CYRET_SUCCESS;
			`$INSTANCE_NAME`_CliHelp(tbl);
		}
		else if (strcmp("CLS",argv[0]) == 0) {
			result = CYRET_SUCCESS;
			`$INSTANCE_NAME`_CliClearScreen();
		}
		else {
			while ( strlen(tbl[idx].name ) > 0) {
				if ( strcmp(tbl[idx].name, argv[0]) == 0 ) {
					fn = tbl[idx].fn;
					if (fn != NULL) {
						result = fn(argc,argv);
					}
					else {
						result = CYRET_INVALID_OBJECT;
						sprintf(`$INSTANCE_NAME`_CLIoutBuffer,"\"%s\" has not yet been implemented.",argv[0]);
						`$INSTANCE_NAME`_SystemMsg(`$INSTANCE_NAME`_CLIoutBuffer,`$INSTANCE_NAME`_WARN);
					}
				}
				++idx;
			}
		}
		
		if (result == CYRET_UNKNOWN) {
			sprintf(`$INSTANCE_NAME`_CLIoutBuffer,"Unknown Command \"%s\"",argv[0]);
			`$INSTANCE_NAME`_SystemMsg(`$INSTANCE_NAME`_CLIoutBuffer, `$INSTANCE_NAME`_ERROR);
		}
	}
	return result;
}
/* ------------------------------------------------------------------------ */
 void `$INSTANCE_NAME`_CliIdle( const `$INSTANCE_NAME`_CLI_COMMAND *tbl, uint8 refresh )
{
	cystatus result;
	int idx;
	int len;
	int argc;
	char* argv[25];
	int comment;
	
	/*
	 * read data from the COM port (USBUART) in to a line buffer without
	 * blocking. Then, when the user has pressed enter, process the data
	 * to split the arguments and commands for the line of text.
	 */
	if (`$INSTANCE_NAME`_CLIrefresh == 0) {
		`$INSTANCE_NAME`_CLIrefresh = 1;
		`$INSTANCE_NAME`_CLIlineBuffer[0] = 0;
		refresh = 1;
	}
	
	/*
	 * when refresh is on, re-send the CLI prompt, along with the contents
	 * of the line buffer.  this is pretty useful when showing received data
	 * along with the CLi.
	 */
	if (refresh) {
		`$INSTANCE_NAME`_PrintStringColor("\r\n\r\n[CLI]: ",15,0);
		`$INSTANCE_NAME`_PrintString(`$INSTANCE_NAME`_CLIlineBuffer);
	}
	comment = 0;
	result = `$INSTANCE_NAME`_GetString( `$INSTANCE_NAME`_CLIlineBuffer );
	if (result == CYRET_FINISHED) {
		len = strlen(`$INSTANCE_NAME`_CLIlineBuffer);
		if (len > 0) {
			/* 
			 * Initialize the argument count to 0 since there are presently
			 * no commands to process.  Also set the operation status to
			 * started to signal that the location of the argument/cmd was
			 * not yet stored.
			 */
			argc = 0;
			result = CYRET_STARTED;
			for( idx = 0;idx<len;++idx) {
				/*
				 * when a space is detected, replace it withe a NULL to
				 * seperate the string from the line. When a trailing space is
				 * at the end of the line, just ignore that argument. Set the
				 * parse status to started to singal that data needs to be
				 * stored for the next argument.
				 */
				if ( isspace( (int) `$INSTANCE_NAME`_CLIlineBuffer[idx]) ) {
					`$INSTANCE_NAME`_CLIlineBuffer[idx] = 0;
					result = CYRET_STARTED;
				}
				/* 
				 * The end of a command can be the end of the buffer, or, a
				 * semicolon can be used for the creation of compound
				 * statements. When a semi is found, process the command +
				 * arguments that have been thus far parsed, AND reset the
				 * argument count to 0 to begin parsing the next command.
				 */
				else if (`$INSTANCE_NAME`_CLIlineBuffer[idx] == ';') {
					/* process the command */
					`$INSTANCE_NAME`_CLIlineBuffer[idx] = 0;
					if (argc > 0) {
						`$INSTANCE_NAME`_CliProcessCommand(tbl,argc,argv);
					}
					/* Start storing the next command */
					argc = 0;
					result = CYRET_STARTED;
				}
				/*
				 * When there was no space, tab or semicolon, AND the parser
				 * stateus is started, store the start of the token in the
				 * argument vector list, and increase the argument count. Also
				 * set the parse state to finished to signal that the argument
				 * start has been saved.
				 */
				else if (result == CYRET_STARTED) {
					if (`$INSTANCE_NAME`_CLIlineBuffer[idx] == '#') {
						comment = 1;
					} else if (comment == 0) {
						argv[argc++] = &`$INSTANCE_NAME`_CLIlineBuffer[idx];
					}
					result = CYRET_FINISHED;
				}
					
			}
			/*
			 * Now that the arguments and the commands have been split
			 * in to an argmunet cound (argc), and argument strings (argV),
			 * process the command.
			 */
			`$INSTANCE_NAME`_CliProcessCommand(tbl,argc,argv);
		}
		`$INSTANCE_NAME`_CLIrefresh = 0;
	}
}
/* ======================================================================== */
#if (`$vCliTask` == 1)
/* ------------------------------------------------------------------------ */
	#include "`$FreeRTOS`.h"
	#include "`$FreeRTOS`_task.h"
	
void vCliTask( void *pvParameters )
{
 	static `$INSTANCE_NAME`_CLI_COMMAND *CommandTable;
	
	/*
	 * Grab the adadress of the command table from the OS parameters
	 * passed to the task, and assign them to the local data used in the CLI
	 */
	CommandTable = (`$INSTANCE_NAME`_CLI_COMMAND*) pvParameters;
	
	/*
	 * CLI Initialization
	 */
	
	/*
	 * User CLI initialization code for registering commands, initialization
	 * of application specific data, or othr stuff (like initialization of
	 * perepherals) required before entering the main task body.
	 */
	/* `#START USER_CLI_INITIALIZATION` */
	
	/* `#END` */
	
	for(;;) {
		/*
		 * Execute Idle processing for hte USB loop.
		 */
		`$INSTANCE_NAME`_Idle();
		
		/*
		 * Execute the shell processor code
		 */
		`$INSTANCE_NAME`_CliIdle(CommandTable,0);
	}
}
/* ------------------------------------------------------------------------ */
#endif

#endif
/* ------------------------------------------------------------------------ */
/* [] END OF FILE */
