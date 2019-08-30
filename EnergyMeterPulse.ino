/*
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi
*/

#define AUTOMATSTOP             0
#define AUTOMATREADYFORRUNNING  5    
#define AUTOMATRUNNING          10
#define AUTOMATREADYFORSTOP     15
#define AUTOMATDONE             20
#define AUTOMATREADYFORDOWN     25
#define AUTOMATREADYFORUP       30

#define TIMERSTEP               15*60UL
#define TIMERMIN                15*60UL
#define TIMERMAX                10*60*60UL

#define METERCONSTANT           2000

void energymeterpulse_display_refresh(void);

#include <LiquidCrystal.h>
const int rs = 2, en = 4, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
const int cidlo = 13;
const int tlOK = 8, tlUP = 7, tlDOWN = 6;
static uint8_t stavcidla;
static uint8_t automat = AUTOMATSTOP;
static long counter;
static long timer; 
static long timer_preset; 
static long milis;
long power;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup()
{
  // cidlo
  pinMode(cidlo, INPUT);

  // tlacitka
  pinMode(tlOK, INPUT);
  pinMode(tlUP, INPUT);
  pinMode(tlDOWN, INPUT);

  // lcd
  lcd.begin(16, 2);           // set up the LCD's number of columns and rows:
  lcd.print("E-meter counter");

  // inicializace cidla a automatu
  stavcidla = digitalRead(cidlo);
  automat = AUTOMATSTOP;
  counter = 0;
  timer = TIMERMIN;
  timer_preset = timer;

  // seriova linka
  Serial.begin(9600);
  Serial.println("Energy meter counter");
  
}

void loop()
{
  // detekce stavu cidla pri behu casovace
  if (automat == AUTOMATRUNNING)
  {
    if ((stavcidla == 0) && (digitalRead(cidlo) != 0))
    {
      // hrana 0->1
      counter++;
    }
    stavcidla = digitalRead(cidlo);
  }

  // obsluha automatu
  switch (automat)
  {
    case AUTOMATSTOP:
      timer = timer_preset;
      counter = 0;
      if (digitalRead(tlOK))
      {
        // je stisknuto OK
        automat = AUTOMATREADYFORRUNNING;
        Serial.println(automat);
      }
      if (digitalRead(tlUP))
      {
        // je stisknuto UP
        automat = AUTOMATREADYFORUP;
        if (timer < TIMERMAX)
        {
          timer += TIMERSTEP;
        }
        timer_preset = timer;
        Serial.println(automat);
      }
      if (digitalRead(tlDOWN))
      {
        // je stisknuto DOWN
        automat = AUTOMATREADYFORDOWN;
        if (timer > TIMERMIN)
        {
          timer -= TIMERSTEP;
        }
        timer_preset = timer;
        Serial.println(automat);
      }
    break;

    case AUTOMATREADYFORRUNNING:
      if (digitalRead(tlOK) == 0)
      {
        // je opusten stisk OK
        automat = AUTOMATRUNNING;
        Serial.println(automat);
      }    
    break;

    case AUTOMATRUNNING:
      if (digitalRead(tlOK))
      {
        // je stisknuto OK
        automat = AUTOMATREADYFORSTOP;
        Serial.println(automat);
      }   
      // zmena casovace
      if (millis()/1000 > milis)
      {
        // doslo ke zmene
        milis = millis()/1000;
        if (timer > 0)
        {
          timer--;
        }
        else
        {
          automat = AUTOMATDONE;
        }
      }
    break;

    case AUTOMATREADYFORSTOP:
      if (digitalRead(tlOK) == 0)
      {
        // je opusten stisk OK
        automat = AUTOMATSTOP;
        Serial.println(automat);
      }    
   
    case AUTOMATREADYFORUP:
      if (digitalRead(tlUP) == 0)
      {
        // je opusten stisk UP
        automat = AUTOMATSTOP;
        Serial.println(automat);
      }
    break;    

    case AUTOMATREADYFORDOWN:
      if (digitalRead(tlDOWN) == 0)
      {
        // je opusten stisk DOWN
        automat = AUTOMATSTOP;
        Serial.println(automat);
      }    
    break;

    case AUTOMATDONE:
      // vypis vysledek
      #define ENERGYTHS     8
      #define ENERGYPOINT   9
      #define ENERGYDECI    10
      #define ENERGYKWH     13
      lcd.setCursor(0, 1);
      lcd.print("ENERGY:         ");
        
      power = 1000 * counter / METERCONSTANT;
        
      lcd.setCursor(ENERGYTHS, 1);
      lcd.print(power / 1000); 
       
      lcd.setCursor(ENERGYPOINT, 1);
      lcd.print(".");

      // museji byt uvodni nuly vzdy do poctu cifer = 3
      if ((power % 1000) >= 100)
      {
        // tri cifry
        lcd.setCursor(ENERGYDECI, 1);
        lcd.print(power % 1000);
      }
      else if ((power % 1000) < 10)
      {
        // jedna cifra
        lcd.setCursor(ENERGYDECI, 1);
        lcd.print("00");
        lcd.setCursor(ENERGYDECI+2, 1);
        lcd.print(power % 1000);
      }
      else 
      {
        // dve cifry
        lcd.setCursor(ENERGYDECI, 1);
        lcd.print("0");
        lcd.setCursor(ENERGYDECI+1, 1);
        lcd.print(power % 1000);          
      }
        
      lcd.setCursor(ENERGYKWH, 1);
      lcd.print("kWh");

      if (digitalRead(tlOK))
      {
        // je stisknuto OK
        automat = AUTOMATREADYFORSTOP;
        Serial.println(automat);
      }   

    break;

    default:
    break;
  }

  // aktualizace displeje
  if (automat != AUTOMATDONE)
  {
    energymeterpulse_display_refresh();
  }

  delay(75);

  #undef ENERGYTHS
  #undef ENERGYPOINT
  #undef ENERGYDECI
  #undef ENERGYKWH
}

void energymeterpulse_display_refresh(void)
{
  // prvni radek
  lcd.setCursor(0, 0);
  lcd.print("PULSES:         ");
  lcd.setCursor(8, 0);
  lcd.print(counter);
  
  // druhy radek
  lcd.setCursor(0, 1);
  lcd.print("TIMER: 00:00:00 ");
  uint8_t hours;
  uint16_t mins;
  uint16_t secs;
  hours = timer / (60 * 60);
  secs = timer % (60 * 60);
  mins = secs / 60;
  secs = secs % 60;

  #define HOURS     7
  #define MINS      10
  #define SECS      13 

  // hodiny
  lcd.setCursor(HOURS, 1);
  if (hours < 10)
  {
    lcd.print("0");
    lcd.setCursor(HOURS+1, 1);
  }
  lcd.print(hours);
  
  lcd.setCursor(HOURS+2, 1);
  lcd.print(":");

  // minuty
  lcd.setCursor(MINS, 1);
  if (mins < 10)
  {
    lcd.print("0");
    lcd.setCursor(MINS+1, 1);
  }
  lcd.print(mins);
  
  lcd.setCursor(MINS+2, 1);
  lcd.print(":");

  // sekundy
  lcd.setCursor(SECS, 1);
  if (secs < 10)
  {
    lcd.print("0");
    lcd.setCursor(SECS+1, 1);
  }
  lcd.print(secs);

  #undef HOURS
  #undef MINS
  #undef SECS
}
