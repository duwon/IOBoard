/**
  ******************************************************************************
  * @file    current.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   전류 측정
  * @details

  */

#include <emp.h>
#include <string.h>
#include "spi.h"
#include "timer.h"

uint8_t spi3RxBuffer[10];                /*!< EMP SPI - SY7T609 수신 버퍼 - 현재 DMA 사용하지 않음 */
static bool flag_spi3RxComplete = false; /*!< EMP SPI - SY7T609 수신 완료 플래그, false로 변경해야 수신 가능 */
uint64_t sumPowerMeter = 0;              /*!< 소비전력 mWh */
static int flag_CalDone = false;         /*!< Calibration 완료 Flag */
float sensingPower = 0;                  /*!< 측정값 - 현재 소비전력 W */
static uint32_t sensingVoltage = 220;          /*!< 설정값 - 입력전압. V*/
static uint32_t sensingRatio = 1;            /*!< 설정값 - 외부 CT 배율. 6~140 */
static uint32_t sensingPhase = 0;            /*!< 설정값 - 단상 or 3상. 0 단상*/
static float sensingPF = 0.65f;

static void SY7T609_WriteRegSingle(uint8_t regNum, uint32_t regData);
static uint32_t SY7T609_ReadRegSingle(uint8_t regNum);
static int S24ToS32(uint32_t s24Data);
static float EMP_GetRMSCurrent(void);

/**
 * @brief EMP Register 쓰기
 * 
 * @param regNum:
 * @param regData:
 */
void SY7T609_WriteReg(uint8_t regNum, uint32_t regData)
{
  if (regNum < 0x40U)
  {
    SY7T609_WriteRegSingle(regNum, regData);
  }
  else
  {
    SY7T609_WriteRegSingle(0x09, regData);
    SY7T609_WriteRegSingle(0x0A, regNum | 0xFF0000);
  }
}

uint32_t SY7T609_ReadReg(uint8_t regNum)
{
  if (regNum < 0x40U)
  {
    return SY7T609_ReadRegSingle(regNum);
  }
  else
  {
    SY7T609_WriteRegSingle(0x0B, regNum);
    return SY7T609_ReadRegSingle(0x0C);
  }
}

static void SY7T609_WriteRegSingle(uint8_t regNum, uint32_t regData)
{
  uint8_t spiTxBuffer[5] = {0x01, (regNum << 2) + 0x02, (regData >> 16) & 0xFF, (regData >> 8) & 0xFF, (regData >> 0) & 0xFF};
  HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&hspi3, spiTxBuffer, 5U, 0xFFFF);
  HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
}

static uint32_t SY7T609_ReadRegSingle(uint8_t regNum)
{
  uint32_t spiRxData = 0;
  uint8_t spiRxBuffer[5] = {0x01, (regNum << 2) + 0x00, 0, 0, 0};

  HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_RESET);
  if (HAL_SPI_Receive(&hspi3, spiRxBuffer, 5U, 0xFFFF) == HAL_OK) /* [0] : 레지스터, [1] : 레지스터 값 */
  {
    spiRxData = (spiRxBuffer[2] << 16) | (spiRxBuffer[3] << 8) | (spiRxBuffer[4]);
  }
  HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);

  return spiRxData;
}

/**
 * @brief SPI RX 인터럽트
 * 
 * @param hspi 
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI2) /* LORA */
  {
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
  }
  if (hspi->Instance == SPI3) /* EMP - SY7T609 */
  {
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
    flag_spi3RxComplete = true;
  }
}

/**
 * @brief SPI TX 인터럽트
 * 
 * @param hspi 
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI3) /* EMP - SY7T609 */
  {
    HAL_GPIO_WritePin(PM_NSS_GPIO_Port, PM_NSS_Pin, GPIO_PIN_SET);
  }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
}

/**
 * @brief Convert Singed 24bit to Singed 32bit.
 * 
 * @param s24Data: signed 24bit data
 * @return int: Converted signed 32bit data
 */
static int S24ToS32(uint32_t s24Data)
{
  int cnvInt = 0;
  if ((s24Data & 0x800000) == 0x800000) /* 음수 ? */
  {
    cnvInt |= 0xFF000000;
    cnvInt |= s24Data;
  }
  else
  {
    cnvInt = s24Data;
  }

  return cnvInt;
}

/**
 * @brief Read register data of EMP IC.
 * 
 * @param powerValue: [0]Volt [1]Current [2]Power Factor [3]Active Power [4]Reactive Power [5]Apparent Power
 */
