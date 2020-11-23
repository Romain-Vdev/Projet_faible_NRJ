/*
 * gpio.c - GPIO functions, only for Nucleo-STM32L476 board
 */
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"
#include "gpio.h"

#define LED_PORT GPIOA
#define LED_PIN LL_GPIO_PIN_5
//#define BUT_PORT GPIOC
#define BUT_PIN LL_GPIO_PIN_13
#define PWM_PIN LL_GPIO_PIN_10

void GPIO_init(void)
{
// PORT A
LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOA );
// Green LED (user LED) - PA5
LL_GPIO_SetPinMode(       LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT );
LL_GPIO_SetPinOutputType( LED_PORT, LED_PIN, LL_GPIO_OUTPUT_PUSHPULL );

// PORT C
LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_GPIOC );
// Blue button - PC13 PA0
//LL_GPIO_SetPinMode( GPIOC, BUT_PIN, LL_GPIO_MODE_INPUT );
LL_GPIO_SetPinMode( GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT );
// Sortie PWM 50 Hz
LL_GPIO_SetPinMode(GPIOC, PWM_PIN, LL_GPIO_MODE_OUTPUT);
LL_GPIO_SetPinOutputType( GPIOC, PWM_PIN, LL_GPIO_OUTPUT_PUSHPULL );

}


void LED_GREEN( int val )
{
if	( val )
	LL_GPIO_SetOutputPin(   LED_PORT, LED_PIN );
else	LL_GPIO_ResetOutputPin( LED_PORT, LED_PIN );
}

int BLUE_BUTTON()
{
//return ( !LL_GPIO_IsInputPinSet( GPIOC, BUT_PIN ) );
	return ( !LL_GPIO_IsInputPinSet( GPIOA,LL_GPIO_PIN_0) );
}

void PWM_50Hz(int val)
{
	if	( val )
		LL_GPIO_SetOutputPin(  GPIOC, PWM_PIN );
	else	LL_GPIO_ResetOutputPin(GPIOC, PWM_PIN );

}








