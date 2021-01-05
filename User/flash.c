/**
  ******************************************************************************
  * @file    flash.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   FLASH 제어
  * @details

  */

#include <stdio.h>
#include <string.h>

#include "flash.h"
#include "uart.h"

#define FIRMWARE_ADDR (FLASH_APPLECATION_ADDRESS_START)
#define UPDATE_FIRMWARE_ADDR (FLASH_FW_ADDRESS_START)
#define UPDATE_CONFIG_ADDR (FLASH_FWINFO_ADDRESS_START)

struct CONFIG_BOOT
{
  unsigned char Idx[4];
  uint32_t size;
  uint32_t flag;
};

struct CONFIG_BOOT Boot_Cfg;

/** @defgroup FLASH FLASH 제어 함수
  * @brief FLASH 제어
  * @{
  */

static void Flash_UserErase(void);
static ErrorStatus Flash_FwWrite(uint16_t flashNo, uint32_t *flashData);
static void Update_Config_Write(uint32_t firmwareSize);
static int Update_Config_Erase(void);

/**
 * @brief 펌웨어 저장 영역 삭제
 * 
 */
void Flash_FwErase(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;                 /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = FLASH_FW_ADDRESS_START;              /* 삭제 할 시작 페이지 */
  EraseInitStruct.NbPages = FLASH_FW_PAGE_END - FLASH_FW_PAGE_START; /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0; /*!< 0xFFFFFFFF 값이면 정상 삭제됨 */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Write Error */
  }
}

/**
 * @brief 사용자 저장 영역 삭제
 * 
 */
static void Flash_UserErase(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;                               /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = 0x08000000U + (FLASH_USER_PAGE * FLASH_PAGE_SIZE); /* 삭제 할 시작 페이지 */
  EraseInitStruct.NbPages = 1;                                                     /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Write Error */
  }
}

/**
 * @brief 펌웨어 영역에 쓰기 함수 Flash_FwErase() 함수 먼저 호출 후 사용.
 * 
 * @param flashData: 192Byte 펌웨어 데이터
 * @param flashNo: 192Byte 단위 펌웨어 순번. 0부터 시작
 * @return ErrorStatus: ERROR or SUCCESS
 */
static ErrorStatus Flash_FwWrite(uint16_t flashNo, uint32_t *flashData)
{
  uint32_t Address = FLASH_FW_ADDRESS_START + (flashNo * 192);

  HAL_FLASH_Unlock();
  for (int8_t flashAddrIndex = 0; flashAddrIndex < 48; flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, flashData[flashAddrIndex]) != HAL_OK)
    {
      return ERROR;
    }
    Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
  }

  return SUCCESS;
}

/**
 * @brief
 *
 * @param flashData: 플래쉬 쓰기 사용자 데이터
 * @param addressIndex: 시작주소 0~512 (주소 당 Double word)
 * @param size: 데이터 크기 (단위 : Byte)
 */
void Flash_UserWrite(uint32_t *flashWriteData, uint32_t addressIndex, uint32_t size)
{
  uint32_t Address = 0x08000000 + (FLASH_USER_PAGE * FLASH_PAGE_SIZE);
  uint32_t tmpFlashReadData[FLASH_PAGE_SIZE / 4U] = {
      0U,
  };

  memcpy((void *)tmpFlashReadData, (void *)Address, FLASH_PAGE_SIZE);
  Flash_UserErase();

  memcpy((void *)&tmpFlashReadData[addressIndex], (void *)flashWriteData, size);

  for (int32_t flashAddrIndex = 0; flashAddrIndex < (FLASH_PAGE_SIZE / 4U); flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, tmpFlashReadData[flashAddrIndex]) != HAL_OK)
    {
      /* Write Error */
    }
    Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
  }
}

/**
 * @brief 사용자 영역에 4Byte 읽기
 * 
 * @param addressIndex: 시작주소 0~512
 * @return uint32_t: 해당 주소의 값
 */
uint32_t Flash_UserRead4Byte(uint32_t addressIndex)
{
  uint32_t Address = 0x08000000 + (FLASH_USER_PAGE * FLASH_PAGE_SIZE) + (addressIndex * 4);
  return *(__IO uint32_t *)Address;
}

