/*
 * Copyright (c) 2017 Simon Goldschmidt
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */


#include <lwip/opt.h>
#include <lwip/arch.h>
#if !NO_SYS
#include "sys_arch.h"
#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>

#include <string.h>

int errno;


u32_t lwip_sys_now;

u32_t
sys_jiffies(void)
{
  if(__get_IPSR() != 0)
    lwip_sys_now = xTaskGetTickCountFromISR();
  else
    lwip_sys_now = xTaskGetTickCount();
  
  return lwip_sys_now;
}

u32_t
sys_now(void)
{
  if(__get_IPSR() != 0)
    lwip_sys_now = xTaskGetTickCountFromISR();
  else
    lwip_sys_now = xTaskGetTickCount();
  
  return lwip_sys_now;
}

void
sys_init(void)
{
  
  printf("system init ok!\n");
}

#if !NO_SYS

test_sys_arch_waiting_fn the_waiting_fn;

void
test_sys_arch_wait_callback(test_sys_arch_waiting_fn waiting_fn)
{
  the_waiting_fn = waiting_fn;
}

err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
  /* ���� sem */
  if(count == 1)    
    *sem = xSemaphoreCreateBinary();
  else
    *sem = xSemaphoreCreateCounting(count,count);
  
  if(*sem != SYS_SEM_NULL)
    return ERR_OK;
  else
  {
    printf("[sys_arch]:new sem fail!\n");
    return ERR_MEM;
  }
}

void
sys_sem_free(sys_sem_t *sem)
{
  /* ɾ�� sem */
  vSemaphoreDelete(*sem);
  *sem = SYS_SEM_NULL;
}


int sys_sem_valid(sys_sem_t *sem)                                               
{
  if (*sem == SYS_SEM_NULL)
    return 0;
  else
    return 1;                                       
}


void
sys_sem_set_invalid(sys_sem_t *sem)
{
  *sem = SYS_SEM_NULL;
}

/* 
 ���timeout������Ϊ�㣬�򷵻�ֵΪ
 �ȴ��ź��������ѵĺ����������
 �ź���δ��ָ��ʱ���ڷ����źţ�����ֵΪ
 SYS_ARCH_TIMEOUT������̲߳��صȴ��ź���
 �ú��������㡣 */
u32_t
sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;
  
  //�����ź����Ƿ���Ч
  if(*sem == SYS_SEM_NULL)
    return SYS_ARCH_TIMEOUT;
  
  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = sys_now();
  
  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  //һֱ����
  else
    wait_tick = portMAX_DELAY;
  
    //�жϵȴ��ź����������Ļ����Ƿ����ж���
  if(__get_IPSR() != 0 )
  {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreTakeFromISR(*sem, &pxHigherPriorityTaskWoken);
    //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
  }
  else
  {
    //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
    if(xSemaphoreTake(*sem, wait_tick) == pdTRUE)
      return (sys_now() - start_tick);
    else
      return SYS_ARCH_TIMEOUT;
  }
  return 0;
}

void
sys_sem_signal(sys_sem_t *sem)
{
  BaseType_t xReturn = pdPASS;
  //�ж��ͷ��ź����������Ļ����Ƿ����ж���
  if(__get_IPSR() != 0 )
  {
    BaseType_t pxHigherPriorityTaskWoken;
    //�ͷ��ź���
    xReturn = xSemaphoreGiveFromISR(*sem , &pxHigherPriorityTaskWoken);	
    
    //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
  }
  else
    xReturn = xSemaphoreGive( *sem );//������ֵ�ź���
  
  if(xReturn != pdTRUE)
    printf("[sys_arch]:sem signal fail!\n");
}

err_t
sys_mutex_new(sys_mutex_t *mutex)
{
  /* ���� sem */   
    *mutex = xSemaphoreCreateMutex();
  if(*mutex != SYS_MRTEX_NULL)
    return ERR_OK;
  else
  {
    printf("[sys_arch]:new mutex fail!\n");
    return ERR_MEM;
  }
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
  vSemaphoreDelete(*mutex);
}

void
sys_mutex_set_invalid(sys_mutex_t *mutex)
{
  *mutex = SYS_MRTEX_NULL;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
  xSemaphoreTake(*mutex,/* ��������� */
                 portMAX_DELAY); /* �ȴ�ʱ�� */
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
  xSemaphoreGive( *mutex );//����������
}


sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  sys_thread_t handle;
  BaseType_t xReturn = pdPASS;
  /* ����MidPriority_Task���� */
  xReturn = xTaskCreate((TaskFunction_t )function,  /* ������ں��� */
                        (const char*    )name,/* �������� */
                        (uint16_t       )stacksize,  /* ����ջ��С */
                        (void*          )arg,/* ������ں������� */
                        (UBaseType_t    )prio, /* ��������ȼ� */
                        (TaskHandle_t*  )&handle);/* ������ƿ�ָ�� */ 
  if(xReturn != pdPASS)
  {
    printf("[sys_arch]:create task fail!err:%#lx\n",xReturn);
    return NULL;
  }
  return handle;
}

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
    /* ����Test_Queue */
  *mbox = xQueueCreate((UBaseType_t ) size,/* ��Ϣ���еĳ��� */
                       (UBaseType_t ) sizeof(void *));/* ��Ϣ�Ĵ�С */
  
	if(NULL == *mbox)
    return ERR_MEM;
  
  return ERR_OK;
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
  vQueueDelete(*mbox);
}

int sys_mbox_valid(sys_mbox_t *mbox)          
{      
  if (*mbox == SYS_MBOX_NULL) 
    return 0;
  else
    return 1;
}   

void
sys_mbox_set_invalid(sys_mbox_t *mbox)
{
  *mbox = SYS_MBOX_NULL; 
}

void
sys_mbox_post(sys_mbox_t *q, void *msg)
{
  xQueueSend( *q, /* ��Ϣ���еľ�� */
              &msg,/* ���͵���Ϣ���� */
              portMAX_DELAY); /* �ȴ�ʱ�� */
}

err_t
sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  if(xQueueSend(*q,&msg,0) == pdPASS)  
    return ERR_OK;
  else
    return ERR_MEM;
}

err_t
sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
  return sys_mbox_trypost(q, msg);
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  u32_t wait_tick = 0;
  u32_t start_tick = 0 ;
  
  //�����ź����Ƿ���Ч
  if(*q == SYS_MBOX_NULL)
  {
    printf("[sys_arch]:SYS_MBOX_NULL!\n");
    return SYS_ARCH_TIMEOUT;
  }
  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = sys_now();
  
  //timeout != 0����Ҫ��ms����ϵͳ��ʱ�ӽ���
  if(timeout != 0)
  {
    //��msת����ʱ�ӽ���
    wait_tick = timeout / portTICK_PERIOD_MS;
    if (wait_tick == 0)
      wait_tick = 1;
  }
  //һֱ����
  else
    wait_tick = portMAX_DELAY;
  
    //�жϵȴ��ź����������Ļ����Ƿ����ж���
  if(__get_IPSR() != 0 )
  {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xQueueReceiveFromISR(*q,&(*msg),&pxHigherPriorityTaskWoken);
    //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
  }
  else
  {
    //�ȴ��ɹ�������ȴ���ʱ�䣬����ͱ�ʾ�ȴ���ʱ
    if(xQueueReceive(*q,&(*msg), wait_tick) == pdTRUE)
      return (sys_now() - start_tick);
    else
    {
      *msg = NULL;
      return SYS_ARCH_TIMEOUT;
    }
  }
  return 0;
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
  u32_t start_tick = 0 ;
  
  //�����ź����Ƿ���Ч
  if(*q == SYS_MBOX_NULL)
    return SYS_MBOX_EMPTY;
  
  //���Ȼ�ȡ��ʼ�ȴ��ź�����ʱ�ӽ���
  start_tick = sys_now();

    //�жϵȴ��ź����������Ļ����Ƿ����ж���
  if(__get_IPSR() != 0 )
  {
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    xQueueReceiveFromISR(*q,&(*msg),&pxHigherPriorityTaskWoken);
    //�����Ҫ�Ļ�����һ�������л���ϵͳ���ж��Ƿ���Ҫ�����л�
    portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
  }
  else
  {
    //�ȴ��ɹ�������ȴ���ʱ��
    if(xQueueReceive(*q,&(*msg), 0) == pdTRUE)
      return (sys_now() - start_tick);
    else
      return SYS_MBOX_EMPTY;
  }
  return 0;
}

#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */
