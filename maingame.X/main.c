// Michael Gomez, Ranbir Singh, Angel Genao
// Dr. Morton, Microcomputer Systems I, Spring 2023

/* PIC HERO */

#include <xc.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include  "header/i2c.h"
#include  "header/i2c_LCD.h"


// PIC16F1829 Configuration Bit Settings
// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

// UART/I2C Config
#define I2C_SLAVE 0x27	/* was 1E Channel of i2c slave depends on soldering on back of board*/
#define BAUD 9600  
#define FOSC 4000000L  
#define DIVIDER ((int)(FOSC/(16UL * BAUD) -1)) // Should be 25 for 9600/4MhZ  
#define NINE_BITS 0  
#define SPEED 0x4  
#define RX_PIN TRISC5  
#define TX_PIN TRISC4
#define _XTAL_FREQ 4000000.0    /*for 4mhz*/

// Define pins for 8x8
#define DIN LATCbits.LATC6
#define CS LATCbits.LATC2
#define CLK LATCbits.LATC3


// Prototypes
void I2C_LCD_Command(unsigned char,unsigned char);
void I2C_LCD_SWrite(unsigned char,unsigned char *, char);
void I2C_LCD_Init(unsigned char);
void I2C_LCD_Pos(unsigned char,unsigned char);
unsigned char I2C_LCD_Busy(unsigned char);
void pinConfig(void);  
void setup_comms(void);  
void putch(unsigned char);  
unsigned char getch(void);  
unsigned char getche(void);
void init8x8(void);
void SPI_Write(uint8_t data);
void MAX7219_Write(uint8_t reg, uint8_t data);
void display(const uint8_t *data);
void timer_config(void);
void clockAndpin_config(void);


//Global variables
int Touch[4];
int indx = 0; 
int Threshold = 0x15ce;     /*touchbutton sensitivity*/
int score = 0;
int push = 4;


void main(void) {

    pinConfig();  
  
    setup_comms(); // set up the USART - settings defined in usart.h  
    
    /*TouchPad Setup*/  
    CPSCON0 = 0x8C;   //Set up Touch sensing module control reg 0
    
    Touch[0] = 0x04;// RC0 pad CPS4
    Touch[1] = 0x05;// RC1 pad CPS5
    Touch[2] = 0x09;// RC7 pad CPS9
    Touch[3] = 0x03;// RA4 pad CPS3
    
    /*Clock and Pin Configuration*/
    clockAndpin_config();      //Configures clock and pins, enables timers
    timer_config();

    //Variables
	unsigned char  Sout[16];
	unsigned char * Sptr;
    int z;
    int song;
    int write;
    unsigned int shift;
	Sptr = Sout;

    //Clock and Pin Configs
    // Get set up for A2D  
    ADCON1 = 0xC0; //Right justify and Fosc/4 and Vss and Vdd references
	OSCCON  = 0x6A; /* b6..4 = 1101 = 4MHz */
    TRISA = 0x00; //0b00000000 TRIS is like mode( i.e. output or input)
    PORTA = 0x00; //0b00000000 PORT is like status (i.e. on or off)
    TRISB = 0x07;
    ANSELB = 0x00;
    LATA5 = 1;  //Green
    LATA2 = 1;  //Blue
    LATC6 = 1;  //Red  

	/*  ********************************I2C**************************** */
	/*  Note the I2C write to LCD uses the 8 bits of the PCF8574 chip to control the LCD  */
	/*  Connected High 4 bits are High 4 data for LCD (use in 4 bit mode)  the other 4 are     */
	/*  bit3=turn on /off bk light  bit 2= E line writes on Hi 2 Lo transition, reads Lo to Hi */
	/*  bit 2=Read write set  to 0 for write   bit 0=RS  command=0 Data=1      */
	/*  ********************************I2C**************************** */
    
    // SDA -> RB4
    // SCL -> RB6
	i2c_Init();				// Start I2C as Master 100KH
	I2C_LCD_Init(I2C_SLAVE); //pass I2C_SLAVE to the init function to create an instance
	


	//Selection Screen

    while(1){

    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE,0x00);
    sprintf(Sout, " Click to Start ");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 16);

    I2C_LCD_Pos(I2C_SLAVE,0x40);  
    sprintf(Sout, "    PIC Hero");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 12);

    while(RB7 == 1){
        __delay_ms(10);   
    }
    
    __delay_ms(300);
        
    I2C_LCD_Command(I2C_SLAVE, 0x01);
    I2C_LCD_Pos(I2C_SLAVE,0x00);
    sprintf(Sout, " Easy Med Hard ");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);
    
    I2C_LCD_Pos(I2C_SLAVE,0x40);  
    sprintf(Sout, " ----");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 5);
    
    song = 1;
    
    do{
           /*Y value read*/
       ADCON0 = 0x1D; // set up for the analog stick directional input channel = AN7
       __delay_ms(10); //Allow cap to recharge  
       ADGO = 1; // initiate conversion on the selected channel  
       while(ADGO)continue;  
       shift = ((ADRESH<<8)+(ADRESL)); //Store 10 bits into Pval, 8 + 2
       
       if (shift < 250){
           if (song == 1)
           {
                I2C_LCD_Command(I2C_SLAVE, 0x01);
                I2C_LCD_Pos(I2C_SLAVE,0x00);
                sprintf(Sout, " Easy Med Hard ");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);

                I2C_LCD_Pos(I2C_SLAVE,0x40);
                sprintf(Sout, "      ---");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 9);   
                __delay_ms(500);
                
                song = 2;
           }

            else if (song == 2)
           {
                I2C_LCD_Command(I2C_SLAVE, 0x01);
                I2C_LCD_Pos(I2C_SLAVE,0x00);
                sprintf(Sout, " Easy Med Hard ");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);

                I2C_LCD_Pos(I2C_SLAVE,0x40);
                sprintf(Sout, "          ----");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 14);
                __delay_ms(500);
                
                song = 3;
           }
       }
       
       else if (shift > 750){
            if (song == 2)
           {
                I2C_LCD_Command(I2C_SLAVE, 0x01);
                I2C_LCD_Pos(I2C_SLAVE,0x00);
                sprintf(Sout, " Easy Med Hard ");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);

                I2C_LCD_Pos(I2C_SLAVE,0x40);
                sprintf(Sout, " ----");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 5);
                __delay_ms(500);
                
                song = 1;
           }

            else if (song == 3)
           {
                I2C_LCD_Command(I2C_SLAVE, 0x01);
                I2C_LCD_Pos(I2C_SLAVE,0x00);
                sprintf(Sout, " Easy Med Hard ");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 15);

                I2C_LCD_Pos(I2C_SLAVE,0x40);
                sprintf(Sout, "      ---");
                I2C_LCD_SWrite(I2C_SLAVE, Sout, 9);
                __delay_ms(500);
                
                song = 2;
           }
       }
       
    } while (RB7 == 1);
    
    I2C_LCD_Command(I2C_SLAVE, 0x01);

    I2C_LCD_Pos(I2C_SLAVE,0x00);
    sprintf(Sout, "     Ready?");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 11);
        
    __delay_ms(2000);
                
    I2C_LCD_Command(I2C_SLAVE, 0x01);
        
    I2C_LCD_Pos(I2C_SLAVE,0x40);
    sprintf(Sout, "      Go!");
    I2C_LCD_SWrite(I2C_SLAVE, Sout, 9);
    
    //Set ISR
    TMR0IE = 1;
    GIE = 1;
    
        //Begin note display on 8x8 matrix
    
    if (song == 1) slowride();
    else if (song == 2) everlong();
    else if (song == 3) heaviest();
    
    TMR0IE = 0;
    GIE = 0;
    
        //Score screen

    I2C_LCD_Command(I2C_SLAVE, 0x01);
    
    I2C_LCD_Pos(I2C_SLAVE,0x00);
    sprintf(Sout, "Score: %d", score);
    
    if (score >= 10000){
        write = 12;
    }
    else if (score >= 1000){
        write = 11;
    }
    else if (score >= 100){
        write = 10;
    }
    else if (score >= 10){
        write = 9;
    }
    else write = 8;
    
    I2C_LCD_SWrite(I2C_SLAVE, Sout, write);
    
    while(RB7 == 1){
        __delay_ms(10);   
    }
    
    __delay_ms(300);

    }
}


