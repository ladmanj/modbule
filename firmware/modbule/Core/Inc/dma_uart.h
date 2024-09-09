/*
 * dma_uart.h
 *
 *  Created on: Jan 3, 2023
 *      Author: Jakub Ladman ladmanj@volny.cz
 *      SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef DMA_UART_H_
#define DMA_UART_H_

#include "modbule_mcu.h"
#include <stdint.h>

#define CIRC_BUF_SIZE 512

typedef struct
{
  uint8_t buffer[CIRC_BUF_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} circ_buf;

int read_received(void);
void init_dma_circ_buf(void);
int nr_received_bytes();

#endif /* DMA_UART_H_ */
