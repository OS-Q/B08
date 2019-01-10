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


                                             /* ------------------------ MISCELLANEOUS ����-------------------------- */
#define OS_CFG_APP_HOOKS_EN             1u   /* Enable (1) or Disable (0) application specific hooks   ���Ӻ�������	  */
#define OS_CFG_ARG_CHK_EN               1u   /* Enable (1) or Disable (0) argument checking            ������鹦��   */
#define OS_CFG_CALLED_FROM_ISR_CHK_EN   1u   /* Enable (1) or Disable (0) check for called from ISR    �жϵ��ú���   */
#define OS_CFG_DBG_EN                   0u   /* Enable (1) debug code/variables                        debug����ʹ��  */
#define OS_CFG_ISR_POST_DEFERRED_EN     1u   /* Enable (1) or Disable (0) Deferred ISR posts           �жϷ�����ʽ	  */
#define OS_CFG_OBJ_TYPE_CHK_EN          0u   /* Enable (1) or Disable (0) object type checking         �������ͼ��   */
#define OS_CFG_TS_EN                    0u   /* Enable (1) or Disable (0) time stamping                TS��ʱ������� */

#define OS_CFG_PEND_MULTI_EN            1u   /* Enable (1) or Disable (0) code generation for multi-pend feature      */
											 /* ���¼��ȴ�����														  */			

#define OS_CFG_PRIO_MAX                32u   /* Defines the maximum number of task priorities (see OS_PRIO data type) */
											 /* ������ȼ�����														  */

#define OS_CFG_SCHED_LOCK_TIME_MEAS_EN  0u   /* Include code to measure scheduler lock time            ������ʱ���� */
#define OS_CFG_SCHED_ROUND_ROBIN_EN     1u   /* Include code for Round-Robin scheduling                ʱ��Ƭ��ת���� */
#define OS_CFG_STK_SIZE_MIN            64u   /* Minimum allowable task stack size                      �����ջ��Сֵ */


                                             /* ---------------------- EVENT FLAGS �¼����� ------------------------- */
#define OS_CFG_FLAG_EN                  0u   /* Enable (1) or Disable (0) code generation for EVENT FLAGS �¼���־����*/
#define OS_CFG_FLAG_DEL_EN              0u   /*     Include code for OSFlagDel()                 �¼���־ɾ������	  */
#define OS_CFG_FLAG_MODE_CLR_EN         0u   /*     Include code for Wait on Clear EVENT FLAGS   �¼���־ģʽ������� */
#define OS_CFG_FLAG_PEND_ABORT_EN       0u   /*     Include code for OSFlagPendAbort()           �¼���־�ȴ���ֹ���� */


                                             /* ------------------- MEMORY MANAGEMENT �ڴ���� ---------------------- */
#define OS_CFG_MEM_EN                   1u   /* Enable (1) or Disable (0) code generation for MEMORY MANAGER �ڴ���� */


                                             /* -------------- MUTUAL EXCLUSION SEMAPHORES �����ź��� --------------- */
#define OS_CFG_MUTEX_EN                 0u   /* Enable (1) or Disable (0) code generation for MUTEX  �����ź�������   */
#define OS_CFG_MUTEX_DEL_EN             0u   /*     Include code for OSMutexDel()               �����ź���ɾ������    */
#define OS_CFG_MUTEX_PEND_ABORT_EN      0u   /*     Include code for OSMutexPendAbort()         �����ź����ȴ���ֹ����*/


                                             /* --------------------- MESSAGE QUEUES ��Ϣ���� ----------------------- */
#define OS_CFG_Q_EN                     0u   /* Enable (1) or Disable (0) code generation for QUEUES  ��Ϣ���й���    */
#define OS_CFG_Q_DEL_EN                 0u   /*     Include code for OSQDel()                         ����ɾ������    */
#define OS_CFG_Q_FLUSH_EN               0u   /*     Include code for OSQFlush()                       ��Ϣ����ˢ�¹���*/
#define OS_CFG_Q_PEND_ABORT_EN          0u   /*     Include code for OSQPendAbort()                   ���еȴ���ֹ����*/


                                             /* ----------------------- SEMAPHORES �ź��� --------------------------- */
#define OS_CFG_SEM_EN                   1u   /* Enable (1) or Disable (0) code generation for SEMAPHORES �ź�������   */
#define OS_CFG_SEM_DEL_EN               1u   /*    Include code for OSSemDel()                      �ź���ɾ������    */
#define OS_CFG_SEM_PEND_ABORT_EN        1u   /*    Include code for OSSemPendAbort()                �ź����ȴ���ֹ����*/
#define OS_CFG_SEM_SET_EN               1u   /*    Include code for OSSemSet()                      �ź������ù���    */


                                             /* ------------------ TASK MANAGEMENT �������-------------------------- */
#define OS_CFG_STAT_TASK_EN             1u   /* Enable (1) or Disable(0) the statistics task  ͳ��������            */
#define OS_CFG_STAT_TASK_STK_CHK_EN     1u   /* Check task stacks from statistic task         ͳ�������ջ��鹦��    */

#define OS_CFG_TASK_CHANGE_PRIO_EN      0u   /* Include code for OSTaskChangePrio()           �������ȼ���������      */
#define OS_CFG_TASK_DEL_EN              1u   /* Include code for OSTaskDel()                  ����ɾ������            */
#define OS_CFG_TASK_Q_EN                0u   /* Include code for OSTaskQXXXX()                ������Ϣ���й���        */
#define OS_CFG_TASK_Q_PEND_ABORT_EN     1u   /* Include code for OSTaskQPendAbort()           ������Ϣ���еȴ���ֹ����*/

#define OS_CFG_TASK_PROFILE_EN          0u   /* Include variables in OS_TCB for profiling     ������ϸ״̬����        */
#define OS_CFG_TASK_REG_TBL_SIZE        1u   /* Number of task specific registers             �������⹦�ܼĴ�����С��*/
#define OS_CFG_TASK_SEM_PEND_ABORT_EN   0u   /* Include code for OSTaskSemPendAbort()         �����ź����ȴ���ֹ����  */
#define OS_CFG_TASK_SUSPEND_EN          1u   /* Include code for OSTaskSuspend() and OSTaskResume()     ���������  */


                                             /* --------------------- TIME MANAGEMENT ��ʱ���� ---------------------- */
#define OS_CFG_TIME_DLY_HMSM_EN         1u   /*     Include code for OSTimeDlyHMSM()           ��ʱ��������           */
#define OS_CFG_TIME_DLY_RESUME_EN       1u   /*     Include code for OSTimeDlyResume()         ��ʱȡ������           */


                                             /* -------------------- TIMER MANAGEMENT ��ʱ������ -------------------- */
#define OS_CFG_TMR_EN                   1u   /* Enable (1) or Disable (0) code generation for TIMERS     	��ʱ������*/
#define OS_CFG_TMR_DEL_EN               1u   /* Enable (1) or Disable (0) code generation for OSTmrDel()��ʱ��ɾ������*/

#endif