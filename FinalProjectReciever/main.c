#include "msp.h"

#define LED1 BIT0  //P1.0
#define S1 BIT1 //P1.1
#define LED2_RED BIT0 //P2.0

/**
 * main.c
 */
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    char data;          //Defines the variable data as a char to be used in a later while statement
    P1->DIR |= LED1;     // set P1.0 as output
    P2->DIR |= LED2_RED;

    // Configure port 3 as UART2.
    P3->SEL0 |= BIT2 | BIT3;    // Set bit 2 and bit 3 of P3SEL0 to 1
    P3->SEL1 &= ~(BIT2 | BIT3);     // Reset bit 2 and bit 3 of P3SEL1 to 0

    EUSCI_A2->CTLW0 |= BIT0;        // Set WRST to put UART0 in reset
                                    // Leave all other bits = 0

    EUSCI_A2->CTLW0 |= BIT6 | BIT7; // While keeping bit 0 = 1, set the
//remaining control bits in
//UCA2CTLW0 to the
// values shown in section
// UART2 (UCA2) Registers.

    EUSCI_A2->BRW= 0x138;       // Baud rate = 9600 clk = 3000000 BRW = clk/BR
                    //convert to Hexadecimal

    EUSCI_A2->MCTLW &= ~BIT0;       // UCOS16 bit = 0

    EUSCI_A2->CTLW0 &= ~BIT0;   // Clear WRST to resume UART operation.
    int i = 0;
    while(1){

        data = UART_InChar(); //Set data equal to the bits read off of the UART
        if(data == 0xAA)
        {
            P1->OUT |= LED1;       // Flash Car lights once
            P2->OUT |= LED2_RED;
            for (i = 0; i < 40000; i++){;}
            P1->OUT &= ~LED1;
            P2->OUT &= ~LED2_RED;
        }
        if(data == 0xBB)
        {
            P1->OUT |= LED1;       // Flash Car lights twice
            P2->OUT |= LED2_RED;
            for (i = 0; i < 40000; i++){;}
            P1->OUT &= ~LED1;
            P2->OUT &= ~LED2_RED;
            for (i = 0; i < 60000; i++){;}
            P1->OUT |= LED1;
            P2->OUT |= LED2_RED;
            for (i = 0; i < 40000; i++){;}
            P1->OUT &= ~LED1;
            P2->OUT &= ~LED2_RED;
        }
        if(data == 0xCC)
        {
            P1->OUT |= LED1;       // Flash Car lights As alert
            P2->OUT |= LED2_RED;
            for (i = 0; i < 40000; i++){}
            P1->OUT &= ~LED1;
            P2->OUT &= ~LED2_RED;
            for (i = 0; i < 60000; i++){}
        }
        if(data == 0x55)
        {
            P1->OUT &= ~LED1; /*turn off LED1 at pin P1.0 if data is=to 0x55*/
        }
    }

}
char UART_InChar (void)
{
    while ( (EUSCI_A2->IFG & BIT0) == 0);  // Busy.  Wait for received data.
    return ( (char) (EUSCI_A2->RXBUF) );   // Get new input when IFG = 1.
}