void setup_comms(void)  {  
RX_PIN = 1;  
TX_PIN = 1;  
SPBRG = DIVIDER;  
RCSTA = (NINE_BITS | 0x90);  
TXSTA = (SPEED | NINE_BITS | 0x20);  
TXEN = 1;  
SYNC = 0;  
SPEN = 1;  
BRGH = 1;  
  }  

void putch(unsigned char byte)  {  
/* output one byte */  
while(!TXIF) /* set when register is empty */  
continue;  
TXREG = byte;  
  }  

unsigned char getch()  {  
/* retrieve one byte */  
while(!RCIF) /* set when register is not empty */  
continue;  
return RCREG;  
  }  

unsigned char getche(void)  {  
unsigned char c;  
putch(c = getch());  
return c;  
  }  
  
void pinConfig(void)  {  
 TXCKSEL = 1; // both bits in APFCON0 MUST BE 1 for 1829 0 for 1825  
 RXDTSEL = 1; /* makes RC4 & 5 TX & RX for USART (Allows ICSP)*/  
  }

void init8x8(void) {
    OSCCON = 0x68; // 4 MHz internal oscillator
    ANSELC = 0x00; // Set PORTC as digital
    TRISC = 0x00;  // Set PORTC as output
    DIN = 0;
    CS = 0;
    CLK = 0;
}

void SPI_Write(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        CLK = 0;
        DIN = (data & 0x80) >> 7; // Send MSB
        data <<= 1;
        CLK = 1;
    }
}

void MAX7219_Write(uint8_t reg, uint8_t data) {
    CS = 0;
    SPI_Write(reg);
    SPI_Write(data);
    CS = 1;
}

void display(const uint8_t *data) {
    for (uint8_t i = 0; i < 8; i++) {
        MAX7219_Write(i + 1, data[i]);
    }
}


void greennote(){
    push = 0;
}

void rednote(){
    push = 1;
}

void yellownote(){
    push = 2;
}

void bluenote(){
    push = 3;
}

void emptynote(){
    push = 4;
}

void delay1(){
    __delay_ms(526); //slowride tempo
}

void delay2(){
    __delay_ms(127); //everlong tempo
}

void delay3(){
    __delay_ms(97);  //THMOTU tempo
}


