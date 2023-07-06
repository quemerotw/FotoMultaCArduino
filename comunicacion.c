/*
 * File:   comunicacion.c
 * Author: Agustin maximiliano Coronel
 *
 * Created on 27 de junio de 2020, 19:12
 */


#include "xc.h"
#include "p33FJ256GP710.h"
#define SOF 0
#define QTY 1
#define DST 2
#define COMANDO 5
int campo,x = 0,flagDeRecepcion = 0,BCClocal = 0,BCC,z = 0;
int marcoRX[11];
extern unsigned char marcoTX[11];
char caracter;


void limpiarMarcoRX()
/*esta funcion se ocupa de limpiar el arreglo donde se reciben los datos que
 llegan de la UART y reinicializa los valores de los indices y la variable donde
 se aloja el bcc calculado en la placa*/
{
    for (x = 0;x < 10;x++)
        marcoRX[x] = 0;
    x = 0;
    z = 0;
    BCClocal = 0;
}

void check()
/*Esta funcion realiza una suma de todos los valores menos el ultimo del arreglo
 y compara el resultado con el BCC proveniente del cliente, para revisar si no
 hubo errores de comunicacion*/
{
    for (x = 0;x < (marcoRX[QTY] - 1);x++)
        BCClocal = BCClocal + marcoRX[x];
    if ((BCClocal % 256) == marcoRX[x])      
        flagDeRecepcion = 1;
    else
        limpiarMarcoRX();
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt( void )
/*Esta rutina atiende las interrupciones que se generan al recibir datos por la
 UART1 y los ordena en un arreglo segun el protocolo de comunicaion MARA-1,
 una vez que la cantidad de datos recibidos iguala a la cantidad de bytes
 indicada en el campo QTY de la comunicacion realiza una llamada a la funcion 
 "check"*/
{
	IFS0bits.U1RXIF = 0;
	campo = U1RXREG;
    if ((campo == 0xFE) && (x == 0))
    {
        marcoRX[SOF] = campo;
    }
    else
    {
        if (marcoRX[SOF] == 0xFE)
        {
            if (marcoRX[QTY] == 0)
            {
                marcoRX[QTY] = campo;
                x = (campo - 4);
            }   
            else
            {
                if ((campo == 2) || (marcoRX[DST] == 2))
                {
                    if (marcoRX[DST] != 2)
                        marcoRX[DST] = campo;
                    else
                    {
                        marcoRX[x] = campo;
                        x++;
                        if (marcoRX[QTY] == x)
                        {
                            IEC0bits.U1RXIE = 0;
                            BCC = marcoRX[x-1];
                            check();
                            x = 0;
                        }
                    }
                }
                else
                    limpiarMarcoRX();
            }
        }
    }
    if (flagDeRecepcion)
        caracter =(char) marcoRX[COMANDO];
}



void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void)
/*esta rutina de atencion se ocupa de enviar ordenadamente segun el protocolo
 MARA-1 los datos que fueron requeridos por el cliente*/
{
    IFS0bits.U1TXIF = 0;
    if(marcoTX[QTY] > z)
    {
        U1TXREG = marcoTX[z];
        z++;
    }
    else
    {
        limpiarMarcoRX();
        IEC0bits.U1RXIE = 1;
        flagDeRecepcion = 0;
        x = 0;
    }
}