
/**
  ******************************************************************************
  * @file    mc_tasks.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file implements tasks definition
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//cstat -MISRAC2012-Rule-21.1
#include "main.h"
//cstat +MISRAC2012-Rule-21.1
#include "mc_type.h"
#include "mc_math.h"
#include "motorcontrol.h"
#include "regular_conversion_manager.h"
#include "mc_interface.h"
#include "digital_output.h"
#include "pwm_common.h"

#include "mc_tasks.h"
#include "parameters_conversion.h"
#include "mcp_config.h"
#include "mc_app_hooks.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private define */

/* Private define ------------------------------------------------------------*/
/* Un-Comment this macro define in order to activate the smooth
   braking action on over voltage */
/* #define  MC.SMOOTH_BRAKING_ACTION_ON_OVERVOLTAGE */

#define STOPPERMANENCY_MS   ((uint16_t)400)
#define STOPPERMANENCY_MS2  ((uint16_t)400)
#define CHARGE_BOOT_CAP_TICKS  (uint16_t)((SYS_TICK_FREQUENCY * 10) / ((uint16_t)1000))
#define CHARGE_BOOT_CAP_TICKS2 (uint16_t)((SYS_TICK_FREQUENCY * 10)/ ((uint16_t)1000))
#define M1_CHARGE_BOOT_CAP_DUTY_CYCLES  (uint32_t)(0.000*(PWM_PERIOD_CYCLES/2))
#define M2_CHARGE_BOOT_CAP_DUTY_CYCLES (uint32_t)(0*(PWM_PERIOD_CYCLES2/2))
#define STOPPERMANENCY_TICKS   (uint16_t)((SYS_TICK_FREQUENCY * STOPPERMANENCY_MS)  / ((uint16_t)1000))
#define STOPPERMANENCY_TICKS2  (uint16_t)((SYS_TICK_FREQUENCY * STOPPERMANENCY_MS2) / ((uint16_t)1000))

/* USER CODE END Private define */
#define VBUS_TEMP_ERR_MASK (MC_OVER_VOLT| MC_UNDER_VOLT| MC_OVER_TEMP)
/* Private variables----------------------------------------------------------*/

static FOCVars_t FOCVars[NBR_OF_MOTORS];
static EncAlign_Handle_t *pEAC[NBR_OF_MOTORS];

static PWMC_Handle_t *pwmcHandle[NBR_OF_MOTORS];
//cstat !MISRAC2012-Rule-8.9_a
static RampExtMngr_Handle_t *pREMNG[NBR_OF_MOTORS];   /*!< Ramp manager used to modify the Iq ref
                                                    during the start-up switch over.*/
OpenLoop_Handle_t *pOpenLoop[1] = {MC_NULL};          /* only if M1 has OPEN LOOP */

static uint16_t hMFTaskCounterM1 = 0; //cstat !MISRAC2012-Rule-8.9_a
static volatile uint16_t hBootCapDelayCounterM1 = ((uint16_t)0);
static volatile uint16_t hStopPermanencyCounterM1 = ((uint16_t)0);

static volatile uint8_t bMCBootCompleted = ((uint8_t)0);

/* USER CODE BEGIN Private Variables */

/* USER CODE END Private Variables */

/* Private functions ---------------------------------------------------------*/
void TSK_MediumFrequencyTaskM1(void);
void FOC_Clear(uint8_t bMotor);
void FOC_InitAdditionalMethods(uint8_t bMotor);
void FOC_CalcCurrRef(uint8_t bMotor);
void TSK_MF_StopProcessing(  MCI_Handle_t * pHandle, uint8_t motor);
MCI_Handle_t * GetMCI(uint8_t bMotor);
static uint16_t FOC_CurrControllerM1(void);
void TSK_SetChargeBootCapDelayM1(uint16_t hTickCount);
bool TSK_ChargeBootCapDelayHasElapsedM1(void);
void TSK_SetStopPermanencyTimeM1(uint16_t hTickCount);
bool TSK_StopPermanencyTimeHasElapsedM1(void);
void TSK_SafetyTask_PWMOFF(uint8_t motor);

/* USER CODE BEGIN Private Functions */

/* USER CODE END Private Functions */
/**
  * @brief  It initializes the whole MC core according to user defined
  *         parameters.
  * @param  pMCIList pointer to the vector of MCInterface objects that will be
  *         created and initialized. The vector must have length equal to the
  *         number of motor drives.
  * @retval None
  */
