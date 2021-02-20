/**
  ******************************************************************************
  * @file    uart.c
  * @author  정두원
  * @date    2020-09-18
  * @brief   UART 제어
  * @details

  */

#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "usart.h"
#include "led.h"

/** @defgroup UART UART 제어 함수
  * @brief UART 제어 및 링 버퍼
  * @{
  */

/* Private variables ---------------------------------------------------------*/
uartFIFO_TypeDef uart1Buffer; /*!< UART1 링 버퍼 구조체 - RS232 */
uartFIFO_TypeDef uart2Buffer; /*!< UART2 링 버퍼 구조체 - RS485 */
uartFIFO_TypeDef uart4Buffer; /*!< UART4 링 버퍼 구조체 - Raspberry Pi */

message_TypeDef uart1Message; /*!< UART1 메시지 구조체 - RS232 */
message_TypeDef uart2Message; /*!< UART2 메시지 구조체 - RS485 */
message_TypeDef uart4Message; /*!< UART4 메시지 구조체- Raspberry Pi */

/* Private functions ---------------------------------------------------------*/
static ErrorStatus putByteToBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t ch);    /*!< 버퍼에 1Byte 쓰기 */
static ErrorStatus getByteFromBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t *ch); /*!< 버퍼에서 1Byte 읽기 */
static void initBuffer(volatile uartFIFO_TypeDef *buffer);
static uint8_t calChecksum(message_TypeDef *messageFrame);

/* printf IO 사용을 위한 설정 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

/**
 * @brief UART 관련 설정
 * 
 */
void Uart_Init(void)
{
  initBuffer(&uart1Buffer);
  initBuffer(&uart2Buffer);
  initBuffer(&uart4Buffer);

  (void)HAL_UART_Receive_DMA(&huart1, &uart1Buffer.rxCh, 1U);
  //(void)HAL_UART_Receive_DMA(&huart1, &uart4Buffer.rxCh, 1U);
  (void)HAL_UART_Receive_DMA(&huart2, &uart2Buffer.rxCh, 1U);
  (void)HAL_UART_Receive_DMA(&huart4, &uart4Buffer.rxCh, 1U);
}

/**
  * @brief  UART RX 인터럽트
  * 
  * @param huart
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  __disable_irq();
  if (huart->Instance == USART1) /* RS232 */
  {
    (void)putByteToBuffer(&uart1Buffer, uart1Buffer.rxCh);
    //(void)putByteToBuffer(&uart4Buffer, uart4Buffer.rxCh);
  }
  if (huart->Instance == USART2) /* RS485 */
  {
    (void)putByteToBuffer(&uart2Buffer, uart2Buffer.rxCh);
  }
  if (huart->Instance == UART4) /* Raspberry Pi */
  {
    (void)putByteToBuffer(&uart4Buffer, uart4Buffer.rxCh);
  }
  __enable_irq();
}

/**
 * @brief UART 에러 발생 인터럽트
 * 
 * @param huart 
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->ErrorCode == HAL_UART_ERROR_ORE)
  {
    /* Overrun error 처리 */
  }
}

/**
 * @brief UART 수신 버퍼 초기화
 * 
 * @param buffer 
 */
static void initBuffer(volatile uartFIFO_TypeDef *buffer)
{
  buffer->count = 0U; /* 버퍼에 저장된 데이터 갯수 초기화 */
  buffer->in = 0U;    /* 버퍼 시작 인덱스 초기화*/
  buffer->out = 0U;   /* 버퍼 끝 인덱스 초기화 */
}

/**
 * @brief 버퍼에 1byte 저장
 * 
 * @param buffer: UART 버퍼 구조체 포인터
 * @param ch: 저장 할 Byte 데이터
 */
static ErrorStatus putByteToBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t ch)
{
  ErrorStatus status = ERROR;

  if (buffer->count != UART_BUFFER_SIZE) /* 데이터가 버퍼에 가득 찼으면 ERROR 리턴 */
  {
    buffer->buff[buffer->in] = ch; /* 버퍼에 1Byte 저장 */
    buffer->in++;
    buffer->count++;                    /* 버퍼에 저장된 갯수 1 증가 */
    if (buffer->in == UART_BUFFER_SIZE) /* 시작 인덱스가 버퍼의 끝이면 */
    {
      buffer->in = 0U; /* 시작 인덱스를 0부터 다시 시작 */
    }
    status = SUCCESS;
  }
  else
  {
    status = ERROR;
  }

  return status;
}

/**
 * @brief 버퍼에서 1byte 읽기
 * 
 * @param buffer: UART 버퍼 구조체 포인터
 * @param ch: 가져 갈 Byte 데이터 포인터
 * @return ErrorStatus: 버퍼에 데이터가 없으면 ERROR
 *         @arg SUCCESS, ERROR
 */
static ErrorStatus getByteFromBuffer(volatile uartFIFO_TypeDef *buffer, uint8_t *ch)
{
  __disable_irq();
  ErrorStatus status = ERROR;
  if (buffer->count != 0U) /* 버퍼에 데이터가 있으면 */
  {

    *ch = buffer->buff[buffer->out]; /* 버퍼에서 1Byte 읽음 */
    //buffer->buff[buffer->out] = 0U;
    buffer->out++;
    buffer->count--;                     /* 버퍼에 저장된 데이터 갯수 1 감소 */
    if (buffer->out == UART_BUFFER_SIZE) /* 끝 인덱스가 버퍼의 끝이면 */
    {
      buffer->out = 0U; /* 끝 인덱스를 0부터 다시 시작 */
    }
    status = SUCCESS;
  }
  else
  {
    status = ERROR;
  }
  __enable_irq();
  return status;
}

