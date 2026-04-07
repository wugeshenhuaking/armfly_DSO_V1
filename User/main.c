/*
*********************************************************************************************************
*	                                  
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V3.0
*	说    明 : uCOS-III任务
*	修改记录 : 
*		版本号    日期         作者        说明
*		V1.0    2015-01-05    Eric2013     首发
*	    V2.0    2015-03-27    Eric2013     1. 升级固件库到V1.5.0
*                                          2. 升级BSP板级支持包 
*                                          3. 升级fatfs到0.11
*                                          4. 升级STemWin到5.26，注意这里使用的是非优化版本
*                                             优化版本和非优化版本的区别看这个帖子：
*                                             (实际测试ST公司发布的两种库STemWin526_CM4_OS_Keil_ot.lib和STemWin526_CM4_OS_Keil.lib的区别)
*                                             http://bbs.armfly.com/read.php?tid=7442          
*                                          5. 更改为新的四点触摸校准算法，并重查编排触摸检测任务
*                                          6. 添加7寸800*480分辨率电容屏支持。
*	    V3.0    2015-12-18  Eric2013       1. 升级BSP板级支持包 
*                                          2. 升级STemWin到5.28
*                                          3. 添加4.3寸和5寸电容屏驱动支持。但是此例程当前仅支持800*480分辨率。
*                                          4. 修改了bsp_adc.c文件里的TIM1初始化错误。
*                                          5. 修改了bsp_dac.c文件里的DAC初始化错误。
*                                             
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include <includes.h>
#include "MainTask.h"

/*
*********************************************************************************************************
*                                      宏定义
*********************************************************************************************************
*/
#define OSTCBSize sizeof(OS_TCB) / sizeof(CPU_STK) + 1

/*
*********************************************************************************************************
*                                  本文件使用的全局变量
*********************************************************************************************************
*/
static OS_TCB *AppTaskStartTCB;
static CPU_STK *AppTaskStartStk;

static OS_TCB *AppTaskUpdateTCB;
static CPU_STK *AppTaskUpdateStk;

static OS_TCB *AppTaskCOMTCB;
static CPU_STK *AppTaskCOMStk;

static OS_TCB *AppTaskUserIFTCB;
static CPU_STK *AppTaskUserIFStk;

static OS_TCB *AppTaskGUITCB;
static CPU_STK *AppTaskGUIStk;

static OS_TCB *AppTaskGUIRefreshTCB;
static CPU_STK *AppTaskGUIRefreshStk;

OS_SEM SEM_SYNCH; /* 此信号量用于实现按键按下后进行屏幕截图       */

/*
*********************************************************************************************************
*                                        本文件内部的函数
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg);
static void AppTaskCreate(void);
static void AppObjCreate(void);
static void AppTaskUserIF(void *p_arg);
static void AppTaskGUI(void *p_arg);
static void AppTaskGUIRefresh(void *p_arg);
static void AppTaskCOM(void *p_arg);

/*
*********************************************************************************************************
*	函 数 名: Stack_Init
*	功能说明: 任务和系统堆栈全部使用CCM，任务控制块也使用CCM空间。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Stack_Init(void)
{
    CPU_STK *STK_Temp;
    OS_TCB *TCB_Temp;

    /***********用户任务控制块，共6个任务的TCB*********************/
    TCB_Temp = (OS_TCB *)0x10000000;
    AppTaskStartTCB = TCB_Temp;
    TCB_Temp++;

    AppTaskUpdateTCB = TCB_Temp;
    TCB_Temp++;

    AppTaskCOMTCB = TCB_Temp;
    TCB_Temp++;

    AppTaskUserIFTCB = TCB_Temp;
    TCB_Temp++;

    AppTaskGUITCB = TCB_Temp;
    TCB_Temp++;

    AppTaskGUIRefreshTCB = TCB_Temp;
    TCB_Temp++;

    /***********系统任务控制块**os.h 1133行修改变量*******************/
    //#if (OS_CFG_TMR_EN > 0u)
    //	OSTmrTaskTCB = TCB_Temp;/***os_tmr***/
    //	TCB_Temp ++;
    //#endif
    //
    //	OSTickTaskTCB = TCB_Temp;/***os_stick***/
    //	TCB_Temp ++;
    //
    //#if OS_CFG_STAT_TASK_EN > 0u
    //	OSStatTaskTCB = TCB_Temp;/***os_stat***/
    //	TCB_Temp ++;
    //#endif
    //
    //#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    //	OSIntQTaskTCB = TCB_Temp;/***os_int***/
    //	TCB_Temp ++;
    //#endif
    //
    //	OSIdleTaskTCB = TCB_Temp;/***os_core 817行*/
    //	TCB_Temp ++;

    /***********用户任务堆栈空间*************************************/
    STK_Temp = (CPU_STK *)TCB_Temp;
    AppTaskStartStk = STK_Temp;

    STK_Temp = STK_Temp + APP_CFG_TASK_START_STK_SIZE;
    AppTaskUpdateStk = STK_Temp;

    STK_Temp = STK_Temp + APP_CFG_TASK_UPDATE_STK_SIZE;
    AppTaskCOMStk = STK_Temp;

    STK_Temp = STK_Temp + APP_CFG_TASK_COM_STK_SIZE;
    AppTaskUserIFStk = STK_Temp;

    STK_Temp = STK_Temp + APP_CFG_TASK_USER_IF_STK_SIZE;
    AppTaskGUIStk = STK_Temp;

    STK_Temp = STK_Temp + APP_CFG_TASK_GUI_STK_SIZE;
    AppTaskGUIRefreshStk = STK_Temp;

    /**********下面的6个是UCOS-III的内核服务程序*****************/
    STK_Temp = STK_Temp + APP_CFG_TASK_GUIRefresh_STK_SIZE;
    OSCfg_IdleTaskStk = STK_Temp;
    OSCfg_IdleTaskStkBasePtr = OSCfg_IdleTaskStk;
    STK_Temp = STK_Temp + OS_CFG_IDLE_TASK_STK_SIZE;

