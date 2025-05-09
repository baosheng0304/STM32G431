/**
  ******************************************************************************
  * @file    Examples_LL/OPAMP/OPAMP_Follower/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to use a operational amplifier
  *          peripheral in follower mode.
  *          To test OPAMP in this example, a voltage waveform is generated 
  *          by DAC peripheral and can be connected to OPAMP input.
  *          This example is based on the STM32G4xx OPAMP LL API;
  *          Peripheral initialization done using LL unitary services functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32G4xx_LL_Examples
  * @{
  */

/** @addtogroup OPAMP_Follower
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Waveform generation for this example purpose,                              */
/* using peripherals: DAC, DMA, TIM.                                          */
#define WAVEFORM_VOLTAGE_GENERATION


#if defined(WAVEFORM_VOLTAGE_GENERATION)
/* Definitions of environment analog values */
  /* Value of analog reference voltage (Vref+), connected to analog voltage   */
  /* supply Vdda (unit: mV).                                                  */
  #define VDDA_APPLI                       ((uint32_t)3300)
  
/* Definitions of data related to this example */
  /* Full-scale digital value with a resolution of 12 bits (voltage range     */
  /* determined by analog voltage references Vref+ and Vref-,                 */
  /* refer to reference manual).                                              */
  #define DIGITAL_SCALE_12BITS             (__LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B))


/* Definitions of waveform generation values */
  /* Waveform generation: parameters of waveform */
  /* Waveform amplitude (unit: mV) */
  #define WAVEFORM_AMPLITUDE          (VDDA_APPLI)
  /* Waveform amplitude (unit: Hz) */
  #define WAVEFORM_FREQUENCY          ((uint32_t)1000)
  /* Size of array containing DAC waveform samples */
  #define WAVEFORM_SAMPLES_SIZE       (sizeof (WaveformSine_12bits_32samples) / sizeof (uint16_t))

  /* Waveform generation: parameters of timer (used as DAC trigger) */
  /* Timer frequency (unit: Hz). With a timer 16 bits and time base           */
  /* freq min 1Hz, range is min=1Hz, max=32kHz.                               */
  #define WAVEFORM_TIMER_FREQUENCY                (WAVEFORM_FREQUENCY * WAVEFORM_SAMPLES_SIZE)
  /* Timer minimum frequency (unit: Hz), used to calculate frequency range.   */
  /* With a timer 16 bits, maximum frequency will be 32000 times this value.  */
  #define WAVEFORM_TIMER_FREQUENCY_RANGE_MIN      ((uint32_t)    1)
  /* Timer prescaler maximum value (0xFFFF for a timer 16 bits) */
  #define WAVEFORM_TIMER_PRESCALER_MAX_VALUE      ((uint32_t)0xFFFF-1)

#endif /* WAVEFORM_VOLTAGE_GENERATION */


/* Private macro -------------------------------------------------------------*/
#if defined(WAVEFORM_VOLTAGE_GENERATION)
/**
  * @brief  Computation of a data from maximum value on digital scale 12 bits 
  *         (corresponding to voltage Vdda)
  *         to a value on the new scale
  *         (corresponding to voltage defined by WAVEFORM_AMPLITUDE).
  * @param  __DATA_12BITS__: Digital value on scale 12 bits
  * @retval None
  */
#define __WAVEFORM_AMPLITUDE_SCALING(__DATA_12BITS__)                          \
  ((__DATA_12BITS__) * WAVEFORM_AMPLITUDE / VDDA_APPLI)
#endif /* WAVEFORM_VOLTAGE_GENERATION */

/* Private variables ---------------------------------------------------------*/
__IO uint8_t ubButtonPress = 0;

