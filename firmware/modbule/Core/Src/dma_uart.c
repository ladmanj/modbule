/*
 * dma_uart.c
 *
 *  Created on: Jan 3, 2023
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */


#include <dma_uart.h>
#include <string.h>
#include <errno.h>
#include <stats.h>

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;

#define DMA_BUF_SIZE   64

uint8_t DMA_buffer[DMA_BUF_SIZE];

circ_buf rx_circ_buf = {0};

void init_dma_circ_buf(void)
{
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, DMA_buffer, DMA_BUF_SIZE);
  __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TC);
  __HAL_DMA_ENABLE_IT(&hdma_usart2_rx, DMA_IT_TE);
}

int read_received(void)
{
  if(rx_circ_buf.head == rx_circ_buf.tail)
  {
    return -1;
  }
  else
  {
    uint8_t c = rx_circ_buf.buffer[rx_circ_buf.tail];
    rx_circ_buf.tail = (uint16_t)(rx_circ_buf.tail + 1) % CIRC_BUF_SIZE;
    return c;
  }
}

int nr_received_bytes()
{
  return (uint16_t)(CIRC_BUF_SIZE + rx_circ_buf.head - rx_circ_buf.tail) % CIRC_BUF_SIZE;
}
void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart){
	init_dma_circ_buf();
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	if (HAL_UART_GetError(huart)) {
		HAL_UART_Abort_IT(huart);

		__HAL_UART_CLEAR_IDLEFLAG(huart);                 // Clear Idle IT-Flag
		__HAL_UART_CLEAR_PEFLAG(huart);
		__HAL_UART_CLEAR_FEFLAG(huart);
		__HAL_UART_CLEAR_NEFLAG(huart);
		__HAL_UART_CLEAR_OREFLAG(huart);
	}
	errno = ENOMSG;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == &huart2)
	{
		int32_t size = Size;
		int32_t offset = 0;

		static uint8_t second_half = 0;

		uint8_t *in_buffer = DMA_buffer;

		if(second_half == 1)
		{
			// first half already read
			size -= DMA_BUF_SIZE/2;
			in_buffer += DMA_BUF_SIZE/2;
		}

		if( huart->RxEventType == HAL_UART_RXEVENT_HT)
		{
			second_half = 1;
		}
		else second_half = 0;

		if (size > CIRC_BUF_SIZE) {
			// buffer overflow, data will be lost
			size = CIRC_BUF_SIZE;
		}

		if (rx_circ_buf.head + size > CIRC_BUF_SIZE)
		{
			offset = CIRC_BUF_SIZE - rx_circ_buf.head;
			memcpy (rx_circ_buf.buffer + rx_circ_buf.head, in_buffer, offset);
			rx_circ_buf.head = 0;
			size -= offset;
		}

		memcpy (rx_circ_buf.buffer + rx_circ_buf.head, in_buffer + offset, size);
		rx_circ_buf.head += size;

		if(second_half == 0)
		{
			HAL_UARTEx_ReceiveToIdle_DMA(&huart2, DMA_buffer, DMA_BUF_SIZE);
		}

#ifdef DEBUG
		record_max_avg(&stats[0], Size);
		record_max_avg(&stats[2], nr_received_bytes());
#endif
	}
}
