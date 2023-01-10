/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TCP_SERVER 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId udpechoTaskHandle;
osThreadId tcpechoTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//extern void tcpecho_init(void);
//extern void udpecho_init(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartUdpEcho(void const * argument);
void StartTcpEcho(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of udpechoTask */
  osThreadDef(udpechoTask, StartUdpEcho, osPriorityRealtime, 0, 1024);
  udpechoTaskHandle = osThreadCreate(osThread(udpechoTask), NULL);

  /* definition and creation of tcpechoTask */
  osThreadDef(tcpechoTask, StartTcpEcho, osPriorityRealtime, 0, 1024);
  tcpechoTaskHandle = osThreadCreate(osThread(tcpechoTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartUdpEcho */
/**
* @brief Function implementing the udpechoTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUdpEcho */
void StartUdpEcho(void const * argument)
{
  /* USER CODE BEGIN StartUdpEcho */
  struct netconn *conn;
  struct netbuf *buf;
  char buffer[128];
  err_t err;
  LWIP_UNUSED_ARG(argument);

  osDelay(100); // TODO: add semaphore

  /* create a new netbuf */
  struct netbuf *prefix;
  prefix = netbuf_new();

  /* reference the text into the netbuf */
  char text[] = "MESSAGE: ";
  netbuf_ref(prefix, text, sizeof(text));

#if LWIP_IPV6
  conn = netconn_new(NETCONN_UDP_IPV6);
  LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);
  netconn_bind(conn, IP6_ADDR_ANY, 8);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_UDP);
  LWIP_ERROR("udpecho: invalid conn", (conn != NULL), return;);
  netconn_bind(conn, IP_ADDR_ANY, 8);
#endif /* LWIP_IPV6 */
  /* Infinite loop */
  for(;;)
  {
	err = netconn_recv(conn, &buf);
	prefix->addr = buf->addr;
	prefix->port = buf->port;
	if (err == ERR_OK)
	{
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
	  /*  no need netconn_connect here, since the netbuf contains the address */
      if(netbuf_copy(buf, buffer, sizeof(buffer)) != buf->p->tot_len)
	  {
        LWIP_DEBUGF(LWIP_DBG_ON, ("netbuf_copy failed\n"));
	  }
	  else
	  {
		err = netconn_send(conn, prefix);
		err = netconn_send(conn, buf);

		if(err != ERR_OK)
		{
		  LWIP_DEBUGF(LWIP_DBG_ON, ("netconn_send failed: %d\n", (int)err));
		}
		else
		{
		  LWIP_DEBUGF(LWIP_DBG_ON, ("got %s\n", buffer));
		}
	  }
	  netbuf_delete(buf);
	  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
	}
  }
  /* USER CODE END StartUdpEcho */
}

/* USER CODE BEGIN Header_StartTcpEcho */
/**
* @brief Function implementing the tcpechoTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTcpEcho */
void StartTcpEcho(void const * argument)
{
  /* USER CODE BEGIN StartTcpEcho */
  struct netconn *conn, *newconn;
  err_t err, accept_err;
  struct netbuf *buf;
  void *data;
  u16_t len;
  int msg_cnt = 0;
  LWIP_UNUSED_ARG(argument);

  osDelay(100); // TODO: add semaphore

  /* Create a new connection identifier. */
  conn = netconn_new(NETCONN_TCP);

  if (conn!=NULL)
  {
	/* Bind connection to well known port number 7. */
	err = netconn_bind(conn, NULL, 7);

	if (err == ERR_OK)
	{
	  /* Tell connection to go into listening mode. */
	  netconn_listen(conn);
      /* Infinite loop */
	  for(;;)
	  {

		/* Grab new connection. */
		accept_err = netconn_accept(conn, &newconn);

		/* Process the new connection. */
		if (accept_err == ERR_OK)
		{

		  while (netconn_recv(newconn, &buf) == ERR_OK)
	      {
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);
		    msg_cnt++;
		    do
		    {
			  netbuf_data(buf, &data, &len);
			  char prefix[32];
			  int prefix_len = sprintf(prefix, "\r\nMessage #%03d: ", msg_cnt);
			  netconn_write(newconn, prefix, prefix_len, NETCONN_COPY);
			  netconn_write(newconn, data, len, NETCONN_COPY);
			  char postfix[] = "\r\n";
			  netconn_write(newconn, postfix, 2, NETCONN_NOFLAG);
		    } while (netbuf_next(buf) >= 0);
            netbuf_delete(buf);
  		    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0);
		  }

		  /* Close connection and discard connection identifier. */
		  netconn_close(newconn);
		  netconn_delete(newconn);
		}
	  }
	}
    else
    {
      netconn_delete(newconn);
    }
  }
  /* USER CODE END StartTcpEcho */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

