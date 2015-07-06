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
#if !defined(CMD_H)
	#define CMD_H

	#include <cytypes.h>
	
/*
 * read function/write function pointer types
 * These types are used to fill in the symbol table, which ultimately defines
 * the symbol table commands for execution.  defining a funtion as NULL will
 * disallow the mode of operation, writes will flag an error, but reads are
 * quiet.  The value is passed as a token type for reads (not an actual value)
 * so that the writer can handle the parsing of the value string for typing.
 * i.e. POWER wants on or off.
 */
typedef cystatus (*CMD_func)( int, char** );

typedef struct {
	char name[15];   /* Command name */
	CMD_func fn;     /* Parser callback to be executed */
	char desc[71];   /* ASCII description of function (for helper) */
} CMD_COMMAND;
	
#define CMD_NOTE       ( 0 )
#define CMD_WARN       ( 1 )
#define CMD_ERROR      ( 2 )
#define CMD_FATAL      ( 0xFF )

 void CMD_Shell( const CMD_COMMAND *tbl, uint8 refresh );

#endif
/* [] END OF FILE */
