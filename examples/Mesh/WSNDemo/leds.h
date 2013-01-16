#ifndef _LEDS_H_
#define _LEDS_H_

#include "hal.h"
#include "halGpio.h"

#if defined(PLATFORM_RCB128RFA1) || defined(PLATFORM_RCB231)
  HAL_GPIO_PIN(LED0, E, 2);
  HAL_GPIO_PIN(LED1, E, 3);
  HAL_GPIO_PIN(LED2, E, 4);

#elif defined(PLATFORM_ZIGBIT)
  HAL_GPIO_PIN(LED0, B, 5);
  HAL_GPIO_PIN(LED1, B, 6);
  HAL_GPIO_PIN(LED2, B, 7);

#elif defined(PLATFORM_XPLAINED)
  HAL_GPIO_PIN(LED0, B, 4);
  HAL_GPIO_PIN(LED1, B, 5);
  HAL_GPIO_PIN(LED2, B, 6);

#else
  #error Unknown platform
#endif

INLINE void ledsInit(void)
{
  HAL_GPIO_LED0_out();
  HAL_GPIO_LED0_set();

  HAL_GPIO_LED1_out();
  HAL_GPIO_LED1_set();

  HAL_GPIO_LED2_out();
  HAL_GPIO_LED2_set();
}

INLINE void ledsClose(void)
{
  HAL_GPIO_LED0_in();
  HAL_GPIO_LED1_in();
  HAL_GPIO_LED2_in();
}

INLINE void ledOn(uint8_t i)
{
#ifdef __INVERT_ON_OFF
  if (0 == i)
    HAL_GPIO_LED0_set();
  else if (1 == i)
    HAL_GPIO_LED1_set();
  else if (2 == i)
    HAL_GPIO_LED2_set();
#else
  if (0 == i)
    HAL_GPIO_LED0_clr();
  else if (1 == i)
    HAL_GPIO_LED1_clr();
  else if (2 == i)
    HAL_GPIO_LED2_clr();
#endif
}

INLINE void ledOff(uint8_t i)
{
#ifdef __INVERT_ON_OFF
  if (0 == i)
    HAL_GPIO_LED0_clr();
  else if (1 == i)
    HAL_GPIO_LED1_clr();
  else if (2 == i)
    HAL_GPIO_LED2_clr();
#else
  if (0 == i)
    HAL_GPIO_LED0_set();
  else if (1 == i)
    HAL_GPIO_LED1_set();
  else if (2 == i)
    HAL_GPIO_LED2_set();
#endif
}

INLINE void ledToggle(uint8_t i)
{
  if (0 == i)
    HAL_GPIO_LED0_toggle();
  else if (1 == i)
    HAL_GPIO_LED1_toggle();
  else if (2 == i)
    HAL_GPIO_LED2_toggle();
}

#endif // _LEDS_H_