/**
 * @brief 사용자 영역의 4Byte 쓰기
 * 
 * @param flashWriteData: 사용자 데이터
 * @param addressIndex: 시작주소 0~512
 */
void Flash_UserWrite4Byte(uint32_t flashWriteData, uint32_t addressIndex)
{
  if (addressIndex < (FLASH_PAGE_SIZE / 4U))
  {
    uint32_t Address = 0x08000000 + (FLASH_USER_PAGE * FLASH_PAGE_SIZE);
    uint32_t tmpFlashReadData[FLASH_PAGE_SIZE / 4U] = {
        0U,
    };

    memcpy((void *)tmpFlashReadData, (void *)Address, FLASH_PAGE_SIZE);
    Flash_UserErase();

    tmpFlashReadData[addressIndex] = flashWriteData;
    HAL_FLASH_Unlock();

    for (int32_t flashAddrIndex = 0; flashAddrIndex < (FLASH_PAGE_SIZE / 4U); flashAddrIndex++)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, tmpFlashReadData[flashAddrIndex]) != HAL_OK)
      {
        /* Write Error */
      }
      Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
    }
  }
}

/**
 * @brief 펌웨어 업데이트 프로세스
 *
 * @param firmwareData
 */
uint16_t firmNumLast = 0;
void procFirmwareUpdate(uint8_t *firmwareData)
{
  int error = 1;
  uint8_t eflag = 0;
  uint16_t firmNum;

  firmNum = (firmwareData[0] & 0x7F) * 256;
  firmNum += firmwareData[1];

  if (firmNum == 0)
    firmNumLast = 0;

  if ((firmwareData[0] & 0x80) == 0x80)
    eflag = 1;

  /* 첫 패킷을 받았으면 지우기 */
  if ((firmNum == 0) && (firmNumLast == 0))
  {
    Flash_FwErase();
  }

  if (firmNum == firmNumLast)
  {
    if (Flash_FwWrite(firmNum, (uint32_t *)&firmwareData[2]) == SUCCESS)
    {
      firmNumLast = firmNum + 1;
      error = 0;
    }
    else
    {
      /* error 처리 */
      error = 1;
    }
  }
  else
  {
    if (firmNumLast != 0)
    {
      error = 1;
    }
  }

  if (error == 0)
  {
    firmwareData[0] = (firmNumLast - 1) / 256;
    firmwareData[1] = (firmNumLast - 1) % 256;
    if (eflag)
      firmwareData[0] |= 0x80;

    sendMessageToRasPi(MSGCMD_RESPONSE_FW_ACK, firmwareData, 2);
    //sendMessageToRS232(MSGCMD_RESPONSE_FW_ACK, firmwareData, 2);

    if (eflag == 1) /* 펌웨어 전송 완료 */
    {
      (void)Update_Config_Erase();
      Update_Config_Write((((firmwareData[0] & 0x7F) << 8) + firmwareData[1]) + 1);
      printf("OK");
      NVIC_SystemReset();
    }
  }
}

/**
 * @brief 펌웨어 업데이트 정보 쓰기
 *
 * @param firmwareSize
 */
static void Update_Config_Write(uint32_t firmwareSize)
{
  uint32_t Address = UPDATE_CONFIG_ADDR;
  uint32_t tmpFlashReadData[sizeof(struct CONFIG_BOOT) / 4] = {
      0U,
  };

  memcpy(Boot_Cfg.Idx, "SBAN", 4);
  Boot_Cfg.size = firmwareSize * 192; // 패킷 펌웨어 사이즈
  Boot_Cfg.flag = 1;

  memcpy((void *)tmpFlashReadData, (void *)&Boot_Cfg, sizeof(struct CONFIG_BOOT));

  for (int flashAddrIndex = 0; flashAddrIndex < (sizeof(struct CONFIG_BOOT) / 4); flashAddrIndex++)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, tmpFlashReadData[flashAddrIndex]);
    Address += 4;
  }
}

/**
 * @brief 펌웨어 업데이트 정보 삭제
 *
 * @return int
 */
static int Update_Config_Erase(void)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = UPDATE_CONFIG_ADDR;  /* 삭제 할 페이지 시작 주소 */
  EraseInitStruct.NbPages = 1;                       /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    return 1;
  }
  return 0;
}

