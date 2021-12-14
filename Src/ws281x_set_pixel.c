/*
 * ws281x_set_pixel.c
 *
 *  Created on: Dec 14, 2021
 *      Author: cyril
 */

#define SETPIX_4
#include <stm32f1xx_hal.h>
#include <stdint.h>

// Gamma correction table
//const uint8_t gammaTable[] = {
//    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
//    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
//    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
//    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
//    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
//   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
//   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
//   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
//   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
//   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
//   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
//   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
//  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
//  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
//  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
//  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#define RAM_BASE 0x20000000
#define RAM_BB_BASE 0x22000000
#define Var_ResetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)
#define Var_SetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)
#define Var_GetBit_BB(VarAddr, BitNumber) (*(volatile uint32_t *) (RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))
#define BITBAND_SRAM(address, bit) ( (__IO uint32_t *) (RAM_BB_BASE + (((uint32_t)address) - RAM_BASE) * 32 + (bit) * 4))

#define varSetBit(var,bit) (Var_SetBit_BB((uint32_t)&var,bit))
#define varResetBit(var,bit) (Var_ResetBit_BB((uint32_t)&var,bit))
#define varGetBit(var,bit) (Var_GetBit_BB((uint32_t)&var,bit))


void ws281x_set_pixel(uint16_t* buffer, uint16_t pin, uint8_t red, uint8_t green, uint8_t blue)
{

	// Apply gamma
//	red = gammaTable[red];
//	green = gammaTable[green];
//	blue = gammaTable[blue];


	uint32_t invRed = ~red;
	uint32_t invGreen = ~green;
	uint32_t invBlue = ~blue;




#if defined(SETPIX_1)
	uint8_t i;
	uint32_t calcClearPin = ~(0x01<<pin);
	for (i = 0; i < 8; i++)
	{
		// clear the data for pixel

		ws2812bDmaBitBuffer[(i)] &= calcClearPin;
		ws2812bDmaBitBuffer[(8+i)] &= calcClearPin;
		ws2812bDmaBitBuffer[(16+i)] &= calcClearPin;

		// write new data for pixel
		ws2812bDmaBitBuffer[(i)] |= (((((invGreen)<<i) & 0x80)>>7)<<pin);
		ws2812bDmaBitBuffer[(8+i)] |= (((((invRed)<<i) & 0x80)>>7)<<pin);
		ws2812bDmaBitBuffer[(16+i)] |= (((((invBlue)<<i) & 0x80)>>7)<<pin);
	}
#elif defined(SETPIX_2)
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		// Set or clear the data for the pixel

		if(((invGreen)<<i) & 0x80)
			varSetBit(ws2812bDmaBitBuffer[(i)], pin);
		else
			varResetBit(ws2812bDmaBitBuffer[(i)], pin);

		if(((invRed)<<i) & 0x80)
			varSetBit(ws2812bDmaBitBuffer[(8+i)], pin);
		else
			varResetBit(ws2812bDmaBitBuffer[(8+i)], pin);

		if(((invBlue)<<i) & 0x80)
			varSetBit(ws2812bDmaBitBuffer[(16+i)], pin);
		else
			varResetBit(ws2812bDmaBitBuffer[(16+i)], pin);

	}
