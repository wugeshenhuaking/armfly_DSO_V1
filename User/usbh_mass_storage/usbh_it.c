/*
*********************************************************************************************************
*
*	模块名称 : USB中断服务程序
*	文件名称 : usbh_it.c
*	版    本 : V1.0
*	说    明 : 本文件存放USB中断服务程序。只需将该文件加入工程即可，无需再到 stm32f4xx_it.c 中添加这些ISR程序
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "usb_bsp.h"
#include "usb_hcd_int.h"
#include "usbh_core.h"
#include  <includes.h>

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;
 

/**	过流检测，安富莱STM32-F4硬件不支持
  * @brief  EXTI1_IRQHandler
  *         This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
#if 0
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
      USB_Host.usr_cb->OverCurrentDetected();
      EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
#endif

/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();

    if (USB_OTG_IsHostMode(&USB_OTG_Core)) /* ensure that we are in device mode */
    {
        USBH_OTG_ISR_Handler(&USB_OTG_Core);
    }
    else
    {
        USBD_OTG_ISR_Handler(&USB_OTG_Core);
    }
    
    /* 在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 */
	OSIntExit(); 
}

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED 
/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();

    USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_Core);

    /* 在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 */
	OSIntExit();
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
    CPU_SR_ALLOC();
    
    CPU_CRITICAL_ENTER();  
    OSIntNestingCtr++;
    CPU_CRITICAL_EXIT();

    USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_Core);
   
    /* 在os_core.c文件里定义,如果有更高优先级的任务就绪了,则执行一次任务切换 */
	OSIntExit();
}
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
