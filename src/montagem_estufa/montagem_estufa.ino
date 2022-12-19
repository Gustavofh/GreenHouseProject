#include <Wire.h>    //INCLUSÃO DA BIBLIOTECA
#include <LiquidCrystal_I2C_Hangul.h>
#include <RTClib.h>  //Real time clock lib
#include <DHT.h>     // Temperature & humidity

// RELE VARS
const int bombaRele = 11;
const int lampRele = 12;
const int coolerRele = 13;


// LCD VARS
LiquidCrystal_I2C_Hangul lcd (0x27, 20, 4);


// SENSOR VARS

////// DHT TEMPERATURE AND UMIDITY
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperature;
float humity;
const int tempIdeal = 0;  // selecionar temperatura ideal


////// WATER LEVEL SENSOR

// DISTANCIAS / PROFUNDIDADE
// 700 ml - FULL == 3
// 500 ml == 10
// 300 ml - EMPTY == 13 ///// DEPENDE DO TAMANHO DA BOMBA

const int TRIG = 9;
const int ECHO = 10;
const int cupFull = 3; // CALIBRAGEM. inserir medida do sensor distância quando armazenamento completo de água
const int cupEmpty = 14; // CALIBRAGEM. inserir medida do sensor distância quando armazenamento vazio de água
int percentDistance = 0;

long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

bool blinkLed = HIGH;


////// PUMP LEDS
const int ledWaterLevel = 2;
const int ledPumpState = 3;


////// MOITURE SENSOR
const int moisturePin = A0;
const int AirValue = 840;   //you need to replace this value with Value_1
const int WaterValue = 390;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilMoisturePercent;
int pumpStatus;
const int soilLed = 4;


// TIME VARS
RTC_DS3231 rtc;  //OBJETO DO TIPO RTC_DS3231 // time and temeprature
char daysOfTheWeek[7][12] = { "Domingo",
                              "Segunda",
                              "Terça",
                              "Quarta",
                              "Quinta",
                              "Sexta",
                              "Sábado"
                            };
char dateBuffer[12];

////////////////////////////////      SETUP      ////////////////////////////////