#if defined(WAVEFORM_VOLTAGE_GENERATION)
const uint16_t WaveformSine_12bits_32samples[] =
{
__WAVEFORM_AMPLITUDE_SCALING(2048),
__WAVEFORM_AMPLITUDE_SCALING(2447),
__WAVEFORM_AMPLITUDE_SCALING(2831),
__WAVEFORM_AMPLITUDE_SCALING(3185),
__WAVEFORM_AMPLITUDE_SCALING(3495),
__WAVEFORM_AMPLITUDE_SCALING(3750),
__WAVEFORM_AMPLITUDE_SCALING(3939),
__WAVEFORM_AMPLITUDE_SCALING(4056),
__WAVEFORM_AMPLITUDE_SCALING(4095),
__WAVEFORM_AMPLITUDE_SCALING(4056),
__WAVEFORM_AMPLITUDE_SCALING(3939),
__WAVEFORM_AMPLITUDE_SCALING(3750),
__WAVEFORM_AMPLITUDE_SCALING(3495),
__WAVEFORM_AMPLITUDE_SCALING(3185),
__WAVEFORM_AMPLITUDE_SCALING(2831),
__WAVEFORM_AMPLITUDE_SCALING(2447),
__WAVEFORM_AMPLITUDE_SCALING(2048),
__WAVEFORM_AMPLITUDE_SCALING(1649),
__WAVEFORM_AMPLITUDE_SCALING(1265),
__WAVEFORM_AMPLITUDE_SCALING(911),
__WAVEFORM_AMPLITUDE_SCALING(601),
__WAVEFORM_AMPLITUDE_SCALING(346),
__WAVEFORM_AMPLITUDE_SCALING(157),
__WAVEFORM_AMPLITUDE_SCALING(40),
__WAVEFORM_AMPLITUDE_SCALING(0),
__WAVEFORM_AMPLITUDE_SCALING(40),
__WAVEFORM_AMPLITUDE_SCALING(157),
__WAVEFORM_AMPLITUDE_SCALING(346),
__WAVEFORM_AMPLITUDE_SCALING(601),
__WAVEFORM_AMPLITUDE_SCALING(911),
__WAVEFORM_AMPLITUDE_SCALING(1265),
__WAVEFORM_AMPLITUDE_SCALING(1649)
};
#endif /* WAVEFORM_VOLTAGE_GENERATION */

/* Private function prototypes -----------------------------------------------*/
void     SystemClock_Config(void);
void     Configure_OPAMP(void);
void     Activate_OPAMP(void);
void     LED_Init(void);
void     LED_On(void);
void     LED_Toggle(void);
void     UserButton_Init(void);
void     WaitForUserButtonPress(void);
#if defined(WAVEFORM_VOLTAGE_GENERATION)
void     Generate_waveform_voltage(void);
#endif /* WAVEFORM_VOLTAGE_GENERATION */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Configure the system clock to 170 MHz */
  SystemClock_Config();
  
  /* Initialize LED2 */
  LED_Init();
  
  /* Initialize button in EXTI mode */
  UserButton_Init();
  
  /* Wait for User push-button press */
  WaitForUserButtonPress();
  ubButtonPress = 1;
  
#if defined(WAVEFORM_VOLTAGE_GENERATION)
  /* Generate waveform voltage to provide to user a voltage                   */
  /* available on board connectors.                                           */
  Generate_waveform_voltage();
#endif /* WAVEFORM_VOLTAGE_GENERATION */
  
  /* Configure OPAMP */
  Configure_OPAMP();
  
  /* Activate OPAMP */
  Activate_OPAMP();
  
  /* Infinite loop */
  while (1)
  {
  }
}

/**
  * @brief  Configure operational amplifier (OPAMP instance: OPAMP2) and GPIO
  *         used by operational amplifier.
  * @note   This function configures the OPAMP but does not enable it,
  *         in order to optimize power consumption (OPAMP enabled only
  *         when needed).
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
void Configure_OPAMP(void)
{
  /*## Configuration of GPIO used by OPAMP ###################################*/
  /* Enable GPIO Clock */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  
  /* Configure GPIO in analog mode to be used as OPAMP non-inverting input */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ANALOG);
  
  /* Configure GPIO in analog mode to be used as OPAMP output */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_ANALOG);
  
  
  /*## Configuration of OPAMP common parameters ##############################*/
  /* Enable OPAMP clock */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  
  /* Note: There is no conofiguration common to several OPAMP peripherals     */
  /*       on this STM32 series.                                               */
  
  /*## Configuration of OPAMP instance parameters ############################*/
  
  /* Set OPAMP power mode */
  // LL_OPAMP_SetPowerMode(OPAMP2, LL_OPAMP_POWERMODE_NORMAL);
  
  /* Set OPAMP mode */
  // LL_OPAMP_SetMode(OPAMP2, LL_OPAMP_MODE_FUNCTIONAL);
  
  /* Set OPAMP functional mode by setting internal connections */
  LL_OPAMP_SetFunctionalMode(OPAMP2, LL_OPAMP_MODE_FOLLOWER);
  
  /* Set OPAMP non-inverting input connection */
  LL_OPAMP_SetInputNonInverting(OPAMP2, LL_OPAMP_INPUT_NONINVERT_IO0);
  
  /* Set OPAMP inverting input connection */
  LL_OPAMP_SetInputInverting(OPAMP2, LL_OPAMP_INPUT_INVERT_CONNECT_NO);
  
  /*## Configuration of OPAMP trimming #######################################*/
  
  /* Note: In this example, default calibration parameters from factory       */
  /*       are used.                                                          */
  /*       User can also perform a calibration procedure to improve offset    */
  /*       accuracy of the transistors differential pair NMOS or PMOS.        */
  /*       Specific constraints on functional mode configuration may be       */
  /*       required.                                                          */
  /*       Refer to function @ref LL_OPAMP_SetMode().                         */
  
  /* Set OPAMP trimming mode */
  // LL_OPAMP_SetTrimmingMode(OPAMP2, LL_OPAMP_TRIMMING_FACTORY);
  
}

