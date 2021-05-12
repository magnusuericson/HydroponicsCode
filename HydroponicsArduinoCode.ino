#include <OneWire.h>

int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
int EC_Read = A0;
int ECPower = A1;
int PH_Read = A2;
int PHPUMP_Pin = 4;
int NPUMP_Pin = 6;
float Vin=5.0;
float probeResistor = 1000.0;
float Constant = -1110.0;
float Slope = 3.73 ;
float Temp_Coef = 0.02; //2% per celcius
float PPM_Con=0.5; //TDS scale
float ad7 = 314.0;
float ad4 = 421.0;


//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2

void setup(void) {
  Serial.begin(9600);
  pinMode(EC_Read,INPUT);
  pinMode(ECPower,OUTPUT);
  pinMode(PHPUMP_Pin, OUTPUT);
  pinMode(NPUMP_Pin, OUTPUT);

}

void loop(void) {
  float temperature = getTemp();
  Serial.print("Temp: ");
  Serial.println(temperature);
  float ppm = getPPM(temperature);
  Serial.print("PPM: ");
  Serial.println(ppm);
  float ph = getPH();
  Serial.print("PH: ");
  Serial.println(ph);

  /*digitalWrite(PHPUMP_Pin, LOW);
  delay(2000);
  digitalWrite(PHPUMP_Pin, HIGH);
  delay(1000);
  digitalWrite(NPUMP_Pin, LOW);
  delay(2000);
  digitalWrite(NPUMP_Pin, HIGH);*/
  digitalWrite(NPUMP_Pin, HIGH);
  digitalWrite(PHPUMP_Pin, HIGH);

  if(ppm<700 && ph>6.6){ //if the nutrient solution level low and ph high
    Serial.println("1");
    digitalWrite(NPUMP_Pin, LOW);
    delay(2000);
    digitalWrite(NPUMP_Pin, HIGH);
  }
  else if (ppm>700 && ph>6.6){ //if the nutrient level ok and ph high
    Serial.println("2");
    digitalWrite(PHPUMP_Pin, LOW);
    delay(2000);
    digitalWrite(PHPUMP_Pin, HIGH);
  }
  else if (ppm<700 && ph<6.6){ //if the nutrient level low and ph ok
    Serial.println("3");
    digitalWrite(NPUMP_Pin, LOW);
    delay(2000);
    digitalWrite(NPUMP_Pin, HIGH);
  }

  
  
  
  delay(6000); //Do not make this less than 6 sec (6000)  

  
  
  
}
float getPH() {
  int currentValue = 0;

  for (int i = 0; i < 10; i++)
  {
    currentValue += analogRead(PH_Read);
    delay(100);
  }
  float sensorValue = currentValue / 10;
  float m = (-3.0/(ad4-ad7));
  float c = 7 - (m*ad7);
  float ph = ((m*sensorValue)+c);

  return ph;
    
  
}

float getPPM(float Temperature) {
  digitalWrite(ECPower, HIGH); //Vin volts to a voltage divider using resistor and water
  delay(200);
  float VoverWater=analogRead(EC_Read); //Measures voltage over water (scale 0-1023)
  delay(100);
  digitalWrite(ECPower, LOW); 

  float VoverWaterInVolts= (Vin*VoverWater)/1023.0; //scaling factor from scale 0-1023 to 0-5V
  float RWater = (probeResistor*VoverWaterInVolts)/(Vin-VoverWaterInVolts);//conductance of water (uS) between electrodes
  float GWater = 1000000.0/RWater; //Conductance in microsiemens
  float EC = Constant + GWater*Slope; //EC calculated using experimentally defined constants
  float EC_at_25 = EC / (1+ Temp_Coef*(Temperature-25.0));
  float ppm=EC_at_25*PPM_Con;
  return ppm;
}


float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