void initMessage(message_TypeDef *messageFrame, void (*parsingFunction)(void))
{
  /**
메시지 프레임 구조체를 초기화
**/
  messageFrame->nextStage = START;
  messageFrame->msgID = 0U;
  messageFrame->datasize = 0U;
  messageFrame->checksum = 0U;
  messageFrame->parsing = parsingFunction;
}

static uint8_t calChecksum(message_TypeDef *messageFrame)
{
#if 0
  uint8_t tmpCalChecksum = MESSAGE_STX ^ MESSAGE_STX ^ MESSAGE_ETX ^ messageFrame->msgID ^ messageFrame->datasize;
#else
  uint8_t tmpCalChecksum = MESSAGE_STX ^ MESSAGE_ETX ^ messageFrame->msgID ^ messageFrame->datasize;
#endif
  for (int i = 0; i < messageFrame->datasize; i++)
  {
    tmpCalChecksum ^= messageFrame->data[i];
  }

  return tmpCalChecksum;
}

void procMesssage(message_TypeDef *messageFrame, uartFIFO_TypeDef *buffer)
{
  //static int cntSTX = 0;
  switch (messageFrame->nextStage)
  {
  case START:
    if (getByteFromBuffer(buffer, &(buffer->buffCh)) == SUCCESS)
    {
#if 0
      if (buffer->buffCh == MESSAGE_STX)
      {
    	  cntSTX++;
      }
      else
      {
    	  cntSTX = 0;
      }
      if(cntSTX == 2)
      {
    	  messageFrame->nextStage = MSGID;
    	  cntSTX = 0;
      }
#else
      if (buffer->buffCh == MESSAGE_STX)
      {
        messageFrame->nextStage = MSGID;
      }
#endif
    }
    break;
  case MSGID:
    if (getByteFromBuffer(buffer, &(buffer->buffCh)) == SUCCESS)
    {
      messageFrame->msgID = buffer->buffCh;
      messageFrame->nextStage = LENGTH;
    }
    break;
  case LENGTH:
    if (getByteFromBuffer(buffer, &(buffer->buffCh)) == SUCCESS)
    {
      messageFrame->datasize = buffer->buffCh;
      if (messageFrame->datasize == 0)
      {
        messageFrame->nextStage = END;
      }
      else if ((messageFrame->datasize == 194) || (messageFrame->datasize < 20))
      {
        messageFrame->nextStage = DATA;
      }
      else
      {
        messageFrame->nextStage = START;
      }
    }
    break;
  case DATA:
    if (buffer->count >= messageFrame->datasize)
    {
      for (int i = 0; i < messageFrame->datasize; i++)
      {
        (void)getByteFromBuffer(buffer, &messageFrame->data[i]);
      }
      messageFrame->nextStage = END;
    }
    break;
  case END:
    if (getByteFromBuffer(buffer, &(buffer->buffCh)) == SUCCESS)
    {
      if (buffer->buffCh == MESSAGE_ETX)
      {
        messageFrame->nextStage = CHECKSUM;
      }
      else
      {
        messageFrame->nextStage = START;
      }
    }
    break;
  case CHECKSUM:
    if (getByteFromBuffer(buffer, &(buffer->buffCh)) == SUCCESS)
    {

      if (buffer->buffCh == calChecksum(messageFrame))
      {
        messageFrame->nextStage = PARSING;
      }
      else
      {
        messageFrame->nextStage = START;
      }
    }
    break;
  case PARSING:
    if (messageFrame->parsing != NULL)
    {
      messageFrame->parsing();
      messageFrame->nextStage = START;
    }
    break;
  default:
    break;
  }
}

/**
 * @brief 라즈베리파이에 데이터 전송
 * 
 * @param msgID: Command 
 * @param txData: Payload Data
 * @param dataLen: Payload Data 길이
 */
void sendMessageToRasPi(uint8_t msgID, uint8_t *txData, uint8_t dataLen)
{
  uint8_t txPacket[MESSAGE_MAX_SIZE] = {
      0,
  };
  txPacket[0] = MESSAGE_STX;
  txPacket[1] = msgID;
  txPacket[2] = dataLen;
  txPacket[dataLen + 3] = MESSAGE_ETX;

  memcpy((void *)&txPacket[3], (void *)txData, dataLen);

  for (int i = 0; i < (dataLen + 4); i++)
  {
    txPacket[dataLen + 4] ^= txPacket[i];
  }

  HAL_UART_Transmit(&huart4, txPacket, dataLen + 5, 0xFFFF);
}

/**
 * @brief RS232에 데이터 전송
 *
 * @param msgID: Command
 * @param txData: Payload Data
 * @param dataLen: Payload Data 길이
 */
void sendMessageToRS232(uint8_t msgID, uint8_t *txData, uint8_t dataLen)
{
  uint8_t txPacket[MESSAGE_MAX_SIZE] = {
      0,
  };
  txPacket[0] = MESSAGE_STX;
  txPacket[1] = msgID;
  txPacket[2] = dataLen;
  txPacket[dataLen + 3] = MESSAGE_ETX;

  memcpy((void *)&txPacket[3], (void *)txData, dataLen);

  for (int i = 0; i < (dataLen + 4); i++)
  {
    txPacket[dataLen + 4] ^= txPacket[i];
  }

  HAL_UART_Transmit(&huart1, txPacket, dataLen + 5, 0xFFFF);
  LED_Toggle(LD_RS232RDY);
}

/**
  * @}
  */
