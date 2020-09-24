/**
  ******************************************************************************
  * @file    flash.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   FLASH 제어
  * @details

  */

#include "flash.h"
#include "stm32f1xx_hal_flash.h"

#define FLASH_FW_ADDRESS_START 0x08020000U /*!< 펌웨어 시작 주소 */
#define FLASH_FW_ADDRESS_END 0x0803FFFFU   /*!< 0x08000000 ~ 0x0803FFFF : 256K, 0~127Page 수정 금지 */
#define FLASH_FW_PAGE_START (((FLASH_FW_ADDRESS_START - 0x08000000U) / FLASH_PAGE_SIZE) - 1)
#define FLASH_FW_PAGE_END 126U
#define FLASH_USER_PAGE 127U

/**
 * @brief 펌웨어 저장 영역 삭제
 * 
 */
void Flash_FwErase(void)
{

  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;                 /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = FLASH_FW_PAGE_START;                 /* 삭제 할 시작 페이지 */
  EraseInitStruct.NbPages = FLASH_FW_PAGE_END - FLASH_FW_PAGE_START; /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0; /*!< 0xFFFFFFFF 값이면 정상 삭제됨 */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Eraser Error */
    while (1)
      ;
  }
}

/**
 * @brief 사용자 저장 영역 삭제
 * 
 */
void Flash_UserErase(void)
{

  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; /* PAGE 단위 지우기 */
  EraseInitStruct.PageAddress = FLASH_USER_PAGE;     /* 삭제 할 시작 페이지 */
  EraseInitStruct.NbPages = 1;                       /* 삭제 할 페이지 수 */

  HAL_FLASH_Unlock();
  uint32_t PageError = 0;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /* Eraser Error */
    while (1)
      ;
  }
}

/**
 * @brief 
 * 
 * @param flashData: 256Byte 펌웨어 데이터
 * @param flashPage: 256Byte 단위 펌웨어 순번. 0부터 시작
 */
void Flash_FwWrite(uint32_t *flashData, uint32_t flashPage)
{
  uint32_t Address = 0x08000000 + (FLASH_FW_PAGE_START * FLASH_PAGE_SIZE) + (flashPage * 256);

  for (int8_t flashAddrIndex = 0; flashAddrIndex < 64; flashAddrIndex++)
  {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, flashData[flashAddrIndex]) != HAL_OK)
    {
      /* Write Error */
      while (1)
        ;
    }
    Address += 4; /* 32bit 단위 쓰기임으로 주소 4byte 증가 */
  }
}
