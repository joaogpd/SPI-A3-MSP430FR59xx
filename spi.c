#include <spi.h>

void SPI_A3_CS_init(void) {
    CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
    CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
    CSCTL2 = SELS__DCOCLK | SELM__DCOCLK;     // SMCLK and MCLK source DCO
    CSCTL3 = DIVS__1 | DIVM__1;               // Set SMCLK and MCLK dividers
    CSCTL0_H = 0;                             // Lock CS registers
}

void SPI_A3_GPIO_init(void) {
    // configure A3 SPI pins, UCA3SIMO: P6.0; UCA3SOMI: P6.1; UCA3CLK: P6.2
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P6,
        GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2,
        GPIO_PRIMARY_MODULE_FUNCTION
    );
    
    PMM_unlockLPM5();
}

void SPI_A3_init(uint8_t clockSource, uint32_t desiredSpiClock, uint16_t clockPhase, uint16_t clockPolarity) {
    // IMPORTANT: the GPIO should be initialized before the CS
    SPI_A3_GPIO_init();
    SPI_A3_CS_init();
    	
    EUSCI_A_SPI_initMasterParam masterParams;
    masterParams.selectClockSource = clockSource;
    masterParams.clockSourceFrequency = CS_getSMCLK();
    masterParams.desiredSpiClock = desiredSpiClock;
    masterParams.msbFirst = EUSCI_A_SPI_MSB_FIRST;
    masterParams.clockPhase = clockPhase;
    masterParams.clockPolarity = clockPolarity;
    masterParams.spiMode = EUSCI_A_SPI_3PIN;
    
    EUSCI_A_SPI_initMaster(EUSCI_A3_BASE, &masterParams);	
    
    EUSCI_A_SPI_enable(EUSCI_A3_BASE);
    
    EUSCI_A_SPI_clearInterrupt(
    	EUSCI_A3_BASE, 
    	EUSCI_A_SPI_TRANSMIT_INTERRUPT | EUSCI_A_SPI_RECEIVE_INTERRUPT
    );
}

void SPI_A3_sendData(uint8_t* sndBuffer, uint8_t sndBufferSize) {
    EUSCI_A_SPI_enableInterrupt(
            EUSCI_A3_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT
    );

    uint8_t byteCounter = 0;

    for (byteCounter = 0; byteCounter < sndBufferSize; byteCounter++) {
        __bic_SR_register(GIE);

       EUSCI_A_SPI_transmitData(EUSCI_A3_BASE, sndBuffer[byteCounter]);

       __bis_SR_register(LPM0_bits | GIE);
    }

    while (EUSCI_A_SPI_isBusy(EUSCI_A3_BASE)); // assure all data has been shifted

    EUSCI_A_SPI_disableInterrupt(
            EUSCI_A3_BASE,
            EUSCI_A_SPI_TRANSMIT_INTERRUPT
    );
}

void SPI_A3_sendByte(uint8_t byte) {
    SPI_A3_sendData(&byte, 1);
}

void SPI_A3_sendRegisterReceiveData(uint8_t regAddr, uint8_t* recvBuffer, uint8_t recvBufferSize) {
    __bic_SR_register(GIE);

    EUSCI_A_SPI_transmitData(EUSCI_A3_BASE, regAddr);

    UCA3IE |= UCRXIE;
    __bis_SR_register(LPM0_bits | GIE); // wait for RX interrupt
     EUSCI_A_SPI_receiveData(EUSCI_A3_BASE); // ignore received value
    UCA3IE &= ~(UCRXIE);

    UCA3IE |= UCTXIE;
    __bis_SR_register(LPM0_bits | GIE); // wait for TX interrupt
    UCA3IE &= ~(UCTXIE);

    __delay_cycles(40);

    // from now on, dummy bytes will be sent for receiving data
    uint8_t byteCounter = 0;

    for (byteCounter = 0; byteCounter < recvBufferSize; byteCounter++) {
        EUSCI_A_SPI_transmitData(EUSCI_A3_BASE, 0x00);

        UCA3IE |= UCRXIE;
        __bis_SR_register(LPM0_bits | GIE);
        recvBuffer[byteCounter] = EUSCI_A_SPI_receiveData(EUSCI_A3_BASE);
        UCA3IE &= ~(UCRXIE);

        UCA3IE |= UCTXIE;
        __bis_SR_register(LPM0_bits | GIE);
        UCA3IE &= ~(UCTXIE);

        __delay_cycles(40);
    }
}

uint8_t SPI_A3_sendRegisterReceiveByte(uint8_t regAddr) {
    uint8_t dataByte = 0x00; // random initialization value

    SPI_A3_sendRegisterReceiveData(regAddr, &dataByte, 1);

    return dataByte;
}

void SPI_A3_receiveData(uint8_t* recvBuffer, uint8_t recvBufferSize) {
    __bic_SR_register(GIE);

    EUSCI_A_SPI_enableInterrupt(
            EUSCI_A3_BASE,
            EUSCI_A_SPI_RECEIVE_INTERRUPT
    );

    uint8_t byteCounter = 0;

	for (byteCounter = 0; byteCounter < recvBufferSize; byteCounter++) {
        __bis_SR_register(LPM0_bits | GIE);

        __bic_SR_register(GIE);

        recvBuffer[recvBufferSize] = EUSCI_A_SPI_receiveData(EUSCI_A3_BASE);
	}

    EUSCI_A_SPI_disableInterrupt(
            EUSCI_A3_BASE,
            EUSCI_A_SPI_RECEIVE_INTERRUPT
    );
}

uint8_t SPI_A3_receiveByte(void) {
    uint8_t dataByte = 0x00;

    SPI_A3_receiveData(&dataByte, 1);

    return dataByte;
}

void __attribute__ ((interrupt(USCI_A3_VECTOR))) USCI_A3_ISR (void) {
	switch (__even_in_range(UCA3IV, USCI_SPI_UCTXIFG)) {
		case USCI_NONE: break;
		case USCI_SPI_UCRXIFG:
            UCA3IFG &= ~(UCRXIFG);

            __bic_SR_register_on_exit(LPM0_bits | GIE);
			break;
		case USCI_SPI_UCTXIFG:
		    UCA3IFG &= ~(UCTXIFG);

            __bic_SR_register_on_exit(LPM0_bits | GIE);
			break;
		default: break;
	}
}
