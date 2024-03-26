#ifndef SPI_H
#define SPI_H

#include <msp430.h>
#include <stdint.h>
#include <driverlib.h>

/**
 * Initializes the SPI A3 GPIO pins. P6.0: UCA3SIMO, P6.1: UCA3SOMI, P6.2: UCA3CLK.
 * This function also calls PMM_unlockLPM5(), so changes are in effect instantly.
 * IMPORTANT: this should be called before Clock System initialization.
 */
void SPI_A3_GPIO_init(void);

/**
 * Initializes the SPI A3 module and enables it. SPI is configured for MSB first and
 * for 3 pin functionality. The CS/STE should be controlled by software.
 * 
 * @param clockSource source to be used for the SPI clock. Can be: EUSCI_A_SPI_CLOCKSOURCE_SMCLK
 * or EUSCI_A_SPI_CLOCKSOURCE_ACLK. If an invalid value is provided, ACLK will be sourced.
 * @param desiredSpiClock selects the SPI clock's desired SPI, the clock source that 
 * was chosen should be taken in mind.
 */
void SPI_A3_init(uint8_t clockSource, uint32_t desiredSpiClock,
                 uint16_t clockPhase, uint16_t clockPolarity);

/** 
 * Sends a buffer of data through SPI. This call is blocking and will only return when
 * all the data has been sent. 
 */
void SPI_A3_sendData(uint8_t* sndBuffer, uint8_t sndBufferSize);

/**
 * Sends a byte of data through SPI. This call is blocking and will only return when
 * the byte has been sent.
 */
void SPI_A3_sendByte(uint8_t byte);

/**
 * Sends a register address through SPI and receives a buffer of up to 'recvBufferSize' elements
 * from the slave. This call is blocking and will only return when the register address has been sent
 * and all the requested bytes have been received.
 */
void SPI_A3_sendRegisterReceiveData(uint8_t regAddr, uint8_t* recvBuffer, uint8_t recvBufferSize);

/**
 * Sends a register address through SPI and receives a single byte from slave. This call is
 * blocking and will only return when the register address has been sent and another received.
 */
uint8_t SPI_A3_sendRegisterReceiveByte(uint8_t regAddr);

/** 
 * Receives a buffer of data through SPI. This call is blocking and will only return when
 * all the requested bytes have been received.
 */
void SPI_A3_receiveData(uint8_t* recvBuffer, uint8_t recvBufferSize);

/**
 * Receives a byte of data through SPI. This call is blocking and will only return when
 * the byte has been received.
 */
uint8_t SPI_A3_receiveByte(void);


#endif
