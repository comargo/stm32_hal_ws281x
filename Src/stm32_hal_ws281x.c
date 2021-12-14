/**
  ******************************************************************************
  * @file     lib.c
  * @author   Auto-generated by STM32CubeIDE
  * @version  V1.0
  * @date     16/04/2021 18:57:16
  * @brief    Default under dev library file.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32_hal_ws281x.h"
#include "ws281x_set_pixel.h"

#define WS281X_RESET_PERIOD 12

/** Functions ----------------------------------------------------------------*/
__weak void assert_failed(uint8_t* file, uint32_t line)
{
	while(1) {
		__NOP();
	}

}

static void dma_transfer_complete_handler(DMA_HandleTypeDef *DmaHandle);
static void dma_transfer_half_handler(DMA_HandleTypeDef *DmaHandle);

static void ws281x_gpio_init(struct CM_HAL_WS281x *ws281x, GPIO_TypeDef  *gpio)
{
	assert_param(IS_GPIO_ALL_INSTANCE(gpio));
	ws281x->gpio = gpio;
	switch((uintptr_t)ws281x->gpio) {
	case GPIOA_BASE:
		__HAL_RCC_GPIOA_CLK_ENABLE();
		break;
	case GPIOB_BASE:
		__HAL_RCC_GPIOB_CLK_ENABLE();
		break;
#ifdef GPIOC_BASE
	case GPIOC_BASE:
		__HAL_RCC_GPIOC_CLK_ENABLE();
		break;
#endif
#ifdef GPIOD_BASE
	case GPIOD_BASE:
		__HAL_RCC_GPIOD_CLK_ENABLE();
		break;
#endif
#ifdef GPIOE_BASE
	case GPIOE_BASE:
		__HAL_RCC_GPIOE_CLK_ENABLE();
		break;
#endif
	}
	ws281x->highPins = 0;
	ws281x->lowPins = 0;
}

static void ws281x_gpio_init_channel(struct CM_HAL_WS281x *ws281x, struct CM_HAL_WS281X_Channel *chan)
{
	GPIO_InitTypeDef GPIO_InitStruct = {
			.Pin =  chan->GPIO_Pin,
			.Mode = GPIO_MODE_OUTPUT_PP,
			.Pull = GPIO_NOPULL,
			.Speed = GPIO_SPEED_FREQ_LOW
	};
	HAL_GPIO_Init(ws281x->gpio, &GPIO_InitStruct);
	ws281x->highPins |= chan->GPIO_Pin;
	ws281x->lowPins |= chan->GPIO_Pin << 16;
}

static void ws281x_tim_init(struct CM_HAL_WS281x *ws281x, TIM_TypeDef *tim)
{
	assert_param(IS_TIM_INSTANCE(tim));

	ws281x->htim.Instance = tim;

	switch((uintptr_t)tim) {
	case TIM2_BASE:
		__HAL_RCC_TIM2_CLK_ENABLE();
		ws281x->TIM_IRQn = TIM2_IRQn;
		break;
	case TIM3_BASE:
		__HAL_RCC_TIM3_CLK_ENABLE();
		ws281x->TIM_IRQn = TIM3_IRQn;
		break;
	case TIM4_BASE:
		__HAL_RCC_TIM4_CLK_ENABLE();
		ws281x->TIM_IRQn = TIM4_IRQn;
		break;
	default:
		assert_param(0);
	}

	// This computation of pulse length should work ok,
	// at some slower core speeds it needs some tuning.
	ws281x->htim.Init.Period =  SystemCoreClock / 800000 - 1; // 0,125us period (10 times lower the 1,25us period to have fixed math below)
	uint32_t cc1 = (10 * ws281x->htim.Init.Period) / 36 - 1;
	uint32_t cc2 = (10 * ws281x->htim.Init.Period) / 15 - 1;

	ws281x->htim.Init.RepetitionCounter = 0;
	ws281x->htim.Init.Prescaler         = 0;
	ws281x->htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
	ws281x->htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
	HAL_TIM_PWM_Init(&ws281x->htim);

	HAL_NVIC_SetPriority(ws281x->TIM_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ws281x->TIM_IRQn);

	TIM_OC_InitTypeDef tim2OC1 = {
			.OCMode       = TIM_OCMODE_PWM1,
			.OCPolarity   = TIM_OCPOLARITY_HIGH,
			.Pulse        = cc1,
			.OCNPolarity  = TIM_OCNPOLARITY_HIGH,
			.OCFastMode   = TIM_OCFAST_DISABLE
	};
	HAL_TIM_PWM_ConfigChannel(&ws281x->htim, &tim2OC1, TIM_CHANNEL_1);

	TIM_OC_InitTypeDef tim2OC2 = {
		.OCMode       = TIM_OCMODE_PWM1,
		.OCPolarity   = TIM_OCPOLARITY_HIGH,
		.Pulse        = cc2,
		.OCNPolarity  = TIM_OCNPOLARITY_HIGH,
		.OCFastMode   = TIM_OCFAST_DISABLE,
		.OCIdleState  = TIM_OCIDLESTATE_RESET,
		.OCNIdleState = TIM_OCNIDLESTATE_RESET
	};
	HAL_TIM_PWM_ConfigChannel(&ws281x->htim, &tim2OC2, TIM_CHANNEL_2);
}

