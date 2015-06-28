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
cystatus `$INSTANCE_NAME`_Write(uint8 value, uint8* fifo)
{
	cystatus result;
	
	/* Assign the default result to success */
	result = CYRET_SUCCESS;
	/* Make sure that the fifo was initialized */
	if (`$INSTANCE_NAME`_MAX(fifo) == 0) return CYRET_INVALID_OBJECT;
	
	if (`$INSTANCE_NAME`_SIZE(fifo) >= `$INSTANCE_NAME`_MAX(fifo) ) {
		/* FIFO is going to overflow, so drop the oldest data and retry */
		`$INSTANCE_NAME`_SIZE(fifo) = `$INSTANCE_NAME`_SIZE(fifo) - 1;
		`$INSTANCE_NAME`_READ_PTR(fifo) = `$INSTANCE_NAME`_READ_PTR(fifo) + 1;
		if (`$INSTANCE_NAME`_READ_PTR(fifo) >= `$INSTANCE_NAME`_MAX(fifo) ) {
			`$INSTANCE_NAME`_READ_PTR(fifo) = 0;
		}
		result = `$INSTANCE_NAME`_Write(value,fifo);
	}
	else {
		/* store the valeu in to the FIFO */
		`$INSTANCE_NAME`_INLINE_WRITE(value,fifo);
	}
	
	return result;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_Read(uint8* value, uint8* fifo)
{
	if (`$INSTANCE_NAME`_MAX(fifo) == 0) return CYRET_INVALID_OBJECT;	
	if (`$INSTANCE_NAME`_SIZE(fifo) == 0) return CYRET_EMPTY; /* No Data */
	
	/*
	 * read data from the FIFO memory block
	 */
	*value = `$INSTANCE_NAME`_DATA(fifo, `$INSTANCE_NAME`_READ_PTR(fifo));
	`$INSTANCE_NAME`_READ_PTR(fifo) = `$INSTANCE_NAME`_READ_PTR(fifo) + 1;
	if (`$INSTANCE_NAME`_READ_PTR(fifo) >= `$INSTANCE_NAME`_MAX(fifo)) {
		`$INSTANCE_NAME`_READ_PTR(fifo) = 0;
	}
	`$INSTANCE_NAME`_SIZE(fifo) = `$INSTANCE_NAME`_SIZE(fifo) - 1;
	
	return CYRET_SUCCESS;
}
/* ------------------------------------------------------------------------ */
cystatus `$INSTANCE_NAME`_Create( uint8 *fifo, uint16 array_size )
{
	if ((array_size <= 8)||(fifo==NULL)) return CYRET_BAD_PARAM;
	
	memset((void*)fifo,0L,array_size);
	`$INSTANCE_NAME`_MAX(fifo) = array_size - 8;
	return CYRET_SUCCESS;
}
/* ------------------------------------------------------------------------ */
void `$INSTANCE_NAME`_Flush( uint8 *fifo )
{
	uint16 max;
	
	/* quit on NULL pointers */
	if (fifo == NULL) return;
	
	/* Store the value of the old MAX */
	max = `$INSTANCE_NAME`_MAX(fifo);
	/* Clear the FIFO Header block */
	memset((void*)fifo,0,8);
	/* restore the MAX length setting */
	`$INSTANCE_NAME`_MAX(fifo) = max;
}
/* ------------------------------------------------------------------------ */
int `$INSTANCE_NAME`_DataReady( uint8* fifo )
{
	return `$INSTANCE_NAME`_SIZE(fifo);
}
/* ------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
/* [] END OF FILE */
