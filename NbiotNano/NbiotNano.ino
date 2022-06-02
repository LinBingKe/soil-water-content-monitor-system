#include <math.h>
#include <SPI.h>
#define num_of_read 1 //READ THE SENSOR NUM NUMBER OF TIMES..SHOULD THE SUCCESIVE READINGS VARY
#include "BC26Init.h"
//#include <avr/sleep.h>
//#include <avr/wdt.h>
//certain time to do certain thing
unsigned long intervals[] = {15000,1000,1000}; //this defines the interval for each task in milliseconds

//unsigned long intervals[] = {30000,1000,1000}; //this defines the interval for each task in milliseconds
unsigned long last[] = {0,0,0};           //this records the last executed time for each task
int Upload=0;

volatile byte data=0;
//For wireless transmission of data
String MQTT_Server = "\"iiot.ideaschain.com.tw\""; //MQTT Server 的 IP 位址，IdeasChain 網站
String MQTT_Port = "1883";                        //MQTT 使用的埠
String MQTT_user = "\"DHT11\"";                   //使用者名稱 <== 任意名稱
String MQTT_pass = "\"BsflPt10atOjmHpZXgin\"";    //使用者密碼 ==> IdeasChain 存取權杖
String MQTTtopic = "\"v1/devices/me/telemetry\""; //固定路徑 IdeasChain 網站

//
const int Rx = 7870;  //fixed resistor attached in series to the sensor and ground...the same value repeated for all WM and Temp Sensor.
const long default_TempC = 24;
const long TempC = 35;
const long open_resistance = INFINITY; //check the open resistance value by replacing sensor with an open and replace the value here...this value might vary slightly with circuit components
const long short_resistance = 93.78; // similarly check short resistance by shorting the sensor terminals and replace the value here.
const long short_CB = 240;
const long open_CB = 255;
int i;
int j = 0;                                               

int ARead_A4 = 0; //ANALOG READ CHANNEL FOR MUX 0-1
int ARead_A5 = 0; //ANALOG READ CHANNEL FOR MUX 0-1
int Hv1=0; //ANALOG READ CHANNEL FOR MUX 0-1
int Lv1=0; //ANALOG READ CHANNEL FOR MUX 0-1
int Hv2=0; //ANALOG READ CHANNEL FOR MUX 2-3
int Lv2=0; //ANALOG READ CHANNEL FOR MUX 2-3

double Resist1;
double Resist2;

double WM1_CB;
double WM2_CB;
int passtime=0;
int passtime2=0;
int msinterval=2;//setting measure interval 
unsigned long now=0 ;
unsigned long now1=0;
unsigned long now2=0;

void Data_publish(String message)          // update data function
{
  Upload=1;
  Serial.println("資料上傳中 ......");
  //連接
  if (!connect_MQTT(MQTT_Server, MQTT_Port, MQTT_user, MQTT_pass)){
    Serial.println("連接失敗");
    Upload=0;
  }
  //發布
  if (!Publish_MQTT(MQTTtopic, message)){
    Serial.println("發布失敗");
    Upload=0;
  }
  //關閉
  if (Close_MQTT()) Serial.println("Close MQTT-Server Connect ...");
  
}

void setup()
{
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);

  digitalWrite(10,HIGH);

  digitalWrite(11,LOW);
  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  //digitalWrite(13,LOW);
  digitalWrite(2,LOW);
  digitalWrite(3,LOW);
  
  Serial.begin(9600);
  
  mySerial.begin(9600);
  
  while (!BC26init()) delay(10000);
  delay(5000);
  Serial.println("初始設定完成 ....");

  //sleep 
//  setup_watchdog(9);

  
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  //Sleep_avr();//Sleep_Mode

  Start(); 
  
  
}


void loop()
{
    
//     Sleep_avr();//Sleep_Mode 
//     Start();

      now=millis();
      if(now-last[0]>=intervals[0]){ 
        last[0]=now; 
        Start(); 
        }
        else{
          digitalWrite(10,LOW);
          delay(1000);
          digitalWrite(10,HIGH);
        }


/* 
      if(data>=(msinterval-passtime)){
        data=0;
      //-------------------------------
       now1=millis();
       Serial.println("開始");
       Serial.println(now1);
                    
       Start();   
       now2 = millis();
       passtime=int((now2 -now1+passtime2+12183 )/8000);
       passtime2= int((now2 -now1+12183  )%8000);
       
    
      //--------------------------------   
      }
      else {
        Sleep_avr();  //Continue Sleep
      }  
*/ 
}

//release different direction of pulse and measure the signal
void firstTask(){
  Serial.println("啟動");
}

void secondTask(){
  Serial.println("啟動");
}

void thirdTask(){
  Serial.println("啟動");
}

