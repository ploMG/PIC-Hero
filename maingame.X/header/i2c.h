/*
 * Hi-Tech C I2C library for 16F1825
 * Master mode routines for I2C MSSP port to read and write to slave device 
 * Copyright (C)2011 HobbyTronics.co.uk 2011
 * Freely distributable.
*/

#define I2C_WRITE 0
#define I2C_READ 1

// Initialise MSSP1 port. (16F1829 on VIVA board - other devices may differ)
void i2c_Init(void){

	// Initialise I2C MSSP 1 module
	// Master 100KHz
	TRISB4=1;           	// set SCL and SDA pins as inputs
	TRISB6=1;

	SSP1CON1 = 0b00101000; 	// I2C enabled, Master mode
	SSP1CON2 = 0x00;
    // I2C Master mode, clock = FOSC/(4 * (SSP1ADD + 1)) 
    SSP1ADD = 9;    		// 100Khz @ 4Mhz Fosc

	SSP1STAT = 0b11000000; 	// Slew rate disabled

}

// i2c_Wait - wait for I2C transfer to finish
void i2c_Wait(void){
    while ( ( SSP1CON2 & 0x1F ) || ( SSP1STAT & 0x04 ) );  
}

// i2c_Start - Start I2C communication
void i2c_Start(void)
{
 	i2c_Wait();
	SSP1CON2bits.SEN=1;
}

// i2c_Restart - Re-Start I2C communication
void i2c_Restart(void){
 	i2c_Wait();
	SSP1CON2bits.RSEN=1;
}

// i2c_Stop - Stop I2C communication
void i2c_Stop(void)
{
 	i2c_Wait();
 	SSP1CON2bits.PEN=1;
}

// i2c_Write - Sends one byte of data
void i2c_Write(unsigned char data)
{
 	i2c_Wait();
 	SSP1BUF = data;
}

// i2c_Address - Sends Slave Address and Read/Write mode
// mode is either I2C_WRITE or I2C_READ
void i2c_Address(unsigned char address, unsigned char mode)
{
	unsigned char l_address;

	l_address=address<<1;
	l_address+=mode;
 	i2c_Wait();
 	SSP1BUF = l_address;
}

// i2c_Read - Reads a byte from Slave device
unsigned char i2c_Read(unsigned char ack)
{
	// Read data from slave
	// ack should be 1 if there is going to be more data read
	// ack should be 0 if this is the last byte of data read
 	unsigned char i2cReadData;

 	i2c_Wait();
	SSP1CON2bits.RCEN=1;
 	i2c_Wait();
 	i2cReadData = SSP1BUF;
 	i2c_Wait();
 	if ( ack ) SSP1CON2bits.ACKDT=0;			// Ack
	else       SSP1CON2bits.ACKDT=1;			// NAck
	SSP1CON2bits.ACKEN=1;   		            // send acknowledge sequence

	return( i2cReadData );
}
