/*
 * Copyright (c) 2026 Realtek Semiconductor Corp.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AMEBA_NN_H_
#define AMEBA_NN_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NN_MAX_DIMS 6
#define NN_NAME_SIZE 64

typedef void *NN_NetworkTypeDef;
typedef void *NN_BufferTypeDef;

typedef enum {
	NN_HW_PROP_CID = 0,
	NN_HW_PROP_DEVICE_COUNT = 1,
	NN_HW_PROP_CORE_COUNT_EACH_DEVICE = 2,
} NN_HwPropertyTypeDef;

typedef enum {
	NN_NETWORK_FROM_NONE = 0x00,
	NN_NETWORK_FROM_FILE = 0x01,
	NN_NETWORK_FROM_MEMORY = 0x02,
	NN_NETWORK_FROM_FLASH = 0x04,
} NN_NetworkSourceTypeDef;

typedef enum {
	NN_NETWORK_PROP_LAYER_COUNT = 0,
	NN_NETWORK_PROP_INPUT_COUNT = 1,
	NN_NETWORK_PROP_OUTPUT_COUNT = 2,
	NN_NETWORK_PROP_NETWORK_NAME = 3,
	NN_NETWORK_PROP_ADDRESS_INFO = 4,
	NN_NETWORK_PROP_MEMORY_POOL_SIZE = 6,
	NN_NETWORK_PROP_PROFILING = 7,
	NN_NETWORK_PROP_CORE_COUNT = 8,
	NN_NETWORK_PROP_CHANGE_PPU_PARAM = 64,
	NN_NETWORK_PROP_SET_MEMORY_POOL = 65,
	NN_NETWORK_PROP_SET_DEVICE_INDEX = 66,
	NN_NETWORK_PROP_SET_PRIORITY = 67,
	NN_NETWORK_PROP_SET_TIME_OUT = 68,
	NN_NETWORK_PROP_SET_COEFF_MEMORY = 69,
	NN_NETWORK_PROP_SET_CORE_INDEX = 70,
	NN_NETWORK_PROP_SET_ENABLE_NPD = 71,
	NN_NETWORK_PROP_SET_VIPSRAM_PRELOAD = 72,
	NN_NETWORK_PROP_SET_LAYER_DUMP_ID = 73,
	NN_NETWORK_PROP_GET_LAYER_DUMP_OUTPUT = 74,
} NN_NetworkPropertyTypeDef;

typedef enum {
	NN_BUFFER_FORMAT_FP32 = 0,
	NN_BUFFER_FORMAT_FP16 = 1,
	NN_BUFFER_FORMAT_UINT8 = 2,
	NN_BUFFER_FORMAT_INT8 = 3,
	NN_BUFFER_FORMAT_UINT16 = 4,
	NN_BUFFER_FORMAT_INT16 = 5,
	NN_BUFFER_FORMAT_CHAR = 6,
	NN_BUFFER_FORMAT_BFP16 = 7,
	NN_BUFFER_FORMAT_INT32 = 8,
	NN_BUFFER_FORMAT_UINT32 = 9,
	NN_BUFFER_FORMAT_INT64 = 10,
	NN_BUFFER_FORMAT_UINT64 = 11,
	NN_BUFFER_FORMAT_FP64 = 12,
	NN_BUFFER_FORMAT_INT4 = 13,
	NN_BUFFER_FORMAT_UINT4 = 14,
	NN_BUFFER_FORMAT_BOOL8 = 16,
} NN_BufferFormatTypeDef;

typedef enum {
	NN_BUFFER_QUANTIZE_NONE = 0,
	NN_BUFFER_QUANTIZE_DYNAMIC_FIXED_POINT = 1,
	NN_BUFFER_QUANTIZE_TF_ASYMM = 2,
} NN_BufferQuantizeFormatTypeDef;

typedef enum {
	NN_BUFFER_MEMORY_TYPE_DEFAULT = 0x000,
	NN_BUFFER_MEMORY_TYPE_HOST = 0x001,
	NN_BUFFER_MEMORY_TYPE_DMA_BUF = 0x003,
} NN_BufferMemoryTypeDef;

typedef enum {
	NN_BUFFER_PROP_QUANT_FORMAT = 0,
	NN_BUFFER_PROP_NUM_OF_DIMENSION = 1,
	NN_BUFFER_PROP_SIZES_OF_DIMENSION = 2,
	NN_BUFFER_PROP_DATA_FORMAT = 3,
	NN_BUFFER_PROP_FIXED_POINT_POS = 4,
	NN_BUFFER_PROP_TF_SCALE = 5,
	NN_BUFFER_PROP_TF_ZERO_POINT = 6,
	NN_BUFFER_PROP_NAME = 7,
} NN_BufferPropertyTypeDef;

typedef enum {
	NN_BUFFER_OPER_NONE = 0,
	NN_BUFFER_OPER_FLUSH = 1,
	NN_BUFFER_OPER_INVALIDATE = 2,
} NN_BufferOperationTypeDef;

typedef enum {
	NN_POWER_PROPERTY_NONE = 0x0000,
	NN_POWER_PROPERTY_SET_FREQUENCY = 0x0001,
	NN_POWER_PROPERTY_OFF = 0x0002,
	NN_POWER_PROPERTY_ON = 0x0004,
	NN_POWER_PROPERTY_STOP = 0x0008,
	NN_POWER_PROPERTY_START = 0x0010,
} NN_PowerPropertyTypeDef;

typedef struct {
	uint32_t num_of_dims;
	uint32_t sizes[NN_MAX_DIMS];
	NN_BufferFormatTypeDef data_format;
	NN_BufferQuantizeFormatTypeDef quant_format;
	union {
		struct {
			int32_t fixed_point_pos;
		} dfp;
		struct {
			float scale;
			int32_t zero_point;
		} affine;
	} quant_data;
	NN_BufferMemoryTypeDef memory_type;
} NN_BufferCreateParamTypeDef;

typedef struct {
	uint32_t dim_count;
	uint32_t dim_size[NN_MAX_DIMS];
	NN_BufferFormatTypeDef data_format;
	NN_BufferQuantizeFormatTypeDef quant_format;
	union {
		struct {
			int32_t fixed_point_pos;
		} dfp;
		struct {
			float scale;
			int32_t zero_point;
		} affine;
	} quant_data;
} NN_BufferParamTypeDef;

typedef struct {
	uint8_t fscale_percent;
} NN_PowerFrequencyTypeDef;

typedef struct {
	uint32_t inference_time;
	uint32_t total_cycle;
} NN_InferenceProfileTypeDef;

uint32_t NN_GetVersion(void);
int NN_Init(void);
int NN_DeInit(void);

int NN_QueryHardware(NN_HwPropertyTypeDef property, size_t size, void *value);

int NN_CreateBuffer(const NN_BufferCreateParamTypeDef *params, NN_BufferTypeDef *buffer);
int NN_CreateBufferFromHandle(const NN_BufferCreateParamTypeDef *params, void *handle,
							  size_t handle_size, NN_BufferTypeDef *buffer);
int NN_DestroyBuffer(NN_BufferTypeDef *buffer);
void *NN_MapBuffer(NN_BufferTypeDef buffer);
int NN_UnmapBuffer(NN_BufferTypeDef buffer);
size_t NN_GetBufferSize(NN_BufferTypeDef buffer);
int NN_FlushBuffer(NN_BufferTypeDef buffer, NN_BufferOperationTypeDef operation);

int NN_CreateNetwork(const void *data, size_t data_size, NN_NetworkSourceTypeDef source,
					 NN_NetworkTypeDef *network);
int NN_DestroyNetwork(NN_NetworkTypeDef *network);
int NN_SetNetwork(NN_NetworkTypeDef network, NN_NetworkPropertyTypeDef property, void *value);
int NN_QueryNetwork(NN_NetworkTypeDef network, NN_NetworkPropertyTypeDef property, void *value);
int NN_PrepareNetwork(NN_NetworkTypeDef network);
int NN_FinishNetwork(NN_NetworkTypeDef network);
int NN_RunNetwork(NN_NetworkTypeDef network);
int NN_TriggerNetwork(NN_NetworkTypeDef network);
int NN_WaitNetwork(NN_NetworkTypeDef network);
int NN_CancelNetwork(NN_NetworkTypeDef network);

int NN_QueryInput(NN_NetworkTypeDef network, uint32_t index,
				  NN_BufferPropertyTypeDef property, void *value);
int NN_QueryOutput(NN_NetworkTypeDef network, uint32_t index,
				   NN_BufferPropertyTypeDef property, void *value);
int NN_QueryInputParam(NN_NetworkTypeDef network, uint32_t index,
					   NN_BufferParamTypeDef *param);
int NN_QueryOutputParam(NN_NetworkTypeDef network, uint32_t index,
						NN_BufferParamTypeDef *param);
int NN_SetInput(NN_NetworkTypeDef network, uint32_t index, NN_BufferTypeDef input);
int NN_SetOutput(NN_NetworkTypeDef network, uint32_t index, NN_BufferTypeDef output);

int NN_PowerManagement(uint32_t device_index, NN_PowerPropertyTypeDef property, void *value);

void NN_FlushDCache(const uintptr_t *base_addr, const size_t *base_addr_size,
					int num_base_addr);
void NN_InvalidateDCache(const uintptr_t *base_addr, const size_t *base_addr_size,
						 int num_base_addr);

#ifdef __cplusplus
}
#endif

#endif /* AMEBA_NN_H_ */