void clockAndpin_config(){
    OSCCON      =	0X6A;  	//set up 4MHz for Fosc
    INTCON      =   0;      // purpose of disabling the interrupts.
    OPTION_REG  =   0XC5;  	// set up timer 0, and then timer 1
    T1CON    	=	0XC1;   //TMR 1 Enable
    T1GCON   	=  	0X81;   //81 is without toggle mode A1 is toggle mode
    TRISA   	= 	0X10;
    TRISC   	=	0XFF;
    PORTA   	= 	0;
    ANSELA  	=   0X10;
}

void timer_config(){
    TMR0    	=   0;
    TMR1H       =   0;
    TMR1L   	=   0;
    TMR1ON      =   1;
    TMR0IF  	=   0;      	//Clear the interrupt flag for Timer 1
    TMR0    	=   0;  
}

void interrupt get_Touch(void){     /*ISR - reads user input on touchbuttons*/
        
    if (TMR0IF != 1) RESET();
    
    CPSCON1 = Touch[indx];
    
    if(((TMR1H<<8)+TMR1L) < Threshold) {

    // ****HereCheck if correct pad pushed is equal to indx and add points
        if(indx == push){
            score +=10;        
        }
    }
    
    timer_config();
    indx +=1;

    if (indx > 3) indx = 0; //cycle indx from 0,1,2,3 and back to 0
    
}



/*Ctrl + Shift + Minus to fold code for easy viewing*/



