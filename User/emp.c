/**
  ******************************************************************************
  * @file    current.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   전류 측정
  * @details

  */

#include <emp.h>
#include "spi.h"
#include "timer.h"

uint8_t spi3RxBuffer[10];                /*!< EMP SPI - SY7T609 수신 버퍼 */
static bool flag_spi3RxComplete = false; /*!< EMP SPI - SY7T609 수신 완료 플래그, false로 변경해야 수신 가능 */
float powerMeter = 0;                    /*!< 현재 소비전력 W */
uint64_t sumPowerMeter = 0;              /*!< 소비전력 mW/h */

static void SY7T609_WriteRegSingle(uint8_t regNum, uint32_t regData);
static uint32_t SY7T609_ReadRegSingle(uint8_t regNum);
static int S24ToS32(uint32_t s24Data);

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

static int S24ToS32(uint32_t s24Data)
{
  int cnvInt=0;
  if((s24Data & 0x800000) == 0x800000)
  {
	  cnvInt |= 0xFF000000;
	  cnvInt |= s24Data ;//& 0x7FFFFF;
  }
  else
  {
	  cnvInt = s24Data;
  }

  return cnvInt;
}

void EMP_Read(float *powerValue)
{
  powerValue[0] = (float)SY7T609_ReadReg(0x11U) / 1000.0f; /* VRMS U24 N Scaled RMS Voltage */
  powerValue[1] = (float)SY7T609_ReadReg(0x12U) / 128000.0f; /* IRMS U24 N Scaled RMS Current */
  powerValue[2] = (float)S24ToS32(SY7T609_ReadReg(0x18U)) / 1000.0f; /* PF S24 N Scaled Power Factor */
  powerValue[3] = (float)S24ToS32(SY7T609_ReadReg(0x13U)) / 20000.0f; /* Power S24 N Scaled Active Power */
  powerValue[4] = (float)S24ToS32(SY7T609_ReadReg(0x14U)) / 20000.0f; /* VAR S24 N Scaled Reactive Power */
  powerValue[5] = (float)S24ToS32(SY7T609_ReadReg(0x15U)) / 20000.0f; /* VA S24 N Scaled Apparent Power */
  powerValue[6] = (float)sumPowerMeter / 1000.0f;
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

void EMP_Init(void)
{
  if(SY7T609_ReadReg(0x47U) == 0x200000) /* IC 초기 값이면 */
  {
    SY7T609_WriteReg(0x51U, 8337500);  /* PScale - Power scaling register. */
    SY7T609_WriteReg(0x51U, 8337500);  /* PScale - Power scaling register. */
    SY7T609_WriteReg(0x51U, 8337500);  /* PScale - Power scaling register. */
    SY7T609_WriteReg(0x48U, CAL_VGAIN);  /* vgain - Voltage gain set. */
    SY7T609_WriteReg(0x47U, CAL_IGAIN);   /* igain - Current gain set. */
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
}

int flag_CalDone = false;
/**
 * @brief Cal 수행
 * 
 * @param VrmsTarget: mV
 * @param IrmsTarger: mA
 */
void SY7T609_Cal(uint32_t VrmsTarget, uint32_t IrmsTarger)
{
  SY7T609_WriteReg(0x5DU, IrmsTarger * 128);   /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  SY7T609_WriteReg(0x5DU, IrmsTarger * 128);   /* irmstarget U24 Y RMS Current target for Calibration. 1,000 */
  SY7T609_WriteReg(0x5EU, VrmsTarget); /* vrmstarget U24 Y RMS Voltage target for Calibration. 120,000 */
  SY7T609_WriteReg(0x5EU, VrmsTarget); /* vrmstarget U24 Y RMS Voltage target for Calibration. 120,000 */
  SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_CAL_VRMS] | EMP_COMMAND[COMMAND_CAL_IRMS]);
  flag_CalDone = true;
}

void EMP_SaveEveragePower(void)
{
#ifdef DEBUG
  powerMeter = (float)S24ToS32(SY7T609_ReadReg(0x17U)) / 20000.0f; /* 테스트 용 */
#endif 
  static int cnt60sTimer = 0;
  static int cntCalDone = 0;

  cnt60sTimer++;

  sumPowerMeter += (uint64_t)(S24ToS32(SY7T609_ReadReg(0x17U)) / 72000);

  if(cnt60sTimer >= 60)
  {
    RTC_SaveValue(sumPowerMeter);
    cnt60sTimer = 0;
  }

  if(flag_CalDone)
  {
    cntCalDone++;
    if(cntCalDone == 10)
    {
      SY7T609_WriteReg(0x00U, EMP_COMMAND[COMMAND_SAVETOFALSH]);
      cntCalDone = 0;
      flag_CalDone = false;
    }
  }
  
}