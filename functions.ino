void bootUP()
{
  //Leggo lastADDRESS all'indirizzo 0x00:
  unsigned int lastADDRESS = readEEPROM(disk, 0x00);

  //Imposto current_BANK e current_PATCH
  currentBANK  = ((lastADDRESS-8) & 0x3C) >> 2; // (address & 0011 1100) >> 2
  currentPATCH = ((lastADDRESS-8) & 0x03) +  1; // (address & 0000 0011) +  1 per avere un conteggio a partire da 1 delle patch come da pedaliera

  //Leggo &ADD e tiro fuori i dati:
  signals = readEEPROM(disk, lastADDRESS);

  //Attivo i segnali:
  switchON(signals);

  //Indirizzo corrente = lastADDRESS
  address = lastADDRESS;

  //Stato macchina = NORMAL_MODE
  STATO = NORMAL_MODE;
}

//Ritorna l'indirizzo a partire da bank e patch:
unsigned int address_generator(byte _bank, byte _patch)
{
  return ((_bank << 2) + _patch + 7);
}

//Accende i segnali relativi ai dati in ingresso
void switchON(unsigned int _signals)
{
  PORTB &= _signals;
}

//Modalità di program
void programMode()
{
  /*  Se sono qui conosco l'ultimo address impostato dalla NORMAL_MODE. Su quello applicherò
   *  allora le mie modifiche.
   */
  
  //Aspetto di rilasciare lo switch per uscire da NORMAL_MODE
  while(digitalRead(PGM_SAVEpin) == HIGH) {}

  //Dichiaro alcune variabili
  bool lastButtonState = LOW;
  bool save = false;
  unsigned int _signals = signals;

  //Adesso che PGM_SAVEpin == LOW leggo eventuali modifiche fino all'uscita
  while(digitalRead(PGM_SAVEpin) != HIGH)
  {
    if (digitalRead(loop1pin == HIGH) && lastButtonState == LOW)
    {
      lastButtonState = HIGH;
      delay(50);
      bitWrite(_signals, 1, !bitRead(_signals, 1)); //Scrivo l'inverso del valore su loop1 in signals
    }   
    else if (digitalRead(loop2pin == HIGH))
    {
      lastButtonState = HIGH;
      delay(50);
      bitWrite(_signals, 2, !bitRead(_signals, 2)); //Scrivo l'inverso del valore su loop1 in signals
    }
    else if (digitalRead(loop3pin == HIGH))
    {
      lastButtonState = HIGH;
      delay(50);
      bitWrite(_signals, 3, !bitRead(_signals, 3)); //Scrivo l'inverso del valore su loop1 in signals
    }
    else if (digitalRead(loop4pin == HIGH))
    {
      lastButtonState = HIGH;
      delay(50);
      bitWrite(_signals, 4, !bitRead(_signals, 4)); //Scrivo l'inverso del valore su loop1 in signals
    }
    else if (digitalRead(ABpin == HIGH))
    {
      lastButtonState = HIGH;
      delay(50);
      bitWrite(_signals, 0, !bitRead(_signals, 0)); //Scrivo l'inverso del valore su loop1 in signals
    }
    else if //Se sono qui ho voluto modificare, allora applico sui relè
    (
      lastButtonState      == HIGH &&
      digitalRead(ABpin    == LOW) &&
      digitalRead(loop1pin == LOW) &&
      digitalRead(loop2pin == LOW) &&
      digitalRead(loop3pin == LOW) &&
      digitalRead(loop4pin == LOW) 
    )
    {
      //Attivo i relè corrispondenti:
      switchON(_signals);
      
      //Imposto l'ultimo stato come LOW per accettare nuove modifiche
      lastButtonState = LOW;
    }
  }

  /*Se sono qui PGM_SAVEpin (per fare STORE) è uguale a HIGH --> salvo la patch*/
  //debounce di PGM_SAVEpin con delay()
  delay(100);

  //Copio _signals su signals
  signals = _signals;

  //Scrivo sulla EEPROM
  writeEEPROM(disk, address, signals);

  //Stampo "STORED" sul display (faccio qualcosa di grafico)

  //Aspetto di rilasciare lo switch per uscire da PROGRAM_MODE
  while(digitalRead(PGM_SAVEpin) == HIGH) {}

  //Vado in Normal Mode
  STATO = NORMAL_MODE;
}

