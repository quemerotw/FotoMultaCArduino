/*
 * File:   interrupciones.c
 * Author: Agustin Maximiliano Coronel
 *
 * Created on 2 de julio de 2020, 11:28
 */


#include "xc.h"
#include "p33FJ256GP710.h"

float vel = 0;
int milisegundos = 0;
int piezo1 = 0,piezo2 = 0,ejes = 0;
unsigned long camiones = 0,autos = 0;
int flagpiezo[2];


void limpiarPiezos()
/*Reinicia los valores de las variables utilizadas por los piezoeléctricos
 y los timers*/
{
    piezo1 = 0;
    piezo2 = 0;
    ejes = 0;
    milisegundos = 0;
}

void calculoVelocidad()
/*calcula la velocidad a partir de la distancia de separacion de los
 piezoeléctricos y la cantidad de milisegundos transcurridos entre el pulso del
 primero y pulso del segundo, convirtiendolo de cm/ms a km/h, si el valor supera
 los 60km/h dispara activa el pin 0 del puerto A donde esta dispuesta la camara
 activando el timer1*/
{
    if (milisegundos == 0)
        limpiarPiezos();
    else
    {
        vel = 30.0/milisegundos;
        vel = vel*36;
        if (vel > 60.0)
        {
            PORTAbits.RA0 = 1;
            T1CONbits.TON = 1;
        }
    }
}

void __attribute__((interrupt, auto_psv)) _T1Interrupt( void )
/*Esta rutina atiende la interrupcion del timer1 e incrementa en 1 a la variable
 milisegundos, si el timer fue activado por el pin 0 del portA este se desactiva
 a si mismo y coloca el valor del pin en 0 reiniciando el valor de "milisegundos"*/
{
    IFS0bits.T1IF = 0;
    milisegundos++;
    if (PORTAbits.RA0)
    {
        T1CONbits.TON = 0;
        PORTAbits.RA0 = 0;
        milisegundos = 0;
    }   
        
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt( void )
/*Esta rutina atiende la interrupcion del timer2 el cual se activa cuando se 
 pisa el primer piezoelectrico para considerar que el mismo se activa por 10ms
 y evitar lecturas dobles en la rutina de atencion del changeNotification
 a travez de la variable flagpiezo*/
{
    IFS0bits.T2IF = 0;
    flagpiezo[0] = 0;
    T2CONbits.TON = 0;
}

void __attribute__((interrupt, auto_psv)) _T3Interrupt( void )
/*Esta rutina atiende la interrupcion del timer3 el cual se activa cuando se 
 pisa el segundo piezoelectrico para considerar que el mismo se activa por 10ms
 y evitar lecturas dobles en la rutina de atencion del changeNotification
 a travez de la variable flagpiezo*/
{
    IFS0bits.T3IF = 0;
    flagpiezo[1] = 0;
    T3CONbits.TON = 0;
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void)
/*Esta rutina controla los pines 6, 7 y 13 del portD. si el primer piezoelectrico
 es presionado por primera vez se incrementa en 1 la variable "autos" y se
 activa el timer1 encargado del calculo de velocidad, cuando se activa el segundo
 piezoelectrico se para el timer1 y se llama a la funcion calculoVelocidad.
 con cada pisada en un piezoelectrico se activa su timer asociado y se levanta
 su flag para considerar que estos quedan activados por 10ms.
 el primer piezoelectrico tambien se ocupa de contar la cantidad de ejes a traves
 de la variable "piezo1" y solo se incrementa este valor si el flag esta en 0
 para evitar las lecturas dobles y unicamente copia este valor a la variable
 "ejes" si tiene un valor mayor a 0 y el lazo inductivo esta desactivado.
 si el valor de "ejes" es mayor a 2 se considera que paso un camion y
 se decrementa el valor de "autos" a la misma vez que se incrementa el de "camiones"*/
{
    IFS1bits.CNIF = 0;
    if (PORTDbits.RD13)
    {
        
        if (PORTDbits.RD6)
        {
            if (piezo1 == 0)
            {
                T1CONbits.TON = 1;
                autos++;
            }
            if (!flagpiezo[0])
            {
                T2CONbits.TON = 1;
                flagpiezo[0] = 1;
                piezo1++;
            }
        }
        if (PORTDbits.RD7)
        {
            if (piezo2 == 0)
            {
                T1CONbits.TON = 0;
                calculoVelocidad();
                milisegundos = 0;
            }
            if (!flagpiezo[1])
            {
                T3CONbits.TON = 1;
                flagpiezo[1] = 1;
                piezo2++;
            }
        }
    }       
    else
    {
        if(piezo1 > 0)
            ejes = piezo1;
        if (ejes > 2)
        {
            autos--;
            camiones++;
        }
    }
}

