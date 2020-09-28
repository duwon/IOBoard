#ifndef UART_H__
#define UART_H__ 1

#include "main.h"

#define UART_BUFFER_SIZE 600U

typedef struct
{
  uint16_t in;
  uint16_t out;
  uint16_t count;
  uint8_t buff[UART_BUFFER_SIZE];
  uint8_t chBuffer;
  uint8_t chGet;
} uartFIFO_TypeDef; /*!< 수신 패킷 저장 버퍼 구조체 */

void Uart_Init(void); /*!< UART 관련 설정 초기화 */
ErrorStatus getByteFromBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t *ch); /*!< 버퍼에서 1Byte 읽기 */
#endif /* UART_H__ */
