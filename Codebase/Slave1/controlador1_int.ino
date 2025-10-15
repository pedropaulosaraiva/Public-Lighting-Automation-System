#include <LiquidCrystal_I2C.h>

#define light_sensor  8
#define switch_slave  3
#define overflow_T_V_I 2
#define led           9

#define SCK           13
#define MISO          12
#define MOSI          11
#define SS            10

#define Temperature   A0
#define Voltage        A1
#define Current      A2

int medTemp;
int medVoltage;
int medCurrent;
byte  state_slaveSwicth_lightSensor = B00000000;

bool master_switch = false;

//Array for the degree symbol
byte degree[8] ={ B00001100,
                B00010010,
                B00010010,
                B00001100,
                B00000000,
                B00000000,
                B00000000,
                B00000000,};

             
// Functions:
LiquidCrystal_I2C lcd(0x20,16,2);

void interruption_priority();  // Function to be executed when some interruption occur

// interruptions:
ISR (INT0_vect) {
  interruption_priority();
}

ISR (INT1_vect) {
  interruption_priority();
}

ISR (PCINT0_vect) {
  interruption_priority();
}

ISR (SPI_STC_vect) {
  uint8_t received = SPDR;
  if  (received == 127){
    master_switch = !master_switch;
    if(master_switch){
      digitalWrite(led,LOW);
    } else {
      interruption_priority();
    }
  } else if(received == 0){
    SPDR = lowByte(medTemp);
  } else if(received == 1){
    SPDR = lowByte(medVoltage-100);
  } else if(received == 2){
    SPDR = lowByte(medCurrent);
  } else if(received == 3){
    SPDR = state_slaveSwicth_lightSensor;
  }
}

void setup() {
  pinMode(light_sensor,INPUT_PULLUP);
  pinMode(switch_slave,INPUT_PULLUP);
  pinMode(overflow_T_V_I,INPUT);
  pinMode(led,OUTPUT);

  pinMode(MISO, OUTPUT);
  pinMode(MOSI, INPUT);
  pinMode(SCK, INPUT);
  pinMode(SS, INPUT);

  // Inital state of the led:
  bool overflow_state = digitalRead(overflow_T_V_I);
  bool switch_state   = !digitalRead(switch_slave);
  bool light_state     = digitalRead(light_sensor);
  if (overflow_state){
    digitalWrite(led,LOW);
  } else if (switch_state) {
    digitalWrite(led,LOW);
  } else if (light_state){
    digitalWrite(led,HIGH);
  } else  {
    digitalWrite(led,LOW);
  }
  
  // Inital state for the LCD display
  lcd.begin(16, 2);
  lcd.createChar(0, degree);
  lcd.setCursor(0,0);
  lcd.print("Con1:");           // This is the only line that chanhge between slaves
  lcd.setCursor(6,0);
  lcd.print("V=");
  lcd.setCursor(0,1);
  lcd.print("T=");

  
  lcd.setCursor(9,1);
  lcd.print("I=");
  
  // Registries configuration: 
  SPCR = B11101101;  // SPI configuration

  Serial.begin(9600);
  
  //interruptions:
  SREG  |= B10000000;    //Enables interrupts
  EICRA  = B00000101;    //Impose INT0 and INT1 per level change
  EIMSK  = B00000011;    //Enables INT0 and INT1
  PCICR  = B00000001;    //Enables PINCHANGE0~7
  PCMSK0 = B00000001;    //Enables PINCHANGE only for PCINT0
}

void loop() {
  medTemp = ((analogRead(Temperature))*500.00/1023.00)-50.00;
  medVoltage = (analogRead(Voltage)/1024.00)*64 +200;
  medCurrent = (analogRead(Current)/1024.00)*32;
  
  
  if ((digitalRead(light_sensor))==HIGH){
    state_slaveSwicth_lightSensor |= B00001111;
  } else {
    state_slaveSwicth_lightSensor &= B11110000;
  } if((digitalRead(switch_slave))==HIGH) {
    state_slaveSwicth_lightSensor |= B11110000;
  } else {
    state_slaveSwicth_lightSensor &= B00001111;
  }
  
  
  lcd.setCursor(8,0);
  lcd.print(medVoltage); 
  lcd.print("V");
  lcd.setCursor(2,1);
  lcd.print(medTemp);
  lcd.write(byte(0));
  lcd.print("C ");
  lcd.setCursor(11,1);
  lcd.print(medCurrent);
  lcd.print("A ");
}

void interruption_priority(){
  bool overflow_state = digitalRead(overflow_T_V_I);
  bool switch_state   = !digitalRead(switch_slave);
  bool light_state     = digitalRead(light_sensor);
  if (master_switch){
    
  } else if (overflow_state){
    digitalWrite(led,LOW);
  } else if (switch_state) {
    digitalWrite(led,LOW);
  } else if (light_state){
    digitalWrite(led,HIGH);
  } else  {
    digitalWrite(led,LOW);
  }
}
