/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
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

 void CMD_Shell( const CMD_COMMAND *tbl );

#endif
/* [] END OF FILE */