void EMP_Read(float *powerValue)
{
#if 0
  powerValue[0] = (float)SY7T609_ReadReg(0x11U) / 1000.0f;           /* VRMS U24 N Scaled RMS Voltage */
  powerValue[0] = (float)SY7T609_ReadReg(0x11U) / 1000.0f;           /* VRMS U24 N Scaled RMS Voltage */
  powerValue[1] = (float)SY7T609_ReadReg(0x12U) / 10000.0f;          //128000 /* IRMS U24 N Scaled RMS Current */
  powerValue[1] = (float)SY7T609_ReadReg(0x12U) / 10000.0f;          /* IRMS U24 N Scaled RMS Current */
  powerValue[2] = (float)S24ToS32(SY7T609_ReadReg(0x18U)) / 1000.0f; /* PF S24 N Scaled Power Factor */
  powerValue[2] = (float)S24ToS32(SY7T609_ReadReg(0x18U)) / 1000.0f; /* PF S24 N Scaled Power Factor */
  powerValue[3] = (float)S24ToS32(SY7T609_ReadReg(0x13U)) / 1000.0f; /* Power S24 N Scaled Active Power */
  powerValue[3] = (float)S24ToS32(SY7T609_ReadReg(0x13U)) / 1000.0f; /* Power S24 N Scaled Active Power */
  powerValue[4] = (float)S24ToS32(SY7T609_ReadReg(0x14U)) / 1000.0f; /* VAR S24 N Scaled Reactive Power */
  powerValue[4] = (float)S24ToS32(SY7T609_ReadReg(0x14U)) / 1000.0f; /* VAR S24 N Scaled Reactive Power */
  powerValue[5] = (float)S24ToS32(SY7T609_ReadReg(0x15U)) / 1000.0f; /* VA S24 N Scaled Apparent Power */
  powerValue[5] = (float)S24ToS32(SY7T609_ReadReg(0x15U)) / 1000.0f; /* VA S24 N Scaled Apparent Power */
#else
powerValue[0] = (float)sensingVoltage;
powerValue[1] = EMP_GetRMSCurrent();
powerValue[2] = sensingPF;
powerValue[5] = powerValue[0] * powerValue[1];
powerValue[3] = powerValue[5] * powerValue[2];
powerValue[4] = powerValue[5] * 0.86f;
#endif
  //powerValue[6] = (float)S24ToS32(SY7T609_ReadReg(0x17U)) / 20000.0f; /* avgpower S24 N Scaled Average Active Power */
  //powerValue[2] = (8388608.0f - (float)SY7T609_ReadReg(0x18U)) / 200.0f; /* PF S24 N Scaled Power Factor */
  //powerValue[3] = (8388608.0f - (float)SY7T609_ReadReg(0x13U)) / 200.0f; /* Power S24 N Scaled Active Power */
  //powerValue[4] = (8388608.0f - (float)SY7T609_ReadReg(0x14U)) / 200.0f; /* VAR S24 N Scaled Reactive Power */
  //powerValue[5] = (8388608.0f - (float)SY7T609_ReadReg(0x15U)) / 200.0f; /* VA S24 N Scaled Apparent Power */
  //powerValue[6] = (8388608.0f - (float)SY7T609_ReadReg(0x17U)) / 200.0f; /* avgpower S24 N Scaled Average Active Power */

  //SY7T609_ReadReg(0x0FU); /* VAVG S24 N Scaled Average Voltage */
  //SY7T609_ReadReg(0x10U); /* IAVG S24 N Scaled Average Current */
  //SY7T609_ReadReg(0x11U); /* VRMS U24 N Scaled RMS Voltage */
  //SY7T609_ReadReg(0x12U); /* IRMS U24 N Scaled RMS Current */
  //SY7T609_ReadReg(0x13U); /* Power S24 N Scaled Active Power */
  //SY7T609_ReadReg(0x14U); /* VAR S24 N Scaled Reactive Power */
  //SY7T609_ReadReg(0x15U); /* VA S24 N Scaled Apparent Power */
  //SY7T609_ReadReg(0x16U); /* Frequency U24 N Scaled Line Frequency */
  //SY7T609_ReadReg(0x17U); /* avgpower S24 N Scaled Average Active Power */
  //SY7T609_ReadReg(0x18U); /* PF S24 N Scaled Power Factor */
  //SY7T609_ReadReg(0x19U); /* Vfund U24 N Scaled Fundamental RMS Voltage */
  //SY7T609_ReadReg(0x1AU); /* Ifund U24 N Scaled Fundamental RMS Current */
  //SY7T609_ReadReg(0x1BU); /* Pfund S24 N Scaled Fundamental Active Power */
  //SY7T609_ReadReg(0x1CU); /* Qfund S24 N Scaled Fundamental Reactive Power */
  //SY7T609_ReadReg(0x1DU); /* VAfund S24 N Scaled Fundamental Apparent Power */
  //SY7T609_ReadReg(0x1EU); /* Vharm U24 N Scaled Harmonic RMS Voltage */
  //SY7T609_ReadReg(0x1FU); /* Iharm U24 N Scaled Harmonic RMS Current */
  //SY7T609_ReadReg(0x20U); /* Pharm S24 N Scaled Harmonic Active Power */
  //SY7T609_ReadReg(0x21U); /* Qharm S24 N Scaled Harmonic Reactive Power */
  //SY7T609_ReadReg(0x22U); /* VAharm S24 N Scaled Harmonic Apparent Power NA */
}

