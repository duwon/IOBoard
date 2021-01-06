#ifndef CURRENT_H__
#define CURRENT_H__ 1

#include "main.h"

#define CAL_IGAIN 0x3D0BA  /* 0x47U Default Value */
#define CAL_VGAIN 0x200000 /* 0x48U Default Value */

#define COMMAND_SAVETOFALSH    0 /* Save to Flash Command */
#define COMMAND_AUTOREPORTING_CLR  1 /* Auto Reporting Command - Clear Control.ar */
#define COMMAND_AUTOREPORTING_SET  2 /* Auto Reporting Command - Set Control.ar */
#define COMMAND_CLRCOUNTER      3 /* Clear Energy counters */
#define COMMAND_SOFTRESET       4 /* Soft-Reset */
#define COMMAND_CAL_POWER       5
#define COMMAND_CAL_VRMS        6
#define COMMAND_CAL_IRMS        7
#define COMNAND_CAL_VOFFS       8
#define COMMAND_CAL_IOFFS       9

static const uint32_t EMP_COMMAND[10] = {0xACC200, 0xAE000, 0xAE001, 0xEC0000, 0xBD0000, 0xCA0100, 0xCA0020, 0xCA0010, 0xCA0008, 0xCA0004};

extern float powerMeter;
extern uint64_t sumPowerMeter;

void SY7T609_Test(void);
uint32_t SY7T609_ReadReg(uint8_t regNum);
void SY7T609_WriteReg(uint8_t regNum, uint32_t regData);
void SY7T609_Cal(uint32_t VrmsTarget, uint32_t IrmsTarger);
void SY7T609_Cal_Offset(void);

void EMP_Init(void);
void EMP_Read(float *powerValue);
void EMP_SaveEveragePower(void);
void EMP_UpdateCalValue(uint8_t *CalValue);
void EMP_GetCalValue(uint8_t *RetrunCalValue);

#endif /* CURRENT_H__ */
