/*
 * File:   main.c
 * Author: Quemero
 *
 * Created on 1 de julio de 2020, 09:16
 */


#include "p33FJ256GP710.h"
#include "xc.h"

#define SOF 0
#define QTY 1
#define DST 2
#define SRC 3
#define SEC 4
#define COMANDO 5
#define DATOS 6 
#define MAX_VEHICULOS 500
struct registro {
int Nejes;
float velocidad;
};
typedef struct registro tRegistro;

unsigned long dato;
unsigned char marcoTX[11];
unsigned char datos[4];
int y = 0,cantidad, bccTX = 0,w = 0,nDatos = 0;
extern int ejes,flagDeRecepcion;
extern char caracter;
extern unsigned long autos,camiones;
extern float vel;
tRegistro listado[MAX_VEHICULOS];

void ordenadorDatos()
/*Se ocupa de separar y convertir los valores de los datos a enviar en tipo char
 para que ocupen 1 byte y se envien ordenados por la uart */
{
    if (dato > 0xFFFFFF)
    {
        datos[0] = dato / 0x1000000;
        datos[1] = (dato  / 0x10000) % 0x1000000;
        datos[2] = (dato / 0x100) % 0x10000;
        datos[3] = dato % 0x100; 
        nDatos = 3;
    }
    else
    {
        if (dato > 0xFFFF)
        {
            datos[0] = dato / 0x10000;
            datos[1] = (dato  / 0x100) % 0x10000;
            datos[2] = dato % 0x100; 
            nDatos = 2;
        }
        else
        {
            if (dato > 0xFF)
            {
                datos[0] = dato / 0x100;
                datos[1] = dato % 0x100;
                nDatos = 1;
            }
            else
            {
                datos[0] = dato;
                nDatos = 0;
            }
        }
    }
}

void creadorPaquete()
/*Ordena el arreglo marcoTX y aloja los valores de los datos requeridos por el 
 * cliente de manera ordenada segun el protocolo MARA-1 forzando al final la
 rutina de atencion de envio de la uart para que comienze el envio del paquete*/
{
    marcoTX[SOF] = (unsigned char) 0xFE;
    marcoTX[QTY] = (unsigned char) cantidad;
    marcoTX[DST] = (unsigned char) 3;
    marcoTX[SRC] = (unsigned char) 2;
    marcoTX[SEC] = (unsigned char) 0x80;
    marcoTX[COMANDO] = caracter;
    if (cantidad > 7)
    {
        for(w = 0; w < cantidad - 7;w++)
            marcoTX[DATOS+w] = datos[w];
    }
    bccTX = 0;
    for (w = 0;w < (marcoTX[QTY] - 1);w++)
        bccTX = bccTX + marcoTX[w];
    marcoTX[marcoTX[QTY]-1] =(bccTX % 0x100);
    flagDeRecepcion = 0;
    nDatos = 0;
    IFS0bits.U1TXIF = 1;
}


void comandoA()
/*coloca la cantidad de autos que pasaron el carril de la doble trocha en 
 la variable "dato" la cual se utiliza luego en la llamada a la funcion que se
 ocupa de crear el paquete que sera enviado por la uart*/
{
    dato = autos;
    ordenadorDatos();
    cantidad = 8 + nDatos;
    creadorPaquete();
}

void comandoB()
{
/*responde al comando "B" reiniciando el indice de la variable "listado" que
 guarda los datos de los vehiculos registrados por el datalogger y coloca
 el comando "D" para responder con un ACK, para luego llamar a la funcion
 creadora de paquetes*/
    y = 0;
    caracter = 'D';
    cantidad = 7;
    creadorPaquete();
}

void comandoC()
/*coloca la cantidad de camiones que pasaron el carril de la doble trocha en 
 la variable "dato" la cual se utiliza luego en la llamada a la funcion que se
 ocupa de crear el paquete que sera enviado por la uart*/
{
    dato = camiones;
    ordenadorDatos();
    cantidad = 8 + nDatos;
    creadorPaquete();
}

int main(void) 
/*Se ocupa de las llamadas para inicializacion al principio y luego consulta
 la variable "ejes" la cual solo se modifica cuando se realizo un ciclo
 completo de el paso de un vehiculo para luego alojar los datos en el listado.
 ademas consulta la variable flagDeRecepcion la cual determina el correcto
 recibimiento de un comando del cliente y en base al comando enviado elije la
 funcion que debe ser llamada*/
{
    InitUART1();
    initPorts();
    initTimer();
    initInterrupt();
    while(1)
    {
        if (ejes > 0)
        {
            listado[y].Nejes = ejes;
            listado[y].velocidad = vel;
            y++;
            limpiarPiezos();
            if (y >= MAX_VEHICULOS)
                y = 0;
        }   
        if (flagDeRecepcion)
            switch (caracter)
            {
                case 'A':
                    comandoA();
                    break;
                case 'B':
                    comandoB();
                    break;
                case 'C':
                    comandoC();
                    break;
                default:
                    caracter = 'E'; 
                    cantidad = 7;
                    creadorPaquete();
            }
    }
    return 0;
}

