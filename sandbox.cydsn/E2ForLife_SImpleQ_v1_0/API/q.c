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
#include <string.h>

#include "`$INSTANCE_NAME`.h"

/*
 * Fifo Format:
 * byte 1:0  -  Fifo Length
 * byte 3:2  -  Maximum data length
 * byte 5:4  -  FIFO Read Ptr
 * byte 7:6  -  FIFO Write Ptr
 * Byte 8... -  FIFO Data
 */

/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_Write(uint8 value, uint8* q)
{
	cystatus result;
	
	/* Assign the default result to success */
	result = CYRET_SUCCESS;
	/* Make sure that the fifo was initialized */
	if (`$INSTANCE_NAME`_MAX(q) == 0) return CYRET_INVALID_OBJECT;
	
	if (`$INSTANCE_NAME`_SIZE(q) >= `$INSTANCE_NAME`_MAX(q) ) {
		/* FIFO is going to overflow, so drop the oldest data and retry */
		`$INSTANCE_NAME`_SIZE(q) = `$INSTANCE_NAME`_SIZE(q) - 1;
		`$INSTANCE_NAME`_READ_PTR(q) = `$INSTANCE_NAME`_READ_PTR(q) + 1;
		if (`$INSTANCE_NAME`_READ_PTR(q) >= `$INSTANCE_NAME`_MAX(q) ) {
			`$INSTANCE_NAME`_READ_PTR(q) = 0;
		}
		result = `$INSTANCE_NAME`_Write(value,q);
	}
	else {
		/* store the valeu in to the FIFO */
		`$INSTANCE_NAME`_INLINE_WRITE(value,q);
	}
	
	return result;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_Read(uint8* value, uint8* q)
{
	if (`$INSTANCE_NAME`_MAX(q) == 0) return CYRET_INVALID_OBJECT;	
	if (`$INSTANCE_NAME`_SIZE(q) == 0) return CYRET_EMPTY; /* No Data */
	
	/*
	 * read data from the FIFO memory block
	 */
	*value = `$INSTANCE_NAME`_DATA(q, `$INSTANCE_NAME`_READ_PTR(q));
	`$INSTANCE_NAME`_READ_PTR(q) = `$INSTANCE_NAME`_READ_PTR(q) + 1;
	if (`$INSTANCE_NAME`_READ_PTR(q) >= `$INSTANCE_NAME`_MAX(q)) {
		`$INSTANCE_NAME`_READ_PTR(q) = 0;
	}
	`$INSTANCE_NAME`_SIZE(q) = `$INSTANCE_NAME`_SIZE(q) - 1;
	
	return CYRET_SUCCESS;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_Create( uint8 *q, uint16 array_size )
{
	if ((array_size <= 8)||(q==NULL)) return CYRET_BAD_PARAM;
	
	memset((void*)q,0L,array_size);
	`$INSTANCE_NAME`_MAX(q) = array_size - 8;
	return CYRET_SUCCESS;
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_Flush( uint8 *q )
{
	uint16 max;
	
	/* quit on NULL pointers */
	if (q == NULL) return;
	
	/* Store the value of the old MAX */
	max = `$INSTANCE_NAME`_MAX(q);
	/* Clear the FIFO Header block */
	memset((void*)q,0,8);
	/* restore the MAX length setting */
	`$INSTANCE_NAME`_MAX(q) = max;
}
/* ------------------------------------------------------------------------ */
int `$INSTANCE_NAME`_DataReady( uint8* q )
{
	return `$INSTANCE_NAME`_SIZE(q);
}
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */

/* [] END OF FILE */
