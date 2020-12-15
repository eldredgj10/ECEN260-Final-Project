#include "msp.h"

#define LED1 BIT0     //P1.0
#define LED2_RED BIT0   //P2.0

#define S1 BIT1  //P1.1
#define S2 BIT4  //P1.4

#define DOOR BIT1   // P4.1
#define DELAY 300 // used for SW switch debouncer using 300 clock cycles
volatile int Open;
/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	//Initialize Variables
    int Alarm = 0;
    Open = 0;
    int Lock = 0;
    int i = 0;

    //Initialize LED1 and Switch 1
    P1->DIR |= LED1;    // set P1.0 as output
    P1->DIR &= ~(S1);   // set P1.1 as input
    P1->REN |= S1;      // turn on P1.1 pull resistor
    P1->OUT |= S1;      // configure P1.1 resistor as pull-up

    //Initialize LED2_RED and Switch 2
    P2->DIR |= LED2_RED;    // set P2.0 as output
    P1->DIR &= ~(S2);   // set P1.4 as input
    P1->REN |= S2;      // turn on P2.1 pull resistor
    P1->OUT |= S2;      // configure P2.1 resistor as pull-up ?????

    // Turn all alarm lights off
    P1->OUT &= ~LED1;       // turn off Car right light
    P2->OUT &= ~LED2_RED;       // turn off Car Left light

    // Configure port 3 as UART2.
    P3->SEL0 |= BIT2 | BIT3;        // Set bit 2 and bit 3 of P3SEL0 to 1
    P3->SEL1 &= ~(BIT2 | BIT3);     // Reset bit 2 and bit 3 of P3SEL1 to 0

    EUSCI_A2->CTLW0 |= BIT0;        // Set WRST to put UART0 in reset
                                       // Leave all other bits = 0

    EUSCI_A2->CTLW0 |= BIT6 | BIT7;  // While keeping bit 0 = 1, set the
                                         //remaining control bits in
                                         //UCA2CTLW0 to the
                                         // values shown in section
                                         // UART2 (UCA2) Registers.

    EUSCI_A2->BRW= 0x138;     // Baud rate = 9600 clk = 3000000 BRW = clk/BR
                        //convert to Hexadecimal
    EUSCI_A2->MCTLW &= ~BIT0;       // UCOS16 bit = 0

    EUSCI_A2->CTLW0 &= ~ BIT0;  // Clear WRST to resume UART operation.

    char data;     //Defining the variable data to be used in the while loop


    //Set up interrupt on Port 3 for door watch
    P3->DIR &= ~BIT0;   /* set up pin P3.0 (DA (interrupt pin)) as input */
    P3->REN |= BIT0;    /* connect pull resistor to pin P3.0 */
    P3->OUT |= BIT0;    /* configure pull resistor as pull up */
    P3->IFG &= ~BIT0;   /* clear interrupt flag for pin P3.0 (DA (int pin)) */
    P3->IE |= BIT0;     /* enable the interrupt for pin P3.0 (DA (interrupt pin)) */
    P3->IES = 0x00;     /*Enables activation on falling edge*/



    NVIC->ISER[1] |= 0x20;  /* enable port 3 interrupts (see p. 89 in text)*/
    _enable_interrupts();



    // While loop handles Lock, Unlock, and Alarm clearing.
    while (1)
        {
            // delay for switch debouncing
            for (i = 0; i < DELAY; i++){;}

            // check the door
            if((P1->IN & S1) == 0x00)
            {

                data = 0xAA;   //If switch is 0, that means the switch is pressed
                               //and data is set equal to 0xAA

                UART_OutChar(data);    //OxAA is passed through the UART in this
                                       //function call.
                P1->OUT |= LED1;       // Flash Car lights once
                P2->OUT |= LED2_RED;
                for (i = 0; i < 40000; i++){;}
                P1->OUT &= ~LED1;
                P2->OUT &= ~LED2_RED;


                Lock = 1;
                if(Alarm ==0)
                { Open = 0;}

            }
            else if((P1->IN & S2) == 0x00)
            {
                data = 0xBB;   //If switch is 0, that means the switch is pressed
                               //and data is set equal to 0xBB

                UART_OutChar(data);    //OxAA is passed through the UART in this
                                       //function call.
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

                Lock = 0;
                Open = 0;
            }

            else if((Open ==1) && (Lock ==1))
            {
                Alarm = 1;
                while(Alarm == 1)
                {
                    if((P1->IN & S2) == 0x00)
                    {
                        Alarm =0;
                        Open = 0;
                    }
                    else{
                        data = 0xCC;   //If switch is 0, that means the switch is pressed
                                       //and data is set equal to 0xCC

                        UART_OutChar(data);    //OxAA is passed through the UART in this
                                               //function call.
                        P1->OUT |= LED1;       // Flash Car lights As alert
                        P2->OUT |= LED2_RED;
                        for (i = 0; i < 40000; i++){}
                        P1->OUT &= ~LED1;
                        P2->OUT &= ~LED2_RED;
                        for (i = 0; i < 60000; i++){}

                    }
                }
            }
            else
            {
                data = 0x55;   //If switch is 0, that means the switch is pressed
                               //and data is set equal to 0xCC

                UART_OutChar(data);    //OxAA is passed through the UART in this
                                       //function call.

            }
        }
}

void PORT3_IRQHandler(void){

  uint32_t status;

  status = P3->IFG;   /* get the interrupt status for port 3 */

  if(status & BIT0){  /* constant for the pin 0 mask */
      Open = 1;
  }
  P3->IFG &= ~BIT0;    /* clear the interrupt for port 3, pin 0 */
}

void UART_OutChar (char data) {
    while ( (EUSCI_A2->IFG & BIT1) == 0);  // Busy.  Wait for previous output.
    EUSCI_A2->TXBUF = data;        // Start transmission when IFG = 1.
}
