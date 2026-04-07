/*
*********************************************************************************************************
*	                                  
*	模块名称 : 主程序模块。
*	文件名称 : usb_bsp.c
*	版    本 : V1.0
*	说    明 : 例程
*	修改记录 : 
*		版本号  日期         作者        说明
*		v1.0    2013-05-02  armfly  ST固件库V2.1.0版本。
*
*	Copyright (C), 2013-2014
*   QQ超级群：216681322
*   BLOG: http://blog.sina.com.cn/u/2565749395	
*********************************************************************************************************
*/
#include  <includes.h>



/* 
* 安富莱STM32-V5 不支持USB HOST接口的供电控制和过流检测 
*/
#if 0
    #define HOST_OVRCURR_PORT                        GPIOE
    #define HOST_OVRCURR_LINE                        GPIO_Pin_1
    #define HOST_OVRCURR_PORT_SOURCE                 GPIO_PortSourceGPIOE
    #define HOST_OVRCURR_PIN_SOURCE                  GPIO_PinSource1
    #define HOST_OVRCURR_PORT_RCC                    RCC_APB2Periph_GPIOE
    #define HOST_OVRCURR_EXTI_LINE                   EXTI_Line1
    #define HOST_OVRCURR_IRQn                        EXTI1_IRQn 

    #ifdef USE_STM3210C_EVAL
         #define HOST_POWERSW_PORT_RCC               RCC_APB2Periph_GPIOC
         #define HOST_POWERSW_PORT                   GPIOC
         #define HOST_POWERSW_VBUS                   GPIO_Pin_9
    #else
        #ifdef USE_USB_OTG_FS 
            #define HOST_POWERSW_PORT_RCC            RCC_AHB1Periph_GPIOH
            #define HOST_POWERSW_PORT                GPIOH
            #define HOST_POWERSW_VBUS                GPIO_Pin_5
        #endif
    #endif

    #define HOST_SOF_OUTPUT_RCC                      RCC_APB2Periph_GPIOA
    #define HOST_SOF_PORT                            GPIOA
    #define HOST_SOF_SIGNAL                          GPIO_Pin_8

#endif

  
/*
*********************************************************************************************************
*	函 数 名: USB_OTG_BSP_Init
*	功能说明: Initilizes BSP configurations
*             保留官方的一些记录，方便以后查阅
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
/* USE_STM3210C_EVAL */ 
#ifdef USE_STM3210C_EVAL

    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE) ;

/* USE_STM322xG_EVAL */ 
#else 
    GPIO_InitTypeDef GPIO_InitStructure;
    /* 使用全速模式 */ 
    #ifdef USE_USB_OTG_FS 

        RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);  

        /* Configure SOF VBUS ID DM DP Pins */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8  | GPIO_Pin_11 | GPIO_Pin_12;  

        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
        GPIO_Init(GPIOA, &GPIO_InitStructure);  

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
        GPIO_Init(GPIOA, &GPIO_InitStructure);    
        
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_OTG1_FS) ;
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ; 
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;
        
        /* this for ID line debug */  
        
        GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;  
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);  
        GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS) ;   
        
        
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;
        /* 使用高速模式 */ 
        #else 
            /* ULPI 是否使用外接PHY芯片 */
            #ifdef USE_ULPI_PHY 
                RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | 
                                        RCC_AHB1Periph_GPIOB | 
                                        RCC_AHB1Periph_GPIOC | 
                                        RCC_AHB1Periph_GPIOH | 
                                        RCC_AHB1Periph_GPIOI, ENABLE);    
    
                GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_OTG2_HS) ; // D0
                GPIO_PinAFConfig(GPIOA,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // CLK
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // D1
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource1, GPIO_AF_OTG2_HS) ; // D2
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // D7
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_OTG2_HS) ; // D3
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_OTG2_HS) ; // D4
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_OTG2_HS) ; // D5
                GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_OTG2_HS) ; // D6
                GPIO_PinAFConfig(GPIOH,GPIO_PinSource4, GPIO_AF_OTG2_HS) ; // NXT
                GPIO_PinAFConfig(GPIOI,GPIO_PinSource11,GPIO_AF_OTG2_HS) ; // DIR
                GPIO_PinAFConfig(GPIOC,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // STP

                // CLK
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ; 
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_Init(GPIOA, &GPIO_InitStructure);  
                
                // D0
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  ; 
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
                GPIO_Init(GPIOA, &GPIO_InitStructure);  
                                             
                // D1 D2 D3 D4 D5 D6 D7
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1  |
                GPIO_Pin_5 | GPIO_Pin_10 | 
                GPIO_Pin_11| GPIO_Pin_12 | 
                GPIO_Pin_13 ;
                
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
                GPIO_Init(GPIOB, &GPIO_InitStructure);  
                
                // STP
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  ;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_Init(GPIOC, &GPIO_InitStructure);  
                
                //NXT  
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_Init(GPIOH, &GPIO_InitStructure);  
                                
                //DIR
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ; 
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                GPIO_Init(GPIOI, &GPIO_InitStructure);  
                                
                RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS | 
                                 RCC_AHB1Periph_OTG_HS_ULPI, ENABLE) ;
                                     
            /* 高速模式，没有外接PHY芯片 */
            #else

                #if 1
                    /* 安富莱只使用2根线 PB14/OTG_HS_DM 和 PB15/OTG_HS_DP 接U盘 */
                    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);
                    
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
                    
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);  
                    
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_OTG2_FS) ; 
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_OTG2_FS) ;
                    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS, ENABLE) ;  
                #else
                    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);
                    
                    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | 
                                                  GPIO_Pin_13 |
                                                  GPIO_Pin_14 | 
                    	                          GPIO_Pin_15;
                                                                     
                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
                    GPIO_Init(GPIOB, &GPIO_InitStructure);  
                    
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource12, GPIO_AF_OTG2_FS) ; 
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_OTG2_FS) ; 
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_OTG2_FS) ; 
                    GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_OTG2_FS) ;
                    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS, ENABLE) ;  	
                #endif
        #endif
    #endif 