void setup() {

  // INITIATING MONITORS
  Serial.begin(9600);  //INICIALIZA A SERIAL

  Serial.println(" **** INICIANDO PROGRAMA **** ");

//  // INITIATING LCD
//  lcd.init();
//  lcd.backlight();

  // INITIATING DHT SENSOR
  dht.begin();

  // INTIATING INPUTS/OUTPUTS
  pinMode(moisturePin, INPUT);  // define moiture sensor pin
  pinMode(lampRele, OUTPUT);    // define lamp pin
  pinMode(bombaRele, OUTPUT);   // define pump pin
  pinMode(coolerRele, OUTPUT);  // define cooler pin
  pinMode(TRIG, OUTPUT);        // distance sensor
  pinMode(ECHO, INPUT);         // distance sensor
  pinMode(ledWaterLevel, OUTPUT);
  pinMode(ledPumpState, OUTPUT);
  pinMode(soilLed, OUTPUT);


  // INITIATING CLOCK
  if (!rtc.begin()) {                         // SE O RTC NÃO FOR INICIALIZADO, FAZ
    Serial.println("DS3231 não encontrado");  //IMPRIME O TEXTO NO MONITOR SERIAL
    while (1);  //SEMPRE ENTRE NO LOOP
  }
  if (rtc.lostPower()) {  //SE RTC FOI LIGADO PELA PRIMEIRA VEZ / FICOU SEM ENERGIA / ESGOTOU A BATERIA, FAZ
    Serial.println("DS3231 OK!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));  //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
    //        rtc.adjust(DateTime(2022, 01, 19, 19, 59, 45)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }

  delay(100);
}

////////////////////////////////      LOOP      ////////////////////////////////

void loop() {

  DateTime now = rtc.now();


  ////////    LAMP TIMER (12h/12h)    ////////

  if (now.hour() >= 20 || now.hour() < 8) {
    digitalWrite(lampRele, HIGH);
  } else {
    digitalWrite(lampRele, LOW);
  }


  ////////    COOLER CONTROL    ////////

  if (now.minute() >= 5 && now.minute() < 10) {
    digitalWrite(coolerRele, HIGH);
  } else if (now.minute() >= 30 && now.minute() < 35) {
    digitalWrite(coolerRele, HIGH);
  } else {
    digitalWrite(coolerRele, LOW);
  }

  ////////    TEMPERATURE AND HUMIDITY DHT22    ////////

  float hum = dht.readHumidity();//lê o valor da umidade e armazena na variável h do tipo float (aceita números com casas decimais)
  float temp = dht.readTemperature();//lê o valor da temperatura e armazena na variável t do tipo float (aceita números com casas decimais)

  if (isnan(hum) || isnan(temp)) {//Verifica se a umidade ou temperatura são ou não um número
    Serial.println("sensor not found");//Caso não seja um número retorna
  }


  ////////    WATER LEVEL SENSOR    ////////

  digitalWrite(TRIG, LOW);
  delayMicroseconds(1);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG, LOW);
  distance = pulseIn(ECHO, HIGH) / 58;
  percentDistance = map(distance, cupEmpty, cupFull, 0, 100);


  ////////    CONTROL WATER PUMP    ////////

  soilMoistureValue = analogRead(moisturePin);

  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  pumpStatus = 0;

  if (soilMoisturePercent < 30) {
    digitalWrite(soilLed, blinkLed);
    blinkLed = !blinkLed;
  } else {
    digitalWrite(soilLed, LOW);
  }

  if (distance < 12) { // ALTERAR O VALOR AQUI

    digitalWrite(ledWaterLevel, LOW);

    // BOMBA DESLIGADA

    if (soilMoisturePercent >= 50) {
      pumpStatus = 0;
      digitalWrite(bombaRele, HIGH);
      digitalWrite(ledPumpState, LOW);
    }

    // BOMBA LIGADA

    else {
      pumpStatus = 1;
      digitalWrite(bombaRele, LOW);
      digitalWrite(ledPumpState, HIGH);
    }
  }


  // WATER LEVEL LOW

  else {
    digitalWrite(bombaRele, HIGH);
    digitalWrite(ledPumpState, LOW);
    digitalWrite(ledWaterLevel, HIGH);
    pumpStatus = 2;
  }


  ////////    CONTROL COOLER    ////////

  //  digitalWrite(coolerRele, HIGH);


  ////////    PRINTS    ////////

//  if (now.second() % 5 == 0) {
  Serial.print("Data: ");
  sprintf(dateBuffer, "%02u-%02u-%04u", now.day(), now.month(), now.year());
  Serial.println(dateBuffer);
  Serial.print("Day: ");
  Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print("Clock: ");
  sprintf(dateBuffer, "%02u:%02u:%02u", now.hour(), now.minute(), now.second());
  Serial.println(dateBuffer);
  Serial.print("Temp Outside: ");
  Serial.println(rtc.getTemperature());
  Serial.print("Temp Inside: ");
  Serial.println(temp);
  Serial.print("Hum Inside: ");
  Serial.println(hum);
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println("%");
  Serial.print("Water Content: ");
  Serial.println(distance);
  Serial.print("Perc Water: "); // AJUSTAR
  Serial.print(percentDistance); // AJUSTAR
  Serial.println("%"); // AJUSTAR
  Serial.print("Pump Status: ");
  if (pumpStatus == 0) {
    Serial.println("Pump OFF");
  } else if (pumpStatus == 1) {
    Serial.println("Pump ON");
  } else {
    Serial.println("Water Level Low - Pump OFF");
  }
  Serial.println("");
  Serial.println("");
  Serial.println("");
//}


  //    ////////    LCD PRINTS    ////////
//    lcd.setCursor(0,0);
//    lcd.print(now.day());
//    lcd.print("/");
//    lcd.print(now.month());
//    lcd.print(" ");
//  
//    if (now.hour() < 10) {
//      lcd.print("0");
//    }
//    lcd.print(now.hour());
//    lcd.print(":");
//    if (now.minute() < 10) {
//      lcd.print("0");
//    }
//    lcd.print(now.minute());
//    lcd.print(":");
//    if (now.second() < 10) {
//      lcd.print("0");
//    }
//    lcd.print(now.second());
//  
//    lcd.setCursor(0,1);
//  
//    if (now.second() % 5 == 0) {
//    //// TEMPERATURE OUTSIDE ////
//    lcd.print("TOut:");
//    lcd.print("24");
//    lcd.print(" ");
//    } else {
//      //// TEMPERATURE INSIDE ////
//    lcd.print("TIns:");
//    lcd.print("24");
//    }
//      //// TEMPERATURE INSIDE ////
//    lcd.setCursor(0,2);
//    lcd.print("Hum:");
//    lcd.print("24");
//  
//    lcd.print(" ");
//    lcd.print("Most:");
//    lcd.print(50);
//    lcd.print("%");
//

  ////////    FINAL DELAY    ////////

  delay(1000);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////  ALTERNATIVE PRINTS --> FOR TABLES  ////

// INITIAL PRINTS
//  Serial.print("   Data     /   ");  //IMPRIME O TEXTO NO MONITOR SERIAL
//  Serial.print("Dia    /    ");      //IMPRIME O TEXTO NA SERIAL
//  Serial.print("Horas   / ");
//  Serial.print("humity  /  ");
//  Serial.print("tempOutside  /  ");
//  Serial.print("tempInside  /  ");
//  Serial.print("soilMoisture  /  ");
//  Serial.print("waterLevel  /  ");
//  Serial.println("pumpStatus");

//  sprintf(dateBuffer, "%02u-%02u-%04u", now.day(), now.month(), now.year());
//  Serial.print(dateBuffer);
//  Serial.print("  /  ");
//  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
//  Serial.print("   /  ");
//  sprintf(dateBuffer, "%02u:%02u:%02u", now.hour(), now.minute(), now.second());
//  Serial.print(dateBuffer);
//  Serial.print("   /  ");
//  Serial.print(hum);
//  Serial.print("   /   ");
//  Serial.print(rtc.getTemperature());
//  Serial.print("   /   ");
//  Serial.print(temp);
//  Serial.print("   /   ");
//  Serial.print(soilMoisturePercent);
//  Serial.print("   /   ");
//  Serial.print(distance);
//  Serial.print("   /   ");
//  if (pumpStatus == 0) {
//    Serial.println("DC Pump is OFF now!!");
//  } else if (pumpStatus == 1) {
//    Serial.println("DC Pump is ON Now!!");
//  } else {
//    Serial.println("Water Level Low");
//  }
