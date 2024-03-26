# MSP430SPIDriverlib #
SPI abstraction implemented on top of Texas Instruments' Driverlib, part of MSPWARE. Only the EUSCI_A3 module
can be used as of now.

Both send and receive operations have been implemented, aswell as a send and receive dual operation.
Moreover, only 3 pin SPI is supported, the CS pin should be controlled by software. 
