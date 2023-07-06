/*
 * File:   config.c
 * Author: Agustin Maximiliano Coronel
 *
 * Created on 23 de junio de 2020, 18:06
 */


#include "xc.h"
#include "p33FJ256GP710.h"
#define FCY 40000000
#define BAUDRATE 9600
#define BRGVAL ((FCY / BAUDRATE) / 16) -1
#define UN_MILISEGUNDO 5000
#define DIEZ_MILISEGUNDOS 50000

void InitUART1()
/*inizializa la unidad UART1 con un baudrate de 9600*/
{
	U1MODEbits.UARTEN = 0;	
	U1MODEbits.RTSMD = 1;	
    U1MODEbits.STSEL = 0;
    U1MODEbits.PDSEL = 0;
    U1STAbits.URXISEL = 0;
	U1BRG = BRGVAL;	
	IFS0bits.U1RXIF = 0;	
	IEC0bits.U1RXIE = 1;	
	U1MODEbits.UARTEN = 1;	
	U1STAbits.UTXEN = 1;	
	IFS0bits.U1TXIF = 0;	
	IEC0bits.U1TXIE = 1;
}

void initPorts()
/*configura el los pines 6 y 7 del portD como entrada para los piezoelectricos 
 * y el pin 13 del mismo port tambien como entrada para el lazo inductivo
 * el pin 0 del portA se establece como salida para la camara fotografica*/
{
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;
    TRISDbits.TRISD13 = 1;
    TRISAbits.TRISA0 = 0;
}

void initTimer()
/*inicializa 3 timers, el timer1 para controlar el tiempo transcurrido entre la
 activacion de los piezoelectricos, interrumpe cada 1ms el timer2 y el timer3 se
 configuran para que interrumpan cada 10ms para considerar el tiempo que
 permanecen encendidos los piezoelectricos*/
{
	T1CONbits.TCKPS = 1;
 	IFS0bits.T1IF = 0;
 	IEC0bits.T1IE = 1;
	PR1 = UN_MILISEGUNDO;
    T2CONbits.TCKPS = 1;
 	IFS0bits.T2IF = 0;
 	IEC0bits.T2IE = 1;
	PR2 = DIEZ_MILISEGUNDOS;
    T3CONbits.TCKPS = 1;
 	IFS0bits.T3IF = 0;
 	IEC0bits.T3IE = 1;
	PR3 = DIEZ_MILISEGUNDOS;
    
}

void initInterrupt()
/*establece que los pines 6,7 y 13 interrumpiran a traves del change notification
 cuando cambien de valor*/
{
    IFS1bits.CNIF = 0;
    CNEN2bits.CN19IE = 1;
    CNEN2bits.CN16IE = 1;
    CNEN1bits.CN15IE = 1;
    IEC1bits.CNIE = 1;
}
