#include "ASCII_CODE_8X16_5X8_VERTICAL.H"

#include <Wire.h>

#define OUTPUT_RNG_SERIAL

#define TMP35 P1_3
#define GEIGER_INT P1_4
#define B1 P2_4
#define BUZZER P2_2
#define LED P2_3

const uint8_t STRING_SIZE = 17;

const uint8_t BRIGHT = 0xFF;
const uint8_t DARK = 0x00;

const unsigned long SENCOND = 1000000;  // us
const uint16_t SENCOND_60 = 60; // minute
const uint16_t HOUR_24 = 24;  // day
const uint16_t DAY_365 =365;  // year

static const float ADC2VOL = (3.3f/1024.0f);  // 3.3v/10bit
const float TEMPERATURE_GAIN = 1.0f + 3.3f; // 1+ (33k/10k)
const float TEMPERATURE_CALIBRATION = 0;  // temperature offset deg_C;
const float TEMPERATURE_ADC_SCALE = 1000.0f;
static unsigned long temperature_update_time = 0;
const unsigned int TEMPERATURE_SAMPLING_RATE = 50000; // 50000us = 20Hz
static unsigned long sampling_update_time = 0;
double temp_sum = 0;
static unsigned int sampling_count = 0;

enum{
  GEIGER_COUNTER_MODE,
  TEMPERATURE_MODE,
  RANDON_NUMBER_GENERATOR_MODE
};
uint8_t mode = GEIGER_COUNTER_MODE;
bool switch_flag = false;

const float CPM2uSv = 153.8f; //CPM to uSv/h conversion rate

static unsigned long usv_update_time = 0;
static unsigned long count = 0;

const unsigned long CLICK_TIME_PERIOD = 900;  // us, Geiger Tube Dead Time
static unsigned long click_time = 0;

static bool count_flag = false;

static unsigned long last_pulse_time = 0;
const uint8_t PULSE_ARRAY_SIZE = 3;
unsigned long pulse_diff_time_array[PULSE_ARRAY_SIZE];
unsigned long pulse_count = 0;
uint8_t random_number = 0;
uint8_t random_number_size = 0;

const unsigned long CPS_TIME_PERIOD = 1;  // Count per Second, unit uSv/h, 1-second count
const unsigned long CPM_TIME_PERIOD = 20; // Count per Minute, unit mSv/Y, count * 60/20 = CPM
static unsigned long cps_update_time = 0;

unsigned char address = 0x3C; // SSD1306 I2C Address

static const uint8_t init_command[] = {
  0xae,
  0xd5,0x80,
  0xa8,0x3f,
  0xd3,0x00,
  0x40,
  0x8d,0x14,
  0x20,0x02,
  0xc8,0xa1,
  0xda,0x12,
  0x81,0xcf,
  0xd9,0xf1,
  0xdb,0x40,
  0xa6,
  0xaf
};

void TransCommand(uint8_t command)
{
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.write(command);
  Wire.endTransmission();
}

void TransData(uint8_t data)
{
  Wire.beginTransmission(address);
  Wire.write(0x40);
  Wire.write(data);
  Wire.endTransmission();
}

void InitSSD1306()
{
  
  for(int i = 0;
  i < (sizeof(init_command) / sizeof(uint8_t));++i){
    TransCommand(init_command[i]);
  }
  
  //byte error = Wire.endTransmission();  
}

void LCMMemoryAddress(uint8_t page,uint8_t column)
{
  column = column - 1;
  page = page - 1;
  TransCommand(0xb0 + page);
  TransCommand(((column>>4) & 0x0f) + 0x12);
  TransCommand(column&0x0f); 
}

void FullDisplay(uint8_t data1)
{
  for(int i = 0;i < 8;++i){
    LCMMemoryAddress(i + 1,1);
    for(int j = 128 - 1;j >= 0;--j){
      TransData(data1);
    }
  }
}

void StringDataWrite(uint8_t page,uint8_t column,char *text)
{
  if(column > 128){
    column = 1;
    page += 2;
  }
  uint8_t i = 0,j = 0;  
  while(text[i] > 0x00){ 
    if((text[i] >= 0x20) && (text[i] <= 0x7e)){
      j = text[i] - 0x20;
      for(uint8_t n = 0;n < 2;++n){     
        LCMMemoryAddress(page + n,column);
        for(uint8_t k = 0;k < 8;++k){         
          TransData(ascii_table_8x16[j][k + (8 * n)]);
        }
      }
      ++i;
      column += 8;
    }
    else{
      ++i;
    }    
  }
}

