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
uint32_t OS_Get_Next_Task(void)
{
    int i=0;
    for (;i<OS_TASK_MAXIMUM_NUMBER;i++)
    {
       if(taskInfo[i].ucValidTag==1 && taskInfo[i].ucState==TASK_STATE_READY)
           return i;
    }
    return OS_IDLE_TASK_ID; //OS idle task
}
void OS_Idle_Task(void)
{
    static uint32_t uiIdleCounter=0;
    while (1)
    {
        uiIdleCounter++;
    }    
}
void OS_Init_Run(void)
{
//set current task to idle task
   iCurrentTask=7;
//set init PSP value
   SET_PSP_ASM(taskInfo[iCurrentTask].uiCurStackPos);
//switch from MSP to PSP
   Switch_to_PSP_ASM();
   
   OS_Idle_Task();
}
void PendSV_Handler(void)
{
//do context switch in PendSV handler
    iNextTask=OS_Get_Next_Task();
    if(iCurrentTask!=iNextTask)
    {
//need do context switch
        uiCSCounter++;
#ifdef SAVE_CPU_REGISTERS
        STORE_REGISTERS_PSP_ASM();
#endif
        uiCurrentPSP=GET_PSP_ASM();

        taskInfo[iCurrentTask].uiCurStackPos=uiCurrentPSP;

//update current task
        iCurrentTask=iNextTask;    
        SET_PSP_ASM(taskInfo[iCurrentTask].uiCurStackPos);

#ifdef SAVE_CPU_REGISTERS
        LOAD_REGISTERS_PSP_ASM();
#endif
    }
}

__asm uint32_t GET_PSP_ASM(void)
{
    MRS r0, psp
    BX lr
}
__asm void SET_PSP_ASM(uint32_t uiPSP)
{
    MSR psp, r0
    BX lr
}

__asm void STORE_REGISTERS_PSP_ASM(void)
{
    MRS r0, psp    
    STMDB r0!, {r4-r11}
    MSR psp, r0
    BX lr
}
__asm void LOAD_REGISTERS_PSP_ASM(void)
{
    MRS r0, psp
    LDMFD r0!, {r4-r11}    
    MSR psp, r0
    BX lr
}
/* [] END OF FILE */