__weak void MCboot( MCI_Handle_t* pMCIList[NBR_OF_MOTORS] )
{
  /* USER CODE BEGIN MCboot 0 */

  /* USER CODE END MCboot 0 */

  if (MC_NULL == pMCIList)
  {
    /* Nothing to do */
  }
  else
  {

    bMCBootCompleted = (uint8_t )0;

    /**********************************************************/
    /*    PWM and current sensing component initialization    */
    /**********************************************************/
    pwmcHandle[M1] = &PWM_Handle_M1._Super;
    R3_2_Init(&PWM_Handle_M1);
    ASPEP_start(&aspepOverUartA);

    /* USER CODE BEGIN MCboot 1 */

    /* USER CODE END MCboot 1 */
    /**************************************/
    /*    Start timers synchronously      */
    /**************************************/
    startTimers();

    /******************************************************/
    /*   PID component initialization: speed regulation   */
    /******************************************************/
    PID_HandleInit(&PIDSpeedHandle_M1);

    /******************************************************/
    /*   Encoder component initialization                 */
    /******************************************************/
    ENC_Init (&ENCODER_M1);

    /******************************************************/
    /*   Encoder alignment component initialization       */
    /******************************************************/
    EAC_Init(&EncAlignCtrlM1,pSTC[M1],&VirtualSpeedSensorM1,&ENCODER_M1);
    pEAC[M1] = &EncAlignCtrlM1;

    /******************************************************/
    /*   BEMF speed sensor component initialization       */
    /******************************************************/
#ifdef OBSERVER_PLL
    STO_PLL_Init (&STO_PLL_M1);
#elif defined(OBSERVER_CORDIC)
    STO_CR_Init (&STO_CR_M1);
#endif


#ifndef SPD_CTRL
    /******************************************************/
    /*   Position Control component initialization        */
    /******************************************************/
  PID_HandleInit(&PID_PosParamsM1);

  TC_Init(&PosCtrlM1, &PID_PosParamsM1, &SpeednTorqCtrlM1, &ENCODER_M1);
#endif
    /******************************************************/
    /*   Speed & torque component initialization          */
    /******************************************************/
#ifdef SENSORLESS
  #ifdef OBSERVER_PLL
    STC_Init(pSTC[M1],&PIDSpeedHandle_M1, &STO_PLL_M1._Super);
  #elif defined(OBSERVER_CORDIC)
    STC_Init(pSTC[M1],&PIDSpeedHandle_M1, &STO_CR_M1._Super);
  #else
  #endif
#else
    STC_Init(pSTC[M1],&PIDSpeedHandle_M1, &ENCODER_M1._Super);
#endif

    /****************************************************/
    /*   Virtual speed sensor component initialization  */
    /****************************************************/
    VSS_Init(&VirtualSpeedSensorM1);

#if defined(SPD_CTRL) && defined(SENSORLESS)
    /**************************************/
    /*   Rev-up component initialization  */
    /**************************************/
    RUC_Init(&RevUpControlM1, pSTC[M1], &VirtualSpeedSensorM1, &STO_M1, pwmcHandle[M1]);
#endif

    /********************************************************/
    /*   PID component initialization: current regulation   */
    /********************************************************/
    PID_HandleInit(&PIDIqHandle_M1);
    PID_HandleInit(&PIDIdHandle_M1);

    /********************************************************/
    /*   Bus voltage sensor component initialization        */
    /********************************************************/
    RVBS_Init(&BusVoltageSensor_M1);

    /*************************************************/
    /*   Power measurement component initialization  */
    /*************************************************/
    pMPM[M1]->pVBS = &(BusVoltageSensor_M1._Super);
    pMPM[M1]->pFOCVars = &FOCVars[M1];

    /*******************************************************/
    /*   Temperature measurement component initialization  */
    /*******************************************************/
    NTC_Init(&TempSensor_M1);

#ifdef FLUX_WEAKENING
    /*******************************************************/
    /*   Flux weakening component initialization           */
    /*******************************************************/
    PID_HandleInit(&PIDFluxWeakeningHandle_M1);
    FW_Init(pFW[M1],&PIDSpeedHandle_M1,&PIDFluxWeakeningHandle_M1);
#endif

#ifdef FEED_FORWARD
    /*******************************************************/
    /*   Feed forward component initialization             */
    /*******************************************************/
    FF_Init(pFF[M1],&(BusVoltageSensor_M1._Super),pPIDId[M1],pPIDIq[M1]);
#endif

    OL_Init(&OpenLoop_ParamsM1, &VirtualSpeedSensorM1);     /* only if M1 has open loop */
    pOpenLoop[M1] = &OpenLoop_ParamsM1;

    pREMNG[M1] = &RampExtMngrHFParamsM1;
    REMNG_Init(pREMNG[M1]);

    FOC_Clear(M1);
    FOCVars[M1].bDriveInput = EXTERNAL;
    FOCVars[M1].Iqdref = STC_GetDefaultIqdref(pSTC[M1]);
    FOCVars[M1].UserIdref = STC_GetDefaultIqdref(pSTC[M1]).d;
#ifdef SPD_CTRL
    MCI_Init(&Mci[M1], pSTC[M1], &FOCVars[M1],pwmcHandle[M1] );
#else
    MCI_Init(&Mci[M1], pSTC[M1], &FOCVars[M1], pPosCtrl[M1], pwmcHandle[M1]);
#endif
    Mci[M1].pVSS =  &VirtualSpeedSensorM1;
    MCI_SetSpeedMode(&Mci[M1]);
    MCI_ExecSpeedRamp(&Mci[M1],
    STC_GetMecSpeedRefUnitDefault(pSTC[M1]),0); /*First command to STC*/
    pMCIList[M1] = &Mci[M1];

    /* Applicative hook in MCBoot() */
    MC_APP_BootHook();

    /* USER CODE BEGIN MCboot 2 */

    /* USER CODE END MCboot 2 */

    bMCBootCompleted = 1U;
  }
}

/**
 * @brief Runs all the Tasks of the Motor Control cockpit
 *
 * This function is to be called periodically at least at the Medium Frequency task
 * rate (It is typically called on the Systick interrupt). Exact invokation rate is
 * the Speed regulator execution rate set in the Motor Contorl Workbench.
 *
 * The following tasks are executed in this order:
 *
 * - Medium Frequency Tasks of each motors
 * - Safety Task
 * - Power Factor Correction Task (if enabled)
 * - User Interface task.
 */
__weak void MC_RunMotorControlTasks(void)
{
  if (0U == bMCBootCompleted)
  {
    /* Nothing to do */
  }
  else
  {
    /* ** Medium Frequency Tasks ** */
    MC_Scheduler();

    /* Safety task is run after Medium Frequency task so that
     * it can overcome actions they initiated if needed. */
    TSK_SafetyTask();

  }
}

/**
 * @brief Performs stop process and update the state machine.This function
 *        shall be called only during medium frequency task
 */
void TSK_MF_StopProcessing(  MCI_Handle_t * pHandle, uint8_t motor)
{
  R3_2_SwitchOffPWM(pwmcHandle[motor]);

  FOC_Clear(motor);
  PQD_Clear(pMPM[motor]);
#ifdef DISC_PWM
  /* Disable DPWM mode */
  PWMC_DPWM_ModeDisable( pwmcHandle[motor] );
#endif
  TSK_SetStopPermanencyTimeM1(STOPPERMANENCY_TICKS);
  Mci[motor].State = STOP;
  return;
}

/**
 * @brief  Executes the Medium Frequency Task functions for each drive instance.
 *
 * It is to be clocked at the Systick frequency.
 */
__weak void MC_Scheduler(void)
{
/* USER CODE BEGIN MC_Scheduler 0 */

/* USER CODE END MC_Scheduler 0 */

  if (((uint8_t)1) == bMCBootCompleted)
  {
    if(hMFTaskCounterM1 > 0u)
    {
      hMFTaskCounterM1--;
    }
    else
    {
      TSK_MediumFrequencyTaskM1();

      /* Applicative hook at end of Medium Frequency for Motor 1 */
      MC_APP_PostMediumFrequencyHook_M1();

      MCP_Over_UartA.rxBuffer = MCP_Over_UartA.pTransportLayer->fRXPacketProcess(MCP_Over_UartA.pTransportLayer,
                                                                                &MCP_Over_UartA.rxLength);
      if ( 0U == MCP_Over_UartA.rxBuffer)
      {
        /* Nothing to do */
      }
      else
      {
        /* Synchronous answer */
        if (0U == MCP_Over_UartA.pTransportLayer->fGetBuffer(MCP_Over_UartA.pTransportLayer,
                                                     (void **) &MCP_Over_UartA.txBuffer, //cstat !MISRAC2012-Rule-11.3
                                                     MCTL_SYNC))
        {
          /* no buffer available to build the answer ... should not occur */
        }
        else
        {
          MCP_ReceivedPacket(&MCP_Over_UartA);
          MCP_Over_UartA.pTransportLayer->fSendPacket(MCP_Over_UartA.pTransportLayer, MCP_Over_UartA.txBuffer,
                                                      MCP_Over_UartA.txLength, MCTL_SYNC);
          /* no buffer available to build the answer ... should not occur */
        }
      }

      /* USER CODE BEGIN MC_Scheduler 1 */

      /* USER CODE END MC_Scheduler 1 */
      hMFTaskCounterM1 = (uint16_t)MF_TASK_OCCURENCE_TICKS;
    }
    if(hBootCapDelayCounterM1 > 0U)
    {
      hBootCapDelayCounterM1--;
    }
    if(hStopPermanencyCounterM1 > 0U)
    {
      hStopPermanencyCounterM1--;
    }
  }
  else
  {
    /* Nothing to do */
  }
  /* USER CODE BEGIN MC_Scheduler 2 */

  /* USER CODE END MC_Scheduler 2 */
}