/**
  * @brief  Perform OPAMP activation procedure
  *         (OPAMP instance: OPAMP2).
  * @note   Operations:
  *         - Enable OPAMP instance
  *         - Wait for OPAMP startup time
  * @param  None
  * @retval None
  */
void Activate_OPAMP(void)
{
  __IO uint32_t wait_loop_index = 0;
  
  /* Enable OPAMP */
  LL_OPAMP_Enable(OPAMP2);
  
  /* Delay for OPAMP startup time.                                            */
  /* Compute number of CPU cycles to wait for, from delay in us.              */
  /* Note: Variable divided by 2 to compensate partially                      */
  /*       CPU processing cycles (depends on compilation optimization).       */
  /* Note: If system core clock frequency is below 200kHz, wait time          */
  /*       is only a few CPU processing cycles.                               */
  wait_loop_index = ((LL_OPAMP_DELAY_STARTUP_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  
}

#if defined(WAVEFORM_VOLTAGE_GENERATION)
/**
  * @brief  Generate voltage waveform for this example purpose,
  *         to provide to user a voltage available on board connectors.
  *         Generation using peripherals:
  *         - GPIO (analog output): pin PA.04
  *         - DMA (to transfer waveform samples to DAC): DMA1 Channel2
  *         - timer (to trig DAC conversions): TIM6
  *         - DAC: DAC1 channel1
  * @note   Timer instance must be on APB1 (clocked by PCLK1)
  *         to be compliant with frequency calculation used in this function.
  * @param  None
  * @retval None
  */
void Generate_waveform_voltage(void)
{
  __IO uint32_t wait_loop_index = 0;
  
  uint32_t timer_clock_frequency = 0;             /* Timer clock frequency */
  uint32_t timer_prescaler = 0;                   /* Time base prescaler to have timebase aligned on minimum frequency possible */
  uint32_t timer_reload = 0;                      /* Timer reload value in function of timer prescaler to achieve time base period */
  
  /*## Configuration of GPIO used by DAC channels ############################*/
  
  /* Enable GPIO Clock */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA); 
  
  /* Configure GPIO in analog mode to be used as DAC output */
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG);
  
  
  /*## Configuration of NVIC #################################################*/ 
  /* Configure the NVIC to handle timer update interrupt */
  NVIC_SetPriority(TIM6_DAC_IRQn, 0);
  NVIC_EnableIRQ(TIM6_DAC_IRQn);
  
  /* Note: In this example, DMA and DAC interruptions are not                 */
  /*       activated.                                                         */
  /*       If needed to be activated, refer to DMA and DAC examples           */

  /*## Configuration of DMA ##################################################*/
  
  /* Enable the peripheral clock of DMA */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1 | LL_AHB1_GRP1_PERIPH_DMAMUX1);
  
  /* Configure the DMA transfer */
  /*  - DMA transfer in circular mode to have an unlimited DAC signal         */
  /*    generation.                                                           */
  /*  - DMA transfer to DAC without address increment.                        */
  /*  - DMA transfer from memory with address increment.                      */
  /*  - DMA transfer to DAC by half-word to match with DAC resolution 12 bits */
  /*  - DMA transfer from memory by half-word to match with DAC data          */
  /*    buffer variable type: half-word.                                      */
  LL_DMA_ConfigTransfer(DMA1,
                        LL_DMA_CHANNEL_2,
                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH |
                        LL_DMA_MODE_CIRCULAR              |
                        LL_DMA_PERIPH_NOINCREMENT         |
                        LL_DMA_MEMORY_INCREMENT           |
                        LL_DMA_PDATAALIGN_WORD        |
                        LL_DMA_MDATAALIGN_HALFWORD        |
                        LL_DMA_PRIORITY_HIGH               );
  
  /* Select DAC as DMA transfer request */
  LL_DMA_SetPeriphRequest(DMA1,
                          LL_DMA_CHANNEL_2,
                          LL_DMAMUX_REQ_DAC1_CH1);
  
  /* Set DMA transfer addresses of source and destination */
  LL_DMA_ConfigAddresses(DMA1,
                         LL_DMA_CHANNEL_2,
                         (uint32_t)&WaveformSine_12bits_32samples,
                         LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED),
                         LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  
  /* Set DMA transfer size */
  LL_DMA_SetDataLength(DMA1,
                       LL_DMA_CHANNEL_2,
                       WAVEFORM_SAMPLES_SIZE);
  
  /* Note: In this example, DMA interruptions are not activated.              */
  /*       If needed, DMA interruption at transfer complete or error event    */
  /*       can be activated.                                                  */
  /*       Refer to DMA examples.                                             */
  
  /*## Configuration of timer ################################################*/
  
  /* Configuration of timer as time base:                                     */ 
  /* Caution: Computation of frequency is done for a timer instance on APB1   */
  /*          (clocked by PCLK1)                                              */
  /* Timer frequency is configured from the following constants:              */
  /* - WAVEFORM_TIMER_FREQUENCY: timer frequency (unit: Hz).                  */
  /* - WAVEFORM_TIMER_FREQUENCY_RANGE_MIN: timer minimum frequency possible   */
  /*   (unit: Hz).                                                            */
  /* Note: Refer to comments at these literals definition for more details.   */
  
  /* Retrieve timer clock source frequency */
  /* If APB1 prescaler is different of 1, timers have a factor x2 on their    */
  /* clock source.                                                            */
  if (LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1)
  {
    timer_clock_frequency = __LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler());
  }
  else
  {
    timer_clock_frequency = (__LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler()) * 2);
  }
  
  /* Timer prescaler calculation */
  /* (computation for timer 16 bits, additional + 1 to round the prescaler up) */
  timer_prescaler = ((timer_clock_frequency / (WAVEFORM_TIMER_PRESCALER_MAX_VALUE * WAVEFORM_TIMER_FREQUENCY_RANGE_MIN)) +1);
  /* Timer reload calculation */
  timer_reload = (timer_clock_frequency / (timer_prescaler * WAVEFORM_TIMER_FREQUENCY));
  
  /* Enable the timer peripheral clock */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
  
  /* Set timer pre-scaler value */
  LL_TIM_SetPrescaler(TIM6, (timer_prescaler - 1));
  
  /* Set timer auto-reload value */
  LL_TIM_SetAutoReload(TIM6, (timer_reload - 1));
  
  /* Set counter mode */
  LL_TIM_SetCounterMode(TIM6, LL_TIM_COUNTERMODE_UP); 
  
  /* Enable timer update interrupt */
  LL_TIM_EnableIT_UPDATE(TIM6);
  
  /* Set timer the trigger output (TRGO) */
  LL_TIM_SetTriggerOutput(TIM6, LL_TIM_TRGO_UPDATE);
  
  /*## Configuration of DAC ##################################################*/
  
  /* Enable DAC clock (core clock) */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_DAC1);
  
  /* Set the mode for the selected DAC channel */
  // LL_DAC_SetMode(DAC1, LL_DAC_CHANNEL_1, LL_DAC_MODE_NORMAL_OPERATION);
  
  /* Set the output for the selected DAC channel */
  LL_DAC_SetOutputBuffer(DAC1, LL_DAC_CHANNEL_1, LL_DAC_OUTPUT_BUFFER_ENABLE);
  
  /* Select trigger source */
  LL_DAC_SetTriggerSource(DAC1, LL_DAC_CHANNEL_1, LL_DAC_TRIG_EXT_TIM6_TRGO);
  
  /* Enable DAC channel DMA request */
  LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);
  
  /* Note: In this example, DAC interruptions are not activated.              */
  /*       If needed, DAC interruption at underrun event                      */
  /*       can be activated.                                                  */
  /*       Refer to DAC examples.                                             */
  
  /*## Activation of DMA #####################################################*/
  /* Enable the DMA transfer */
  LL_DMA_EnableChannel(DMA1,
                       LL_DMA_CHANNEL_2);
  
  /*## Activation of DAC #####################################################*/
  
  /* Enable DAC channel */
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
  
  /* Delay for DAC channel voltage settling time from DAC channel startup.    */
  /* Compute number of CPU cycles to wait for, from delay in us.              */
  /* Note: Variable divided by 2 to compensate partially                      */
  /*       CPU processing cycles (depends on compilation optimization).       */
  /* Note: If system core clock frequency is below 200kHz, wait time          */
  /*       is only a few CPU processing cycles.                               */
  wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  
  /* Enable trigger */
  LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);
  
  /*## Activation of timer ###################################################*/
  /* Enable counter */
  LL_TIM_EnableCounter(TIM6);
  
}
#endif /* WAVEFORM_VOLTAGE_GENERATION */

