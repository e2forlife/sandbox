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
#if !defined(`$INSTANCE_NAME`_H)
	#define `$INSTANCE_NAME`_H
	
	
void `$INSTANCE_NAME`_Flush( uint8 *q );
cystatus `$INSTANCE_NAME`_Create( uint8 *q, uint16 array_size );
cystatus `$INSTANCE_NAME`_Read(uint8* value, uint8* q);
cystatus `$INSTANCE_NAME`_Write(uint8 value, uint8* q);
int `$INSTANCE_NAME`_DataReady( uint8* q );
uint8 `$INSTANCE_NAME`_Peek(uint8* q);


#define `$INSTANCE_NAME`_ARRAY_SIZE(s)                ( s + 8 )

/*
 * Fifo Format:
 * byte 1:0  -  Fifo Length
 * byte 3:2  -  Maximum data length
 * byte 5:4  -  FIFO Read Ptr
 * byte 7:6  -  FIFO Write Ptr
 * Byte 8... -  FIFO Data
 */
#define `$INSTANCE_NAME`_IDX_ReadPtr              ( 4 )
#define `$INSTANCE_NAME`_IDX_WritePtr             ( 6 )
#define `$INSTANCE_NAME`_IDX_Size                 ( 2 )
#define `$INSTANCE_NAME`_IDX_Max                  ( 0 )
#define `$INSTANCE_NAME`_IDX_QStart               ( 8 )

#define `$INSTANCE_NAME`_QReadPtr(q)              ( *((uint16*)&q[`$INSTANCE_NAME`_IDX_ReadPtr]) )
#define `$INSTANCE_NAME`_QWritePtr(q)             ( *((uint16*)&q[`$INSTANCE_NAME`_IDX_WritePtr]) )
#define `$INSTANCE_NAME`_QSize(q)                 ( *((uint16*)&q[`$INSTANCE_NAME`_IDX_Size]) )
#define `$INSTANCE_NAME`_QMax(q)                  ( *((uint16*)&q[`$INSTANCE_NAME`_IDX_Max]) )
#define `$INSTANCE_NAME`_QData(q,i)               ( *((uint8*)&q[`$INSTANCE_NAME`_IDX_QStart+i]) )


/*
 * MACRO used to insert value (v) in to initialized fifo (f).  this does
 * 0 error checking and when the FIFO is full does not insert data to prevent
 * overflow, so there should be enough room in the fifo to allow for inserting
 * all of the expected bytes to prevent loss. mainly, use this for ISR handling
 * and use the interface functions for FIFO operations when not in ISR context.
 */
#define `$INSTANCE_NAME`_INLINE_QWrite(v,q) \
		if (`$INSTANCE_NAME`_QSize(q) < `$INSTANCE_NAME`_QMax(q)) { \
    		`$INSTANCE_NAME`_QData(q, `$INSTANCE_NAME`_QWritePtr(q)) = v;\
			`$INSTANCE_NAME`_QWritePtr(q) = `$INSTANCE_NAME`_QWritePtr(q) + 1;\
			if (`$INSTANCE_NAME`_QWritePtr(q) >= `$INSTANCE_NAME`_QMax(q)) {\
				`$INSTANCE_NAME`_QWritePtr(q) = 0;\
			}\
			`$INSTANCE_NAME`_QSize(q) = `$INSTANCE_NAME`_QSize(q) + 1;\
		}
		
		
#endif
/* [] END OF FILE */