/**
  * @brief Executes medium frequency periodic Motor Control tasks
  *
  * This function performs some of the control duties on Motor 1 according to the
  * present state of its state machine. In particular, duties requiring a periodic
  * execution at a medium frequency rate (such as the speed controller for instance)
  * are executed here.
  */
__weak void TSK_MediumFrequencyTaskM1(void)
{
  /* USER CODE BEGIN MediumFrequencyTask M1 0 */

  /* USER CODE END MediumFrequencyTask M1 0 */

  int16_t wAux = 0;
  MC_ControlMode_t mode;

  mode = MCI_GetControlMode( &Mci[M1] );
  (void)ENC_CalcAvrgMecSpeedUnit(&ENCODER_M1, &wAux);
#if defined(SPD_CTRL) && defined(SENSORLESS)
  bool IsSpeedReliable = 
#else
  (void)
#endif
#ifdef OBSERVER_PLL
  STO_PLL_CalcAvrgMecSpeedUnit(&STO_PLL_M1, &wAux);
#elif defined(OBSERVER_CORDIC)
  STO_CR_CalcAvrgMecSpeedUnit(&STO_CR_M1, &wAux);
#else
  #if defined(SPD_CTRL) && defined(SENSORLESS)
    #error For Sensorless control must define OBSERVER_PLL or OBSERVER_CORDIC
  #endif
#endif
  PQD_CalcElMotorPower(pMPM[M1]);

  if (MCI_GetCurrentFaults(&Mci[M1]) == MC_NO_FAULTS)
  {
    if (MCI_GetOccurredFaults(&Mci[M1]) == MC_NO_FAULTS)
    {
      switch (Mci[M1].State)
      {
        case IDLE:
        {
          if ((MCI_START == Mci[M1].DirectCommand) || (MCI_MEASURE_OFFSETS == Mci[M1].DirectCommand))
          {
#if defined(SPD_CTRL) && defined(SENSORLESS)
            if ( mode != MCM_OPEN_LOOP_VOLTAGE_MODE && mode != MCM_OPEN_LOOP_CURRENT_MODE)
            {
              RUC_Clear(&RevUpControlM1, MCI_GetImposedMotorDirection(&Mci[M1]));
            }
#endif

           if (pwmcHandle[M1]->offsetCalibStatus == false)
           {
             PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_START);
             Mci[M1].State = OFFSET_CALIB;
           }
           else
           {
             /* calibration already done. Enables only TIM channels */
             pwmcHandle[M1]->OffCalibrWaitTimeCounter = 1u;
             PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_EXEC);
             R3_2_TurnOnLowSides(pwmcHandle[M1],M1_CHARGE_BOOT_CAP_DUTY_CYCLES);
             TSK_SetChargeBootCapDelayM1(CHARGE_BOOT_CAP_TICKS);
             Mci[M1].State = CHARGE_BOOT_CAP;

           }

          }
          else
          {
            /* nothing to be done, FW stays in IDLE state */
          }
          break;
        }

        case OFFSET_CALIB:
          {
            if (MCI_STOP == Mci[M1].DirectCommand)
            {
              TSK_MF_StopProcessing(&Mci[M1], M1);
            }
            else
            {
              if (PWMC_CurrentReadingCalibr(pwmcHandle[M1], CRC_EXEC))
              {
                if (MCI_MEASURE_OFFSETS == Mci[M1].DirectCommand)
                {
                  FOC_Clear(M1);
                  PQD_Clear(pMPM[M1]);
                  Mci[M1].DirectCommand = MCI_NO_COMMAND;
                  Mci[M1].State = IDLE;
                }
                else
                {
                  R3_2_TurnOnLowSides(pwmcHandle[M1],M1_CHARGE_BOOT_CAP_DUTY_CYCLES);
                  TSK_SetChargeBootCapDelayM1(CHARGE_BOOT_CAP_TICKS);
                  Mci[M1].State = CHARGE_BOOT_CAP;

                }
              }
              else
              {
                /* nothing to be done, FW waits for offset calibration to finish */
              }
            }
            break;
          }

        case CHARGE_BOOT_CAP:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            if (TSK_ChargeBootCapDelayHasElapsedM1())
            {
              R3_2_SwitchOffPWM(pwmcHandle[M1]);
              FOCVars[M1].bDriveInput = EXTERNAL;
              STC_SetSpeedSensor( pSTC[M1], &VirtualSpeedSensorM1._Super );
              ENC_Clear(&ENCODER_M1);
#ifdef OBSERVER_PLL
              STO_PLL_Clear(&STO_PLL_M1);
#elif defined(OBSERVER_CORDIC)
              STO_CR_Clear(&STO_CR_M1);
#endif
#ifdef DISC_PWM
              /* Enable DPWM mode before Start */
              PWMC_DPWM_ModeEnable( pwmcHandle[M1] );
#endif
#ifdef OVM_PWM
              PWMC_Clear(pwmcHandle[M1]);
#endif
              FOC_Clear( M1 );

              if (EAC_IsAligned(&EncAlignCtrlM1) == false )
              {
                EAC_StartAlignment(&EncAlignCtrlM1);
                Mci[M1].State = ALIGNMENT;
              }
              else
              {

#if defined(SPD_CTRL) && defined(SENSORLESS)
                VSS_Clear(&VirtualSpeedSensorM1); /* Reset measured speed in IDLE */
                FOC_Clear(M1);
                Mci[M1].State = START;
#else
                STC_SetControlMode(pSTC[M1], MCM_SPEED_MODE);
                STC_SetSpeedSensor(pSTC[M1], &ENCODER_M1._Super);
                FOC_InitAdditionalMethods(M1);
                FOC_CalcCurrRef( M1 );
                STC_ForceSpeedReferenceToCurrentSpeed( pSTC[M1] ); /* Init the reference speed to current speed */
                MCI_ExecBufferedCommands( &Mci[M1] ); /* Exec the speed ramp after changing of the speed sensor */
                Mci[M1].State = RUN;
#endif
              }

              PWMC_SwitchOnPWM(pwmcHandle[M1]);
            }
            else
            {
              /* nothing to be done, FW waits for bootstrap capacitor to charge */
            }
          }
          break;
        }

        case ALIGNMENT:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            bool isAligned = EAC_IsAligned(&EncAlignCtrlM1);
            bool EACDone = EAC_Exec(&EncAlignCtrlM1);
            if ((isAligned == false)  && (EACDone == false))
            {
                qd_t IqdRef;
                IqdRef.q = 0;
                IqdRef.d = STC_CalcTorqueReference(pSTC[M1]);
                FOCVars[M1].Iqdref = IqdRef;
            }
            else
            {
              R3_2_SwitchOffPWM( pwmcHandle[M1] );
#ifndef SPD_CTRL
              STC_SetControlMode(pSTC[M1], MCM_SPEED_MODE);
              STC_SetSpeedSensor(pSTC[M1], &ENCODER_M1._Super);
#endif

              FOC_Clear(M1);
              TSK_SetStopPermanencyTimeM1(STOPPERMANENCY_TICKS);
              Mci[M1].State = WAIT_STOP_MOTOR;

              /* USER CODE BEGIN MediumFrequencyTask M1 EndOfEncAlignment */

              /* USER CODE END MediumFrequencyTask M1 EndOfEncAlignment */
            }
          }
          break;
        }