void DisplayString(uint8_t page,uint8_t column,char *text)
{
  char temp[2];
  uint8_t i = 0;
  while(text[i] != '\0'){
    temp[0] = text[i];    
    temp[1] = '\0';
    StringDataWrite(page, column, temp);
    column += 8;
    ++i;
  }
}

void GeigerDetected()
{
  detachInterrupt(GEIGER_INT);
  count_flag = true;  
  ++count;
  //attachInterrupt(P1_4,func2,RISING);
}

void ModeSet()
{
  detachInterrupt(B1);
  mode += 1;
  mode %= 3;  
  //attachInterrupt(B1,ModeSet,FALLING);

  switch_flag = true;
}

void Set32_768KHz()
{
  BCSCTL1 &=~ XT2OFF;

  do{
    IFG1 &=~ OFIFG;
    for(uint8_t a = 0xFF;a > 0;--a);
  }
  while((IFG1 & OFIFG));
  BCSCTL3 |= LFXT1S_0 + XCAP_3;
  while(IFG1 & OFIFG){
    IFG1 &= ~OFIFG;
  }  
}

void setup() {

  Set32_768KHz();

  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);   
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,HIGH);   
  
#ifdef OUTPUT_RNG_SERIAL
  Serial.begin(9600);
  Serial.println("MSP430Geiger");
#endif  

  pinMode(GEIGER_INT,INPUT);
  attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);
  
  pinMode(B1,INPUT);
  attachInterrupt(B1,ModeSet,FALLING);

  
  Wire.setModule(0); 
  Wire.begin();

  InitSSD1306();

  FullDisplay(DARK);  
  
  FullDisplay(BRIGHT);
  delay(1000);
  FullDisplay(DARK);  
  DisplayString(2,1,"TonyGuo");
  DisplayString(4,1,"Ver 1.0");
  DisplayString(6,1,"2020/02/14");
  delay(1000);
  FullDisplay(DARK);  
  DisplayString(2,1,"MSP430G2");
  DisplayString(4,1,"GeigerCount");
  DisplayString(6,1,"RNG");
  delay(1800);
  FullDisplay(DARK);  

  temperature_update_time = micros();
  usv_update_time = micros();
  cps_update_time = micros();
  click_time = micros();

}

