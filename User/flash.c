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

/** @defgroup FLASH FLASH 제어 함수
  * @brief FLASH 제어
  * @{
  */

#define FLASH_FW_ADDRESS_START 0x08020000U                                                   /*!< 펌웨어 시작 주소 */
#define FLASH_FW_PAGE_START (((FLASH_FW_ADDRESS_START - 0x08000000U) / FLASH_PAGE_SIZE) - 1) /*!< 펌웨어 시작 페이지 */
#define FLASH_FW_PAGE_END 126U                                                               /*!< 펌웨어 끝 페이지 */
#define FLASH_USER_PAGE 127U                                                                 /*!< 사용자 페이지 */

static void Flash_UserErase(void);

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
 * @param flashData: 256Byte 펌웨어 데이터
 * @param flashPage: 256Byte 단위 펌웨어 순번. 0부터 시작
 */
void Flash_FwWrite(uint32_t *flashData, uint32_t flashPage)
{
  uint32_t Address = 0x08000000 + (FLASH_FW_PAGE_START * FLASH_PAGE_SIZE) + (flashPage * 256);

  HAL_FLASH_Unlock();
  for (int8_t flashAddrIndex = 0; flashAddrIndex < 64; flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, flashData[flashAddrIndex]) != HAL_OK)
    {
      /* Write Error */
      /* while (1); */
    }
    Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
  }
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

/**
  * @}
  */