#if defined(SPD_CTRL) && defined(SENSORLESS)
        case START:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            /* Mechanical speed as imposed by the Virtual Speed Sensor during the Rev Up phase. */
            int16_t hForcedMecSpeedUnit;
            qd_t IqdRef;
            bool ObserverConverged = false;

            /* Execute the Rev Up procedure */
            if(! RUC_Exec(&RevUpControlM1))

            {
            /* The time allowed for the startup sequence has expired */
              MCI_FaultProcessing(&Mci[M1], MC_START_UP, 0);

           }
           else
           {
             /* Execute the torque open loop current start-up ramp:
              * Compute the Iq reference current as configured in the Rev Up sequence */
             IqdRef.q = STC_CalcTorqueReference( pSTC[M1] );
             IqdRef.d = FOCVars[M1].UserIdref;
             /* Iqd reference current used by the High Frequency Loop to generate the PWM output */
             FOCVars[M1].Iqdref = IqdRef;
           }

           (void) VSS_CalcAvrgMecSpeedUnit(&VirtualSpeedSensorM1, &hForcedMecSpeedUnit);

           /* check that startup stage where the observer has to be used has been reached */
           if (true == RUC_FirstAccelerationStageReached(&RevUpControlM1))

            {
#ifdef OBSERVER_PLL
             ObserverConverged = STO_PLL_IsObserverConverged(&STO_PLL_M1, &hForcedMecSpeedUnit);
             STO_SetDirection(&STO_PLL_M1, (int8_t)MCI_GetImposedMotorDirection(&Mci[M1]));
#elif defined(OBSERVER_CORDIC)
              ObserverConverged = STO_CR_IsObserverConverged(&STO_CR_M1, hForcedMecSpeedUnit);
#else
#error For Sensorless control must define OBSERVER_PLL or OBSERVER_CORDIC
#endif
              (void)VSS_SetStartTransition(&VirtualSpeedSensorM1, ObserverConverged);
            }

            if (ObserverConverged)
            {
#ifdef OBSERVER_PLL
              qd_t StatorCurrent = MCM_Park(FOCVars[M1].Ialphabeta, SPD_GetElAngle(&STO_PLL_M1._Super));
#elif defined(OBSERVER_CORDIC)
              qd_t StatorCurrent = MCM_Park(FOCVars[M1].Ialphabeta, SPD_GetElAngle(&STO_CR_M1._Super));
#else
#error For Sensorless control must define OBSERVER_PLL or OBSERVER_CORDIC
#endif

              /* Start switch over ramp. This ramp will transition from the revup to the closed loop FOC. */
              REMNG_Init(pREMNG[M1]);
              (void)REMNG_ExecRamp(pREMNG[M1], FOCVars[M1].Iqdref.q, 0);
              (void)REMNG_ExecRamp(pREMNG[M1], StatorCurrent.q, TRANSITION_DURATION);
              Mci[M1].State = SWITCH_OVER;
            }
          }
          break;
        }

        case SWITCH_OVER:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            bool LoopClosed;
            int16_t hForcedMecSpeedUnit;

            if(! RUC_Exec(&RevUpControlM1))

            {
              /* The time allowed for the startup sequence has expired */
              MCI_FaultProcessing(&Mci[M1], MC_START_UP, 0);

            }
            else

            {
              /* Compute the virtual speed and positions of the rotor.
                 The function returns true if the virtual speed is in the reliability range */
              LoopClosed = VSS_CalcAvrgMecSpeedUnit(&VirtualSpeedSensorM1, &hForcedMecSpeedUnit);
              /* Check if the transition ramp has completed. */
              bool tempBool;
              tempBool = VSS_TransitionEnded(&VirtualSpeedSensorM1);
              LoopClosed = LoopClosed || tempBool;

              /* If any of the above conditions is true, the loop is considered closed.
                 The state machine transitions to the START_RUN state. */
              if (true ==  LoopClosed)
              {
                #if ( PID_SPEED_INTEGRAL_INIT_DIV == 0 )
                PID_SetIntegralTerm(&PIDSpeedHandle_M1, 0);
                #else
                PID_SetIntegralTerm(&PIDSpeedHandle_M1,
                                    (((int32_t)FOCVars[M1].Iqdref.q * (int16_t)PID_GetKIDivisor(&PIDSpeedHandle_M1))
                                    / PID_SPEED_INTEGRAL_INIT_DIV));
				#endif

                /* USER CODE BEGIN MediumFrequencyTask M1 1 */

                /* USER CODE END MediumFrequencyTask M1 1 */
#ifdef OBSERVER_PLL
                STC_SetSpeedSensor(pSTC[M1], &STO_PLL_M1._Super); /*Observer has converged*/
#elif defined(OBSERVER_CORDIC)
                STC_SetSpeedSensor(pSTC[M1], &STO_CR_M1._Super); /*Observer has converged*/
#else
#error For Sensorless control must define OBSERVER_PLL or OBSERVER_CORDIC
#endif
                FOC_InitAdditionalMethods(M1);
                FOC_CalcCurrRef( M1 );
                STC_ForceSpeedReferenceToCurrentSpeed(pSTC[M1]); /* Init the reference speed to current speed */
                MCI_ExecBufferedCommands(&Mci[M1]); /* Exec the speed ramp after changing of the speed sensor */
                Mci[M1].State = RUN;
              }
            }
          }
          break;
        }
#endif

        case RUN:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            /* USER CODE BEGIN MediumFrequencyTask M1 2 */

            /* USER CODE END MediumFrequencyTask M1 2 */

#ifndef SPD_CTRL
            TC_PositionRegulation(pPosCtrl[M1]);