void Start(){
        Serial.println("啟動");
        StartMeasurement();
        StartMeasurement();
        StartMeasurement();
        Upload=1;
        String MQTTmessage;
        String Sensor1 = "Resist";
        String Sensor2 = "WM1_CB";
        String Sensor3 = "Resist";
        String Sensor4 = "WM2_CB";        
                                                                                                                                                                                 
        Serial.print(" , Resist1,  ");
        Serial.print(Resist1);
        Serial.print(", WM1_CB  ,");
        Serial.println(WM1_CB);
        
        Serial.print(" , Resist2,  ");
        Serial.print(Resist2);
        Serial.print(", WM2_CB  ,");
        Serial.println(WM2_CB);
                
        
        //MQTTmessage="\"{\"" + Sensor1 + "\":" +  DHTtemp + "," + "\"" + Sensor2 + "\":" +  DHThumi + "}\"";
        MQTTmessage = "\"{\"" + Sensor3 + "\":" +  Resist2 + "," + "\"" + Sensor4 + "\":" +  WM2_CB+"}\"";
    
        //傳送的data
        while(Upload==0){
        Data_publish(MQTTmessage);
        Serial.println("傳遞資料 !!");
        }   
}
void StartMeasurement(){
        digitalWrite(11,LOW);
        digitalWrite(12,LOW);
        digitalWrite(13,LOW);
        //digitalWrite(13,LOW);
        digitalWrite(2,HIGH);
        digitalWrite(3,HIGH);
        digitalWrite(11,LOW);

        
        //Serial.println("sensor1 !!");
        SelectSensor(LOW,LOW,LOW);
//ShowData(Lv1,Lv2);        
        Resist1 = 7870.0 *  float(2044-Lv1-Lv2)/float(Lv1+Lv2);
        WM1_CB=CalCb(Resist1);
        
        //delay(15000);
        digitalWrite(2,HIGH);
        digitalWrite(3,HIGH);
        digitalWrite(11,LOW);

        //Serial.println("sensor2 !!");
        SelectSensor(HIGH,LOW,LOW);
//ShowData(Lv1,Lv2);        
        Resist2 = 7870.0 *  float(2044-Lv1-Lv2)/float(Lv1+Lv2);
        WM2_CB=CalCb(Resist2);

        //delay(15000);

}

void SelectSensor(int s0,int s1,int s2){
    digitalWrite(2, LOW);   //Enable MUX 0-1
    digitalWrite(3, HIGH); //Disable MUX 2-3 pair to prevent ground leakage on corresponding channels on path B 
                  
    digitalWrite(12, s0);
    digitalWrite(13, s1);

    delay(10); //Make sure the MUX channels for sensor on Channel 0 is ready for activation
    // sensor on channel 0 is the 10 k calibration resistor 
    digitalWrite(11, HIGH);   //energize the sensor through path A 
//Serial.println("開啟");
//delay(1000);
    delay(0.09); //wait 90 micro seconds and take sensor read...do not exceed 100uS
    //Hv1=analogRead(A0);   //read the supply voltage past the Diode and Switching transistor
    Lv1=analogRead(A4);   // read the sensor voltage right after the sensor and right before the series resistor Rx-7870
    digitalWrite(11, LOW);      //set the excitation voltage to OFF/LOW
delay(10); //0.1 second wait before moving to next channel or switching MUX
 
    // Now lets take reading reading through path B
    digitalWrite(2, HIGH);   //Enable MUX 2-3
    digitalWrite(3, LOW);   //Disable MUX 0-1 pair to prevent ground leakage on corresponding channels on path B                 
    digitalWrite(12, s0);
    digitalWrite(13, s1);
    delay(10); //Make sure the MUX channels for sensor on Channel 0 is ready for activation
    // sensor on channel 0 is the 10 k calibration resistor 
    digitalWrite(11, HIGH);   //energize the sensor through path A
//Serial.println("開啟");
//delay(1000);
    delay(0.09); //wait 90 micro seconds and take sensor read...do not exceed 100uS
    //Hv2=analogRead(A2);   //read the supply voltage past the Diode and Switching transistor
    Lv2=analogRead(A5);   // read the sensor voltage right after the sensor and right before the series resistor Rx-7870
    digitalWrite(11, LOW);      //set the excitation voltage to OFF/LOW
delay(2000);

//ShowData(Hv1,Lv1);
//ShowData(Hv2,Lv2);


 
}

//show measure volt
void ShowData(int subHv,int subLv){
  Serial.print("高電位  ");
  Serial.println(subHv);
  Serial.print("低電位  ");
  Serial.println(subLv);
}
//convert resistent to water tension
double CalCb(double subResist){
  float subWM1_CB;
  
   if (subResist < 300) {
      subWM1_CB = short_CB; //240 is a fault code for open circuit or sensor not present
      Serial.print("WM1_CB=short_CB ");
      Serial.print(subWM1_CB);
      Serial.print("\n");
    }
    else if (subResist == INFINITY) {
      subWM1_CB = open_CB;//255 is a fault code for open circuit or sensor not present
      Serial.print("WM1_CB=open_CB  ");
      Serial.print(subWM1_CB);
      Serial.print("\n");
    }
    else if (300.00 < subResist && subResist < 550.00) {
      subWM1_CB = 0.00;
    }
    else if (550.00 < subResist && subResist < 1000.00) {
      subWM1_CB = abs(-20.00 * ((subResist / 1000.00) * (1.00 + 0.018 * (TempC - 24.00)) - 0.55));
    }
    else if (1000.00 < subResist && subResist < 8000) {
      subWM1_CB = abs((-3.213 * (subResist / 1000.00) - 4.093) / (1 - 0.009733 * (subResist / 1000.00) - 0.01205 * (TempC))) ;
    }
    else if (subResist > 8000.00) {
      subWM1_CB = abs((-2.246) - 5.239 * (float(subResist) / 1000.00) * (1.00 + 0.018 * (float(TempC) - 24.00)) - 0.06756 * (float(subResist) / 1000.00) * (float(subResist) / 1000.00) * ((1.00 + 0.018 * (float(TempC) - 24.00)) * (1.00 + 0.018 * (float(TempC) - 24.00))));
    }

    return subWM1_CB;
}
//--------------------------------sleepMode
/*
void setup_watchdog(int ii) {

  byte bb;

  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

//WDT interrupt
ISR(WDT_vect) {
  ++data;
// wdt_reset();
}

void Sleep_avr(){
//  ACSR |=_BV(ACD);//OFF ACD
//  ADCSRA=0;//OFF ADC
  set_sleep_mode(SLEEP_MODE_IDLE  ); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System sleeps here
}
*/