static void ws281x_dma_init(struct CM_HAL_WS281x *ws281x)
{
	assert_param(IS_TIM_DMA_INSTANCE(ws281x->htim.Instance));
	__HAL_RCC_DMA1_CLK_ENABLE();
	switch((uintptr_t)ws281x->htim.Instance) {
	case TIM2_BASE:
		ws281x->dmaUpdate.Instance = DMA1_Channel2;
		ws281x->dmaCC1.Instance = DMA1_Channel5;
		ws281x->dmaCC2.Instance = DMA1_Channel7;
		ws281x->CC2_IRQn = DMA1_Channel7_IRQn;
		break;
	case TIM3_BASE:
		// TIM3 do not support DMA for CH2
		assert_param(0);
		break;
	case TIM4_BASE:
		ws281x->dmaUpdate.Instance = DMA1_Channel7;
		ws281x->dmaCC1.Instance = DMA1_Channel1;
		ws281x->dmaCC2.Instance = DMA1_Channel4;
		ws281x->CC2_IRQn = DMA1_Channel4_IRQn;
		break;
	default:
		assert_param(0);
	}
	// Update Event
	ws281x->dmaUpdate.Init.Direction = DMA_MEMORY_TO_PERIPH;
	ws281x->dmaUpdate.Init.PeriphInc = DMA_PINC_DISABLE;
	ws281x->dmaUpdate.Init.MemInc = DMA_MINC_DISABLE;
	ws281x->dmaUpdate.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	ws281x->dmaUpdate.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	ws281x->dmaUpdate.Init.Mode = DMA_CIRCULAR;
	ws281x->dmaUpdate.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	HAL_DMA_Init(&ws281x->dmaUpdate);
//	HAL_DMA_Start(&ws281x->dmaUpdate, (uintptr_t)&ws281x->highPins, (uintptr_t)&ws281x->GPIOx->BSRR, sizeof(ws281x->highPins));

	// TIM2 CC1 event
	ws281x->dmaCC1.Init.Direction = DMA_MEMORY_TO_PERIPH;
	ws281x->dmaCC1.Init.PeriphInc = DMA_PINC_DISABLE;
	ws281x->dmaCC1.Init.MemInc = DMA_MINC_ENABLE;
	ws281x->dmaCC1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	ws281x->dmaCC1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	ws281x->dmaCC1.Init.Mode = DMA_CIRCULAR;
	ws281x->dmaCC1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	HAL_DMA_Init(&ws281x->dmaCC1);
//	HAL_DMA_Start(&ws281x->dmaCC1, (uintptr_t)ws281x->dmaBitBuffer, (uintptr_t)&ws281x->GPIOx->BRR, sizeof(ws281x->dmaBitBuffer)/sizeof(ws281x->dmaBitBuffer[0]));

	// TIM2 CC2 event
	ws281x->dmaCC2.Init.Direction = DMA_MEMORY_TO_PERIPH;
	ws281x->dmaCC2.Init.PeriphInc = DMA_PINC_DISABLE;
	ws281x->dmaCC2.Init.MemInc = DMA_MINC_DISABLE;
	ws281x->dmaCC2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	ws281x->dmaCC2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	ws281x->dmaCC2.Init.Mode = DMA_CIRCULAR;
	ws281x->dmaCC2.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	HAL_DMA_Init(&ws281x->dmaCC2);
	HAL_NVIC_SetPriority(ws281x->CC2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ws281x->CC2_IRQn);
//	HAL_DMA_Start_IT(&ws281x->dmaCC2, (uintptr_t)&ws281x->lowPins, (uintptr_t)&ws281x->GPIOx->BSRR, sizeof(ws281x->highPins));
//	HAL_DMA_RegisterCallback(&ws281x->dmaCC2, HAL_DMA_XFER_CPLT_CB_ID, dma_transfer_complete_handler);
//	HAL_DMA_RegisterCallback(&ws281x->dmaCC2, HAL_DMA_XFER_HALFCPLT_CB_ID, dma_transfer_half_handler);
}

