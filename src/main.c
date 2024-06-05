
#include "Mcal.h"
#include "S32K312.h"
#include "S32K312_MC_ME.h"
#include "Clock_Ip.h"
#include "Siul2_Port_Ip.h"
#include "Siul2_Dio_Ip.h"
#include "OsIf.h"
#include "Stm_Ip.h"
#include "IntCtrl_Ip.h"
#include "Lpuart_Uart_Ip.h"
#include "Lpuart_Uart_Ip_Irq.h"
#include <string.h>

/* Macro definitions*/
#define STM0_INST 0
#define STM0_CH0 0
#define MSG_LEN  50U
#define UART_LPUART_INTERNAL_CHANNEL  0

/* External Variables and Functions*/
extern ISR(STM_0_ISR);
extern ISR(LPUART_UART_IP_0_IRQHandler);

/* Global Variables*/
uint16_t Global_Counter = 0;
/* Welcome messages displayed at the console */
#define WELCOME_MSG_1 "Hello, This message is sent via Uart!\r\n"
#define WELCOME_MSG_2 "Have a nice day!\r\n"
/* Error message displayed at the console, in case data is received erroneously */
#define ERROR_MSG "An error occurred! The application will stop!\r\n"

void Stm0_Ch0_Callback (uint8 channel)
{
	(void)channel;
	if(Global_Counter == 0XFFFF){
		Global_Counter = 0;
	}
	Global_Counter++;
	return;
}

Lpuart_Uart_Ip_StatusType Send_Data(uint8 transChannel, const uint8* pBuffer, uint32 length)
{
    volatile Lpuart_Uart_Ip_StatusType lpuartStatus = LPUART_UART_IP_STATUS_ERROR;
    uint32_t remainingBytes;

    lpuartStatus = Lpuart_Uart_Ip_AsyncSend(transChannel, pBuffer, length);

    if (LPUART_UART_IP_STATUS_SUCCESS != lpuartStatus)
    {
    	lpuartStatus = LPUART_UART_IP_STATUS_ERROR;
    }

	do
	{
		lpuartStatus = Lpuart_Uart_Ip_GetTransmitStatus(UART_LPUART_INTERNAL_CHANNEL, &remainingBytes);
	} while (LPUART_UART_IP_STATUS_BUSY == lpuartStatus);

	return lpuartStatus;

}



int main(void)
{
	//Clock Initialization
    Clock_Ip_Init(&Clock_Ip_aClockConfig[0]);
    Clock_Ip_EnableModuleClock(SIUL2_CLK);

    //SIUL2 Port Configuration
    Siul2_Port_Ip_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    //LED
    Siul2_Port_Ip_SetOutputBuffer(PORTA_H_HALF, 30, TRUE, PORT_MUX_AS_GPIO); //PTA30 as GREEN_LED (Output)

    /*OsIf initialization */
    OsIf_Init(NULL);

    /* STM configuration, STM clock is determined by CGM settings default is FIRC 48MHz */
    /* STM timer is enabled here */
    /* STM pre-scaler = 48, so 1cnt=1uS*/
    Clock_Ip_EnableModuleClock(STM0_CLK);
    Stm_Ip_Init(STM0_INST, &STM_0_InitConfig_PB);

    /* Initialize STM0 compare channel 0 */
    Stm_Ip_InitChannel(STM0_INST, STM_0_ChannelConfig_PB);
    /* Start STM0 channel 0 ,set compare value, enable this channel, interrupt every 1ms */
    Stm_Ip_StartCounting (STM0_INST, STM0_CH0, 1000);

    /* Install IRQ handler for STM0 */
    IntCtrl_Ip_InstallHandler(STM0_IRQn , STM_0_ISR, NULL_PTR);
    /* Enable STM0 interrupt in NVIC */
    IntCtrl_Ip_EnableIrq(STM0_IRQn);

    //LPUART Initialization
    Clock_Ip_EnableModuleClock(LPUART0_CLK);
    Lpuart_Uart_Ip_Init(UART_LPUART_INTERNAL_CHANNEL, &Lpuart_Uart_Ip_xHwConfigPB_0);

    Lpuart_Uart_Ip_StatusType LpuartStatus = Send_Data(UART_LPUART_INTERNAL_CHANNEL, (const uint8 *)WELCOME_MSG_1, strlen(WELCOME_MSG_1));

    return 0;


}

