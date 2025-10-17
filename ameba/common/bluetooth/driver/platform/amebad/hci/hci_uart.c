/*
 * Copyright (c) 2025 Realtek Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <osif.h>
#include <string.h>
#include <stdbool.h>
#include "ameba_soc.h"
#include "hci_uart.h"
#include "bt_debug.h"
#include "hal_platform.h"
#include "hci_platform.h"
#include "hci/hci_common.h"
#include <zephyr/irq.h>

#define HCI_UART_IDX  1      /* only [0, 1, 3] */

/*BT CTS   PA0  -----  RTS_PIN*/
/*BT TX    PA2  -----  RX_PIN*/
/*BT RX    PA4  -----  TX_PIN*/
#if (HCI_UART_IDX == 0)
#define HCI_UART_OUT
#define HCI_UART_DEV    UART0_DEV
#define HCI_UART_IRQ    UART0_IRQ
#if 1
#define HCI_TX_PIN  _PA_18
#define HCI_RX_PIN  _PA_19
#else
#define HCI_TX_PIN  _PA_21
#define HCI_RX_PIN  _PA_22
#endif
#elif (HCI_UART_IDX == 3)
#define HCI_UART_OUT
#define HCI_UART_DEV    UART3_DEV
#define HCI_UART_IRQ    UARTLP_IRQ
#define HCI_TX_PIN      _PA_26
#define HCI_RX_PIN      _PA_25
#else
#define HCI_UART_DEV    UART1_DEV
#define HCI_UART_IRQ    UART1_IRQ
#endif

#define HCI_UART_IRQ_PRIO        (INT_PRI_LOWEST)

static struct hci_uart_t {
	/* UART */
	UART_InitTypeDef UART_InitStruct;

	/* UART TX */
	uint8_t         *tx_buf;
	uint16_t         tx_len;
	void            *tx_done_sem;
} *g_uart = NULL;

uint8_t hci_uart_set_bdrate(uint32_t baudrate)
{
	UART_SetBaud(HCI_UART_DEV, baudrate);
	BT_LOGA("Set baudrate to %d success!\r\n", baudrate);
	return HCI_SUCCESS;
}

static inline void transmit_chars(void)
{
	uint16_t cnt = 0;

	if (g_uart->tx_len == 0) { /* Set TX done after TX FIFO is empty. */
		UART_INTConfig(HCI_UART_DEV, RUART_BIT_ETBEI, DISABLE);
		if (g_uart->tx_done_sem) {
			osif_sem_give(g_uart->tx_done_sem);
		}
		return;
	}

	/* Send data to TX FIFO */
	cnt = (uint16_t)UART_SendDataTO(HCI_UART_DEV, g_uart->tx_buf, g_uart->tx_len, 0);
	g_uart->tx_buf += cnt;
	g_uart->tx_len -= cnt;
}

static inline void receive_chars(void)
{
	hci_uart_rx_irq_handler(true);
}

static uint32_t _uart_irq(void *data)
{
	(void)data;
	volatile uint8_t reg_iir;
	uint8_t int_id;
	uint32_t reg_val;

	reg_iir = UART_IntStatus(HCI_UART_DEV);
	if ((reg_iir & RUART_IIR_INT_PEND) != 0) {
		/* No pending IRQ */
		return 0;
	}

	int_id = (reg_iir & RUART_IIR_INT_ID) >> 1;
	switch (int_id) {
	case RUART_LP_RX_MONITOR_DONE:
		reg_val = UART_RxMonitorSatusGet(HCI_UART_DEV);
		BT_LOGD("monitor done\r\n");
		break;
	case RUART_MODEM_STATUS:
		reg_val = UART_ModemStatusGet(HCI_UART_DEV);
		break;
	case RUART_TX_FIFO_EMPTY:
		transmit_chars();
		break;
	case RUART_RECEIVER_DATA_AVAILABLE:
	case RUART_TIME_OUT_INDICATION:
		receive_chars();
		break;
	case RUART_RECEIVE_LINE_STATUS:
		reg_val = (UART_LineStatusGet(HCI_UART_DEV));
		if (reg_val & RUART_LINE_STATUS_ERR_OVERRUN) {
			BT_LOGD("LSR over run interrupt\r\n");
		}

		if (reg_val & RUART_LINE_STATUS_ERR_PARITY) {
			BT_LOGD("LSR parity error interrupt\r\n");
		}

		if (reg_val & RUART_LINE_STATUS_ERR_FRAMING) {
			BT_LOGD("LSR frame error (stop bit error) interrupt\r\n");
		}

		if (reg_val & RUART_LINE_STATUS_ERR_BREAK) {
			BT_LOGD("LSR break error interrupt\r\n");
		}
		break;
	default:
		BT_LOGD("Unknown interrupt type %u \r\n", int_id);
		break;
	}

	return 0;
}

uint16_t hci_uart_send(uint8_t *buf, uint16_t len)
{
	if (!g_uart) {
		BT_LOGE("g_uart is NULL!\r\n");
		return 0;
	}

	/* UART_SendData() does not work */
	g_uart->tx_buf = buf;
	g_uart->tx_len = len;

	UART_INTConfig(HCI_UART_DEV, RUART_IER_ETBEI, ENABLE);

	if (g_uart->tx_done_sem) {
		if (osif_sem_take(g_uart->tx_done_sem, 0xFFFFFFFF) == false) {
			BT_LOGE("g_uart->tx_done_sem take fail!\r\n");
			return 0;
		}
	}

	/* Trigger TX Empty Interrrupt, so TX done here */
	return len;
}

