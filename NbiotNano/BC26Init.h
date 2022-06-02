#include <SoftwareSerial.h>

SoftwareSerial mySerial(8, 9); // ATMega328P 跟 BC26 固定使用的溝通腳位

void(* resetFunc) (void) = 0; // 宣告系統重置參數
int reset_count = 0;          //  系統重新啟動計時器宣告
int waitingTime = 10000;      // 等候 10 秒的回覆.

String Check_RevData(int z)  // 讀取收到的每一字元資料 ，彙整成一個字串
{
 String data= "";  
 char c;
 long int StartTime=millis();  
 while (!mySerial.available())
 {
      Serial.print(".");
      delay(100);
      if ((StartTime+waitingTime) < millis() && z==0)
      {
        Serial.println("No response.");
        resetFunc();        
        break;
      }
  }
  Serial.println(); 
   while (mySerial.available())
  {
    delay(100);
    c = mySerial.read(); //Conduct a serial read  
    if (c=='\n' || c=='\r') continue;
    data+=c; //Shorthand for data = data + c
  }
  return data;  
}

bool Send_ATcommand(String msg,byte stepnum) // 傳送 AT command ，並加以判斷
{
 String Showmsg=""; 
 mySerial.println(msg);
 Showmsg=Check_RevData(0);
 //Serial.println(Showmsg);
  switch (stepnum)
  {
    case 0:         // Reset BC26
    case 1:         // Close show message 
         break;
    case 2:         // Check IPAddress 
         if (!Showmsg.startsWith("+CGPADDR:")) return false;
         break;
    case 3:         // Connect to Server
         if (Showmsg.startsWith("OK+QIOPEN: 0,0")) return true;
         if (Showmsg.startsWith("OK"))
         {
          Showmsg=Check_RevData(0);
          if (!Showmsg.startsWith("+QIOPEN: 0,0")) return false;
         }  
         break;      
    case 4:         // Other Data
         if (!Showmsg.startsWith("OK")){ 
         Serial.println("關閉通道失敗");
         return false;
         }
         break; 

    case 10:  // build MQTT Server
         if (Showmsg.startsWith("OK+QMTOPEN: 0,0")){
          Serial.println("通道建立成功");
          return true;
         }
         if (Showmsg.startsWith("OK"))
         {
          Showmsg=Check_RevData(0);
          if (!Showmsg.startsWith("+QMTOPEN: 0,0")) return false;
         }
         break;  
    case 11: // Connect to MQTT server by username and password 
         if (Showmsg.startsWith("OK+QMTCONN: 0,0,0")){
          Serial.println("感測端連接伺服器成功");
          return true;
         }
         if (Showmsg.startsWith("OK"))
         {
          Showmsg=Check_RevData(0);
          if (!Showmsg.startsWith("+QMTCONN: 0,0,0")) return false;
         }
         break;
    case 12: // Publisher MQTT Data       
         if (!Showmsg.startsWith("OK+QMTPUB: 0,0,0")){ 
         Serial.println("成功上傳資料");
         return false;
         }
         break; 
  }
 return true;
}

bool BC26init()  // 初始化 BC26
{
 Send_ATcommand("AT+QRST=1",0);
 Send_ATcommand("ATE0",1);
 if (!Send_ATcommand("AT+CGPADDR=1",2)) return false;
 if (!Send_ATcommand("AT+IPR=9600",4)) return false;
 return true;
}

bool Connect_Server(String S_name) // 建立 HTTP 連線通道
{
 String S_temp="";
 S_temp="AT+QIOPEN=1,0,\"TCP\",\"" + S_name + "\",80,0,0";
 Serial.println(S_temp);
 if (!Send_ATcommand(S_temp,3)) return false;
 //Send_ATcommand("AT+QICFG=\"viewmode\",1",4);
 delay(100);
 return true;  
}