#endif 

}

/*
*********************************************************************************************************
*	函 数 名: USB_OTG_BSP_EnableInterrupt
*	功能说明: Configures USB Global interrupt
*             保留官方的一些记录，方便以后查阅
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    #ifdef USE_USB_OTG_HS   
        NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
    #else
        NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;  
    #endif
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);  
    #ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
        NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);  
    
        NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);   
    #endif  
}

/*
*********************************************************************************************************
*	函 数 名: BSP_Drive_VBUS
*	功能说明: Drives the Vbus signal through IO
*             保留官方的一些记录，方便以后查阅
*             安富莱STM32-F4开发板不支持对外供电的控制
*	形    参：state : VBUS states
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
#if 0
    /*
    On-chip 5 V VBUS generation is not supported. For this reason, a charge pump 
    or, if 5 V are available on the application board, a basic power switch, must 
    be added externally to drive the 5 V VBUS line. The external charge pump can 
    be driven by any GPIO output. When the application decides to power on VBUS 
    using the chosen GPIO, it must also set the port power bit in the host port 
    control and status register (PPWR bit in OTG_FS_HPRT).
    
    Bit 12 PPWR: Port power
    The application uses this field to control power to this port, and the core 
    clears this bit on an overcurrent condition.
    */
  
    #ifndef USE_USB_OTG_HS   
        if (0 == state)
        { 
            /* DISABLE is needed on output of the Power Switch */
            GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
        }
        else
        {
            /*ENABLE the Power Switch by driving the Enable LOW */
            GPIO_ResetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
        }
    #endif 
     
#endif  
}

/*
*********************************************************************************************************
*	函 数 名: USB_OTG_BSP_ConfigVBUS
*	功能说明: Configures the IO for the Vbus and OverCurrent
*             保留官方的一些记录，方便以后查阅
*             安富莱STM32-F4开发板不支持对外供电的控制
*	形    参：state : VBUS states
*	返 回 值: 无
*********************************************************************************************************
*/
void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
#if 0	/* 安富莱STM32-F4 不支持VBUS */
    #ifndef USE_USB_OTG_HS 
        GPIO_InitTypeDef GPIO_InitStructure; 
          
        #ifdef USE_STM3210C_EVAL
            RCC_APB2PeriphClockCmd(HOST_POWERSW_PORT_RCC, ENABLE);
            
            
            /* Configure Power Switch Vbus Pin */
            GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
            GPIO_Init(HOST_POWERSW_PORT, &GPIO_InitStructure);
        #else
            #ifdef USE_USB_OTG_FS  
                RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOH , ENABLE);  
                
                GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
                GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
                GPIO_Init(HOST_POWERSW_PORT,&GPIO_InitStructure);
            #endif  
        #endif
    
      /* By Default, DISABLE is needed on output of the Power Switch */
      GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
      
      USB_OTG_BSP_mDelay(200);   /* Delay is need for stabilising the Vbus Low 
      in Reset Condition, when Vbus=1 and Reset-button is pressed by user */
    #endif  
#endif
}

/*
*********************************************************************************************************
*	函 数 名: USB_OTG_BSP_uDelay
*	功能说明: This function provides delay time in micro sec
*	形    参：usec : Value of delay required in micro sec
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_OTG_BSP_uDelay (const uint32_t usec)
{ 

	bsp_DelayUS(usec);
	
// 	uint32_t count = 0;
// 	
// 	const uint32_t utime = (80 * usec);
// 	do
// 	{
// 		if ( ++count > utime )
// 		{
// 			return ;
// 		}
// 	}
// 	while (1);
}

/*
*********************************************************************************************************
*	函 数 名: USB_OTG_BSP_mDelay
*	功能说明: This function provides delay time in milli sec
*	形    参：msec : Value of delay required in milli sec
*	返 回 值: 无
*********************************************************************************************************
*/
void USB_OTG_BSP_mDelay (const uint32_t msec)
{  
	//BSP_OS_TimeDlyMs(msec);
	USB_OTG_BSP_uDelay(msec * 1000);
}
