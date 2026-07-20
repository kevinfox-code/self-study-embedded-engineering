# Profile: NucleoU575 (BOARD_NUCLEO_U575)

This folder receives the **STM32CubeMX-generated project** for the
Nucleo-U575ZI-Q (STM32U575ZIT6Q). It is empty until you run CubeMX per
[`foc-lib/docs/cubemx_setup_guide.md`](../../foc-lib/docs/cubemx_setup_guide.md)
sections 1–9 (clock 160 MHz, TIM1 center-aligned 20 kHz, ADC1 injected
group on TIM1 TRGO, SPI1 mode 1 16-bit, DRV GPIO/EXTI, FreeRTOS
CMSIS_V2, TIM6 HAL timebase, NVIC map §8).

## What CubeMX generates here

```
NucleoU575/
  <project>.ioc
  Core/
    Inc/  main.h, stm32u5xx_hal_conf.h, stm32u5xx_it.h, FreeRTOSConfig.h
    Src/  main.c, tim.c, adc.c, spi.c, gpio.c, stm32u5xx_it.c,
          stm32u5xx_hal_msp.c, stm32u5xx_hal_timebase_tim.c, freertos.c
  Drivers/          STM32U5xx HAL + CMSIS (vendor)
  Middlewares/      FreeRTOS source
  startup_stm32u575xx.s
  <linker>.ld
```

## Rules (decoupling contract)

1. **Never hand-edit generated files outside `USER CODE` markers.**
2. Nothing in `App/` may `#include` a generated header directly —
   `App/constants.h` is the only bridge (it includes `main.h`).
3. Exactly **three** user-code insertions wire the application in:

   In `Core/Src/main.c`, inside `USER CODE BEGIN 2` (after all
   `MX_*_Init()` calls, before `osKernelStart()`):

   ```c
   /* USER CODE BEGIN 2 */
   CoreAppMain();   /* App/Entry — init policy + foc_app_init() + app tasks */
   /* USER CODE END 2 */
   ```

   In `Core/Src/stm32u5xx_it.c`, inside the generated handlers' markers
   (library hook **before** the HAL generic handler):

   ```c
   void ADC1_IRQHandler(void)     { /* USER CODE */ isr_adc_eoc();     /* … */ HAL_ADC_IRQHandler(&hadc1); }
   void TIM1_BRK_IRQHandler(void) { /* USER CODE */ isr_tim_break();   /* … */ }
   void EXTIx_IRQHandler(void)    { /* USER CODE */ isr_nfault_exti(); /* … */ }
   ```

4. After generating: fill the `TODO` pin macros in `App/constants.h`
   (DRV ENABLE, nFAULT/EXTI), enable the hooks in `FreeRTOSConfig.h`
   (`configCHECK_FOR_STACK_OVERFLOW=2`, `configUSE_MALLOC_FAILED_HOOK=1`),
   then build with `-DAPP_HAVE_CUBEMX_PROFILE=ON` (see `App/CMakeLists.txt`).