// Notes for Song 1
const uint8_t green1_1[8] ={ 

    0b00100000,

    0b00000000,    

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t green1_2[8] ={

    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000

};
const uint8_t green1_3[8] ={

    0b00100000,

    

    0b00000000,

    

    0b00100000,



    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000

};
const uint8_t green1[8] = {





    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00000000,



    0b00000000,



    0b00000000



};
const uint8_t green2[8] = {



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00000000,



    0b00000000





};
const uint8_t green3[8] = {



    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00000000



};//delay yellow
const uint8_t green4[8] = {



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000000



};
const uint8_t green5[8] = {



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000



};
const uint8_t green6[8] = {



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000

};
const uint8_t green7[8] = {



    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000

            

};// 2nd delay yellow
const uint8_t green8[8] = {



    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000

};
const uint8_t green9[8] = {



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000

};
const uint8_t green10[8] = {



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000

}; // yellow
const uint8_t green11[8] = {



    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000

};
const uint8_t green12[8] = {



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000

};
const uint8_t green13[8] = {



    0b00000100,



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000

}; //yellow
const uint8_t green14[8] = {





    0b00100000,



    0b00000100,



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000

};
const uint8_t green15[8] = {



    0b00000000,



    0b00100000,



    0b00000100,



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000

};
const uint8_t green16[8] = {



    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000100,



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000

};
const uint8_t green17[8] = {



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000100,



    0b00000100,

    

    0b00000100,



    0b00100000

};
const uint8_t green18[8] = {



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000100,



    0b00000100,

    

    0b00000100

};
const uint8_t green19[8] = {



    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000100,



    0b00000100

};
const uint8_t green20[8] = {



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00000100

};
const uint8_t green21[8] = {



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000

};
const uint8_t green22[8] = {



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000

};
const uint8_t green23[8] = {



    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000,

    

    0b00100000

};
const uint8_t green24[8] = {



    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000,



    0b00000000

};
const uint8_t green25[8] = {



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000,



    0b00100000

};
const uint8_t green26[8] = {



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000,

    

    0b00000000

};
const uint8_t green27[8] = {



    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000,



    0b00010000

};
const uint8_t green28[8] = {



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000,



    0b00001000

};
const uint8_t green29[8] = {



    0b00000100, //36



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000,



    0b00100000

};
const uint8_t green30[8] = {



    0b00100000,

    

    0b00000100, //36



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000,

    

    0b00000000

};
const uint8_t green31[8] = {





    0b00000000,



    0b00100000,



    0b00000100, //36



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000,

    

    0b00100000

};
const uint8_t green32[8] = {



    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000100, //36



    0b00000100,

    

    0b00000100,



    0b00100000,



    0b00000000

};
const uint8_t green33[8] = {



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000100, //36



    0b00000100,

    

    0b00000100,



    0b00100000 //33

};
const uint8_t green34[8] = {



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000100, //36



    0b00000100,

    

    0b00000100

};
const uint8_t green35[8] = {



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000100, //36



    0b00000100

};
const uint8_t green36[8] = {



    0b00100000,



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000100 //36

};
const uint8_t green37[8] = {



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000

};
const uint8_t green38[8] = {



    0b00100000,



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000

};
const uint8_t green39[8] = {



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000

};
const uint8_t green40[8] = {



    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00010000 //40

};
const uint8_t green41[8] = {



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000

};
const uint8_t green42[8] = {





    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000,



    0b00000000

};
const uint8_t green43[8] = {



    0b00000000,



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000000,



    0b00100000

};
const uint8_t green44[8] = {



    0b00100000,



    0b00000000,//50



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000,



    0b00000000 //44



};
const uint8_t green45[8] = {



    0b00000000,



    0b00100000,



    0b00000000,//50



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000,



    0b00100000



};
const uint8_t green46[8] = {



    0b00000000,



    0b00000000,



    0b00100000,



    0b00000000,//50



    0b00100000,



    0b00010000,

    

    0b00001000,

    

    0b00000000



};
const uint8_t green47[8] = {



    0b00000000,



    0b00000000,



    0b00000000,



    0b00100000,



    0b00000000,//50



    0b00100000,



    0b00010000,

    

    0b00001000



};
const uint8_t green48[8] = {



    0b00000000,

    

    0b00000000,



    0b00000000,



    0b00000000,



    0b00100000,



    0b00000000,//50



    0b00100000,



    0b00010000



};
const uint8_t green49[8] = {







    0b00000000,

    

    0b00000000,

    

    0b00000000,



    0b00000000,



    0b00000000,



    0b00100000,



    0b00000000,//50



    0b00100000



};
const uint8_t green50[8] = {



    0b00000000,



    0b00000000,

    

    0b00000000,

    

    0b00000000,



    0b00000000,



    0b00000000,



    0b00100000,



    0b00000000 //50



};
const uint8_t green51[8] = {



    0b00000000, //



    0b00000000,



    0b00000000,

    

    0b00000000,

    

    0b00000000,



    0b00000000,



    0b00000000,



    0b00100000



};
const uint8_t green52[8] = {



    0b00000000,



    0b00000000, //



    0b00000000,



    0b00000000,

    

    0b00000000,

    

    0b00000000,



    0b00000000,



    0b00000000



};
 
void greenbegins(){

        printf("1");

        display(green1_1); 

        delay1();

        

        display(green1_2);

        delay1();

        

        display(green1_3);

        delay1();

    

        display(green1);

        delay1();

        

        display(green2);

        delay1();

       

        display(green3);

        delay1();

        

        display(green4);

        delay1();

        

        display(green5);

        greennote();
        
        delay1();




        display(green6);

        emptynote();

        delay1();

        

        display(green7);

        greennote();

        delay1();

        

        display(green8);

        emptynote();

        delay1();

        

        display(green9);

        greennote();

        delay1();

        

        display(green10);

        emptynote();

        delay1();

        

        display(green11);

        rednote();

        delay1();

        

        display(green12);

        yellownote();

        delay1();

        

        display(green13);

        greennote();

        delay1();

        

        display(green14);

        emptynote();

        delay1();

        

        display(green15);

        greennote();

        delay1();

        

        display(green16);

        emptynote();

        delay1();

        

        display(green17);

        greennote();

        delay1();

        

        display(green18);

        bluenote();

        delay1();

        

        display(green19);

        delay1();

        

        display(green20);

        delay1();

        

        display(green21);

        greennote();

        delay1();

} //Prints 1 to Arduino
void greenrepeat(){

        display(green22);

        emptynote();

        delay1();

        

        display(green23);

        greennote();

        delay1();

        

        display(green24);

        emptynote();

        delay1();

        

        display(green25);

        greennote();

        delay1();

        

        display(green26);

        emptynote();

        delay1();

        

        display(green27);

        rednote();

        delay1();

        

        display(green28);

        yellownote();

        delay1();

        

        display(green29);

        greennote();

        delay1();

        

        display(green30);

        emptynote();

        delay1();

        

        display(green31);

        greennote();

        delay1();

        

        display(green32);

        emptynote();

        delay1();

        

        display(green33);

        greennote();

        delay1();

        

        display(green34);

        bluenote();

        delay1();

        

        display(green35);

        delay1();

        

        display(green36);

        delay1();

        

    

    

    

}
void mainriff(){

        display(green37);

        greennote();

        delay1();

        

        display(green38);

        emptynote();

        delay1();

       

        display(green39);

        yellownote();

        delay1();

        

        display(green40);

        rednote();

        delay1();

        

        display(green41);

        greennote();

        delay1();

        

        display(green42);

        emptynote();

        delay1(); 

        

        display(green43);

        greennote();

        delay1();

        

        display(green44);

        emptynote();

        delay1(); 

}

void mainriffends(){



        display(green45);

        greennote();

        delay1(); 

        

        display(green46);

        emptynote();

        delay1();

        

        display(green47);

        yellownote();

        delay1(); 

        

        display(green48);

        rednote();

        delay1();

        

        display(green49);

        greennote();

        delay1(); 

        

        display(green50);

        emptynote();

        delay1();

        

        display(green51);

        greennote();

        delay1(); 

        

        display(green52);

        emptynote();

        delay1(); 

}

//Display notes for Song 1
int slowride(void) {

    init8x8();

    MAX7219_Write(0x0C, 0x01); // Turn on the display
    MAX7219_Write(0x0B, 0x07); // Set scan limit to 7 (0-7)
    MAX7219_Write(0x0A, 0x0F); // Set intensity to max (0-15)
    MAX7219_Write(0x09, 0x00); // Set decode mode off
    MAX7219_Write(0x0F, 0x00); // Set display test off


        greenbegins();
        
        greenrepeat();

        mainriff();        

        mainriffends();

     
    return 0;
}


// Notes for Song 2
const uint8_t yellow1[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow2[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow3[8] = {

    0b00001000, //delay

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};//delay yellow
const uint8_t yellow4[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow5[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow6[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t yellow7[8] = {

    0b00001000,//second delay

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000
};// 2nd delay yellow
const uint8_t yellow8[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow9[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow10[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000
}; // yellow
const uint8_t yellow11[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000
};
const uint8_t yellow12[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow13[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
}; //yellow
const uint8_t yellow14[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t yellow15[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow16[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
}; //red
const uint8_t yellow17[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t yellow18[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow19[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
}; //yellow
const uint8_t yellow20[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t yellow21[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t yellow22[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//yellow
const uint8_t yellow23[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t yellow24[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow25[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow26[8] = {

    0b00001000, //delay

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};//delay yellow
const uint8_t yellow27[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow28[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow29[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t yellow30[8] = {

    0b00001000,//second delay

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000
};// 2nd delay yellow
const uint8_t yellow31[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow32[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};
const uint8_t yellow33[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000
}; // yellow
const uint8_t yellow34[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000
};
const uint8_t yellow35[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellow36[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
}; //yellow
const uint8_t yellow37[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
void yellownotes(){
        display(yellow1);
        emptynote();
        delay2();
       
        display(yellow2);
        delay2();
       
        display(yellow3);
        yellownote();
        delay2();
       
        display(yellow4);
        emptynote();
        delay2();
       
        display(yellow5);
        delay2();
       
        display(yellow6);
        yellownote();
        delay2();
       
        display(yellow7);
        emptynote();
        delay2();
       
        display(yellow8);
        delay2();
       
        display(yellow9);
        delay2();
       
        display(yellow10);
        yellownote();
        delay2();
       
        display(yellow11);
        emptynote();
        delay2();
       
        display(yellow12);
        delay2();
       
        display(yellow13);
        delay2();
       
        display(yellow14);
        yellownote();
        delay2();
       
        display(yellow15);
        emptynote();
        delay2();
       
        display(yellow16);
        delay2();
       
        display(yellow17);
        yellownote();
        delay2();
       
        display(yellow18);
        emptynote();
        delay2();
       
        display(yellow19);
        delay2();
       
        display(yellow20);
        yellownote();
        delay2();
       
        display(yellow21);
        emptynote();
        delay2();
       
        display(yellow22);
        delay2();
       
        display(yellow23);
        rednote();
        delay2();  
       
        display(yellow24);
        emptynote();
        delay2();  
       
        display(yellow25);
        delay2();  
       
        display(yellow26);
        yellownote();
        delay2();  
       
        display(yellow27);
        emptynote();
        delay2();  
       
        display(yellow28);
        delay2();  
       
        display(yellow29);
        yellownote();
        delay2();  
       
        display(yellow30);
        emptynote();
        delay2();  
       
        display(yellow31);
        delay2();  
       
        display(yellow32);
        delay2();  
       
        display(yellow33);
        yellownote();
        delay2();  
       
        display(yellow34);
        emptynote();
        delay2();  
       
        display(yellow35);
        delay2();  
       
        display(yellow36);
        delay2();  
       
        display(yellow37);
        yellownote();
        delay2();  
}

const uint8_t yellowred1[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t yellowred2[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//red
const uint8_t yellowred3[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t yellowred4[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t yellowred5[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t yellowred6[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000
};
void yellowrednotes(){
        display(yellowred1);
        emptynote();
        delay2();
       
        display(yellowred2);
        delay2();
       
        display(yellowred3);
        rednote();
        delay2();
       
        display(yellowred4);
        emptynote();
        delay2();
       
        display(yellowred5);
        delay2();
   
        display(yellowred6);
        yellownote();
        delay2();
}

const uint8_t reddy1[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t reddy2[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//red
const uint8_t reddy3[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000
};
void rednotes(){
    
    display (reddy1);
    emptynote();
    delay2();

    display (reddy2);
    delay2();
  
    display (reddy3);
    rednote();
    delay2();

}

const uint8_t note1[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000
};//green
const uint8_t note2[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000
};
const uint8_t note3[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000
};
const uint8_t note4[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000
};//yellow
const uint8_t note5[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000
};
const uint8_t note6[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note7[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//yellow
const uint8_t note8[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note9[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note10[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//green
const uint8_t note11[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note12[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note13[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//red
const uint8_t note14[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note15[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note16[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//green
const uint8_t note17[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note18[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note19[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//red
const uint8_t note20[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note21[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note22[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//red
const uint8_t note23[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note24[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note25[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//yellow
const uint8_t note26[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note27[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};//continuation
const uint8_t note28[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//green
const uint8_t note29[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note30[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note31[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t note32[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note33[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note34[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//yellow
const uint8_t note35[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note36[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note37[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//green
const uint8_t note38[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note39[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note40[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//red
const uint8_t note41[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note42[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note43[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//green
const uint8_t note44[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note45[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note46[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//red
const uint8_t note47[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note48[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000
};
const uint8_t note49[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000
};//red
const uint8_t note50[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00100000
};
const uint8_t note51[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note52[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//yellow
const uint8_t note53[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note54[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note55[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//red
const uint8_t note56[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note57[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note58[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t note59[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note60[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note61[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//yellow
const uint8_t note62[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note63[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note64[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t note65[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note66[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note67[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t note68[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note69[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note70[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//yellow
const uint8_t note71[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note72[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note73[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//red
const uint8_t note74[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note75[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000
};
const uint8_t note76[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000
};//red
const uint8_t note77[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00001000
};
const uint8_t note78[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};//after 14 red
const uint8_t note79[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00010000
};
const uint8_t note80[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000
};
const uint8_t note81[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00000000
};
const uint8_t note82[8] = {

    0b00000000,

    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00010000
};
const uint8_t note83[8] = {
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000,
    
    0b00000000
};

//Display notes for song 2
int everlong(void){

    init8x8();

    MAX7219_Write(0x0C, 0x01); // Turn on the display
    MAX7219_Write(0x0B, 0x07); // Set scan limit to 7 (0-7)
    MAX7219_Write(0x0A, 0x0F); // Set intensity to max (0-15)
    MAX7219_Write(0x09, 0x00); // Set decode mode off
    MAX7219_Write(0x0F, 0x00); // Set display test off

    
        display(note1);
        
        delay2();       
        
        display(note2);
        
        delay2();       
        
        display(note3);
        
        delay2();       
        
        display(note4);
        
        delay2();       
        
        display(note5);
        
        delay2();       
        
        display(note6);
        
        delay2();       
        
        display(note7);
        
        printf("2");
        
        delay2();       
        
        display(note8);

        greennote();
        
        delay2();       
        
        yellownotes();       
        
        display(note9);

        emptynote();
        
        delay2();       
        
        display(note10);
        
        delay2();       
        
        display(note11);
        
        yellownote();
        
        delay2();       
        
        display(note12);
        
        emptynote();
        
        delay2();       
        
        display(note13);
        
        delay2();       
        
        display(note14);
        
        yellownote();
        
        delay2();       
        
        display(note15);
        
        emptynote();
        
        delay2();       
        
        display(note16);
        
        delay2();       
        
        display(note17);
        
        greennote();
        
        delay2();       
        
        display(note18);
        
        emptynote();
        
        delay2();       
        
        display(note19);
        
        delay2();       
        
        display(note20);
        
        rednote();
        
        delay2();       
        
        display(note21);
        
        emptynote();
        
        delay2();       
        
        display(note22);
        
        delay2();       
        
        display(note23);
        
        greennote();
        
        delay2();       
        
        display(note24);
        
        emptynote();
        
        delay2();       
        
        display(note25);
        
        delay2();       
        
        display(note26);
        
        rednote();
        
        delay2();       
        
        yellowrednotes();       
        
        yellowrednotes();       
        
        yellowrednotes();       
        
        display(note27);
        
        emptynote();
        
        delay2();       
        
        display(note28);
        
        delay2();       
        
        display(note29);
        
        rednote();
        
        delay2();       
        
        display(note30);
        
        emptynote();
        
        delay2();       
        
        display(note31);
        
        delay2();       
        
        display(note32);
        
        yellownote();
        
        delay2();       
        
        display(note33);
        
        emptynote();
        
        delay2();        
        
        display(note34);
        
        delay2();
        
        display(note35);
        
        greennote();
        
        delay2();       
        
        yellownotes();
        
        display(note36);
        
        emptynote();
        
        delay2();
        
        display(note37);
        
        delay2();
        
        display(note38);
        
        yellownote();
        
        delay2();
        
        display(note39);
        
        emptynote();
        
        delay2();
        
        display(note40);
        
        delay2();
        
        display(note41);
        
        yellownote();
        
        delay2();
        
        display(note42);
        
        emptynote();
        
        delay2();
        
        display(note43);
        
        delay2();
        
        display(note44);
        
        greennote();
        
        delay2();
        
        display(note45);
        
        emptynote();
        
        delay2();
        
        display(note46);
        
        delay2();
        
        display(note47);
        
        rednote();
        
        delay2();
        
        display(note48);
        
        emptynote();
        
        delay2();
        
        display(note49);
        
        delay2();
        
        display(note50);
        
        greennote();
        
        delay2();
        
        display(note51);
        
        emptynote();
        
        delay2();
        
        display(note52);
        
        delay2();
        
        display(note53);
        
        rednote();
        
        delay2();
        
        display(note54);
        
        emptynote();
        
        delay2();
        
        display(note55);
        
        delay2();
        
        display(note56);
        
        rednote();
        
        delay2();
        
        display(note57);
        
        emptynote();
        
        delay2();
        
        display(note58);
        
        delay2();
        
        display(note59);
        
        yellownote();
        
        delay2();
        
        display(note60);
        
        emptynote();
        
        delay2();
        
        display(note61);
        
        delay2();
        
        display(note62);
        
        rednote();
        
        delay2();
        
        display(note63);
        
        emptynote();
        
        delay2();
        
        display(note64);
        
        delay2();
        
        display(note65);
        
        yellownote();
        
        delay2();
        
        display(note66);
        
        emptynote();
        
        delay2();
        
        display(note67);
        
        delay2();
        
        display(note68);
        
        yellownote();
        
        delay2();
        
        display(note69);
        
        emptynote();
        
        delay2();
        
        display(note70);
        
        delay2();
        
        display(note71);
        
        yellownote();
        
        delay2();
        
        display(note72);
        
        emptynote();
        
        delay2();
        
        display(note73);
        
        delay2();
        
        display(note74);
        
        yellownote();
        
        delay2();
        
        display(note75);
        
        emptynote();
        
        delay2();
        
        display(note76);
        
        delay2();
        
        display(note77);
        
        yellownote();
        
        delay2();
        
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        rednotes();
        
        display(note78);
        
        emptynote();
        
        delay2();
        
        display(note79);
        
        rednote();
        
        delay2();
        
        display(note80);
        
        emptynote();
        
        delay2();
        
        display(note81);
        
        delay2();

        display(note82);
        
        rednote();

        delay2();

        display(note83);
        
        emptynote();

    return 0;

}   //Prints 2 to Arduino


// Notes for Song 3
const uint8_t repeat1[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t repeat2[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000

};
void startsong(){

    display(repeat1);
        
    greennote();

    delay3();;

    display(repeat2);
        
    rednote();

    delay3();;

}

const uint8_t everyother1[8] = {

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

};
const uint8_t everyother2[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000

};
const uint8_t everyother3[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t everyother4[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

};
void everyother(){

    display(everyother1);
        
    emptynote();

    delay3();

    display(everyother2);
        
    greennote();

    delay3();

    display(everyother3);
        
    emptynote();

    delay3();

    display(everyother4);
        
    rednote();

    delay3();

}

const uint8_t riff1[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff2[8] = {

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff3[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff4[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff5[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff6[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000

};
const uint8_t riff7[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000

};
const uint8_t riff8[8] = {

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff9[8] = {

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000

};
const uint8_t riff10[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff11[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000

};
const uint8_t riff12[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff13[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000

};
const uint8_t riff14[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000

};
const uint8_t riff15[8] = {

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff16[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000

};
const uint8_t riff17[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff18[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000

};
const uint8_t riff19[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000,

    0b00000000

};
const uint8_t riff20[8] = {

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00000000

};
const uint8_t riff21[8] = {

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff22[8] = {

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000

};
const uint8_t riff23[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff24[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000,

    0b00010000

};
const uint8_t riff25[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000,

    0b00100000

};
const uint8_t riff26[8] = {

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00010000

};
const uint8_t riff27[8] = {

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000

};
const uint8_t riff28[8] = {

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff29[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000

};//every other starts
const uint8_t riff30[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff31[8] = {

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000

};
const uint8_t riff32[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000

};
const uint8_t riff33[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00000100

};
const uint8_t riff34[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000

};
const uint8_t riff35[8] = {

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000

};
const uint8_t riff36[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000

};
const uint8_t riff37[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff38[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000

};//loop
const uint8_t riff39[8] = {

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000

};//breakdown
const uint8_t riff40[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000

};
const uint8_t riff41[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff42[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000

};
const uint8_t riff43[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000

};
const uint8_t riff44[8] = {

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000

};//short every other
const uint8_t riff45[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff46[8] = {

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000

};
const uint8_t riff47[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff48[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000,

    0b00001000

};
const uint8_t riff49[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00000000

};
const uint8_t riff50[8] = {

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000

};
const uint8_t riff51[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000

};
const uint8_t riff52[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff53[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000

};//loop
const uint8_t riff54[8] = {

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000,

    0b00000000

};//breakdown
const uint8_t riff55[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00100000

};
const uint8_t riff56[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff57[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000

};
const uint8_t riff58[8] = {

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000,

    0b00000000

};
const uint8_t riff59[8] = {

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00100000

};
const uint8_t riff60[8] = {

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000

};//sweep introduced
const uint8_t riff61[8] = {

    0b00000100,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000

};
const uint8_t riff62[8] = {

    0b00001000,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff63[8] = {

    0b00010000,

    0b00001000,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000,

    0b00001000

};
const uint8_t riff64[8] = {

    0b00100000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000000

};
const uint8_t riff65[8] = {

    0b00010000,

    0b00100000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00000100

};
const uint8_t riff66[8] = {

    0b00001000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00000000,

    0b00000000

};//screech intro
const uint8_t riff67[8] = {

    0b00000100,

    0b00001000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00000000

};
const uint8_t riff68[8] = {

    0b00001000,

    0b00000100,

    0b00001000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00001000,

    0b00000100

};
const uint8_t riff69[8] = {

    0b00010000,

    0b00001000,

    0b00000100,

    0b00001000,

    0b00010000,

    0b00100000,

    0b00010000,

    0b00001000,

};
const uint8_t riff70[8] = {

    0b00000000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00001000,

    0b00010000,

    0b00100000,

    0b00010000

};
const uint8_t riff71[8] = {

    0b00000000,

    0b00000000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00001000,

    0b00010000,

    0b00100000

};
const uint8_t riff72[8] = {

    0b00000100,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00001000,

    0b00010000

};
const uint8_t riff73[8] = {

    0b00000100,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00001000,

    0b00000100,

    0b00001000

};
const uint8_t riff74[8] = {

    0b00000000,

    0b00000100,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00001000,

    0b00000100

};
const uint8_t riff75[8] = {

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00010000,

    0b00001000

};
const uint8_t riff76[8] = {

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000100,

    0b00000000,

    0b00000000,

    0b00010000

};
const uint8_t riff77[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000100,

    0b00000000,

    0b00000000

};
const uint8_t riff78[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000100,

    0b00000000

};
const uint8_t riff79[8] = {

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000100,

    0b00000100

};
const uint8_t riff80[8] = {

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000100

};
const uint8_t riff81[8] = {

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff82[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00000000

};
const uint8_t riff83[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff84[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000,

    0b00001000

};
const uint8_t riff85[8] = {

    0b00010000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000,

    0b00000000

};
const uint8_t riff86[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00001000

};
const uint8_t riff87[8] = {

    0b00010000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff88[8] = {

    0b00000000,

    0b00010000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff89[8] = {

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000,

    0b00000000

};
const uint8_t riff90[8] = {

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00000000

};
const uint8_t riff91[8] = {

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff92[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000,

    0b00010000

};
const uint8_t riff93[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000,

    0b00000000

};
const uint8_t riff94[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000,

    0b00010000

};
const uint8_t riff95[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00100000,

    0b00000000

};
const uint8_t riff96[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00100000

};
const uint8_t riff97[8] = {

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000,

    0b00000000

};

//Display notes for song 3
int heaviest(void){

    init8x8();

    MAX7219_Write(0x0C, 0x01); // Turn on the display
    MAX7219_Write(0x0B, 0x07); // Set scan limit to 7 (0-7)
    MAX7219_Write(0x0A, 0x0F); // Set intensity to max (0-15)
    MAX7219_Write(0x09, 0x00); // Set decode mode off
    MAX7219_Write(0x0F, 0x00); // Set display test off


    display(riff1);

        delay3();

    display(riff2);

        delay3();

    display(riff3);

        delay3();

    display(riff4);

        delay3();

    display(riff5);
    
        printf("3");

        delay3();

    display(riff6);

        delay3();

    display(riff7);

        delay3();

    startsong();
    startsong();
    startsong();
    startsong();
    startsong();
    startsong();
    
    display(riff8);
        
        greennote();

        delay3();

    display(riff9);
        
        rednote();

        delay3();

    display(riff10);
        
        greennote();

        delay3();

    display(riff11);
        
        rednote();

        delay3();

    display(riff12);
        
        greennote();

        delay3();

    display(riff13);
        
        rednote();

        delay3();

    display(riff14);
        
        greennote();

        delay3();

    display(riff15);
        
        emptynote();

        delay3();

    display(riff16);
        
        yellownote();

        delay3();

    display(riff17);
        
        emptynote();

        delay3();

    display(riff18);
        
        yellownote();

        delay3();

    display(riff19);
        
        emptynote();

        delay3();

    display(riff20);

        delay3();

    startsong();
    startsong();
    startsong();

    display(riff21);
        
        greennote();

        delay3();

    display(riff22);
        
        rednote();

        delay3();

    display(riff23);
        
        greennote();

        delay3();

    display(riff24);
        
        rednote();

        delay3();

    display(riff25);
        
        greennote();

        delay3();

    display(riff26);
        
        rednote();

        delay3();

    display(riff27);
        
        greennote();

        delay3();

    display(riff28);
        
        emptynote();

        delay3();

    display(riff29);
        
        yellownote();

        delay3();

    display(riff30);
        
        emptynote();

        delay3();

    display(riff31);
        
        yellownote();

        delay3();

    display(riff32);
        
        emptynote();

        delay3();

    display(riff33);
        
        bluenote();

        delay3();

    display(riff34);
        
        emptynote();

        delay3();

    display(riff35);

        delay3();

    display(riff36);
        
        greennote();

        delay3();

    display(riff37);
        
        emptynote();

        delay3();

    display(riff38);
        
        rednote();

        delay3();

    everyother();
    everyother();

    display(riff39);
        
        emptynote();

        delay3();

    display(riff40);
        
        greennote();

        delay3();

    display(riff41);
        
        emptynote();

        delay3();

    display(riff42);
        
        rednote();

        delay3();

    display(riff43);
        
        emptynote();

        delay3();

    display(riff44);
        
        greennote();

        delay3();

    display(riff45);
        
        emptynote();

        delay3();

    display(riff46);
        
        yellownote();

        delay3();

    display(riff47);
        
        emptynote();

        delay3();

    display(riff48);
        
        yellownote();

        delay3();

    display(riff49);
        
        emptynote();

        delay3();

    display(riff50);

        delay3();

    display(riff51);
        
        greennote();

        delay3();

    display(riff52);
        
        emptynote();

        delay3();

    display(riff53);
        
        rednote();

        delay3();

    everyother();

    display(riff54);
        
        emptynote();

        delay3();

    display(riff55);
        
        greennote();

        delay3();

    display(riff56);
        
        emptynote();

        delay3();

    display(riff57);
        
        rednote();

        delay3();

    display(riff58);
        
        emptynote();

        delay3();

    display(riff59);
        
        greennote();

        delay3();

    display(riff60);
        
        emptynote();

        delay3();

    display(riff61);
        
        yellownote();

        delay3();

    display(riff62);
        
        emptynote();

        delay3();

    display(riff63);
        
        yellownote();

        delay3();

    display(riff64);
        
        emptynote();

        delay3();

    display(riff65);
        
        bluenote();

        delay3();

    display(riff66);
        
        emptynote();

        delay3();

    display(riff67);

        delay3();

    display(riff69);
        
        yellownote();

        delay3();

    display(riff70);
        
        rednote();

        delay3();

    display(riff71);
        
        greennote();

        delay3();

    display(riff72);
        
        rednote();

        delay3();

    display(riff73);
        
        yellownote();

        delay3();

    display(riff74);
        
        bluenote();

        delay3();

    display(riff75);
        
        yellownote();

        delay3();

    display(riff76);
        
        rednote();

        delay3();

    display(riff77);
        
        emptynote();

        delay3();

    display(riff78);

        delay3();

    display(riff79);
        
        bluenote();

        delay3();

    display(riff80);

        delay3();

    display(riff81);
        
        emptynote();

        delay3();

    display(riff82);

        delay3();

    display(riff83);

        delay3();

    display(riff84);
        
        yellownote();

        delay3();

    display(riff85);
        
        emptynote();

        delay3();

    display(riff86);
        
        yellownote();

        delay3();

    display(riff87);
        
        emptynote();

        delay3();

    display(riff88);

        delay3();

    display(riff89);

        delay3();

    display(riff90);

        delay3();

    display(riff91);

        delay3();

    display(riff92);
        
        rednote();

        delay3();

    display(riff93);
        
        emptynote();

        delay3();

    display(riff94);
        
        rednote();

        delay3();

    display(riff95);
        
        emptynote();

        delay3();

    display(riff96);
        
        greennote();

        delay3();

    display(riff97);
        
        emptynote();

        delay1();

    return 0;

}   //Prints 3 to Arduino