static bool load_next_framebuffer_data(struct CM_HAL_WS281x *ws281x, uint8_t half)
{
	bool has_data = false; // Is there any data in any channel (by default NO)
	for (int chanId = 0; chanId < ws281x->nChannels; ++chanId) {
		// Is there any data in channel[chanId]
		struct CM_HAL_WS281X_Channel *channel = ws281x->channels[chanId];
		if (channel && channel->frameBufferPos < channel->frameBufferSize) {
			uint8_t r = channel->frameBuffer[channel->frameBufferPos++];
			uint8_t g = channel->frameBuffer[channel->frameBufferPos++];
			uint8_t b = channel->frameBuffer[channel->frameBufferPos++];

			for (int i = 0; i < 16; ++i) {
				if ((channel->GPIO_Pin & (GPIO_PIN_0 << i)) != 0) {
					ws281x_set_pixel(ws281x->dmaBitBuffer+half*24, i, r, g, b);
				}
			}

			has_data = true; // There are some data in one of the channels
		}
	}
	return has_data;
}

static void dma_transfer_complete_handler(DMA_HandleTypeDef *DmaHandle)
{
	struct CM_HAL_WS281x *ws281x = DmaHandle->Parent;
	// load upper half
	if(!load_next_framebuffer_data(ws281x, 1)) {
		// No more data in current buffer.

		// Transfer of all LEDs is done, disable DMA but enable timer update IRQ to stop the 50us Treset pulse
		ws281x->state = WS281x_Finish;
		ws281x->reset_timer = 0;
		__HAL_TIM_ENABLE_IT(&ws281x->htim, TIM_IT_UPDATE);

		HAL_DMA_Abort(&ws281x->dmaUpdate);
		HAL_DMA_Abort(&ws281x->dmaCC1);
		HAL_DMA_Abort_IT(&ws281x->dmaCC2);

		__HAL_TIM_DISABLE_DMA(&ws281x->htim, TIM_DMA_UPDATE|TIM_DMA_CC1|TIM_DMA_CC2);

		ws281x->gpio->BSRR = ws281x->lowPins;
	}
}

static void dma_transfer_half_handler(DMA_HandleTypeDef *DmaHandle)
{
	struct CM_HAL_WS281x *ws281x = DmaHandle->Parent;
	// load lower half
	load_next_framebuffer_data(ws281x, 0);
}

static void tim_period_elapsed_callback(struct CM_HAL_WS281x *ws281x)
{
	if(ws281x->state == WS281x_Finish) {
		// Waiting for Treset time
		if(ws281x->reset_timer < WS281X_RESET_PERIOD) {
			ws281x->reset_timer++;
			return;
		}

		ws281x->reset_timer = 0;
		HAL_TIM_Base_Stop_IT(&ws281x->htim);
		HAL_TIM_PWM_Stop(&ws281x->htim, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&ws281x->htim, TIM_CHANNEL_2);
		ws281x->state = WS281x_Ready;
	}
}