/***************中断队列任务*********************************/
#if (OS_CFG_ISR_POST_DEFERRED_EN > 0u)

    OSCfg_IntQTaskStk = STK_Temp;
    OSCfg_IntQTaskStkBasePtr = OSCfg_IntQTaskStk;
    STK_Temp = STK_Temp + OS_CFG_INT_Q_TASK_STK_SIZE;
#else
    OSCfg_IntQTaskStkBasePtr = (CPU_STK *)0;
#endif

/****************ISR堆栈*************************************/
#if (OS_CFG_ISR_STK_SIZE > 0u)
    OSCfg_ISRStk = STK_Temp;
    OSCfg_ISRStkBasePtr = OSCfg_ISRStk;
    STK_Temp = STK_Temp + OS_CFG_ISR_STK_SIZE;
#else
    OSCfg_ISRStkBasePtr = (CPU_STK *)0;
#endif

/**************统计任务*************************************/
#if (OS_CFG_STAT_TASK_EN > 0u)
    OSCfg_StatTaskStk = STK_Temp;
    OSCfg_StatTaskStkBasePtr = OSCfg_StatTaskStk;
    STK_Temp = STK_Temp + OS_CFG_STAT_TASK_STK_SIZE;
#else
    OSCfg_StatTaskStkBasePtr = (CPU_STK *)0;
#endif

    /****************滴答任务**********************************/
    OSCfg_TickTaskStk = STK_Temp;
    OSCfg_TickTaskStkBasePtr = OSCfg_TickTaskStk;
    STK_Temp = STK_Temp + OS_CFG_TICK_TASK_STK_SIZE;

/*****************定时器任务*******************************/
#if (OS_CFG_TMR_EN > 0u)
    OSCfg_TmrTaskStk = STK_Temp;
    OSCfg_TmrTaskStkBasePtr = OSCfg_TmrTaskStk;
    STK_Temp = STK_Temp + OS_CFG_TMR_TASK_STK_SIZE;