/**
 * @brief Initialize the EMP IC.
 * 
 */
void EMP_Init(void)
{
  SY7T609_WriteReg(0x02U, 0x001815); /* Control */
  SY7T609_WriteReg(0x02U, 0x001815); /* Control */
  SY7T609_WriteReg(0x4FU, 500000); /* IScale - Current scaling register. */
  SY7T609_WriteReg(0x4FU, 500000); /* IScale - Current scaling register. */
  SY7T609_WriteReg(0x51U, 130273); /* PScale - Power scaling register. 667kV * 50A / 0.001 / 256 */
  SY7T609_WriteReg(0x51U, 130273); /* PScale - Power scaling register. */
  SY7T609_WriteReg(0x51U, 130273); /* PScale - Power scaling register. */
  SY7T609_WriteReg(0x40U, 3999366);
  SY7T609_WriteReg(0x40U, 3999366); /* Bucket Register Low */
  SY7T609_WriteReg(0x41U, 723);    /* Bucket Register High */
  SY7T609_WriteReg(0x41U, 723);
  if (SY7T609_ReadReg(0x47U) == 0x200000) /* IC 초기 값이면 */
  {
    SY7T609_WriteReg(0x48U, CAL_VGAIN); /* vgain - Voltage gain set. */
    SY7T609_WriteReg(0x47U, CAL_IGAIN); /* igain - Current gain set. */
  }

  //SY7T609_WriteReg(0x47U, 0xCA95);   /* igain - Current gain set. */
  //SY7T609_WriteReg(0x48U, 0x20f6cc);  /* vgain - Voltage gain set. */
  //SY7T609_WriteReg(0x4FU, 62000);   /* IScale - Current scaling register. */
  //SY7T609_WriteReg(0x50U, 667000);  /* VScale - Voltage scaling register */
  //SY7T609_WriteReg(0x51U, 8337500);  /* PScale - Power scaling register. */
  //SY7T609_WriteReg(0x52U, 1000);    /* PFSCALE - Power Factor scaling register. */
  //SY7T609_WriteReg(0x53U, 1000);    /* FSCALE - Frequency scaling register.  */
  //SY7T609_WriteReg(0x54u, 1000);    /* Temperature Scaling register. */
  //SY7T609_WriteReg(0x5BU, 0);      /* iavgtarget U24 Y Average Current target for Calibration. 0 */
  //SY7T609_WriteReg(0x5CU, 0);      /* vavgtarget U24 Y Average Voltage target for Calibration. 0 */
  //SY7T609_WriteReg(0x5DU, 1);   /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  //SY7T609_WriteReg(0x5EU, 220000); /* vrmstarget U24 Y RMS Voltage target for Calibration. 120,000 */
  //SY7T609_WriteReg(0x5FU, 120000); /* powertarget U24 Y Active Power target for Calibration. 120,000 */

  //SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_CAL_VRMS] | EMP_COMMAND[COMMAND_CAL_IRMS]);
  //SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_CAL_IRMS]);

  sumPowerMeter = RTC_LoadValue();

  if (sumPowerMeter > 0xFFFFFFFFFF)
  {
    sumPowerMeter = 0;
  }
}

/**
 * @brief Cal 수행
 * 
 * @param VrmsTarget: mV
 * @param IrmsTarger: mA
 */
void SY7T609_Cal(uint32_t VrmsTarget, uint32_t IrmsTarger)
{
  SY7T609_WriteReg(0x5DU, IrmsTarger * 10); /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  SY7T609_WriteReg(0x5DU, IrmsTarger * 10); /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  SY7T609_WriteReg(0x5DU, IrmsTarger * 10); /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  SY7T609_WriteReg(0x5EU, VrmsTarget);      /* vrmstarget U24 Y RMS Voltage target for Calibration. 120,000 */
  SY7T609_WriteReg(0x5EU, VrmsTarget);      /* vrmstarget U24 Y RMS Voltage target for Calibration. 120,000 */
  SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_CAL_VRMS] | EMP_COMMAND[COMMAND_CAL_IRMS]);
  flag_CalDone = true;
}

/**
 * @brief Save the igain and vgain values.
 * 
 * @param CalValue: igain, vgain Each Unsigned int 4byte
 */