#endif
            MCI_ExecBufferedCommands(&Mci[M1]);
            if ( mode != MCM_OPEN_LOOP_VOLTAGE_MODE && mode != MCM_OPEN_LOOP_CURRENT_MODE)
            {

              FOC_CalcCurrRef(M1);

#if defined(SPD_CTRL) && defined(SENSORLESS)
              if(!IsSpeedReliable)
              {
                MCI_FaultProcessing(&Mci[M1], MC_SPEED_FDBK, 0);
              }
#endif

            }
            else
            {
              int16_t hForcedMecSpeedUnit;
              /* Open Loop */
              VSS_CalcAvrgMecSpeedUnit( &VirtualSpeedSensorM1, &hForcedMecSpeedUnit );
     	      OL_Calc( pOpenLoop[M1] );
            }

          }
          break;
        }

        case STOP:
        {
          if (TSK_StopPermanencyTimeHasElapsedM1())
          {

#if defined(SPD_CTRL) && defined(SENSORLESS)
            STC_SetSpeedSensor(pSTC[M1], &VirtualSpeedSensorM1._Super);  	/*  sensor-less */
            VSS_Clear(&VirtualSpeedSensorM1); /* Reset measured speed in IDLE */
#endif

            /* USER CODE BEGIN MediumFrequencyTask M1 5 */

            /* USER CODE END MediumFrequencyTask M1 5 */
            Mci[M1].DirectCommand = MCI_NO_COMMAND;
            Mci[M1].State = IDLE;
          }
          else
          {
            /* nothing to do, FW waits for to stop */
          }
          break;
        }

        case FAULT_OVER:
        {
          if (MCI_ACK_FAULTS == Mci[M1].DirectCommand)
          {
            Mci[M1].DirectCommand = MCI_NO_COMMAND;
            Mci[M1].State = IDLE;
          }
          else
          {
            /* nothing to do, FW stays in FAULT_OVER state until acknowledgement */
          }
        }
        break;

        case FAULT_NOW:
        {
          Mci[M1].State = FAULT_OVER;
        }
        break;

        case WAIT_STOP_MOTOR:
        {
          if (MCI_STOP == Mci[M1].DirectCommand)
          {
            TSK_MF_StopProcessing(&Mci[M1], M1);
          }
          else
          {
            if (TSK_StopPermanencyTimeHasElapsedM1())
            {
#if defined(SPD_CTRL) && defined(SENSORLESS)
              RUC_Clear(&RevUpControlM1, MCI_GetImposedMotorDirection(&Mci[M1]));
#endif
#ifdef SENSORLESS
  #ifdef OBSERVER_PLL
              STO_PLL_Clear( &STO_PLL_M1 );
  #elif defined(OBSERVER_CORDIC)
              STO_CR_Clear( &STO_CR_M1 );
  #else
    #error For Sensorless control must define OBSERVER_PLL or OBSERVER_CORDIC
  #endif
#endif
              ENC_Clear(&ENCODER_M1);
#if defined(SPD_CTRL) && defined(SENSORLESS)
              VSS_Clear(&VirtualSpeedSensorM1);
#endif
              R3_2_SwitchOnPWM( pwmcHandle[M1] );

#if defined(SPD_CTRL) && defined(SENSORLESS)
              Mci[M1].State = START;
#else
  #ifndef SPD_CTRL)
              TC_EncAlignmentCommand(pPosCtrl[M1]);
  #endif
              FOC_InitAdditionalMethods(M1);
              STC_ForceSpeedReferenceToCurrentSpeed( pSTC[M1] ); /* Init the reference speed to current speed */
              MCI_ExecBufferedCommands( &Mci[M1] ); /* Exec the speed ramp after changing of the speed sensor */
              FOC_CalcCurrRef(M1);
              Mci[M1].State = RUN;
#endif
            }
            else
            {
              /* nothing to do */
            }

          }
        }
        break;

        default:
          break;
       }
    }
    else
    {
      Mci[M1].State = FAULT_OVER;
    }
  }
  else
  {
    Mci[M1].State = FAULT_NOW;
  }

  /* USER CODE BEGIN MediumFrequencyTask M1 6 */

  /* USER CODE END MediumFrequencyTask M1 6 */
}

/**
  * @brief  It re-initializes the current and voltage variables. Moreover
  *         it clears qd currents PI controllers, voltage sensor and SpeednTorque
  *         controller. It must be called before each motor restart.
  *         It does not clear speed sensor.
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_Clear(uint8_t bMotor)
{
  /* USER CODE BEGIN FOC_Clear 0 */

  /* USER CODE END FOC_Clear 0 */
  MC_ControlMode_t mode;

  mode = MCI_GetControlMode( &Mci[bMotor] );

  ab_t NULL_ab = {((int16_t)0), ((int16_t)0)};
  qd_t NULL_qd = {((int16_t)0), ((int16_t)0)};
  alphabeta_t NULL_alphabeta = {((int16_t)0), ((int16_t)0)};

  FOCVars[bMotor].Iab = NULL_ab;
  FOCVars[bMotor].Ialphabeta = NULL_alphabeta;
  FOCVars[bMotor].Iqd = NULL_qd;
  if ( mode != MCM_OPEN_LOOP_VOLTAGE_MODE && mode != MCM_OPEN_LOOP_CURRENT_MODE)
  {
    FOCVars[bMotor].Iqdref = NULL_qd;
  }
  FOCVars[bMotor].hTeref = (int16_t)0;
  FOCVars[bMotor].Vqd = NULL_qd;
  FOCVars[bMotor].Valphabeta = NULL_alphabeta;
  FOCVars[bMotor].hElAngle = (int16_t)0;

  PID_SetIntegralTerm(pPIDIq[bMotor], ((int32_t)0));
  PID_SetIntegralTerm(pPIDId[bMotor], ((int32_t)0));

  STC_Clear(pSTC[bMotor]);

  PWMC_SwitchOffPWM(pwmcHandle[bMotor]);

#ifdef FLUX_WEAKENING
  if (NULL == pFW[bMotor])
  {
    /* Nothing to do */
  }
  else
  {
    FW_Clear(pFW[bMotor]);
  }
#endif
#ifdef FEED_FORWARD
  if (NULL == pFF[bMotor])
  {
    /* Nothing to do */
  }
  else
  {
    FF_Clear(pFF[bMotor]);
  }
#endif

  /* USER CODE BEGIN FOC_Clear 1 */

  /* USER CODE END FOC_Clear 1 */
}

/**
  * @brief  Use this method to initialize additional methods (if any) in
  *         START_TO_RUN state
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_InitAdditionalMethods(uint8_t bMotor) //cstat !RED-func-no-effect
{
    if (M_NONE == bMotor)
    {
      /* Nothing to do */
    }
    else
    {
#ifdef FEED_FORWARD
      if (NULL == pFF[bMotor])
      {
        /* Nothing to do */
      }
      else
      {
        FF_InitFOCAdditionalMethods(pFF[bMotor]);
      }
#endif
  /* USER CODE BEGIN FOC_InitAdditionalMethods 0 */

  /* USER CODE END FOC_InitAdditionalMethods 0 */
    }
}

