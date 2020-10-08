#ifndef FLASH_H__
#define FLASH_H__ 1

#include "main.h"

#define COMPILE_BOOTLOADER  /*!< 부트로더 컴파일용 */

#define FLASH_BOOT_ADDRESS_START 0x08000000U                                                              /*!< 부트로더 시작 주소 */
#define FLASH_APPLECATION_ADDRESS_START 0x08006000U                                                       /*!< 어플리케이션 펌웨어 시작 주소 */
#define FLASH_FW_ADDRESS_START 0x801F000U                                                                 /*!< 업데이트 펌웨어 시작 주소 */
#define FLASH_FW_PAGE_START (((FLASH_FW_ADDRESS_START - FLASH_BOOT_ADDRESS_START) / FLASH_PAGE_SIZE) - 1) /*!< 업데이트 펌웨어 시작 페이지 */
#define FLASH_FW_PAGE_END 110U                                                                            /*!< 업데이트 펌웨어 끝 페이지 */
#define FLASH_USER_PAGE 127U                                                                              /*!< 사용자 페이지 */

void Flash_UserWrite(uint32_t *flashWriteData, uint32_t addressIndex, uint32_t size);
void Flash_UserWrite4Byte(uint32_t flashWriteData, uint32_t addressIndex);
uint32_t Flash_UserRead4Byte(uint32_t addressIndex);

void procFirmwareUpdate(uint8_t *firmwareData);

void bootloader(void);

#endif /* FLASH_H__ */
