# MSP430SPIDriverlib #
SPI abstraction implemented on top of Texas Instruments' Driverlib, part of MSPWARE.

Both send and receive operations have been implemented, aswell as a send and receive dual operation.
This library should be used with care, as it initializes the clock and the GPIO for the EUSCI_A3 module.
More details can be found in the documentation. Moreover, the initialization functions may be commented 
out and called separately.
