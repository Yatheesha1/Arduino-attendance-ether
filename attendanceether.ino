
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };//MAC address

IPAddress ip(192,168,137,20);//
EthernetServer server(80);
//byte serverName[] = { 192, 168, 1, 5 };

int attendance[50];
uint8_t id=0,idtemp=0;;

String st="";

LiquidCrystal lcd(9, 8, 7, 6, 5, 4); /// REGISTER SELECT PIN,ENABLE PIN,D4 PIN,D5 PIN, D6 PIN, D7 PIN
// pin #2 is IN from sensor (YELLOW wire)
// pin #3 is OUT from arduino  (BLUE wire)
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()  
{  
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcdprint("  ATTENDANCE",0,1,0,0);//print name
  lcdprint("    SYSTEM",1,1,0,1);//print name
  delay(3000);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  if (finger.verifyPassword()) 
  {
    Serial.println("Found fingerprint sensor!");
  } 
  else 
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }
  Ethernet.begin(mac, ip);
  server.begin();
}


void loop()                     // run over and over again
{
  id=0;
  while(getFingerprintIDez());
  while(loadid());
  getFingerprintEnroll();
}

int loadid()
{
    uint8_t p = finger.loadModel(id);
    if(p!=12)
    {
      id++;
      return 1;
    }
    return 0; 
}


int getFingerprintIDez() 
{
  uint8_t p=-1;
  Serial.println();
  Serial.println("Waiting for Finger");
  lcdprint(" PLACE FINGER!",1,1,1,0);
  while(p!=0)
  {
     p = finger.getImage();
  }
     
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return 0;
  
  // found a match!
  id=finger.fingerID;
  

  delay(500);
  lcdprint(" REMOVE FINGER",1,1,1,0);
  Serial.println("Remove finger");
  delay(500);
  
   p = 0;
  while (p != FINGERPRINT_NOFINGER) 
  {
    p = finger.getImage();
  }
  p = -1;
  Serial.print("Found ID #"); 
  Serial.println(id); 
  lcdprint("ID :"+String(id),0,2,1,0);
  attendance[id]++;
  Serial.print("Attendance:");
  lcdprint("Attendence:"+String(attendance[id]),1,2,1,1);
  Serial.println(attendance[id]);
  delay(1000);
  ether();
  return 1; 
}

uint8_t getFingerprintEnroll() 
{
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); 
  lcdprint("ENROLLING..",1,1,1,0);
  Serial.println(id);
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) 
    {
    case FINGERPRINT_OK:
      Serial.println();
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      Serial.println("Error while taking image");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) 
  {
     case FINGERPRINT_OK:
          Serial.println("Image converted");
          break;
     default:     
         Serial.println("Error while converting image");
         return p;
  }
  
  Serial.println("Remove finger");
  delay(500);
  lcdprint(" REMOVE FINGER",1,1,1,0);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) 
  {
    p = finger.getImage();
  }
  Serial.print("ID "); 
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcdprint("PLACE SAME",0,2,1,0);
  lcdprint("FINGER",1,2,1,1);
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) 
    {
    case FINGERPRINT_OK:
      Serial.println();
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    default:
      Serial.println("Error while taking image");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    default:
      Serial.println("Error while converting image");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  
  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Prints matched!");
  } 
  else if (p == FINGERPRINT_ENROLLMISMATCH) 
  {
    Serial.println("Fingerprints did not match");
    return p;
  } 
  else 
  {
    Serial.println("Error while matching finger");
    return p;
  }   
  
  Serial.print("ID "); 
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Stored!");
    lcdprint("     SUCCESS!",0,1,1,0);
    lcdprint("   YOUR ID:"+String(id),1,1,1,1);
    delay(1000);
  } 
  else 
  {
    Serial.println("Error while loading data");
    return p;
  }   
}

void ether()
{
  EthernetClient client;
  String readString;
  client = server.available();
  if (client) 
  {
    while (client.connected()) 
    { 
      if (client.available()) 
      {
         char c = client.read(); 
         //read char by char HTTP request
         if (readString.length() < 100) 
         {
          //store characters to string
          readString += c;
         }   
         //if HTTP request has ended
         if (c == '\n') 
         {              
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           client.println();  
              
           client.println("<HTML>");
           
           client.println("<HEAD>");
           client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
           client.println("<link rel='stylesheet' type='text/css' href='http://randomnerdtutorials.com/ethernetcss.css' />");
           client.println("<TITLE>Arduino Project</TITLE>");
           client.println("</HEAD>");
           
           client.println("<BODY>");  
                  
           client.println("<br/>");  
           client.println("<H2>Finger print based Attendance System</H2>"); 
           client.println("ID:");
           client.println(id);
           client.println("<br/>");
           client.print("Attendance:");
           client.println(attendance[id]);
           client.println("<br/>");
           
           client.println("</BODY>");
           client.println("<head>");
           client.println("<meta http-equiv='refresh' content='1'>");
           client.println("</head>");
           
           client.println("</HTML>");
     
           //delay(1);
           //stopping client
           client.stop();      

         }  
      } 
    }     
    readString="";          
  }
}

void lcdprint(String st,int row,int column,int scroll,int clr)
{
  if(clr==0)
    lcd.clear();
  lcd.setCursor(column,row); // set the cursor to column 0, line 2
  lcd.print(st);//print name
  if(scroll==1)
    lcd.scrollDisplayLeft();//shifting data on LCD 
}
