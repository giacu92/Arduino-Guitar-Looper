/* ***************
 * 
 * GUITAR LOOPER v0.1
 * 
 * ***************
 * Pin assigment:
 * 
 * - D3:  A/B relay switch
 * - D4:  patch 1 relay switch
 * - D5:  patch 2 relay switch
 * - D6:  patch 3 relay switch
 * - D7:  patch 4 relay switch
 */

/////////// INCLUDE ////////////:
#include <Wire.h>    

/////////// DEFINE  ////////////:
#define disk 0x50    //Address of 24LC256 eeprom IC

//ingressi switch:
#define loop1pin    0
#define loop2pin    1
#define loop3pin    2
#define loop4pin    3
#define bankUPpin   4
#define bankDWNpin  5
#define ABpin       6 
#define PGM_SAVEpin 7

//uscite relè:    uscita: 000 AAAA B: AAAA patch[4 - 1], B = AB
#define ABrele    8
#define loop1rele 9
#define loop2rele 10
#define loop3rele 11
#define loop4rele 12

/////////// STATO ////////////:
typedef enum MACHINE_STATE
{
  PROGRAM_MODE,
  NORMAL_MODE
} machine_state_t;

/////////// VARIABILI GLOBALI ////////////:
unsigned int address = 0x00;
unsigned int signals = 0x00;

unsigned int currentBANK  = 0x00;
unsigned int currentPATCH = 0x00;

machine_state_t STATO = NORMAL_MODE;

/////////// SETUP ////////////: 
void setup(void)
{
  Serial.begin(9600);
  Wire.begin();  

  //Imposto le come ingressi per gli switch i pin [D0 - D7] (ingressi con 0)
  DDRD = B00000000;

  //Imposto come uscite per i relè i pin [D8 - D12] (uscite con 1)
  DDRB = DDRB | B00011111;
 
  delay(10);

  //Chiamo la routine di avvio del dispositivo
  bootUP();
}
 
void loop()
{
  switch (STATO)
  {
    case PROGRAM_MODE:
    programMode();
    break;

    case NORMAL_MODE:
    normalMode();    
    break;
  }
  
}
