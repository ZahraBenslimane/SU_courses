#include "LPC23xx.h"
#define PCLK 12e6
#define PERIODE 2.5e-3 
#define INCREM 1e-3

unsigned int  dc = (PERIODE * PCLK)/100;   
unsigned char sens = 0; // 0: mont�e 1: descente

void isr_EINT0(void)__irq
{	
	PWM1MR0 += INCREM * PCLK;
	if ( PWM1MR0 > PCLK /100 )
	{
		PWM1MR0 = PCLK * PERIODE;
		PWM1MR1 = PWM1MR0/100;
	}	
	PWM1LER = 0x3;
	EXTINT = 1;     // Acquittement p�rif int.externe
	VICVectAddr = 0;
}


void init_EINT0()
{
	PINSEL4 |= 1<<20;   //P0.10 en int�ruption externe : BP
	EXTMODE = 0x01;        // Front 
	EXTPOLAR = 0x00; 			// Front d�scendant 	
	VICIntEnable = 1<<14;
	VICVectAddr14 = (unsigned long) isr_EINT0;	
}

void isr_heartbeat(void) __irq
{	
	if ( dc >= PWM1MR0 - PWM1MR0/100 ){
		sens = 1;
		dc = PWM1MR0 - PWM1MR0/100;
	}
	if ( dc <= PWM1MR0/100) {
		sens = 0;
		dc = PWM1MR0 /100;
	}
	
	if ( !sens ) dc += PWM1MR0/100;
	else dc -= PWM1MR0/100;
		
	PWM1MR1 = dc;
	PWM1LER = 0x03;
	
	VICVectAddr = 0;
	PWM1IR = 0x2;	
}


void init_PWM(void){
	
	PINSEL4 = 0x01;    // P2.0 en mode PMW1.1 le reste en GPIO

	PWM1PCR = 1<<9;    // PMW1.1 Output Enable + Single edge
	PWM1CTCR = 0x00;
	
	PWM1MR0 = PERIODE * PCLK;
	PWM1MR1 = PWM1MR0/100;

	PWM1TCR = 0x9;     // Counter Enable + PWM enable
	PWM1MCR = 0xA;     // RESET si TC = MR0   && INTERUPTION si TC = MR1 
	PWM1LER = 0x3;     // Enable PWM Match 0 and 1 Latches
	
	FIO2DIR |= 0xFFFE;  // P2.1 to P2.7 en sortie
	FIO2SET0 = 0xFE;    // P2.1 to P2.7 Allum�es
	
	VICIntEnable = 1<<8;
	VICVectAddr8 = (unsigned long) isr_heartbeat;	
}



int main(){
	
	init_PWM();
	init_EINT0();
	
	while(1);
	
}