uint16_t hci_uart_read(uint8_t *buf, uint16_t len)
{
	if (len) {
		return UART_ReceiveDataTO(HCI_UART_DEV, buf, len, 0);
	}

	return 0;
}

void hci_uart_rx_irq(bool enable)
{
	UART_INTConfig(HCI_UART_DEV, RUART_BIT_ERBI | RUART_BIT_ETOI, enable ? ENABLE : DISABLE);
}

uint8_t hci_uart_open(void)
{
	/* Init g_uart */
	if (!g_uart) {
		g_uart = (struct hci_uart_t *)osif_mem_alloc(RAM_TYPE_DATA_ON, sizeof(struct hci_uart_t));
		if (!g_uart) {
			BT_LOGE("g_uart is NULL!\r\n");
			return HCI_FAIL;
		}
		memset(g_uart, 0, sizeof(struct hci_uart_t));
	}

	if (osif_sem_create(&g_uart->tx_done_sem, 0, 1) == false) {
		BT_LOGE("g_uart->tx_done_sem create fail!\r\n");
		osif_mem_free(g_uart);
		return HCI_FAIL;
	}

#ifdef HCI_UART_OUT
	/* PINMUX THE PIN */
	Pinmux_Config(HCI_TX_PIN, PINMUX_FUNCTION_UART);
	Pinmux_Config(HCI_RX_PIN, PINMUX_FUNCTION_UART);
	Pinmux_Config(HCI_RTS_PIN, PINMUX_FUNCTION_UART_RTSCTS);
	Pinmux_Config(HCI_CTS_PIN, PINMUX_FUNCTION_UART_RTSCTS);

	PAD_PullCtrl(HCI_TX_PIN, GPIO_PuPd_UP);
	PAD_PullCtrl(HCI_RX_PIN, GPIO_PuPd_NOPULL);
#endif

	/* Enable UART
	 * Use Flow Control (When rx FIFO reaches level, RTS will be pulled high)
	 * Use Baudrate 115200 (Default)
	 */
	UART_InitTypeDef *pUARTStruct = &g_uart->UART_InitStruct;
	UART_StructInit(pUARTStruct);
	pUARTStruct->WordLen = RUART_WLS_8BITS;
	pUARTStruct->StopBit = RUART_STOP_BIT_1;
	pUARTStruct->Parity = RUART_PARITY_DISABLE;
	pUARTStruct->ParityType = RUART_EVEN_PARITY;
	pUARTStruct->StickParity = RUART_STICK_PARITY_DISABLE;
	pUARTStruct->RxFifoTrigLevel = UART_RX_FIFOTRIG_LEVEL_14BYTES;
	pUARTStruct->FlowControl = ENABLE;
	UART_Init(HCI_UART_DEV, pUARTStruct);
	UART_ClearTxFifo(HCI_UART_DEV);
	UART_SetBaud(HCI_UART_DEV, 115200);

	/* Disable and Enable UART Interrupt */
	InterruptDis(HCI_UART_IRQ);
	/* InterruptUnRegister(HCI_UART_IRQ); */
	irq_disconnect_dynamic(HCI_UART_IRQ, HCI_UART_IRQ_PRIO, (void (*)(const void *))_uart_irq, NULL, 0);
	/* InterruptRegister((IRQ_FUN)_uart_irq, HCI_UART_IRQ, NULL, HCI_UART_IRQ_PRIO); */
	irq_connect_dynamic(HCI_UART_IRQ, HCI_UART_IRQ_PRIO, (void (*)(const void *))_uart_irq, NULL, 0);
	InterruptEn(HCI_UART_IRQ, HCI_UART_IRQ_PRIO);

	UART_INTConfig(HCI_UART_DEV, RUART_IER_ETBEI, DISABLE);
	UART_INTConfig(HCI_UART_DEV, RUART_IER_ERBI | RUART_IER_ETOI | RUART_IER_ELSI, ENABLE);

	/* Enable Uart High Rate Rx Path */
	UART_RxCmd(HCI_UART_DEV, ENABLE);

	return HCI_SUCCESS;
}

uint8_t hci_uart_close(void)
{
	if (!g_uart) {
		return HCI_FAIL;
	}

	/* Disable UART Interrupt and UART */
	InterruptDis(HCI_UART_IRQ);
	/* InterruptUnRegister(HCI_UART_IRQ); */
	irq_disconnect_dynamic(HCI_UART_IRQ, HCI_UART_IRQ_PRIO, (void (*)(const void *))_uart_irq, NULL, 0);
	UART_DeInit(HCI_UART_DEV);

	return HCI_SUCCESS;
}

uint8_t hci_uart_free(void)
{
	if (!g_uart) {
		return HCI_FAIL;
	}

	if (g_uart->tx_done_sem) {
		osif_sem_delete(g_uart->tx_done_sem);
		g_uart->tx_done_sem = NULL;
	}

	osif_mem_free(g_uart);
	g_uart = NULL;

	return HCI_SUCCESS;
}