void loop() {
  
  if(count_flag){
    count_flag = false;
    click_time = micros();
    attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);
  }
  
  if(mode != RANDON_NUMBER_GENERATOR_MODE){
    if((micros() - click_time) < CLICK_TIME_PERIOD){
      digitalWrite(LED,HIGH);
      digitalWrite(BUZZER, LOW);
    }
    else{
      digitalWrite(LED,LOW); 
      digitalWrite(BUZZER,HIGH);    
    }
  }

  if(mode == GEIGER_COUNTER_MODE){

    if(switch_flag){
      detachInterrupt(GEIGER_INT);
      
      FullDisplay(DARK);  
      switch_flag = false;
      
      count = 0;
      usv_update_time = micros();
      
      attachInterrupt(B1,ModeSet,FALLING);
      attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);            
    }
    
    if((micros() - usv_update_time) > CPM_TIME_PERIOD * SENCOND){
      detachInterrupt(GEIGER_INT);
      
      digitalWrite(LED,LOW); 
      digitalWrite(BUZZER,HIGH);  
        
      usv_update_time = micros();
  
      char str[STRING_SIZE]="";
      sprintf(str,"CPM:%d     ",count*(SENCOND_60/CPM_TIME_PERIOD));   
      DisplayString(1,1,str);

      float uSv = ((count*(SENCOND_60/CPM_TIME_PERIOD))*1000)/CPM2uSv;
      if(uSv < 100){
        sprintf(str,"uSv/h:0.0%ld   ",(unsigned long)uSv);
      }
      else if(uSv < 1000){
        sprintf(str,"uSv/h:0.%ld   ",(unsigned long)uSv);
      }
      else{
        float dot_uSv = uSv - (1000*(unsigned long)(uSv/1000));
        sprintf(str,"uSv/h:%ld.%ld   ",(unsigned long) uSv/1000,(unsigned long) dot_uSv);
      }

      DisplayString(3,1,str); 

      uSv /= 1000;
      float mSv = uSv * HOUR_24 * DAY_365;
      float dot_mSv = mSv - (1000*(unsigned long)(mSv/1000));
      sprintf(str,"mSv/Y:%ld.%ld   ",(unsigned long) mSv/1000,(unsigned long) dot_mSv);
      DisplayString(5,1,str); 
  
      count = 0;

      attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);
    }
    else if((micros() - cps_update_time) > CPS_TIME_PERIOD * SENCOND)/* if( (micros() - count_time) < 1000)*/{
      detachInterrupt(GEIGER_INT);
      
      digitalWrite(LED,LOW); 
      digitalWrite(BUZZER,HIGH);    
      
      char str[STRING_SIZE]={""};
      sprintf(str,"CPS:%ld   ",count);
      //inttostr(str,count);
      //DisplayString(7,1,"CPS: ");
      DisplayString(7,1,str);
      cps_update_time = micros();

      attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);
    }
  }
  else if(mode == TEMPERATURE_MODE){
    if(switch_flag){
      FullDisplay(DARK);  
      switch_flag = false;

      temp_sum = 0;
      sampling_count = 0;

      attachInterrupt(B1,ModeSet,FALLING);
    }
    
    if((micros() - sampling_update_time) > TEMPERATURE_SAMPLING_RATE){
      float temp = (analogRead(TMP35) * ADC2VOL) / TEMPERATURE_GAIN;
      temp_sum += temp;
      ++sampling_count;
      sampling_update_time = micros();
    }    
    
    if((micros() - temperature_update_time) > SENCOND){
      detachInterrupt(GEIGER_INT);
 
      temp_sum /= sampling_count;
      temp_sum += TEMPERATURE_CALIBRATION;
      temp_sum *= TEMPERATURE_ADC_SCALE;
      char str[STRING_SIZE]={""};
      //sprintf(str,"Temp:%ld.%ldC",(unsigned long)(temp/10),(unsigned long)(temp-(10*(unsigned long)(temp/10))));
      sprintf(str,"Temp:%ld.%ldC",(unsigned long)(temp_sum/10),(unsigned long)(temp_sum-(10*(unsigned long)(temp_sum/10))));
      DisplayString(5,1,str);

      temp_sum = 0;
      sampling_count = 0;
      
      temperature_update_time = micros(); 

      attachInterrupt(GEIGER_INT,GeigerDetected,FALLING);
     }
  }
  else if(mode == RANDON_NUMBER_GENERATOR_MODE){
    if(switch_flag){
      FullDisplay(DARK);  
      switch_flag = false;    
      attachInterrupt(B1,ModeSet,FALLING);  
    }    
    if(click_time != last_pulse_time){
      unsigned long diff_time = click_time - last_pulse_time;
      last_pulse_time = click_time;
      ++pulse_count;
      pulse_diff_time_array[pulse_count%PULSE_ARRAY_SIZE] = diff_time;
      if(pulse_count > PULSE_ARRAY_SIZE){
        unsigned long T1 = pulse_diff_time_array[(pulse_count-1)%PULSE_ARRAY_SIZE] - pulse_diff_time_array[(pulse_count-2)%PULSE_ARRAY_SIZE];
        unsigned long T2 = pulse_diff_time_array[(pulse_count-2)%PULSE_ARRAY_SIZE] - pulse_diff_time_array[(pulse_count-3)%PULSE_ARRAY_SIZE];
        uint8_t b = 0;
        if(T1 > T2){
          DisplayString(5,1,"BIT:0");
          b = 0;
        }
        else if(T1 <= T2){
          DisplayString(5,1,"BIT:1");
          b = 1;
        }
        random_number |= (b <<= random_number_size);
        ++random_number_size;
        if(random_number_size == 8){
          
          char str[STRING_SIZE]="";
          sprintf(str,"%d  ",random_number);   
          DisplayString(7,1,str); 

#ifdef OUTPUT_RNG_SERIAL
          Serial.println(str);
#endif
          
          random_number = 0;
          random_number_size = 0;
        }
      }
      /*
      char str[STRING_SIZE]="";
      sprintf(str,"%ldms  ",(unsigned long)(diff_time/1000));   
      DisplayString(4,1,str);      
      sprintf(str,"%ldus  ",diff_time);   
      DisplayString(6,1,str);
      */
      DisplayString(3,1,"Random");
      //cps_update_time = micros();
    }
  }
  
}