/**
 * @brief IO Config 초기값 Flash에 저장
 * 
 * @param flashWriteData: ioConfig 구조체 시작주소
 * @param size: 구조체 크기 (Byte)
 */
void System_Config_Write(uint32_t *flashWriteData, uint32_t size)
{
  uint32_t FlagCompare = 0;
  uint8_t ioConfigFlashData[20];
  uint8_t ioConfigMemData[20];
  memcpy(ioConfigFlashData, (unsigned char *)FLASH_SYSTEM_CONFIG_ADDRESS, size);
  memcpy(ioConfigMemData, flashWriteData, size);

  for (int i = 0; i < size; i++) /* Flash에 쓰여진 값과 비교 */
  {
    if (ioConfigFlashData[i] != ioConfigMemData[i])
    {
      FlagCompare |= 0x01;
    }
  }

  if (FlagCompare == 0x01) /* Flash에 쓰여진 값과 다르면 쓰기 */
  {
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;         /* PAGE 단위 지우기 */
    EraseInitStruct.PageAddress = FLASH_SYSTEM_CONFIG_ADDRESS; /* 삭제 할 페이지 시작 주소 */
    EraseInitStruct.NbPages = 1;                               /* 삭제 할 페이지 수 */

    HAL_FLASH_Unlock();
    uint32_t PageError = 0;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
      /* Error 처리 */
    }

    uint32_t Address = FLASH_SYSTEM_CONFIG_ADDRESS;
    for (int32_t flashAddrIndex = 0; flashAddrIndex < ((size / 4U) + 1); flashAddrIndex++)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *flashWriteData++) != HAL_OK)
      {
        /* Write Error */
      }
      Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
    }
  }
}

/*****************************************부트로더 코드 ********************************************/
typedef void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;

//--------------------------------------------------------
ErrorStatus updateFirmware(void)
{
  /* 어플리케이션 영역 삭제 */
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;                                                         /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = FLASH_APPLECATION_ADDRESS_START;                                             /* 삭제 할 시작주소 */
  EraseInitStruct.NbPages = ((UPDATE_CONFIG_ADDR - FLASH_APPLECATION_ADDRESS_START) / FLASH_PAGE_SIZE) - 1U; /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0U; /*!< 0xFFFFFFFF 값이면 정상 삭제됨 */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Erase Error */
    return ERROR;
  }

  /* 복사 */
  uint32_t Address_Application = FLASH_APPLECATION_ADDRESS_START;
  uint32_t Address_Fiwmare = FLASH_FW_ADDRESS_START;
  uint32_t flashAddrIndexEnd = (Boot_Cfg.size / 4) + 1;

  HAL_FLASH_Unlock();
  for (uint32_t flashAddrIndex = 0U; flashAddrIndex < flashAddrIndexEnd; flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address_Application, *(__IO uint32_t *)Address_Fiwmare) != HAL_OK)
    {
      /* error 처리 */
      return ERROR;
    }
    Address_Application += 4U;
    Address_Fiwmare += 4U;
  }

  return SUCCESS;
}

//--------------------------------------------------------
int Update_Config_Read(void)
{
  memcpy(&Boot_Cfg, (unsigned char *)UPDATE_CONFIG_ADDR, sizeof(struct CONFIG_BOOT));

  if (memcmp(&Boot_Cfg.Idx[0], "SBAN", 4) == 0)
    return 1;
  else
    return 0;
}

//--------------------------------------------------------
void bootloader(void)
{
  memcpy(&Boot_Cfg, (unsigned char *)UPDATE_CONFIG_ADDR, sizeof(struct CONFIG_BOOT));

  if (memcmp(&Boot_Cfg.Idx[0], "SBAN", 4) == 0)
  {
    if (Boot_Cfg.flag && Boot_Cfg.size && Boot_Cfg.size <= 100000)
    {
      if (updateFirmware() == SUCCESS)
      {
        Flash_FwErase();
      }
    }
    Update_Config_Erase();
  }

  /* Jump to user application */
  JumpAddress = *(__IO uint32_t *)(FLASH_APPLECATION_ADDRESS_START + 4);
  Jump_To_Application = (pFunction)JumpAddress;
  /* Initialize user application's Stack Pointer */
  __set_MSP(*(__IO uint32_t *)FLASH_APPLECATION_ADDRESS_START);
  Jump_To_Application();
}

/**
  * @}
  */