#elif defined(SETPIX_3)
	ws2812bDmaBitBuffer[(0)] |= (((((invGreen)<<0) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+0)] |= (((((invRed)<<0) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+0)] |= (((((invBlue)<<0) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(1)] |= (((((invGreen)<<1) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+1)] |= (((((invRed)<<1) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+1)] |= (((((invBlue)<<1) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(2)] |= (((((invGreen)<<2) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+2)] |= (((((invRed)<<2) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+2)] |= (((((invBlue)<<2) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(3)] |= (((((invGreen)<<3) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+3)] |= (((((invRed)<<3) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+3)] |= (((((invBlue)<<3) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(4)] |= (((((invGreen)<<4) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+4)] |= (((((invRed)<<4) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+4)] |= (((((invBlue)<<4) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(5)] |= (((((invGreen)<<5) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+5)] |= (((((invRed)<<5) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+5)] |= (((((invBlue)<<5) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(6)] |= (((((invGreen)<<6) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+6)] |= (((((invRed)<<6) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+6)] |= (((((invBlue)<<6) & 0x80)>>7)<<pin);

	ws2812bDmaBitBuffer[(7)] |= (((((invGreen)<<7) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(8+7)] |= (((((invRed)<<7) & 0x80)>>7)<<pin);
	ws2812bDmaBitBuffer[(16+7)] |= (((((invBlue)<<7) & 0x80)>>7)<<pin);
#elif defined(SETPIX_4)

	// Bitband optimizations with pure increments, 5us interrupts
	__IO uint32_t *bitBand = BITBAND_SRAM(buffer, pin);

	*bitBand =  (invGreen >> 7);
	bitBand+=16;

	*bitBand = (invGreen >> 6);
	bitBand+=16;

	*bitBand = (invGreen >> 5);
	bitBand+=16;

	*bitBand = (invGreen >> 4);
	bitBand+=16;

	*bitBand = (invGreen >> 3);
	bitBand+=16;

	*bitBand = (invGreen >> 2);
	bitBand+=16;

	*bitBand = (invGreen >> 1);
	bitBand+=16;

	*bitBand = (invGreen >> 0);
	bitBand+=16;

	// RED
	*bitBand =  (invRed >> 7);
	bitBand+=16;

	*bitBand = (invRed >> 6);
	bitBand+=16;

	*bitBand = (invRed >> 5);
	bitBand+=16;

	*bitBand = (invRed >> 4);
	bitBand+=16;

	*bitBand = (invRed >> 3);
	bitBand+=16;

	*bitBand = (invRed >> 2);
	bitBand+=16;

	*bitBand = (invRed >> 1);
	bitBand+=16;

	*bitBand = (invRed >> 0);
	bitBand+=16;

	// BLUE
	*bitBand =  (invBlue >> 7);
	bitBand+=16;

	*bitBand = (invBlue >> 6);
	bitBand+=16;

	*bitBand = (invBlue >> 5);
	bitBand+=16;

	*bitBand = (invBlue >> 4);
	bitBand+=16;

	*bitBand = (invBlue >> 3);
	bitBand+=16;

	*bitBand = (invBlue >> 2);
	bitBand+=16;

	*bitBand = (invBlue >> 1);
	bitBand+=16;

	*bitBand = (invBlue >> 0);
	bitBand+=16;

#elif defined(SETPIX_5)


/* How many channels (strips) of LEDs we want to control. This must not exceed 16. */
#define WS2812_NUM_CHANNELS     16

/* If all your channel framebuffers are the same size (ie, if each channel has the same number
 * of pixels), you can set this to 1 to bypass some error checking. This will speed things up
 * substantially, potentially allowing you to slow down your CPU and still meet the WS2812
 * timing requirements. "If unsure, set this to 0".
 */
#define WS212_ALL_CHANNELS_SAME_LENGTH  0

/*
 * We support up to 16 LED channels (that is, up to 16 distinct strips of LEDs.
 * Channels must be used sequentially, but their GPIOs do not have to be sequential.
 * Although all the LED strips must be on the same GPIO bank, the channel number does not
 * necessarily have to match its GPIO number. This is useful if we want fewer than 16 channels,
 * but we don't want them to start from GPIO 0.
 *
 * Change these assignments based on which strip you are wiring to which GPIO.
 *
 * To change the GPIO associated with each channel, modify the code below:
 */
#define WS2812_CH0_GPIO      0
#define WS2812_CH1_GPIO      1
#define WS2812_CH2_GPIO      2
#define WS2812_CH3_GPIO      3
#define WS2812_CH4_GPIO      4
#define WS2812_CH5_GPIO      5
#define WS2812_CH6_GPIO      6
#define WS2812_CH7_GPIO      7
#define WS2812_CH8_GPIO      8
#define WS2812_CH9_GPIO      9
#define WS2812_CH10_GPIO    10
#define WS2812_CH11_GPIO    11
#define WS2812_CH12_GPIO    12
#define WS2812_CH13_GPIO    13
#define WS2812_CH14_GPIO    14
#define WS2812_CH15_GPIO    15

/*
 * Unpack the bits of ch_val and pack them into the bit positions of cur0-cur7 that correspond to
 * the given GPIO number. Later, cur0-cur7 will be DMAed directly to a register within our GPIO
 * bank.
 */
#define UNPACK_CHANNEL(gpio_num)                    \
    asm volatile (                                  \
    "ubfx   r0, %[ch_val], #7, #1 \n"               \
    "bfi    %[cur0], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #6, #1 \n"               \
    "bfi    %[cur1], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #5, #1 \n"               \
    "bfi    %[cur2], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #4, #1 \n"               \
    "bfi    %[cur3], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #3, #1 \n"               \
    "bfi    %[cur4], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #2, #1 \n"               \
    "bfi    %[cur5], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #1, #1 \n"               \
    "bfi    %[cur6], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    "ubfx   r0, %[ch_val], #0, #1 \n"               \
    "bfi    %[cur7], r0,   #" #gpio_num ", 1  \n"   \
                                                    \
    : [cur0]"+r" (cur0),                            \
        [cur1]"+r" (cur1),                          \
        [cur2]"+r" (cur2),                          \
        [cur3]"+r" (cur3),                          \
        [cur4]"+r" (cur4),                          \
        [cur5]"+r" (cur5),                          \
        [cur6]"+r" (cur6),                          \
        [cur7]"+r" (cur7)                           \
    : [ch_val]"r" (ch_val)                          \
    : "r0", "cc");  /* r0 is a temp variable */



/*
 * Unpack the bits for one byte of one channel, and pack them into the bit positions of
 * the cur0 - cur7 variables, corresponding to the GPIO number for that channel.
 * The 'if' statement will be optimized away by the compiler, depending on how many channels
 * are actually defined.
 */
#define HANDLE_CHANNEL(ch_num, gpio_num)                    \
    if (ch_num < WS2812_NUM_CHANNELS) {                     \
        ch_val = 0xaa; /*get_channel_byte(channels + ch_num, pos);*/  \
        UNPACK_CHANNEL(gpio_num);                           \
    }

	uint8_t channel = pin;
	uint16_t led_index = column;

	register uint16_t cur0 = 0, cur1 = 0, cur2 = 0, cur3 = 0, cur4 = 0, cur5 = 0, cur6 = 0, cur7 = 0;

    uint8_t ch_val;
    HANDLE_CHANNEL( 0, WS2812_CH0_GPIO);
    HANDLE_CHANNEL( 1, WS2812_CH1_GPIO);
    HANDLE_CHANNEL( 2, WS2812_CH2_GPIO);


    /*
     * Store the repacked bits in our DMA buffer, ready to be sent to the GPIO bit-reset register.
     * cur0-cur7 represents bits0 - bits7 of all our channels. Each bit within curX is one channel.
     */
    ws2812bDmaBitBuffer[0] = cur0;
    ws2812bDmaBitBuffer[1] = cur1;
    ws2812bDmaBitBuffer[2] = cur2;
    ws2812bDmaBitBuffer[3] = cur3;
    ws2812bDmaBitBuffer[4] = cur4;
    ws2812bDmaBitBuffer[5] = cur5;
    ws2812bDmaBitBuffer[6] = cur6;
    ws2812bDmaBitBuffer[7] = cur7;

#endif
}