bool Send_Data_IFTTT(String SHost,String SKey,String SEvent,String v1) // 傳送資料給 IFTTT 與 關閉連線
{
 String Send_check=""; 
 String S_tempdata=""; 
 String S_temphost="";
 S_tempdata="POST /trigger/" + SEvent +"/with/key/"+SKey+"?value1="+v1+" HTTP/1.1";
 S_temphost="HOST: " + SHost ;
 Serial.println("AT+QISEND=0");
 mySerial.println("AT+QISEND=0");
 delay(100);
  mySerial.println(S_tempdata);
 mySerial.println(S_temphost);
 mySerial.println();
 mySerial.print("\x1A");
 while (!Send_check.startsWith(">OKSEND OK+QIURC: \"recv\",0"))
 {
  Serial.println("Wait ...");  
  Send_check=Check_RevData(0);
 }
 delay (100);
 Send_ATcommand("AT+QICLOSE=0",4);
 return true; 
}

bool Send_Data_IdeasChain(String token,String key , String value) // HTTP 方式傳送資料給網站
{
 String Send_check=""; 
 String D_message="";
 D_message="{\"" + key + "\":" + "\""+ value + "\"" + "}"; 
 Serial.println("AT+QISEND=0");   
 mySerial.println("AT+QISEND=0"); 
 Send_check=Check_RevData(0);
  while (!Send_check.startsWith(">"))
 {
  Serial.println("Wait ...");
  Send_check=Check_RevData(0);
 }   
 //Serial.println("POST /api/v1/" + token + "/telemetry HTTP/1.1");
 mySerial.println("POST /api/v1/" + token + "/telemetry HTTP/1.1");
 //Serial.println("Host: iiot.ideaschain.com.tw");
 mySerial.println("Host: iiot.ideaschain.com.tw");
 //Serial.println("Content-Type: application/json");
 mySerial.println("Content-Type: application/json");
 //Serial.println("Content-Length:" + String(D_message.length()));
 mySerial.println("Content-Length:" + String(D_message.length()));
 mySerial.println();
 //Serial.println(D_message); 
 mySerial.print(D_message); //JSON Data
 mySerial.print("\x1A");
 
 Send_check=Check_RevData(0); 
 while (!Send_check.startsWith("OKSEND OK+QIURC: \"recv\",0"))
 {
  Serial.println("Wait ...");
  Send_check=Check_RevData(0);
 }
 Serial.println(Send_check); 
 Send_ATcommand("AT+QIRD=0,512",1);
 delay (100);
 Send_ATcommand("AT+QICLOSE=0",4);
 //Serial.println("Send data ok");
 return true; 
}


bool connect_MQTT(String IP, String port,String user,String pass) // 建立 MQTT 連線通道
{
 String S_temp="";
 S_temp="AT+QMTOPEN=0," + IP + "," + port;
 //Serial.println(S_temp);
 
 Serial.println("built NET");
 if (!Send_ATcommand(S_temp,10)) return false;
 //delay (100);
 S_temp="";
 S_temp="AT+QMTCONN=0," + user + "," + pass;
 //Serial.println(S_temp);
 
 Serial.println("connect NET");
 if (!Send_ATcommand(S_temp,11)) return false;
 //delay(100);
 return true;
}

bool Publish_MQTT(String topic, String message) // 發佈資料
{ 
 String S_temp="";
 S_temp="AT+QMTPUB=0,0,0,0," + topic + "," + message;
 //Serial.println(S_temp);
 Serial.println("data publish");
 if (!Send_ATcommand(S_temp,12)) return false; 
 delay(100);
 return true;
}

bool Sub_MQTT(String topic) // 訂閱資料
{
 String S_temp="";
 String Send_check=""; 
 S_temp="AT+QMTSUB=0,1," + topic + "," + "2";  // Qos 2 
 mySerial.println(S_temp);
 delay(100); 
 return true; 
}

bool Close_MQTT() // 關閉連線
{ 
 String S_temp="";
 S_temp="AT+QMTCLOSE=0";
 if (!Send_ATcommand(S_temp,4)) return false; 
 delay(100);
 return true;
}
