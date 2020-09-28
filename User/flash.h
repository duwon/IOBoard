#ifndef FLASH_H__
#define FLASH_H__ 1

#include "main.h"

void Flash_FwWrite(uint32_t *flashData, uint32_t flashPage);

void Flash_UserWrite(uint32_t *flashWriteData, uint32_t addressIndex, uint32_t size);
void Flash_UserWrite4Byte(uint32_t flashWriteData, uint32_t addressIndex);
uint32_t Flash_UserRead4Byte(uint32_t addressIndex);

#endif /* FLASH_H__ */