/**
  * @brief  It computes the new values of Iqdref (current references on qd
  *         reference frame) based on the required electrical torque information
  *         provided by oTSC object (internally clocked).
  *         If implemented in the derived class it executes flux weakening and/or
  *         MTPA algorithm(s). It must be called with the periodicity specified
  *         in oTSC parameters
  * @param  bMotor related motor it can be M1 or M2
  * @retval none
  */
__weak void FOC_CalcCurrRef(uint8_t bMotor)
{
#ifdef FLUX_WEAKENING
  qd_t IqdTmp;
#endif

  /* USER CODE BEGIN FOC_CalcCurrRef 0 */

  /* USER CODE END FOC_CalcCurrRef 0 */
  MC_ControlMode_t mode;

  mode = MCI_GetControlMode( &Mci[bMotor] );
  if (INTERNAL == FOCVars[bMotor].bDriveInput &&  (mode != MCM_OPEN_LOOP_VOLTAGE_MODE && mode != MCM_OPEN_LOOP_CURRENT_MODE) )
  {
    FOCVars[bMotor].hTeref = STC_CalcTorqueReference(pSTC[bMotor]);
    FOCVars[bMotor].Iqdref.q = FOCVars[bMotor].hTeref;

#ifdef FLUX_WEAKENING
    if (NULL == pFW[bMotor])
    {
      /* Nothing to do */
    }
    else
    {
      IqdTmp.q = FOCVars[bMotor].Iqdref.q;
      IqdTmp.d = FOCVars[bMotor].UserIdref;
      FOCVars[bMotor].Iqdref = FW_CalcCurrRef(pFW[bMotor], IqdTmp);
    }
#endif
#ifdef FEED_FORWARD
    if (NULL == pFF[bMotor])
    {
      /* Nothing to do */
    }
    else
    {
      FF_VqdffComputation(pFF[bMotor], FOCVars[bMotor].Iqdref, pSTC[bMotor]);
    }
#endif
  }
  else
  {
    /* Nothing to do */
  }
  /* USER CODE BEGIN FOC_CalcCurrRef 1 */

  /* USER CODE END FOC_CalcCurrRef 1 */
}

/**
  * @brief  It set a counter intended to be used for counting the delay required
  *         for drivers boot capacitors charging of motor 1
  * @param  hTickCount number of ticks to be counted
  * @retval void
  */
__weak void TSK_SetChargeBootCapDelayM1(uint16_t hTickCount)
{
   hBootCapDelayCounterM1 = hTickCount;
}

/**
  * @brief  Use this function to know whether the time required to charge boot
  *         capacitors of motor 1 has elapsed
  * @param  none
  * @retval bool true if time has elapsed, false otherwise
  */
__weak bool TSK_ChargeBootCapDelayHasElapsedM1(void)
{
  bool retVal = false;
  if (((uint16_t)0) == hBootCapDelayCounterM1)
  {
    retVal = true;
  }
  return (retVal);
}

/**
  * @brief  It set a counter intended to be used for counting the permanency
  *         time in STOP state of motor 1
  * @param  hTickCount number of ticks to be counted
  * @retval void
  */
__weak void TSK_SetStopPermanencyTimeM1(uint16_t hTickCount)
{
  hStopPermanencyCounterM1 = hTickCount;
}

/**
  * @brief  Use this function to know whether the permanency time in STOP state
  *         of motor 1 has elapsed
  * @param  none
  * @retval bool true if time is elapsed, false otherwise
  */
__weak bool TSK_StopPermanencyTimeHasElapsedM1(void)
{
  bool retVal = false;
  if (((uint16_t)0) == hStopPermanencyCounterM1)
  {
    retVal = true;
  }
  return (retVal);
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section (".ccmram")))
#endif
#endif

/**
  * @brief  Executes the Motor Control duties that require a high frequency rate and a precise timing
  *
  *  This is mainly the FOC current control loop. It is executed depending on the state of the Motor Control
  * subsystem (see the state machine(s)).
  *
  * @retval Number of the  motor instance which FOC loop was executed.
  */
