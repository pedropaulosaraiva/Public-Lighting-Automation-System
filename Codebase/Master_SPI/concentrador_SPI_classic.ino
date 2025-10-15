#include  <SPI.h>
#include <LiquidCrystal_I2C.h>   


#define SS_1  10
#define SS_2  9
#define SS_3  8

#define led_light_sensor  7
#define led_slave_switch 6
#define change_displayed_slave  3
#define global_switch   2

#define MISO  12

int  state_displayed_slave = 0;
int  requeriment_master = 0;
bool send2ss1 = false;
bool send2ss2 = false;
bool send2ss3 = false;

volatile byte  data_received_s1_i;
volatile byte  data_received_s2_i;
volatile byte  data_received_s3_i;

byte  data_received_s1[4];
byte  data_received_s2[4];
byte  data_received_s3[4];

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

// interruptions:
ISR (INT0_vect) {
    
  digitalWrite(SS_1, LOW); // Enables slave 1 
  delayMicroseconds(10);
  SPI.transfer(127);          //Send flag of the global switch
  digitalWrite(SS_1, HIGH);
  
  digitalWrite(SS_2, LOW); // Enables slave 2
  delayMicroseconds(10);
  SPI.transfer(127);          // Send flag of the global switch
  digitalWrite(SS_2, HIGH);
  
  digitalWrite(SS_3, LOW); // Enables slave 3
  delayMicroseconds(10);
  SPI.transfer(127);          // Send flag of the global switch
  digitalWrite(SS_3, HIGH);
}

ISR (INT1_vect) {
  state_displayed_slave++;
  if (state_displayed_slave == 3){
    state_displayed_slave = 0;
  }
}

ISR (TIMER1_COMPA_vect) {
  if (requeriment_master ==0){
    requeriment_master++;
    send2ss1 = true; 
  }else if (requeriment_master ==1){
    requeriment_master++;
    send2ss2 = true;        
  }else if (requeriment_master ==2){
    requeriment_master = 0;
    send2ss3 = true; 
  }     
}

void setup() {
  Serial.begin(9600);               
  
  pinMode(SS_1,OUTPUT);              
  pinMode(SS_2,OUTPUT);
  pinMode(SS_3,OUTPUT);
  pinMode(MISO,INPUT);
  
  pinMode(change_displayed_slave,INPUT_PULLUP);  // This triger an interruption (INT1)
  pinMode(global_switch,INPUT_PULLUP);  // This triger an interruption (INT0)
  
  pinMode(led_light_sensor,OUTPUT);
  pinMode(led_slave_switch,OUTPUT);
       
  
  // Inital state for the LCD display
  lcd.begin(16, 2);
  lcd.createChar(0, degree);
  lcd.setCursor(0,0);
  lcd.print("Con1:");
  lcd.setCursor(6,0);
  lcd.print("V=");
  lcd.setCursor(0,1);
  lcd.print("T=");
  lcd.setCursor(9,1);
  lcd.print("I=");
  
  //Interrupções:
  SREG  |= B10000000;    //Enables interrupts
  EICRA  = B00000001;    //Impose INT0 and INT1 per level change
  EIMSK  = B00000011;    //Enables INT0 and INT1
  
                    
  digitalWrite(SS_1,HIGH);
  digitalWrite(SS_2,HIGH);
  digitalWrite(SS_3,HIGH);
  
  SPI.begin();
                              
  SPCR  =B01111101; // SPI configuration
  delay(1000);  // This delay assures that the slaves are functional before the master
  //Timer configuration (For request the information to the slaves):
  TCCR1A = B00000000;
  OCR1AH = 61;
  OCR1AL = 9;
  TIMSK1 = B00000010;  
  TCCR1B = B00001101;
}

