#ifndef UART_H__
#define UART_H__ 1

#include "main.h"

#define UART_BUFFER_SIZE 600U
#define MESSAGE_MAX_SIZE 300U

#define MESSAGE_STX 0x02U
#define MESSAGE_ETX 0x03U

#define MSGCMD_REQUEST_TIME 0x11U
#define MSGCMD_UPDATE_CONFIG 0x12U
#define MSGCMD_REQUEST_CONFIG 0x13U
#define MSGCMD_UPDATE_TIME 0x14U
#define MSGCMD_SET_DO 0x15U
#define MSGCMD_UPDATE_FW 0x1FU
#define MSGCMD_UPDATE_WATEMETER_CAL 0x1AU
#define MSGCMD_REQUEST_WATMETER_VAL 0x1BU

#define MSGCMD_INFO_IOVALUE 0x20U
#define MSGCMD_RESPONSE_TIME 0x21U
#define MSGCMD_RESPONSE_CONFIG 0x23U
#define MSGCMD_RESPONSE_FW_ACK 0x2FU
#define MSGCMD_RESPONSE_WATMETER_VAL 0x2BU

typedef enum
{
    START = 0,
    MSGID = 1,
	LENGTH = 10,
    DATA = 2,
    PARSING = 3,
    CHECKSUM = 4,
    SEND = 5,
    RESEND = 6,
    STOP = 7,
    WRITE = 8,
    END = 9
} MessageStage; /*!< 메시지 패킷 분석 단계 */

typedef struct
{
  uint16_t in;
  uint16_t out;
  uint16_t count;
  uint8_t buff[UART_BUFFER_SIZE];
  uint8_t rxCh;
  uint8_t buffCh;
} uartFIFO_TypeDef; /*!< 수신 패킷 저장 버퍼 구조체 */

typedef struct
{
    MessageStage nextStage;
    uint8_t msgID;
    uint8_t datasize;
    uint8_t data[MESSAGE_MAX_SIZE];
    uint8_t checksum;
    void (*parsing)(void);
} message_TypeDef; /*!< 수신 메시지 구조체 */

/* Extern variables ---------------------------------------------------------*/
extern uartFIFO_TypeDef uart1Buffer; /*!< UART1 링 버퍼 구조체 - RS232 */
extern uartFIFO_TypeDef uart2Buffer; /*!< UART2 링 버퍼 구조체 - RS485 */
extern uartFIFO_TypeDef uart4Buffer; /*!< UART4 링 버퍼 구조체 - Raspberry Pi */

extern message_TypeDef uart1Message; /*!< UART1 메시지 구조체 - RS232 */
extern message_TypeDef uart2Message; /*!< UART2 메시지 구조체 - RS485 */
extern message_TypeDef uart4Message; /*!< UART4 메시지 구조체- Raspberry Pi */

/* Extern functions ---------------------------------------------------------*/
void Uart_Init(void); /*!< UART 관련 설정 초기화 */
void initMessage(message_TypeDef *messageFrame, void (*parsingFunction)(void));
void procMesssage(message_TypeDef* messageFrame, uartFIFO_TypeDef* buffer);
void sendMessageToRasPi(uint8_t msgID, uint8_t *txData, uint8_t dataLen);
void sendMessageToRS232(uint8_t msgID, uint8_t *txData, uint8_t dataLen);
#endif /* UART_H__ */
