/*
 * stm32_hal_ws281x.h
 *
 *  Created on: Apr 13, 2021
 *      Author: cyril
 */

#ifndef _STM32_HAL_WS281X_H_
#define _STM32_HAL_WS281X_H_

#include "stm32f1xx_hal.h"

#define CM_HAL_WS281X_MAX_CHANNELS 4

enum WS281x_ColorMode {
  WS281x_RGB,
  WS281x_RBG,
  WS281x_GRB,
  WS281x_GBR,
  WS281x_BRG,
  WS281x_BGR
};

struct CM_HAL_WS281X_Channel {
	uint32_t GPIO_Pin;
	uint8_t *frameBuffer;
	size_t frameBufferSize;
	size_t frameBufferPos;
	enum WS281x_ColorMode colorMode;
};

enum WS281x_State {
	WS281x_Ready,
	WS281x_Busy,
	WS281x_Finish,
};


struct CM_HAL_WS281x {
	GPIO_TypeDef *gpio;
	TIM_HandleTypeDef htim;
	IRQn_Type TIM_IRQn;

	DMA_HandleTypeDef dmaUpdate;
	DMA_HandleTypeDef dmaCC1;
	DMA_HandleTypeDef dmaCC2;
	IRQn_Type CC2_IRQn;

	enum WS281x_State state;
	uint8_t reset_timer;

	uint32_t highPins;
	uint32_t lowPins;

	struct CM_HAL_WS281X_Channel * channels[CM_HAL_WS281X_MAX_CHANNELS];
	int nChannels;


	// framebuffer - buffer for 2 LEDs - two times 24 bits
	uint16_t dmaBitBuffer[24*2];
};

void CM_HAL_WS281X_Init(struct CM_HAL_WS281x *ws281x, GPIO_TypeDef *GPIOx, TIM_TypeDef *tim);
int CM_HAL_WS281X_AddChannel(struct CM_HAL_WS281x *ws281x, struct CM_HAL_WS281X_Channel *chan);
void CM_HAL_WS281X_IRQHandler(struct CM_HAL_WS281x *ws281x, IRQn_Type IRQn);
HAL_StatusTypeDef CM_HAL_WS281X_SendBuffer(struct CM_HAL_WS281x *ws281x);

#endif//_STM32_HAL_WS281X_H_
