#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"

void SystemClock_Config(void);

int main() {
  HAL_Init();
  SystemClock_Config();

  GPIO_InitTypeDef def;
  def.Mode = GPIO_MODE_OUTPUT_PP;
  def.Pin = GPIO_PIN_13;
  def.Speed = GPIO_SPEED_FREQ_LOW;
  def.Pull = GPIO_PULLDOWN;
  __HAL_RCC_GPIOC_CLK_ENABLE();
  HAL_GPIO_Init(GPIOC, &def);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  while (1) {
    HAL_Delay(1000);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}