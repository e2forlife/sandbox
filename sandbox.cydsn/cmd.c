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

#include "cmd.h"



/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* Buffer to hold received user input on the line */
char CMD_lineBuffer[256];
char CMD_outBuffer[128];
int CMD_initVar;

/* ------------------------------------------------------------------------ */
void CMD_systemMsg(const char *str, uint8 level)
{
	switch(level) {
		case CMD_NOTE:
			COMIO_PrintStringColor("\r\n[NOTE]",15,0);
			COMIO_PrintString(": ");
			COMIO_PrintStringColor(str,2,0);
			break;
		case CMD_WARN:
			COMIO_PrintStringColor("\r\n[WARNING]",15,0);
			COMIO_PrintString(": ");
			COMIO_PrintStringColor(str,3,0);
			break;
		case CMD_ERROR:
			COMIO_PrintStringColor("\r\n[ERROR]",15,0);
			COMIO_PrintString(": ");
			COMIO_PrintStringColor(str,1,0);
			break;
		case CMD_FATAL:
			COMIO_PrintStringColor("\r\n[FATAL]",15,0);
			COMIO_PrintString(": ");
			COMIO_PrintStringColor(str,9,0);
			break;
		default:
			COMIO_PrintStringColor("\r\n[????]",15,0);
			COMIO_PrintString(": ");
			COMIO_PrintStringColor(str,5,0);
			break;
	}
}
/* ------------------------------------------------------------------------ */
void CMD_Help( const CMD_COMMAND *tbl )
{
	int idx;
	char bfr[51];
	
	COMIO_PrintString("\x1b[1;1H\x1b[2J");
	
	idx = 0;
	while ( strlen(tbl[idx].name) != 0) {
		if ( strlen(tbl[idx].desc) > 0 ) {
			sprintf(bfr,"\r\n[%10s]",tbl[idx].name);
			COMIO_PrintStringColor(bfr,15,0);
			COMIO_PrintString(" : ");
			COMIO_PrintStringColor(tbl[idx].desc, ((idx&0x01)?10:2),1);
		}
		++idx;
	}
	COMIO_PrintString("\r\n\n");
}
/* ------------------------------------------------------------------------ */
void CMD_ClearScreen( void )
{
	COMIO_PrintString("\x1b[1;1H\x1b[2J");
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
cystatus CMD_ProcessCommand(const CMD_COMMAND *tbl, int argc, char **argv)
{
	int idx;
	CMD_func fn;
	cystatus result;
	
	result = CYRET_UNKNOWN;
	fn = NULL;
	if (argc > 0) {
		/* look for the processed command */
		idx = 0;
		if (strcmp("HELP", argv[0]) == 0) {
			result = CYRET_SUCCESS;
			CMD_Help(tbl);
		}
		else if (strcmp("CLS",argv[0]) == 0) {
			result = CYRET_SUCCESS;
			CMD_ClearScreen();
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
						sprintf(CMD_outBuffer,"\"%s\" has not yet been implemented.",argv[0]);
						CMD_systemMsg(CMD_outBuffer,CMD_WARN);
					}
				}
				++idx;
			}
		}
		
		if (result == CYRET_UNKNOWN) {
			sprintf(CMD_outBuffer,"Unknown Command \"%s\"",argv[0]);
			CMD_systemMsg(CMD_outBuffer, CMD_ERROR);
		}
	}
	return result;
}
/* ------------------------------------------------------------------------ */
 void CMD_Shell( const CMD_COMMAND *tbl, uint8 refresh )
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
	if (CMD_initVar == 0) {
		CMD_initVar = 1;
		CMD_lineBuffer[0] = 0;
		refresh = 1;
	}
	
	/*
	 * when refresh is on, re-send the CLI prompt, along with the contents
	 * of the line buffer.  this is pretty useful when showing received data
	 * along with the CLi.
	 */
	if (refresh) {
		COMIO_PrintStringColor("\r\n\r\n[CLI]: ",15,0);
		COMIO_PrintString(CMD_lineBuffer);
	}
	comment = 0;
	result = COMIO_GetString( CMD_lineBuffer );
	if (result == CYRET_FINISHED) {
		len = strlen(CMD_lineBuffer);
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
				if ( isspace( (int) CMD_lineBuffer[idx]) ) {
					CMD_lineBuffer[idx] = 0;
					result = CYRET_STARTED;
				}
				/* 
				 * The end of a command can be the end of the buffer, or, a
				 * semicolon can be used for the creation of compound
				 * statements. When a semi is found, process the command +
				 * arguments that have been thus far parsed, AND reset the
				 * argument count to 0 to begin parsing the next command.
				 */
				else if (CMD_lineBuffer[idx] == ';') {
					/* process the command */
					CMD_lineBuffer[idx] = 0;
					if (argc > 0) {
						CMD_ProcessCommand(tbl,argc,argv);
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
					if (CMD_lineBuffer[idx] == '#') {
						comment = 1;
					} else if (comment == 0) {
						argv[argc++] = &CMD_lineBuffer[idx];
					}
					result = CYRET_FINISHED;
				}
					
			}
			/*
			 * Now that the arguments and the commands have been split
			 * in to an argmunet cound (argc), and argument strings (argV),
			 * process the command.
			 */
			CMD_ProcessCommand(tbl,argc,argv);
		}
		CMD_initVar = 0;
	}
}

/* ------------------------------------------------------------------------ */
/* [] END OF FILE */