#else
    OSCfg_TmrTaskStkBasePtr = (CPU_STK *)0;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
    OS_ERR err;

    Stack_Init(); /* 堆栈初始化 */
    OSInit(&err); /* 初始化uC/OS-III */

    OSTaskCreate((OS_TCB *)AppTaskStartTCB, /* 创建启动任务 */
                 (CPU_CHAR *)"App Task Start",
                 (OS_TASK_PTR)AppTaskStart,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_START_PRIO,
                 (CPU_STK *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    OSStart(&err); /* 开始多任务的执行 */

    (void)&err;

    return (0);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskStart
*	功能说明: 这是一个启动任务，在多任务系统启动后，必须初始化滴答计数器(在BSP_Init中实现)
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 2
*********************************************************************************************************
*/
static void AppTaskStart(void *p_arg)
{
    OS_ERR err;
    uint8_t ucCount = 0;
    uint8_t ucCountGT811 = 0;
    uint8_t ucCountFT = 0;

    (void)p_arg;
    CPU_Init();
    bsp_Init();
    BSP_Tick_Init();

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    AppObjCreate();
    AppTaskCreate();

    while (1)
    {
        /* 1ms一次触摸扫描，电阻触摸屏 */
        if (g_tTP.Enable == 1)
        {
            TOUCH_Scan();
        }
        /* 10ms一次触摸扫描，电容触摸屏GT811 */
        else if (g_GT811.Enable == 1)
        {
            ucCountGT811++;
            if (ucCountGT811 == 10)
            {
                ucCountGT811 = 0;
                GT811_OnePiontScan();
            }
        }
        /* 10ms一次触摸扫描，电容触摸屏FT5X06 */
        else if (g_tFT5X06.Enable == 1)
        {
            ucCountFT++;
            if (ucCountFT == 10)
            {
                ucCountFT = 0;
                FT5X06_OnePiontScan();
            }
        }

        /* 10ms一次按键检测 */
        ucCount++;
        if (ucCount == 10)
        {
            ucCount = 0;
            bsp_KeyScan();
        }
        BSP_OS_TimeDlyMs(1);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUIUpdate
*	功能说明: 此任务主要实现截图功能.	
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 3
*********************************************************************************************************
*/
static void AppTaskGUIUpdate(void *p_arg)
{
    OS_ERR err;
    uint8_t Pic_Name = 0;
    char buf[20];
    CPU_BOOLEAN SemFlag;

    (void)p_arg;

    while (1)
    {
        SemFlag = BSP_OS_SemWait(&SEM_SYNCH, 0);

        if (SemFlag == DEF_OK)
        {
            sprintf(buf, "0:/PicSave/%d.bmp", Pic_Name);
            OSSchedLock(&err);

            /* 如果SD卡中没有PicSave文件，会进行创建 */
            result = f_mkdir("0:/PicSave");
            /* 创建截图 */
            result = f_open(&file, buf, FA_WRITE | FA_CREATE_ALWAYS);
            /* 向SD卡绘制BMP图片 */
            GUI_BMP_Serialize(_WriteByte2File, &file);

            /* 创建完成后关闭file */
            result = f_close(&file);

            OSSchedUnlock(&err);
            Pic_Name++;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCom
*	功能说明: 留待以后使用		
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：3
*********************************************************************************************************
*/
static void AppTaskCOM(void *p_arg)
{
    (void)p_arg;

    while (1)
    {
        /* 100s执行一次 */
        BSP_OS_TimeDlyMs(1000000);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskUserIF
*	功能说明: 留待以后使用。
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级: 2
*********************************************************************************************************
*/
static void AppTaskUserIF(void *p_arg)
{
    (void)p_arg; /* 避免编译器报警 */

    while (1)
    {
        /* 100s执行一次 */
        BSP_OS_TimeDlyMs(1000000);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUI
*	功能说明: GUI任务			  			  
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级：OS_CFG_PRIO_MAX - 4u
*********************************************************************************************************
*/
static void AppTaskGUI(void *p_arg)
{
    (void)p_arg; /* 避免编译器告警 */

    while (1)
    {
        MainTask();
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskGUIRefresh
*	功能说明: Led闪烁任务，表示系统运行	  			  
*	形    参: p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: OS_CFG_PRIO_MAX - 5u
*********************************************************************************************************
*/
static void AppTaskGUIRefresh(void *p_arg)
{
    (void)p_arg; /* 避免编译器告警 */

    while (1)
    {
        BSP_OS_TimeDlyMs(200);
        bsp_LedToggle(2);
    }
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate(void)
{
    OS_ERR err;

    /**************创建UPDATE任务*********************/
    OSTaskCreate((OS_TCB *)AppTaskUpdateTCB,
                 (CPU_CHAR *)"App Task Update",
                 (OS_TASK_PTR)AppTaskGUIUpdate,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_UPDATE_PRIO,
                 (CPU_STK *)&AppTaskUpdateStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_UPDATE_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_UPDATE_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    /**************创建COM任务*********************/
    OSTaskCreate((OS_TCB *)AppTaskCOMTCB,
                 (CPU_CHAR *)"App Task COM",
                 (OS_TASK_PTR)AppTaskCOM,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_COM_PRIO,
                 (CPU_STK *)&AppTaskCOMStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_COM_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_COM_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    /**************创建USER IF任务*********************/
    OSTaskCreate((OS_TCB *)AppTaskUserIFTCB,
                 (CPU_CHAR *)"App Task UserIF",
                 (OS_TASK_PTR)AppTaskUserIF,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_USER_IF_PRIO,
                 (CPU_STK *)&AppTaskUserIFStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_USER_IF_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_USER_IF_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    /**************创建GUI任务*********************/
    OSTaskCreate((OS_TCB *)AppTaskGUITCB,
                 (CPU_CHAR *)"App Task GUI",
                 (OS_TASK_PTR)AppTaskGUI,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_GUI_PRIO,
                 (CPU_STK *)&AppTaskGUIStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_GUI_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_GUI_STK_SIZE,
                 (OS_MSG_QTY)1,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);

    /**************创建GUI刷新任务*********************/
    OSTaskCreate((OS_TCB *)AppTaskGUIRefreshTCB,
                 (CPU_CHAR *)"App Task GUIRefresh",
                 (OS_TASK_PTR)AppTaskGUIRefresh,
                 (void *)0,
                 (OS_PRIO)APP_CFG_TASK_GUIRefresh_PRIO,
                 (CPU_STK *)&AppTaskGUIRefreshStk[0],
                 (CPU_STK_SIZE)APP_CFG_TASK_GUIRefresh_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_CFG_TASK_GUIRefresh_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR *)&err);
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建信号量
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate(void)
{
    /* 创建同步信号量 */
    BSP_OS_SemCreate(&SEM_SYNCH, 0, (CPU_CHAR *)"SEM_SYNCH");
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