__weak uint8_t TSK_HighFrequencyTask(void)
{
  /* USER CODE BEGIN HighFrequencyTask 0 */

  /* USER CODE END HighFrequencyTask 0 */

  uint16_t hFOCreturn;
  uint8_t bMotorNbr = 0;

#ifdef SENSORLESS
  Observer_Inputs_t STO_Inputs; /*  only if sensorless main*/
#else
  #if defined(OBSERVER_PLL) || defined(OBSERVER_CORDIC)

  Observer_Inputs_t STO_aux_Inputs; /*  only if sensorless aux*/
  STO_aux_Inputs.Valfa_beta = FOCVars[M1].Valphabeta;  /* only if sensorless*/
  #endif
#endif

  (void)ENC_CalcAngle(&ENCODER_M1);   /* if not sensorless then 2nd parameter is MC_NULL*/

#ifdef SENSORLESS
  STO_Inputs.Valfa_beta = FOCVars[M1].Valphabeta;  /* only if sensorless*/
  if (SWITCH_OVER == Mci[M1].State)
  {
    if (!REMNG_RampCompleted(pREMNG[M1]))
    {
      FOCVars[M1].Iqdref.q = (int16_t)REMNG_Calc(pREMNG[M1]);
    }
  }
#endif
  /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_1 */

  /* USER CODE END HighFrequencyTask SINGLEDRIVE_1 */
  hFOCreturn = FOC_CurrControllerM1();
  /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_2 */

  /* USER CODE END HighFrequencyTask SINGLEDRIVE_2 */
  if(hFOCreturn == MC_DURATION)
  {
    MCI_FaultProcessing(&Mci[M1], MC_DURATION, 0);
  }
  else
  {
#ifdef SENSORLESS
  #ifdef OBSERVER_PLL
    bool IsAccelerationStageReached = RUC_FirstAccelerationStageReached(&RevUpControlM1);
  #endif
    STO_Inputs.Ialfa_beta = FOCVars[M1].Ialphabeta; /*  only if sensorless*/
    STO_Inputs.Vbus = VBS_GetAvBusVoltage_d(&(BusVoltageSensor_M1._Super)); /*  only for sensorless*/
  #ifdef OBSERVER_PLL
    (void)( void )STO_PLL_CalcElAngle(&STO_PLL_M1, &STO_Inputs);
    STO_PLL_CalcAvrgElSpeedDpp(&STO_PLL_M1); /*  Only in case of Sensor-less */
	 if (false == IsAccelerationStageReached)
    {
      STO_ResetPLL(&STO_PLL_M1);
    }
  #elif defined(OBSERVER_CORDIC)
    (void)STO_CR_CalcElAngle(&STO_CR_M1, &STO_Inputs);
    STO_CR_CalcAvrgElSpeedDpp(&STO_CR_M1); /*  Only in case of Sensor-less */
  #else
  #error Not implemented
  #endif
    /*  only for sensor-less or open loop */
    if(((uint16_t)START == Mci[M1].State) || ((uint16_t)SWITCH_OVER == Mci[M1].State) || ((uint16_t)RUN == Mci[M1].State))
    {
  #ifdef OBSERVER_PLL
      int16_t hObsAngle = SPD_GetElAngle(&STO_PLL_M1._Super);
  #elif defined(OBSERVER_CORDIC)
      int16_t hObsAngle = SPD_GetElAngle(&STO_CR_M1._Super);
  #else
  #error Not implemented
  #endif
      (void)VSS_CalcElAngle(&VirtualSpeedSensorM1, &hObsAngle);
    }
#else
  #if defined(OBSERVER_PLL) || defined(OBSERVER_CORDIC)
    if((uint16_t)RUN == Mci[M1].State)
    {
      int16_t hObsAngle = SPD_GetElAngle(&ENCODER_M1._Super);
      (void)VSS_CalcElAngle(&VirtualSpeedSensorM1, &hObsAngle);
    }
    STO_aux_Inputs.Ialfa_beta = FOCVars[M1].Ialphabeta; /*  only if sensorless*/
    STO_aux_Inputs.Vbus = VBS_GetAvBusVoltage_d(&(BusVoltageSensor_M1._Super)); /*  only for sensorless*/
    #ifdef OBSERVER_PLL
    (void)( void )STO_PLL_CalcElAngle (&STO_PLL_M1, &STO_aux_Inputs);
	STO_PLL_CalcAvrgElSpeedDpp (&STO_PLL_M1);
    #elif defined(OBSERVER_CORDIC)
    (void)STO_CR_CalcElAngle (&STO_CR_M1, &STO_aux_Inputs);
	STO_CR_CalcAvrgElSpeedDpp (&STO_CR_M1);
    #endif
  #endif
#endif
    /* USER CODE BEGIN HighFrequencyTask SINGLEDRIVE_3 */

    /* USER CODE END HighFrequencyTask SINGLEDRIVE_3 */
  }
  /* USER CODE BEGIN HighFrequencyTask 1 */

  /* USER CODE END HighFrequencyTask 1 */

  GLOBAL_TIMESTAMP++;
  if (0U == MCPA_UART_A.Mark)
  {
    /* Nothing to do */
  }
  else
  {
    MCPA_dataLog (&MCPA_UART_A);
  }

  return (bMotorNbr);
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section (".ccmram")))
#endif
#endif
/**
  * @brief It executes the core of FOC drive that is the controllers for Iqd
  *        currents regulation. Reference frame transformations are carried out
  *        accordingly to the active speed sensor. It must be called periodically
  *        when new motor currents have been converted
  * @param this related object of class CFOC.
  * @retval int16_t It returns MC_NO_FAULTS if the FOC has been ended before
  *         next PWM Update event, MC_DURATION otherwise
  */
inline uint16_t FOC_CurrControllerM1(void)
{
  qd_t Iqd, Vqd;
  ab_t Iab;
  alphabeta_t Ialphabeta, Valphabeta;
  int16_t hElAngle;
  uint16_t hCodeError;
  SpeednPosFdbk_Handle_t *speedHandle;
  MC_ControlMode_t mode;

  mode = MCI_GetControlMode( &Mci[M1] );
  speedHandle = STC_GetSpeedSensor(pSTC[M1]);
  hElAngle = SPD_GetElAngle(speedHandle);
#ifdef SENSORLESS
  hElAngle += SPD_GetInstElSpeedDpp(speedHandle)*REV_PARK_ANGLE_COMPENSATION_FACTOR;
#endif
  PWMC_GetPhaseCurrents(pwmcHandle[M1], &Iab);
  RCM_ReadOngoingConv();
  RCM_ExecNextConv();
  Ialphabeta = MCM_Clarke(Iab);
  Iqd = MCM_Park(Ialphabeta, hElAngle);
  Vqd.q = PI_Controller(pPIDIq[M1], (int32_t)(FOCVars[M1].Iqdref.q) - Iqd.q);
  Vqd.d = PI_Controller(pPIDId[M1], (int32_t)(FOCVars[M1].Iqdref.d) - Iqd.d);
#ifdef FEED_FORWARD
  Vqd = FF_VqdConditioning(pFF[M1],Vqd);
#endif
  if (mode == MCM_OPEN_LOOP_VOLTAGE_MODE)
  {
    Vqd = OL_VqdConditioning(pOpenLoop[M1]);
  }
  Vqd = Circle_Limitation(&CircleLimitationM1, Vqd);
  hElAngle += SPD_GetInstElSpeedDpp(speedHandle)*REV_PARK_ANGLE_COMPENSATION_FACTOR;
  Valphabeta = MCM_Rev_Park(Vqd, hElAngle);
#ifdef OVM_PWM
  hCodeError = PWMC_SetPhaseVoltage_OVM(pwmcHandle[M1], Valphabeta);
  PWMC_CalcPhaseCurrentsEst(pwmcHandle[M1],Iqd, hElAngle);
#else
  hCodeError = PWMC_SetPhaseVoltage(pwmcHandle[M1], Valphabeta);
#endif

  FOCVars[M1].Vqd = Vqd;
  FOCVars[M1].Iab = Iab;
  FOCVars[M1].Ialphabeta = Ialphabeta;
  FOCVars[M1].Iqd = Iqd;
  FOCVars[M1].Valphabeta = Valphabeta;
  FOCVars[M1].hElAngle = hElAngle;

#ifdef FLUX_WEAKENING
  FW_DataProcess(pFW[M1], Vqd);
#endif

#ifdef FEED_FORWARD
  FF_DataProcess(pFF[M1]);
#endif
  return(hCodeError);
}

/**
  * @brief  Executes safety checks (e.g. bus voltage and temperature) for all drive instances.
  *
  * Faults flags are updated here.
  */
__weak void TSK_SafetyTask(void)
{
  /* USER CODE BEGIN TSK_SafetyTask 0 */

  /* USER CODE END TSK_SafetyTask 0 */
  if (1U == bMCBootCompleted)
  {
    TSK_SafetyTask_PWMOFF(M1);
    /* User conversion execution */
    RCM_ExecUserConv();
  /* USER CODE BEGIN TSK_SafetyTask 1 */

  /* USER CODE END TSK_SafetyTask 1 */
  }
}

/**
  * @brief  Safety task implementation if  MC.ON_OVER_VOLTAGE == TURN_OFF_PWM
  * @param  bMotor Motor reference number defined
  *         \link Motors_reference_number here \endlink
  * @retval None
  */