void loop() {
  if (send2ss1){
    digitalWrite(SS_1, LOW);  // Enables slave 1 
    delayMicroseconds(10);
    SPI.transfer(0);  // Send a dummy byte to the slave, sometimes the first recieved byte was problems
    digitalWrite(SS_1, HIGH);
    // Received 5 bytes from the first slave
    for (int i = 0; i < 4; i++) {
      digitalWrite(SS_1, LOW);
      // The following delay is necessaire for the communication, the value has tested with Proteus SPI
      // debug component. It assures that the respective slave is ready to communicate.
      delayMicroseconds(10); 
      data_received_s1_i = SPI.transfer(i+1); //Send request bytes to the slave 
      digitalWrite(SS_1, HIGH);
      data_received_s1[i] = data_received_s1_i;
    } send2ss1 = false;
  }  
  if  (state_displayed_slave == 0){
  lcd.setCursor(0,0);
  lcd.print("Con1:");
  lcd.setCursor(8,0);
  lcd.print(int(data_received_s1[1])+100); 
  lcd.print("V");
  lcd.setCursor(2,1);
  lcd.print(int(data_received_s1[0]));
  lcd.write(byte(0));
  lcd.print("C ");   
  lcd.setCursor(11,1);
  lcd.print(int(data_received_s1[2]));
  lcd.print("A ");
  if (data_received_s1[3] == B11111111){
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s1[3] == B11110000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s1[3] == B00001111) {
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,HIGH);
  } else if(data_received_s1[3] == B00000000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,HIGH);
  }
  }
  // Same code for the slave 2
  if (send2ss2){
    digitalWrite(SS_2, LOW); 
    delayMicroseconds(10);
    SPI.transfer(0);          
    digitalWrite(SS_2, HIGH);
    
    for (int i = 0; i < 4; i++) {
      digitalWrite(SS_2, LOW);
      delayMicroseconds(10);
      data_received_s2_i = SPI.transfer(i+1); 
      digitalWrite(SS_2, HIGH);
      data_received_s2[i] = data_received_s2_i; 
    }send2ss2 = false;        
  }
  
  if  (state_displayed_slave == 1){
  lcd.setCursor(0,0);
  lcd.print("Con2:");
  lcd.setCursor(8,0);
  lcd.print(int(data_received_s2[1])+100); 
  lcd.print("V");
  lcd.setCursor(2,1);
  lcd.print(int(data_received_s2[0]));
  lcd.write(byte(0));
  lcd.print("C ");   
  lcd.setCursor(11,1);
  lcd.print(int(data_received_s2[2]));
  lcd.print("A ");
  if (data_received_s2[3] == B11111111){
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s2[3] == B11110000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s2[3] == B00001111) {
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,HIGH);
  } else if(data_received_s2[3] == B00000000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,HIGH);
  }
  }
  // Same code for the slave 3
  if (send2ss3){
    digitalWrite(SS_3, LOW); 
    delayMicroseconds(10);
    SPI.transfer(0);          
    digitalWrite(SS_3, HIGH);
    
    for (int i = 0; i < 4; i++) {
      digitalWrite(SS_3, LOW);
      delayMicroseconds(10);
      data_received_s3_i = SPI.transfer(i+1); // Envio de um byte "dummy" (0) para receber dados do escravo
      digitalWrite(SS_3, HIGH);
      data_received_s3[i] = data_received_s3_i; 
    }send2ss3 = false;
  }
    
  if  (state_displayed_slave == 2){
  lcd.setCursor(0,0);
  lcd.print("Con3:");
  lcd.setCursor(8,0);
  lcd.print(int(data_received_s3[1])+100); 
  lcd.print("V");
  lcd.setCursor(2,1);
  lcd.print(int(data_received_s3[0]));
  lcd.write(byte(0));
  lcd.print("C ");   
  lcd.setCursor(11,1);
  lcd.print(int(data_received_s3[2]));
  lcd.print("A ");
  if (data_received_s3[3] == B11111111){
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s3[3] == B11110000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,LOW);
  } else if(data_received_s3[3] == B00001111) {
    digitalWrite(led_light_sensor,LOW);
    digitalWrite(led_slave_switch,HIGH);
  } else if(data_received_s3[3] == B00000000) {
    digitalWrite(led_light_sensor,HIGH);
    digitalWrite(led_slave_switch,HIGH);
  }
  }
}
