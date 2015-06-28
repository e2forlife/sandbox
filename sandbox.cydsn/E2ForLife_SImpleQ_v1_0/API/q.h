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


#define `$INSTANCE_NAME`_ARRAY_SIZE(s)                ( s + 8 )

/*
 * Fifo Format:
 * byte 1:0  -  Fifo Length
 * byte 3:2  -  Maximum data length
 * byte 5:4  -  FIFO Read Ptr
 * byte 7:6  -  FIFO Write Ptr
 * Byte 8... -  FIFO Data
 */

#define `$INSTANCE_NAME`_READ_PTR(f)              ( *((uint16*)&f[4]) )
#define `$INSTANCE_NAME`_WRITE_PTR(f)             ( *((uint16*)&f[6]) )
#define `$INSTANCE_NAME`_SIZE(f)                  ( *((uint16*)&f[0]) )
#define `$INSTANCE_NAME`_MAX(f)                   ( *((uint16*)&f[2]) )
#define `$INSTANCE_NAME`_DATA(f,i)                ( *((uint8*)&f[8+i]) )


/*
 * MACRO used to insert value (v) in to initialized fifo (f).  this does
 * 0 error checking and when the FIFO is full does not insert data to prevent
 * overflow, so there should be enough room in the fifo to allow for inserting
 * all of the expected bytes to prevent loss. mainly, use this for ISR handling
 * and use the interface functions for FIFO operations when not in ISR context.
 */
#define `$INSTANCE_NAME`_INLINE_WRITE(v,f) \
		if (`$INSTANCE_NAME`_SIZE(f) < `$INSTANCE_NAME`_MAX(f)) { \
    		`$INSTANCE_NAME`_DATA(f, `$INSTANCE_NAME`_WRITE_PTR(f)) = v;\
			`$INSTANCE_NAME`_WRITE_PTR(f) = `$INSTANCE_NAME`_WRITE_PTR(f) + 1;\
			if (`$INSTANCE_NAME`_WRITE_PTR(f) >= `$INSTANCE_NAME`_MAX(f)) {\
				`$INSTANCE_NAME`_WRITE_PTR(f) = 0;\
			}\
			`$INSTANCE_NAME`_SIZE(f) = `$INSTANCE_NAME`_SIZE(f) + 1;\
		}
		
		
#endif
/* [] END OF FILE */