void CM_HAL_WS281X_Init(struct CM_HAL_WS281x *ws281x, GPIO_TypeDef  *gpio, TIM_TypeDef *tim)
{
	ws281x->nChannels = 0;
	for(int chanId=0; chanId < CM_HAL_WS281X_MAX_CHANNELS; chanId++) {
		ws281x->channels[chanId] = NULL;
	}
	ws281x_gpio_init(ws281x, gpio);
	ws281x_tim_init(ws281x, tim);
	ws281x_dma_init(ws281x);
	ws281x->state=WS281x_Ready;
}

int CM_HAL_WS281X_AddChannel(struct CM_HAL_WS281x *ws281x, struct CM_HAL_WS281X_Channel *chan)
{
	int chanId = ws281x->nChannels++;
	ws281x->channels[chanId] = chan;
	ws281x_gpio_init_channel(ws281x, chan);
	return chanId;
}

void CM_HAL_WS281X_IRQHandler(struct CM_HAL_WS281x *ws281x, IRQn_Type IRQn)
{
	if (IRQn == ws281x->CC2_IRQn) {
		HAL_DMA_IRQHandler(&ws281x->dmaCC2);
		return;
	}
	if (IRQn == ws281x->TIM_IRQn) {
		/* TIM Update event */
		if (__HAL_TIM_GET_FLAG(&ws281x->htim, TIM_FLAG_UPDATE) != RESET) {
			if (__HAL_TIM_GET_IT_SOURCE(&ws281x->htim, TIM_IT_UPDATE) != RESET) {
				tim_period_elapsed_callback(ws281x);
			}
		}
		HAL_TIM_IRQHandler(&ws281x->htim);
	}
}

HAL_StatusTypeDef CM_HAL_WS281X_SendBuffer(struct CM_HAL_WS281x *ws281x)
{
	if(ws281x->state != WS281x_Ready) {
		return HAL_BUSY;
	}

	for(int chanId = 0; chanId < ws281x->nChannels; chanId++) {
		if(ws281x->channels[chanId]) {
			ws281x->channels[chanId]->frameBufferPos = 0;
		}
	}

	if(!load_next_framebuffer_data(ws281x, 0)) {
		ws281x->gpio->BSRR = ws281x->lowPins;
		// No data
		return HAL_OK;
	}
	load_next_framebuffer_data(ws281x, 1);

	ws281x->state = WS281x_Busy;

	HAL_DMA_RegisterCallback(&ws281x->dmaCC2, HAL_DMA_XFER_CPLT_CB_ID, &dma_transfer_complete_handler);
	HAL_DMA_RegisterCallback(&ws281x->dmaCC2, HAL_DMA_XFER_HALFCPLT_CB_ID, &dma_transfer_half_handler);
	ws281x->dmaCC2.Parent = ws281x;

	const uint32_t dmaBufferSize = sizeof(ws281x->dmaBitBuffer)/sizeof(ws281x->dmaBitBuffer[0]);
	HAL_DMA_Start(&ws281x->dmaUpdate, (uintptr_t)&ws281x->highPins, (uint32_t)&ws281x->gpio->BSRR, dmaBufferSize);
	HAL_DMA_Start(&ws281x->dmaCC1, (uintptr_t)&ws281x->dmaBitBuffer, (uint32_t)&ws281x->gpio->BRR, dmaBufferSize);
	HAL_DMA_Start_IT(&ws281x->dmaCC2, (uintptr_t)&ws281x->lowPins, (uint32_t)&ws281x->gpio->BSRR, dmaBufferSize);

	__HAL_TIM_ENABLE_DMA(&ws281x->htim, TIM_DMA_UPDATE|TIM_DMA_CC1|TIM_DMA_CC2);
	HAL_TIM_Base_Start(&ws281x->htim);
	HAL_TIM_PWM_Start(&ws281x->htim, TIM_CHANNEL_1);
	return HAL_OK;
}