void EMP_UpdateCalValue(uint8_t *CalValue)
{
  uint32_t igain = ((uint32_t)CalValue[2] << 16) + ((uint32_t)CalValue[1] << 8) + (uint32_t)CalValue[0];
  uint32_t vgain = ((uint32_t)CalValue[6] << 16) + ((uint32_t)CalValue[5] << 8) + (uint32_t)CalValue[4];

  SY7T609_WriteReg(0x47U, igain);                            /* igain - Current gain set. */
  SY7T609_WriteReg(0x47U, igain);                            /* igain - Current gain set. */
  SY7T609_WriteReg(0x48U, vgain);                            /* vgain - Voltage gain set. */
  SY7T609_WriteReg(0x48U, vgain);                            /* vgain - Voltage gain set. */
  SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_SAVETOFALSH]); /* Save to flash */
}

/**
 * @brief Returns the igain and vgain values.
 * 
 * @param RetrunCalValue: igain, vgain Each Unsigned int 4byte
 */
void EMP_GetCalValue(uint8_t *RetrunCalValue)
{
  uint32_t igain = SY7T609_ReadReg(0x47U);
  igain = SY7T609_ReadReg(0x47U);
  uint32_t vgain = SY7T609_ReadReg(0x48U);
  vgain = SY7T609_ReadReg(0x48U);
  memcpy((void *)&RetrunCalValue[0], (void *)&igain, 4);
  memcpy((void *)&RetrunCalValue[4], (void *)&vgain, 4);
}

void EMP_SetDefaultValue(uint8_t ratio, uint8_t volt, uint8_t phase, uint8_t pf)
{
  switch(volt)
  {
    case 0:
    sensingVoltage = 220;
    break;
    case 1:
    sensingVoltage = 380;
    break;
    case 2:
    sensingVoltage = 460;
    break;
    default:
    break;
  }

  if(ratio > 0)
  {
    sensingRatio = ratio;
  }
  sensingPhase = phase;
  sensingPF = (float)pf / 100;
}

/**
 * @brief Return RMS Current
 * 
 * @return float: A
 */
static float EMP_GetRMSCurrent(void)
{
  float rmsCurrent;
  rmsCurrent = (float)SY7T609_ReadReg(0x12U) * sensingRatio / 10000.0f;
  rmsCurrent = (float)SY7T609_ReadReg(0x12U) * sensingRatio / 10000.0f;

  return rmsCurrent; /* 전류 Offet 보정식 추가 필요 */
}

/**
 * @brief It is called once every 16ms to calculate the average power.
 * 
 */
void EMP_SaveEveragePower(void)
{
#ifdef DEBUG
  sensingPower = (float)S24ToS32(SY7T609_ReadReg(0x17U)) / 1000.0f; /* 테스트 용 */
#endif

  /**** Save calibaration data ****/
  static int cntCalDone = 0;
  if (flag_CalDone) /* Calibration 후 10초 뒤 Cal 값 Flash에 저장 */
  {
    cntCalDone++;
    if (cntCalDone == 600)
    {
      SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_SAVETOFALSH]);
      cntCalDone = 0;
      flag_CalDone = false;
    }
  }

  /**** Cal average power ****/
#if 0
  uint32_t avgPower = 0;
  avgPower = SY7T609_ReadReg(0x17U);
  avgPower = SY7T609_ReadReg(0x17U);

  sumPowerMeter += (uint64_t)(S24ToS32(avgPower) / 3600);

#else
  static float rmsCurrent[60];
  static int cntCollectCurrent = 0;
  rmsCurrent[cntCollectCurrent] = EMP_GetRMSCurrent();
  cntCollectCurrent++;

  if(cntCollectCurrent >= 60)
  {
    cntCollectCurrent = 0;
    float sumCurr = 0; 
    for(int i=0; i<60; i++)
    {
      sumCurr += rmsCurrent[i];
    }
    sumPowerMeter += (uint64_t)((sumCurr / 60.0f * (float)(sensingVoltage) * sensingPF) / 3.75f);
    //sumPowerMeter += (uint64_t)((float)(EMP_GetRMSCurrent() * sensingRatio * sensingVoltage) * sensingPF / 3600.0f); /* 계산식 수정 필요 */
  }
#endif

  /**** Save average power ****/
  static int cnt60sTimer = 0;
  cnt60sTimer++;
  if (cnt60sTimer >= 3600) /* 주기적(1분)으로 RTC SRAM에 소비전력 저장 */
  {
    RTC_SaveValue(sumPowerMeter);
    cnt60sTimer = 0;
  }
}

/**
 * @brief Calibration Offset. Voltage and Current
 * 
 */
void SY7T609_Cal_Offset(void)
{
  SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_CAL_IOFFS] | EMP_COMMAND[COMNAND_CAL_VOFFS]);
  flag_CalDone = true;
}
