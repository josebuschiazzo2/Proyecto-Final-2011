// Programa que convierte a un microcontrolador en un expansor de I/O.
// Usaremos como esclavo a un PIC16F877A.
// Programadores: Aranzamendi Gabriel. Buschiazzo José
// Fecha: 24/07/09.

// Encabezado del programa:

#include <16F877A.h>            // Definición de registros internos del microcontrolador.
#FUSES NOWDT                    // No utilizaremos el Watch Dog.
#FUSES HS                       // Utilizaremos un cristal de alta velocidad.
#FUSES PUT                      // Reset de power up - Activado.
#FUSES NOPROTECT                // No hay protección contra lecturas.
#FUSES NODEBUG                  // No se programa código para debug.
#FUSES NOBROWNOUT               // No se activa el reset por Brownout.
#FUSES NOLVP                    // No se activa el modo de programación a bajo voltaje.
#FUSES NOCPD                    // No se proteje a la eeprom contra escritura.
#FUSES NOWRT                    // No hay protección contra escritura.


#use delay(clock=20000000) 
#use i2c(Slave,sda=PIN_C4,scl=PIN_C3,address=0xB8) // Dirección del esclavo: 0xB8 ; I2C por hardware.



void entrada(int8 Pata);


// Variables globales:
int i2cstat,dato;
int flag=0;
int valor=0;
int ringueando=0;
BOOLEAN newstatus=false;
BOOLEAN Estado[8];
void Actualizar(void);

// Rutina de tratamiento de la interrupción por I2C.
#int_SSP
void SSP_isr()
{
   i2cstat=i2c_isr_state();
   if((i2cstat>=0)&&(i2cstat<0x80)){
         disable_interrupts(INT_SSP);
         dato=i2c_read(1);
         //output_high(PIN_B1); // Indica que se ha recibido un dato de parte del master.
         flag=1;
      }
   if(i2cstat>=0x80){
      disable_interrupts(INT_SSP);
      i2c_write(valor);
      i2c_read(1);
      output_high(PIN_B5);
      //newstatus=1;
      }
} 



void main(){
   int alarma =0;
   Estado[0]=0;
   Estado[1]=0;
   set_tris_b(0xC9);
   set_tris_d(0xFF);
   
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_SSP);
   output_high(PIN_B5);
   delay_ms(200);
   
   
   do{
      if((input(PIN_A0)==FALSE)&&(ringueando==0)){
         ringueando=1;
         Estado[0]=1;
         Actualizar();
         }
         
      if((input(PIN_B0)==FALSE)&&(alarma<3)){
         Estado[1]=1;
         Actualizar();
         alarma++;
         }
     
     
    /////////////////////////////   
    //   Bit 0 = Ring          //
    //   Bit 1 = Alerta Juana  //
    //   Bit 2 = Entrada 1     //
    //   Bit 3 = Entrada 2     //
    //   Bit 4 = Entrada 3     //
    //   Bit 5 = Salida 1      //
    //   Bit 6 = Salida 2      //
    //   Bit 7 = Salida 3      //
    /////////////////////////////
 
//************************************************************************************************************* 
 if((Estado[2]==TRUE)&&(input(PIN_A1)==TRUE)&&(input(PIN_B5)==TRUE)){
      Estado[2]=FALSE;
      newstatus=1;
      }
   if((input(PIN_A1)==FALSE)&&(Estado[2]==FALSE)&&(input(PIN_B5)==TRUE)){
         delay_ms(250);
         newstatus=1;
         Estado[2]=TRUE;
         output_toggle(PIN_B4);
         Estado[5]^=1;        //Hace un toggle en el elemento 6 del vector Estado Estado[5]=Estado[5]XOR 1
      }
  
//*************************************************************************************************************  
 
  if((Estado[3]==TRUE)&&(input(PIN_A2)==TRUE)&&(input(PIN_B5)==TRUE)){
      Estado[3]=FALSE;
      newstatus=1;
      }
   if((input(PIN_A2)==FALSE)&&(Estado[3]==FALSE)&&(input(PIN_B5)==TRUE)){
         delay_ms(250);
         newstatus=1;
         Estado[3]=TRUE;
         output_toggle(PIN_B2);
         Estado[6]^=1;        //Hace un toggle en el elemento 7 del vector Estado Estado[6]=Estado[6]XOR 1
      }  
//*************************************************************************************************************      
   if((Estado[4]==TRUE)&&(input(PIN_A3)==TRUE)&&(input(PIN_B5)==TRUE)){
      Estado[4]=FALSE;
      newstatus=1;
      }
   if((input(PIN_A3)==FALSE)&&(Estado[4]==FALSE)&&(input(PIN_B5)==TRUE)){
         delay_ms(250);
         newstatus=1;
         Estado[4]=TRUE;
         output_toggle(PIN_B1);
         Estado[7]^=1;        //Hace un toggle en el elemento 8 del vector Estado Estado[7]=Estado[7]XOR 1
      } 
//************************************************************************************************************* 

/*****************************
****  RECIBIO DATO X I2C  ****     
*****************************/
   
   if(flag==1){
      if((dato==1)){
        output_high(PIN_B1);
        dato=0;
        }     
            
      if((dato==2)){
        output_high(PIN_B2);
        dato=0;
        }
            
      if((dato==3)){
        output_high(PIN_B4);
        dato=0;
        }
      if(dato==11){
        output_low(PIN_B1);
        dato=0;
        }
      if(dato==12){
         output_low(PIN_B2);
         dato=0;
      }
      if(dato==13){
         output_low(PIN_B4);
         dato=0;
      }
      if(dato==20){                       // dato enviado automaticamente por el kit avisando que se atendio la llamada de la central
         ringueando=0;
         Estado[0]=0;
         dato=0;
      }
      if(dato==22){                       // Re-arma el aviso de Alarma 
         Alarma=0;
         Estado[1]=0;
         dato=0;
      }
      
      flag=0;
      newstatus=1;                      //levanta el flag para que actualice el vector de estado y lo envie al kit
     }
           
   if((newstatus==1)||(flag==1)){
     Actualizar(); 
     }
              
                 
   delay_ms(150);
   enable_interrupts(INT_SSP);
     
     
     
     
     
   /*
   fijarse antes de terminar el while que es lo que se ha recibido por i2c y poner ringueando a 0 si corresponde el mensaje enviado, acordarse de 
   ponerlo en el programa del kit
   */
   
   
   }while(true);


}
 
  
 void entrada(int8 Pata){
   if(input(Pata)==FALSE){
         delay_ms(50);
         if(input(Pata)==FALSE){
            delay_ms(150);
            }
         }
 }
 
 void Actualizar(void){
   valor=0;
   if(Estado[0]==TRUE){
      valor = valor+1;
      }
   if(Estado[1]==TRUE){
      valor = valor+2;
      }   
   if(Estado[2]==TRUE){
      valor = valor+4;
      }
   if(Estado[3]==TRUE){
      valor = valor+8;
      }
   if(Estado[4]==TRUE){
      valor = valor+16;
      }
   if(Estado[5]==TRUE){
      valor = valor+32;
      }
   if(Estado[6]==TRUE){
      valor = valor+64;
      }
   if(Estado[7]==TRUE){
      valor = valor+128;
      }
      output_low(PIN_B5);         // le avisa a juana que hubo cambios en el vector de estado para que lo solicite
      newstatus=0;
}