//Modalità normale
void normalMode()
{
  bool change = false;
  bool pgm_mode = false;
  
  if(digitalRead(loop1pin) == HIGH && currentPATCH != 1)
  {
    currentPATCH == 1;

    change = true;
    do{} while(digitalRead(loop1pin) == HIGH); //Aspetto di rilasciare il pin
  }
  if(digitalRead(loop2pin) == HIGH && currentPATCH != 2)
  {
    currentPATCH == 2;
    change = true;
    do{} while(digitalRead(loop2pin) == HIGH); //Aspetto di rilasciare il pin
  }
  if(digitalRead(loop3pin) == HIGH && currentPATCH != 3)
  {
    currentPATCH == 3;
    change = true;
    do{} while(digitalRead(loop3pin) == HIGH); //Aspetto di rilasciare il pin
  }
  if(digitalRead(loop4pin) == HIGH && currentPATCH != 4)
  {
    currentPATCH == 4;
    change = true;
    do{} while(digitalRead(loop4pin) == HIGH); //Aspetto di rilasciare il pin
  }
  if(digitalRead(ABpin) == HIGH)
  {
    bitWrite(signals, 0, !bitRead(signals, 0)); //Scrivo l'inverso del valore su loop1 in signals

    switchON(signals);  //Inverto AB
    
    do{} while(digitalRead(ABpin) == HIGH); //Aspetto di rilasciare il pin
  }
  
  if(digitalRead(bankUPpin) == HIGH)
  {
    if(currentBANK < 10)
    {
      currentBANK += 1;
      change = true;
    }
    do{} while(digitalRead(bankUPpin) == HIGH); //Aspetto di rilasciare il pin
  }
  if(digitalRead(bankDWNpin) == HIGH)
  {
    if(currentBANK > 0)
    {
      currentBANK -= 1;
      change = true;
    }
    do{} while(digitalRead(bankUPpin) == HIGH); //Aspetto di rilasciare il pin
  }

  //Se devo leggere la EEPROM perchè ho cambiato banco o patch
  if(change == true)
  {
    //Genero l'indirizzo per la lettura di BANK e PATCH
    address = address_generator(currentBANK, currentPATCH);
    
    //Leggo l'indirizzo e tiro fuori i dati:
    signals = readEEPROM(disk, address);

    //Attivo i segnali:
    switchON(signals);

    //Imposto change a false
    change = false;
  }

  //Vedo se devo andare in PROGRAM_MODE
  if (digitalRead(PGM_SAVEpin) == HIGH)
  {
    delay(200); //debounce
    
    int time_now = millis();
    while(millis() < time_now + 800)  //quando supero un secondo vado in PROGRAM MODE
    {
      if(digitalRead(PGM_SAVEpin) == LOW)
      {
        pgm_mode = false;
        break;
      }
      else pgm_mode = true;
    }
    if (pgm_mode == true) STATO = PROGRAM_MODE;
  }
}

//legge la EEPROM
byte readEEPROM(int _deviceADDRESS, unsigned int _logicalADDRESS ) 
{
  byte rdata = 0xFF;
 
  Wire.beginTransmission(_deviceADDRESS);
  Wire.write((int)(_logicalADDRESS >> 8));   //indirizzo da 16 bit quindi devo mandare 2 byte, qui mando il primo
  Wire.write((int)(_logicalADDRESS & 0xFF)); //e qui mando il secondo
  Wire.endTransmission();
 
  Wire.requestFrom(_deviceADDRESS, 1);
 
  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}

void writeEEPROM(int _deviceADDRESS, unsigned int _logicalADDRESS, byte _data ) 
{
  Wire.beginTransmission(_deviceADDRESS);
  Wire.write((int)(_logicalADDRESS >> 8));   //indirizzo da 16 bit quindi devo mandare 2 byte, qui mando il primo
  Wire.write((int)(_logicalADDRESS & 0xFF)); //e qui mando il secondo
  Wire.write(_data);
  Wire.endTransmission();
 
  delay(5);
}

