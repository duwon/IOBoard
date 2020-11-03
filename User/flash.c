/**
  ******************************************************************************
  * @file    flash.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   FLASH 제어
  * @details

  */

#include <string.h>
#include "flash.h"
#include "stm32f1xx_hal_flash.h"

#include "uart.h"

/** @defgroup FLASH FLASH 제어 함수
  * @brief FLASH 제어
  * @{
  */

static void Flash_UserErase(void);
static ErrorStatus Flash_FwWrite(uint16_t flashNo, uint32_t *flashData);

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
    /* while (1); */
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
    /* while (1); */
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
      /* while (1); */
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
        /* while (1); */
      }
      Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
    }
  }
}

void procFirmwareUpdate(uint8_t *firmwareData)
{
  //static uint16_t firmNumLast = 0;

  uint16_t firmNum = (firmwareData[0] << 8) + firmwareData[1];

  if (Flash_FwWrite(firmNum, (uint32_t *)&firmwareData[2]) == SUCCESS)
  {
    sendMessageToRasPi(MSGCMD_RESPONSE_FW_ACK, firmwareData, 2);
  }
  else
  {
    /* error 처리 */
  }
}

/*****************************************부트로더 코드 ********************************************/
#include <stdio.h>
#include "iwdg.h"
#include "rtc.h"

typedef void (*pFunction)(void);

ErrorStatus updateFirmware(void)
{
  HAL_IWDG_Refresh(&hiwdg);

  /* 어플리케이션 영역 삭제 */
  printf("App Erase.\r\n");
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;                                                             /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = FLASH_APPLECATION_ADDRESS_START;                                                 /* 삭제 할 시작주소 */
  EraseInitStruct.NbPages = ((FLASH_FW_ADDRESS_START - FLASH_APPLECATION_ADDRESS_START) / FLASH_PAGE_SIZE) - 1U; /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0U; /*!< 0xFFFFFFFF 값이면 정상 삭제됨 */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Erase Error */
    printf("Erase Error.\r\n");
    return ERROR;
  }
  HAL_IWDG_Refresh(&hiwdg);

  /* 복사 */
  printf("Fw Copy.\r\n");
  uint32_t Address_Application = FLASH_APPLECATION_ADDRESS_START;
  uint32_t Address_Fiwmare = FLASH_FW_ADDRESS_START;

  for (uint32_t flashAddrIndex = 0U; flashAddrIndex < ((FLASH_FW_ADDRESS_START - FLASH_APPLECATION_ADDRESS_START) / 4U); flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address_Application, *(__IO uint32_t *)Address_Fiwmare) != HAL_OK)
    {
      /* error 처리 */
      printf("Write Error.\r\n");
      return ERROR;
    }
    Address_Application += 4U;
    Address_Fiwmare += 4U;
    HAL_IWDG_Refresh(&hiwdg);
  }

  printf("Update Completed.\r\n");
  return SUCCESS;
}

void receivedFirmware(void)
{
	switch (uart4Message.msgID)
	{
	case MSGCMD_UPDATE_FW:
		procFirmwareUpdate(uart4Message.data);
		break;
	default:
		printf("MSG ERROR\r\n");
	    NVIC_SystemReset();
		break;
	}
}


void bootloader(void)
{
  printf("\r\n\r\nSTART BOOTLOADER\r\n");


  Uart_Init();
  initMessage(&uart4Message, receivedFirmware);

  uint16_t restartCount = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
  printf("RESET Count : %0x\r\n", restartCount++);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, restartCount);


  uint32_t mspValue = *(__IO uint32_t *)FLASH_FW_ADDRESS_START;
  printf("MSP : %#010X\r\n", (unsigned int)mspValue);
  if (mspValue != 0xFFFFFFFF)
  {
    if (updateFirmware() == SUCCESS)
    {
      HAL_IWDG_Refresh(&hiwdg);
      Flash_FwErase();
      printf("Fw Erase.\r\n");
    }
  }


  if(restartCount > 5U)
  {
	  printf("Application booting Fail\r\n");
	  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
	  while(1)
	  {
		  procMesssage(&uart4Message, &uart4Buffer);
		  HAL_IWDG_Refresh(&hiwdg);
	  }
  }



  pFunction JumpToApplication;
  /* 어플리케이션 포인터 함수 */
  uint32_t JumpAddress = *(__IO uint32_t *)(FLASH_APPLECATION_ADDRESS_START + 4U);
  JumpToApplication = (pFunction)JumpAddress;
  /* 어플리케이션 스택포인터 설정*/
  __set_MSP(*(__IO uint32_t *)FLASH_APPLECATION_ADDRESS_START);
  JumpToApplication();
}

/**
  * @}
  */
