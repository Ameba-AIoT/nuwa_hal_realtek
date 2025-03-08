/*
 * Copyright (c) 2024 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "os_wrapper.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(os_if_queue);

int rtos_queue_create(rtos_queue_t *pp_handle, uint32_t msg_num, uint32_t msg_size)
{
	struct k_msgq *p_queue;
	int status;

	if (pp_handle == NULL) {
		return FAIL;
	}

#if (CONFIG_HEAP_MEM_POOL_SIZE > 0)
	p_queue = (struct k_msgq *)k_malloc(sizeof(struct k_msgq));
	if (p_queue == NULL) {
		return FAIL;
	}
#else
	LOG_ERR("%s <<< k_malloc not support. >>>\n", __FUNCTION__);
	return FAIL;
#endif

	status = k_msgq_alloc_init(p_queue, msg_size, msg_num);
	if (status == 0) {
		k_free(p_queue);
		return FAIL;
	}

	*pp_handle = p_queue;
	return SUCCESS;
}

int rtos_queue_delete(rtos_queue_t p_handle)
{
	int status;
	struct k_msgq *p_queue = (struct k_msgq *)p_handle;

	if (p_handle == NULL) {
		return FAIL;
	}

	status = k_msgq_cleanup(p_handle);
	if (status != 0) {
		RTK_LOGS(NOTAG, RTK_LOG_ERROR, "%s <<< The queue is not empty, but the queue has been deleted. >>>\n", __FUNCTION__);
		k_free(p_handle);
		return FAIL;
	}

	k_free(p_handle);
	return SUCCESS;
}

uint32_t rtos_queue_message_waiting(rtos_queue_t p_handle)
{
	if (p_handle == NULL) {
		return FAIL;
	}

	return k_msgq_num_used_get(p_handle);
}

int rtos_queue_send(rtos_queue_t p_handle, void *p_msg, uint32_t wait_ms)
{
	BaseType_t ret;
	BaseType_t task_woken = pdFALSE;

	if (p_handle == NULL) {
		return FAIL;
	}

	int status;
	k_timeout_t wait_ticks;

	if (wait_ms == 0xFFFFFFFFUL) {
		wait_ticks = K_FOREVER;
	} else {
		wait_ticks = K_MSEC(wait_ms);
	}

	status = k_msgq_put(p_handle, p_msg, wait_ticks);
	if (status == 0) {
		return SUCCESS;
	} else {
		return FAIL;
	}
}

int rtos_queue_send_to_front(rtos_queue_t p_handle, void *p_msg, uint32_t wait_ms)
{
	ARG_UNUSED(p_handle);
	ARG_UNUSED(p_msg);
	ARG_UNUSED(wait_ms);
	LOG_ERR("%s Not Support\n", __FUNCTION__);
	return FAIL;
}

int rtos_queue_receive(rtos_queue_t p_handle, void *p_msg, uint32_t wait_ms)
{
	int status;
	k_timeout_t wait_ticks;

	if (wait_ms == 0xFFFFFFFFUL) {
		wait_ticks = K_FOREVER;
	} else {
		wait_ticks = K_MSEC(wait_ms);
	}

	if (p_handle == NULL) {
		return FAIL;
	}

	status = k_msgq_get(p_handle, p_msg, wait_ticks);
	if (status == 0) {
		return SUCCESS;
	} else {
		return FAIL;
	}
}

int rtos_queue_peek(rtos_queue_t p_handle, void *p_msg, uint32_t wait_ms)
{
	ARG_UNUSED(wait_ms);
	int status;
	if (p_handle == NULL) {
		return FAIL;
	}

	status = k_msgq_peek(p_handle, p_msg);
	if (status == 0) {
		return SUCCESS;
	} else {
		return FAIL;
	}
}
