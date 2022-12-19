/////////////////////////////////////////////////////////////////////////
////                                                                 ////
//// RR2_USB_Cdc_Monitor.c                                           ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////



#include <18F14K50.h>
#fuses HS,CPUDIV1,NOWDT,NOPROTECT,NODEBUG,PUT,PLLEN,NOMCLR
#use delay(clock=48000000)
#use rs232(baud=19200, xmit=PIN_B7,rcv=PIN_B5,PARITY=N,BITS=8,STOP=1)
#use i2c(Master,sloW,sda=PIN_B4,scl=PIN_B6,force_hw) // Utilizamos hardware I2C a 400Khz.



#include "input.c"
#include "string.h"

//*************************
//*     DECLARACIONES     *
//*************************

int banderas(void);
void espera(void);
void Borrar(void);
void outgsm(void);
void Llamar(void);
void send_i2c(int dato, int dir); // Función que escribe en el PIC esclavo.


char dato;
//char Telefono[12] = "3434705186";  // es el telefono a buscar cuando se recibe un llamado para que atienda automaticamente
char Telefono[12] = "3434466933";  // es el telefono a buscar cuando se recibe un llamado para que atienda automaticamente
char Texto[13] = "ActivarAB01";         // es el texto que busca en un sms, si se busca un telefono en el encabezado ojo que viene sin el +
char Ring[5] = "RING";               // cuando se recibe un llamado el SIM340 envia un RING
char Mensaje[5] = "+CMT";             // +CMT es parte del encabezado de un sms por eso se utiliza para detectar sms
char Entrada[256];
char strgsm[20];
//char [256];

int n = 0;
int flag = 0;
int Estado = 0;
int l=0;                            //Lo uso para guardar un dato a enviar en I2C
short int status;                   //Lo usa para I2C

//BYTE address = 0x00; 
//int pu = 0;

//*************************
//*   INTERRUPCION RS232  *
//*************************

#INT_RDA
void serial_isr(){
   disable_interrupts(GLOBAL);
   disable_interrupts(INT_RDA);
   flag=0;
   Estado=0;
   while(kbhit()){
      dato=getc();
  
   if(dato==10||dato==13){
      flag=1;
      }                // ESTE FLAG INDICA QUE EL SIM340 DEVOLVIO UN ENTER
   if((dato>=11)&&(dato<=165)){
      Entrada[n++]=dato;
  //    jose[pu++]=dato;
      }
      }
   if(n==255){
      n=0;
      }
end: 
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
}

//*************************
//*  PROGRAMA PRINCIPAL   *
//*************************

void main() {
   output_low(PIN_C4);        // para encender el modulo debe estar en bajo C4 durante 3 segundos o mas
   delay_ms(3100);
   output_high(PIN_C4);       // Hasta aca es para Encender el sim340
   delay_ms(15000);           // espero a que conecte hasta q sepamos que comando AT mandarle para que diga que esta logeado en la red
   output_high(PIN_C5);
   delay_ms(2000);
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);

//*************************
//* CONFIGURACION DEL KIT *
//*************************

   printf("AT\r");            // Envia un AT para sincronizar la velocidad del rs232
   strcpy(strgsm,"AT+CLIP=1\r");
   outgsm();
   strcpy(strgsm,"AT+CMGF=1\r");
   outgsm();
   strcpy(strgsm,"AT+CNMI=2,2,0,0,0\r");
   outgsm();
   output_low(PIN_C5);        // AVISA QUE TERMINO LA CONFIGURACION POSIBLE INDICADOR DE KIT OCUPADO
   
   
 do{
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
    

   if(input(PIN_A3)==FALSE){
         output_high(PIN_C5);
         send_i2c(l,0xB8);
         delay_ms(200);
         output_LOW(PIN_C5);
         l++;
         if(l==3){
            l=0;
         }
      }

  
   if(input(PIN_C2)==FALSE){
      output_high(PIN_C5);
      Llamar();
      delay_ms(200);
      output_LOW(PIN_C5);  
      }
  
   if(input(PIN_C3)==FALSE){
      output_high(PIN_C1);
      output_high(PIN_C0);
      output_high(PIN_C5);
      delay_ms(2000);
      flag=1;
      banderas();
      output_low(PIN_C1);
      output_low(PIN_C0);
      output_low(PIN_C5);
      }
      
   if(input(PIN_B6)==FALSE){
  //    int np;
      output_high(PIN_C5);
      delay_ms(200);
  //    for(np=0;np<255;np++) {
  //       write_eeprom(address,jose[np]);
  //       address++;
  //       }
      output_LOW(PIN_C5);
      }
   if(flag==1){
      banderas();
      }
}while(true);
  
 
}

void outgsm (void){   //A function that has one CHAR parameter will accept a constant string where it is called

   delay_ms(250);
loop:   
   while((flag!=1)){
      printf(strgsm);
      delay_ms(1500);
      }
   if(banderas()!=1){
      goto loop;
      }
}

int banderas(void){
   if(flag==1){
   
//*************************
//*      BUSCA SMS        *
//*************************
   
        if((Estado==0)&&((strstr(Entrada,Mensaje))!=NULL)){      // por alguna puta razon no anda esto
         if((strstr(Entrada,Telefono)!=NULL)){     
            if((strstr(Entrada,Texto)!=NULL)){
               Borrar();
               Estado=4;
               output_high(PIN_C5);
               output_high(PIN_C1);
               delay_ms(150);
               output_low(PIN_C5);
               output_low(PIN_C1);
               delay_ms(250);      
               }
            }
         }

//*************************
//*      BUSCA OK         *
//*************************

      if((Entrada[n-1]==13)&&(Entrada[n-2]=='K')&&(Entrada[n-3]=='O')){
         output_high(PIN_C6);
         delay_ms(50);         //prende y apaga la luz
         OUTPUT_LOW(PIN_C6);
         Borrar();
         n=0;
         Estado=1;
         } 
//*************************
//*      BUSCA ERROR      *
//*************************
   
      if (( Entrada[n-1]==13)&&(Entrada[n-2]=='R')&&(Entrada[n-3]=='O')&&(Entrada[n-4]=='R')){
         OUTPUT_HIGH(PIN_C7);
         delay_ms(50);         //prende y apaga la luz 
         OUTPUT_LOW(PIN_C7);
         n=0;
         Borrar();
         Estado=2;
         }
//*************************
//*BUSCA LLAMADA ENTRANTE *
//*************************      

      if((Estado==0)&&((strstr(Entrada,Ring))!=NULL)){      // 
         if((strstr(Entrada,Telefono)!=NULL)){     
            Borrar();
            Estado=3;
            output_high(PIN_C5);
            output_high(PIN_C1);
            delay_ms(150);
            printf("ATA\r");
            output_low(PIN_C5);
            output_low(PIN_C1);
            delay_ms(250);      
            }
         }

   flag=0;
   Return Estado;}
ELSE 
   Return 0;
}
 
 
 void Borrar(void){
   for(n=0;n<255;n++){
      Entrada[n]=0;
      }
   n=0;
 }
 void Llamar(void){
 printf("ATD%s;\r",Telefono);
 }
 
 void send_i2c(int dato, BYTE dir)
{
   i2c_start();
   status=i2c_write(dir);
   status=i2c_write(dato);
   i2c_stop();
   delay_us(50);
  
}
