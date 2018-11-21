/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2010; Micrium, Inc.; Weston, FL
*                          All rights reserved.  Protected by international copyright laws.
*
*                                                  CONFIGURATION  FILE
*
* File    : OS_CFG.H
* By      : JJL
* Version : V3.01.2
*
* LICENSING TERMS:
* ---------------
*               uC/OS-III is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
************************************************************************************************************************
*/

#ifndef OS_CFG_H
#define OS_CFG_H


                                             /* ------------------------ MISCELLANEOUS 杂项-------------------------- */
#define OS_CFG_APP_HOOKS_EN             1u   /* Enable (1) or Disable (0) application specific hooks   钩子函数功能	  */
#define OS_CFG_ARG_CHK_EN               1u   /* Enable (1) or Disable (0) argument checking            参数检查功能   */
#define OS_CFG_CALLED_FROM_ISR_CHK_EN   1u   /* Enable (1) or Disable (0) check for called from ISR    中断调用函数   */
#define OS_CFG_DBG_EN                   0u   /* Enable (1) debug code/variables                        debug功能使能  */
#define OS_CFG_ISR_POST_DEFERRED_EN     1u   /* Enable (1) or Disable (0) Deferred ISR posts           中断发布方式	  */
#define OS_CFG_OBJ_TYPE_CHK_EN          0u   /* Enable (1) or Disable (0) object type checking         对象类型检测   */
#define OS_CFG_TS_EN                    0u   /* Enable (1) or Disable (0) time stamping                TS是时间戳功能 */

#define OS_CFG_PEND_MULTI_EN            1u   /* Enable (1) or Disable (0) code generation for multi-pend feature      */
											 /* 多事件等待功能														  */			

#define OS_CFG_PRIO_MAX                32u   /* Defines the maximum number of task priorities (see OS_PRIO data type) */
											 /* 最大优先级数量														  */

#define OS_CFG_SCHED_LOCK_TIME_MEAS_EN  0u   /* Include code to measure scheduler lock time            调度锁时间检测 */
#define OS_CFG_SCHED_ROUND_ROBIN_EN     1u   /* Include code for Round-Robin scheduling                时间片轮转功能 */
#define OS_CFG_STK_SIZE_MIN            64u   /* Minimum allowable task stack size                      任务堆栈最小值 */


                                             /* ---------------------- EVENT FLAGS 事件管理 ------------------------- */
#define OS_CFG_FLAG_EN                  0u   /* Enable (1) or Disable (0) code generation for EVENT FLAGS 事件标志功能*/
#define OS_CFG_FLAG_DEL_EN              0u   /*     Include code for OSFlagDel()                 事件标志删除功能	  */
#define OS_CFG_FLAG_MODE_CLR_EN         0u   /*     Include code for Wait on Clear EVENT FLAGS   事件标志模式清除功能 */
#define OS_CFG_FLAG_PEND_ABORT_EN       0u   /*     Include code for OSFlagPendAbort()           事件标志等待终止功能 */


                                             /* ------------------- MEMORY MANAGEMENT 内存管理 ---------------------- */
#define OS_CFG_MEM_EN                   1u   /* Enable (1) or Disable (0) code generation for MEMORY MANAGER 内存管理 */


                                             /* -------------- MUTUAL EXCLUSION SEMAPHORES 互斥信号量 --------------- */
#define OS_CFG_MUTEX_EN                 0u   /* Enable (1) or Disable (0) code generation for MUTEX  互斥信号量功能   */
#define OS_CFG_MUTEX_DEL_EN             0u   /*     Include code for OSMutexDel()               互斥信号量删除功能    */
#define OS_CFG_MUTEX_PEND_ABORT_EN      0u   /*     Include code for OSMutexPendAbort()         互斥信号量等待终止功能*/


                                             /* --------------------- MESSAGE QUEUES 消息队列 ----------------------- */
#define OS_CFG_Q_EN                     0u   /* Enable (1) or Disable (0) code generation for QUEUES  消息队列功能    */
#define OS_CFG_Q_DEL_EN                 0u   /*     Include code for OSQDel()                         队列删除功能    */
#define OS_CFG_Q_FLUSH_EN               0u   /*     Include code for OSQFlush()                       消息队列刷新功能*/
#define OS_CFG_Q_PEND_ABORT_EN          0u   /*     Include code for OSQPendAbort()                   队列等待终止功能*/


                                             /* ----------------------- SEMAPHORES 信号量 --------------------------- */
#define OS_CFG_SEM_EN                   1u   /* Enable (1) or Disable (0) code generation for SEMAPHORES 信号量功能   */
#define OS_CFG_SEM_DEL_EN               1u   /*    Include code for OSSemDel()                      信号量删除功能    */
#define OS_CFG_SEM_PEND_ABORT_EN        1u   /*    Include code for OSSemPendAbort()                信号量等待终止功能*/
#define OS_CFG_SEM_SET_EN               1u   /*    Include code for OSSemSet()                      信号量设置功能    */


                                             /* ------------------ TASK MANAGEMENT 任务管理-------------------------- */
#define OS_CFG_STAT_TASK_EN             1u   /* Enable (1) or Disable(0) the statistics task  统计任务功能            */
#define OS_CFG_STAT_TASK_STK_CHK_EN     1u   /* Check task stacks from statistic task         统计任务堆栈检查功能    */

#define OS_CFG_TASK_CHANGE_PRIO_EN      0u   /* Include code for OSTaskChangePrio()           任务优先级调整功能      */
#define OS_CFG_TASK_DEL_EN              1u   /* Include code for OSTaskDel()                  任务删除功能            */
#define OS_CFG_TASK_Q_EN                0u   /* Include code for OSTaskQXXXX()                任务消息队列功能        */
#define OS_CFG_TASK_Q_PEND_ABORT_EN     1u   /* Include code for OSTaskQPendAbort()           任务消息队列等待终止功能*/

#define OS_CFG_TASK_PROFILE_EN          0u   /* Include variables in OS_TCB for profiling     任务详细状态功能        */
#define OS_CFG_TASK_REG_TBL_SIZE        1u   /* Number of task specific registers             任务特殊功能寄存器大小？*/
#define OS_CFG_TASK_SEM_PEND_ABORT_EN   0u   /* Include code for OSTaskSemPendAbort()         任务信号量等待终止功能  */
#define OS_CFG_TASK_SUSPEND_EN          1u   /* Include code for OSTaskSuspend() and OSTaskResume()     任务挂起功能  */


                                             /* --------------------- TIME MANAGEMENT 延时管理 ---------------------- */
#define OS_CFG_TIME_DLY_HMSM_EN         1u   /*     Include code for OSTimeDlyHMSM()           延时函数功能           */
#define OS_CFG_TIME_DLY_RESUME_EN       1u   /*     Include code for OSTimeDlyResume()         延时取消功能           */


                                             /* -------------------- TIMER MANAGEMENT 定时器管理 -------------------- */
#define OS_CFG_TMR_EN                   1u   /* Enable (1) or Disable (0) code generation for TIMERS     	定时器功能*/
#define OS_CFG_TMR_DEL_EN               1u   /* Enable (1) or Disable (0) code generation for OSTmrDel()定时器删除功能*/

#endif
