/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1272-Hal.c
 * \brief      SX1272 Hardware Abstraction Layer
 *
 * \version    1.0.B2 ( PRELIMINARY )
 * \date       Nov 21 2012
 * \author     Miguel Luis
 */
#include <stdint.h>
#include <stdbool.h> 

//#include "platform.h"

#if defined( USE_SX1272_RADIO )

#include "stm32f10x_gpio.h"

#include "spi.h"
#include "sx1272-Hal.h"

/*!
 * SX1272 SPI NSS I/O definitions
 */
#define NSS_IOPORT                                  GPIOA
#define NSS_PIN                                     GPIO_Pin_15

#ifdef LORA_MODULE
/*!
 * SX1272 RESET I/O definitions
 */
#define RESET_IOPORT                                GPIOC
#define RESET_PIN                                   GPIO_Pin_15

/*!
 * SX1272 DIO pins  I/O definitions
 */
#define DIO0_IOPORT                                 GPIOC
#define DIO0_PIN                                    GPIO_Pin_0

#define DIO1_IOPORT                                 GPIOC
#define DIO1_PIN                                    GPIO_Pin_1

#define DIO2_IOPORT                                 GPIOC
#define DIO2_PIN                                    GPIO_Pin_2

#define DIO3_IOPORT                                 GPIOC
#define DIO3_PIN                                    GPIO_Pin_3

#define DIO4_IOPORT                                 GPIOB
#define DIO4_PIN                                    GPIO_Pin_10

#define DIO5_IOPORT                                 GPIOB
#define DIO5_PIN                                    GPIO_Pin_11

#else

/*!
 * SX1272 RESET I/O definitions
 */
#define RESET_IOPORT                                GPIOD
#define RESET_PIN                                   GPIO_Pin_0

/*!
 * SX1272 DIO pins  I/O definitions
 */
#define DIO0_IOPORT                                 GPIOD
#define DIO0_PIN                                    GPIO_Pin_1

#define DIO1_IOPORT                                 GPIOD
#define DIO1_PIN                                    GPIO_Pin_2

#define DIO2_IOPORT                                 GPIOD
#define DIO2_PIN                                    GPIO_Pin_3

#define DIO3_IOPORT                                 GPIOD
#define DIO3_PIN                                    GPIO_Pin_4

#define DIO4_IOPORT                                 GPIOD
#define DIO4_PIN                                    GPIO_Pin_5

#define DIO5_IOPORT                                 GPIOD
#define DIO5_PIN                                    GPIO_Pin_6

#endif

#define RXTX_IOPORT                                 
#define RXTX_PIN                                    

void SX1272InitIo( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
    //                        RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // Configure NSS as output
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
    GPIO_InitStructure.GPIO_Pin = NSS_PIN;
    GPIO_Init( NSS_IOPORT, &GPIO_InitStructure );
   
    // Configure radio DIO as inputs
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

    // Configure DIO0
    GPIO_InitStructure.GPIO_Pin =  DIO0_PIN;
    GPIO_Init( DIO0_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO1
    GPIO_InitStructure.GPIO_Pin =  DIO1_PIN;
    GPIO_Init( DIO1_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO2
    GPIO_InitStructure.GPIO_Pin =  DIO2_PIN;
    GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );
    
    // REAMARK: DIO3/4/5 configured are connected to IO expander

    // Configure DIO3 as input
    GPIO_InitStructure.GPIO_Pin =  DIO3_PIN;
    GPIO_Init( DIO3_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO4 as input
    GPIO_InitStructure.GPIO_Pin =  DIO4_PIN;
    GPIO_Init( DIO4_IOPORT, &GPIO_InitStructure );
    
    // Configure DIO5 as input
    GPIO_InitStructure.GPIO_Pin =  DIO5_PIN;
    GPIO_Init( DIO5_IOPORT, &GPIO_InitStructure );
}

void SX1272SetReset( uint8_t state )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if( state == RADIO_RESET_ON )
    {
        // Set RESET pin to 1
        GPIO_WriteBit( RESET_IOPORT, RESET_PIN, Bit_SET );

        // Configure RESET as output
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Pin = RESET_PIN;
        GPIO_Init( RESET_IOPORT, &GPIO_InitStructure );
    }
    else
    {
        GPIO_WriteBit( RESET_IOPORT, RESET_PIN, Bit_RESET );
    }
}

void SX1272Write( uint8_t addr, uint8_t data )
{
    SX1272WriteBuffer( addr, &data, 1 );
}

void SX1272Read( uint8_t addr, uint8_t *data )
{
    SX1272ReadBuffer( addr, data, 1 );
}

void SX1272WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1272ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_RESET );

    SpiInOut( addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    //NSS = 1;
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
}

void SX1272WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1272WriteBuffer( 0, buffer, size );
}

void SX1272ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1272ReadBuffer( 0, buffer, size );
}

uint8_t SX1272ReadDio0( void )
{
    return GPIO_ReadInputDataBit( DIO0_IOPORT, DIO0_PIN );
}

uint8_t SX1272ReadDio1( void )
{
    return GPIO_ReadInputDataBit( DIO1_IOPORT, DIO1_PIN );
}

uint8_t SX1272ReadDio2( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO2_PIN );
}

uint8_t SX1272ReadDio3( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO3_PIN );
}

uint8_t SX1272ReadDio4( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO4_PIN );
}

uint8_t SX1272ReadDio5( void )
{
    return GPIO_ReadInputDataBit( DIO2_IOPORT, DIO5_PIN );
}

void SX1272WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
        //IoePinOn( FEM_CTX_PIN );
    }
    else
    {
        //IoePinOff( FEM_CTX_PIN );
    }
}

#endif // USE_SX1272_RADIO