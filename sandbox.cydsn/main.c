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
#include <project.h>

int main()
{
	char buffer[21];
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	COMIO_Start();
	PWM_Start();
    /* CyGlobalIntEnable; */ /* Uncomment this line to enable global interrupts. */
    for(;;)
    {
        /* Place your application code here. */
		COMIO_Idle();
		COMIO_PutStringColor(" Please enter a command ",15,4);
		COMIO_PutStringColor(" : ",7,0);
		while( COMIO_GetString(buffer) != CYRET_FINISHED );
		COMIO_PutStringColor("\r\n\r\nThe quick brown fox says... ",2,0);
		COMIO_PutStringColor(buffer,10,0);
		buffer[0] = 0;
    }
}

/* [] END OF FILE */