/**
  * @brief  Initialize LED2.
  * @param  None
  * @retval None
  */
void LED_Init(void)
{
  /* Enable the LED2 Clock */
  LED2_GPIO_CLK_ENABLE();

  /* Configure IO in output push-pull mode to drive external LED2 */
  LL_GPIO_SetPinMode(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_MODE_OUTPUT);
  /* Reset value is LL_GPIO_OUTPUT_PUSHPULL */
  //LL_GPIO_SetPinOutputType(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  /* Reset value is LL_GPIO_SPEED_FREQ_LOW */
  //LL_GPIO_SetPinSpeed(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_SPEED_FREQ_LOW);
  /* Reset value is LL_GPIO_PULL_NO */
  //LL_GPIO_SetPinPull(LED2_GPIO_PORT, LED2_PIN, LL_GPIO_PULL_NO);
}

/**
  * @brief  Turn-on LED2.
  * @param  None
  * @retval None
  */
void LED_On(void)
{
  /* Turn LED2 on */
  LL_GPIO_SetOutputPin(LED2_GPIO_PORT, LED2_PIN);
}

/**
  * @brief  Toggle LED2.
  * @param  None
  * @retval None
  */
void LED_Toggle(void)
{
  /* Toggle LED2 */
  LL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
}

/**
  * @brief  Configures User push-button in EXTI Line Mode.
  * @param  None
  * @retval None
  */