__weak void TSK_SafetyTask_PWMOFF(uint8_t bMotor)
{
  /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 0 */

  /* USER CODE END TSK_SafetyTask_PWMOFF 0 */
  uint16_t CodeReturn = MC_NO_ERROR;
  uint16_t errMask[NBR_OF_MOTORS] = {VBUS_TEMP_ERR_MASK};

  CodeReturn |= errMask[bMotor] & NTC_CalcAvTemp(pTemperatureSensor[bMotor]); /* check for fault if FW protection is activated. It returns MC_OVER_TEMP or MC_NO_ERROR */
  CodeReturn |= PWMC_CheckOverCurrent(pwmcHandle[bMotor]);                    /* check for fault. It return MC_BREAK_IN or MC_NO_FAULTS
                                                                                 (for STM32F30x can return MC_OVER_VOLT in case of HW Overvoltage) */
  if(M1 == bMotor)
  {
    CodeReturn |= errMask[bMotor] & RVBS_CalcAvVbus(&BusVoltageSensor_M1);
  }
  MCI_FaultProcessing(&Mci[bMotor], CodeReturn, ~CodeReturn); /* process faults */

  if (MCI_GetFaultState(&Mci[bMotor]) != (uint32_t)MC_NO_FAULTS)
  {
    /* reset Encoder state */
    if (pEAC[bMotor] != MC_NULL)
    {
      EAC_SetRestartState(pEAC[bMotor], false);
    }
    PWMC_SwitchOffPWM(pwmcHandle[bMotor]);
    if (MCPA_UART_A.Mark != 0)
    {
      MCPA_flushDataLog (&MCPA_UART_A);
    }
    FOC_Clear(bMotor);
    PQD_Clear(pMPM[bMotor]); //cstat !MISRAC2012-Rule-11.3
    /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 1 */

    /* USER CODE END TSK_SafetyTask_PWMOFF 1 */
  }
  else
  {
    /* no errors */
  }

  /* USER CODE BEGIN TSK_SafetyTask_PWMOFF 3 */

  /* USER CODE END TSK_SafetyTask_PWMOFF 3 */
}

/**
  * @brief  This function returns the reference of the MCInterface relative to
  *         the selected drive.
  * @param  bMotor Motor reference number defined
  *         \link Motors_reference_number here \endlink
  * @retval MCI_Handle_t * Reference to MCInterface relative to the selected drive.
  *         Note: it can be MC_NULL if MCInterface of selected drive is not
  *         allocated.
  */
__weak MCI_Handle_t * GetMCI(uint8_t bMotor)
{
  MCI_Handle_t * retVal = MC_NULL;
  if (bMotor < (uint8_t)NBR_OF_MOTORS)
  {
    retVal = &Mci[bMotor];
  }
  return (retVal);
}

/**
  * @brief  Puts the Motor Control subsystem in in safety conditions on a Hard Fault
  *
  *  This function is to be executed when a general hardware failure has been detected
  * by the microcontroller and is used to put the system in safety condition.
  */
__weak void TSK_HardwareFaultTask(void)
{
  /* USER CODE BEGIN TSK_HardwareFaultTask 0 */

  /* USER CODE END TSK_HardwareFaultTask 0 */
  R3_2_SwitchOffPWM(pwmcHandle[M1]);
  MCI_FaultProcessing(&Mci[M1], MC_SW_ERROR, 0);

  /* USER CODE BEGIN TSK_HardwareFaultTask 1 */

  /* USER CODE END TSK_HardwareFaultTask 1 */
}

__weak void UI_HandleStartStopButton_cb (void)
{
/* USER CODE BEGIN START_STOP_BTN */
  if (IDLE == MC_GetSTMStateMotor1())
  {
    /* Ramp parameters should be tuned for the actual motor */
    (void)MC_StartMotor1();
  }
  else
  {
    (void)MC_StopMotor1();
  }
/* USER CODE END START_STOP_BTN */
}

 /**
  * @brief  Locks GPIO pins used for Motor Control to prevent accidental reconfiguration
  */
__weak void mc_lock_pins (void)
{
LL_GPIO_LockPin(M1_OPAMP3_INT_GAIN_GPIO_Port, M1_OPAMP3_INT_GAIN_Pin);
LL_GPIO_LockPin(M1_OPAMP3_OUT_GPIO_Port, M1_OPAMP3_OUT_Pin);
LL_GPIO_LockPin(M1_ENCODER_Z_GPIO_Port, M1_ENCODER_Z_Pin);
LL_GPIO_LockPin(M1_TEMPERATURE_GPIO_Port, M1_TEMPERATURE_Pin);
LL_GPIO_LockPin(M1_BUS_VOLTAGE_GPIO_Port, M1_BUS_VOLTAGE_Pin);
LL_GPIO_LockPin(M1_CURR_SHUNT_W_GPIO_Port, M1_CURR_SHUNT_W_Pin);
LL_GPIO_LockPin(M1_CURR_SHUNT_V_GPIO_Port, M1_CURR_SHUNT_V_Pin);
LL_GPIO_LockPin(M1_CURR_SHUNT_U_GPIO_Port, M1_CURR_SHUNT_U_Pin);
LL_GPIO_LockPin(M1_OPAMP1_INT_GAIN_GPIO_Port, M1_OPAMP1_INT_GAIN_Pin);
LL_GPIO_LockPin(M1_OPAMP1_OUT_GPIO_Port, M1_OPAMP1_OUT_Pin);
LL_GPIO_LockPin(M1_OPAMP2_OUT_GPIO_Port, M1_OPAMP2_OUT_Pin);
LL_GPIO_LockPin(M1_OPAMP2_INT_GAIN_GPIO_Port, M1_OPAMP2_INT_GAIN_Pin);
LL_GPIO_LockPin(M1_ENCODER_A_GPIO_Port, M1_ENCODER_A_Pin);
LL_GPIO_LockPin(M1_ENCODER_B_GPIO_Port, M1_ENCODER_B_Pin);
LL_GPIO_LockPin(M1_PWM_INPUT_GPIO_Port, M1_PWM_INPUT_Pin);
LL_GPIO_LockPin(M1_PWM_UH_GPIO_Port, M1_PWM_UH_Pin);
LL_GPIO_LockPin(M1_PWM_VH_GPIO_Port, M1_PWM_VH_Pin);
LL_GPIO_LockPin(M1_PWM_WH_GPIO_Port, M1_PWM_WH_Pin);
LL_GPIO_LockPin(M1_PWM_VL_GPIO_Port, M1_PWM_VL_Pin);
LL_GPIO_LockPin(M1_PWM_WL_GPIO_Port, M1_PWM_WL_Pin);
LL_GPIO_LockPin(M1_PWM_UL_GPIO_Port, M1_PWM_UL_Pin);
}
/* USER CODE BEGIN mc_task 0 */

/* USER CODE END mc_task 0 */

/******************* (C) COPYRIGHT 2022 STMicroelectronics *****END OF FILE****/