void UserButton_Init(void)
{
  /* Enable the BUTTON Clock */
  USER_BUTTON_GPIO_CLK_ENABLE();
  
  /* Configure GPIO for BUTTON */
  LL_GPIO_SetPinMode(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN, LL_GPIO_PULL_NO);
  
  /* Connect External Line to the GPIO */
  USER_BUTTON_SYSCFG_SET_EXTI();
  
  /* Enable a rising trigger EXTI line 13 Interrupt */
  USER_BUTTON_EXTI_LINE_ENABLE();
  USER_BUTTON_EXTI_FALLING_TRIG_ENABLE();
  
  /* Configure NVIC for USER_BUTTON_EXTI_IRQn */
  NVIC_EnableIRQ(USER_BUTTON_EXTI_IRQn); 
  NVIC_SetPriority(USER_BUTTON_EXTI_IRQn,0x03);  
  
}

/**
  * @brief  Wait for User push-button press to start transfer.
  * @param  None 
  * @retval None
  */
void WaitForUserButtonPress(void)
{
  while (ubButtonPress == 0)
  {
    LL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
    LL_mDelay(LED_BLINK_FAST);
  }
  ubButtonPress = 0;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 170000000
  *            HCLK(Hz)                       = 170000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            PLL_M                          = 4
  *            PLL_N                          = 85
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  /* Flash Latency configuration */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

  /* Enable boost mode to be able to reach 170MHz */
  LL_PWR_EnableRange1BoostMode();

  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  };

  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_4, 85, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };

  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  /* Insure 1µs transition state at intermediate medium speed clock based on DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  while(DWT->CYCCNT < 100);
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 170MHz */
  /* This frequency can be calculated through LL RCC macro */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ(__LL_RCC_CALC_HSI_FREQ(),
                                  LL_RCC_PLLM_DIV_4, 85, LL_RCC_PLLR_DIV_2)*/
  LL_Init1msTick(170000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(170000000);
}

/******************************************************************************/
/*   USER IRQ HANDLER TREATMENT                                               */
/******************************************************************************/

/**
  * @brief  Function to manage IRQ Handler
  * @param  None
  * @retval None
  */
void UserButton_Callback(void)
{
  /* Update User push-button variable, for waiting loop in main program */
  ubButtonPress = 1;
}

/**
  * @brief  Timer update interrupt processing
  * @param  None
  * @retval None
  */
void TimerUpdate_Callback(void)
{
  /* Generate a pulse of duration 10 CPU clock cycles on LED  */
  /* (approximative duration, depending on compilation        */
  /* optimization)                                            */
  uint32_t tmp = 5;
  LL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
  while (tmp-- != 0)
  {
  }
  LL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */


