/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Gardunkel";
const char* password = "goofy123";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void InitOutput(int aPin)
{
  pinMode(aPin, OUTPUT);
  // digitalWrite(aPin, LOW);
}

void StatusPrint(const String& aString)
{
  lcd.print(aString);
  Serial.print(aString);
}

void StatusPrintLn(const String& aString)
{
  StatusPrint(aString);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  Serial.println();
}

String DecodeURL(const String& aString)
{
  String vString = aString;

  const int vLength = vString.length();
  int vIndexSrc = 0;
  int vIndexDst = 0;

  while (vIndexSrc < vLength) {
    char vChar = vString[vIndexSrc];
    if (vChar == '+') vChar = ' ';
    else if (vChar == '%') {
      char vH = vString[++vIndexSrc];
      char vL = vString[++vIndexSrc];
      if (vH > 0x39) vH -= 7;
      if (vL > 0x39) vL -= 7;
      vChar = (vH << 4) | (vL & 0x0f);
    }
    vString[vIndexDst] = vChar;
    ++vIndexSrc;
    ++vIndexDst;
  }

  vString.remove(vIndexDst);

  return vString;
}


void setup() {

  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();

  Serial.begin(115200);
  delay(10);

  // prepare GPIO
  InitOutput(0); // OK
  // InitOutput(1); crash
  InitOutput(2); // OK
  InitOutput(3); // OK
  InitOutput(4); // OK
  InitOutput(5); // OK
  // InitOutput(6); // wdt reset
  // InitOutput(7); // wdt reset
  // InitOutput(8); // wdt reset
  // InitOutput(9); // wdt reset
  InitOutput(10); // OK on ESP-12E (wdt reset on ESP-12)
  // InitOutput(11); // wdt reset
  InitOutput(12); // OK
  InitOutput(13); // OK
  InitOutput(14); // OK
  InitOutput(15); // OK
  InitOutput(16); // OK
  // InitOutput(17); // wdt reset
  // InitOutput(18); // wdt reset
  // InitOutput(19); // wdt reset
  
  // Connect to WiFi network
  StatusPrintLn("");
  StatusPrintLn("");
  StatusPrint(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    StatusPrint(".");
  }
  StatusPrintLn("");
  // Print the IP address
  StatusPrintLn(WiFi.localIP().toString());
  
  // Start the server
  server.begin();
  StatusPrintLn("Server started.");

  StatusPrintLn(WiFi.localIP().toString());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int pin = -1;
  if (req.indexOf("pin=00") != -1) pin = 0;
  else if (req.indexOf("pin=01") != -1) pin = 1;
  else if (req.indexOf("pin=02") != -1) pin = 2;
  else if (req.indexOf("pin=03") != -1) pin = 3;
  else if (req.indexOf("pin=04") != -1) pin = 4;
  else if (req.indexOf("pin=05") != -1) pin = 5;
  else if (req.indexOf("pin=06") != -1) pin = 6;
  else if (req.indexOf("pin=07") != -1) pin = 7;
  else if (req.indexOf("pin=08") != -1) pin = 8;
  else if (req.indexOf("pin=09") != -1) pin = 9;
  else if (req.indexOf("pin=10") != -1) pin = 10;
  else if (req.indexOf("pin=11") != -1) pin = 11;
  else if (req.indexOf("pin=12") != -1) pin = 12;
  else if (req.indexOf("pin=13") != -1) pin = 13;
  else if (req.indexOf("pin=14") != -1) pin = 14;
  else if (req.indexOf("pin=15") != -1) pin = 15;
  else if (req.indexOf("pin=16") != -1) pin = 16;
  else if (req.indexOf("pin=17") != -1) pin = 17;
  else if (req.indexOf("pin=18") != -1) pin = 18;
  else if (req.indexOf("pin=19") != -1) pin = 19;
  else if (req.indexOf("message=") != -1) {
    int vIndexA = req.indexOf("message=");
    int vIndexB = req.indexOf(" ", vIndexA);
    String vMessage = DecodeURL(req.substring(vIndexA + 8, vIndexB));
    Serial.print("vMessage='");
    Serial.print(vMessage);
    Serial.println("'");
    lcd.setCursor(0,1);
    lcd.print(vMessage);
    lcd.print("                ");
  }
  else {
    Serial.print("unknown request:");
    Serial.println(req);
  }

  // Set GPIO according to the request
  if (pin != -1) {
    digitalWrite(pin, digitalRead(pin) ? LOW: HIGH);
  }
  
  client.flush();

  // respond
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n");
  client.print("<form action='gpio' method='get'>\r\n");
  client.print("<table border='0' bgcolor='white' cellpadding='5' width='100%'>\r\n");
  for (int i=0; i<20; i++) {
    if (i==0 || i==2 || i==3 || i==4 || i==5 || i==10 || i==12 || i==13 || i==14 || i==15 || i==16) {
      String s = "";
      String vBgColor = digitalRead(i) ? "'orange'" : "'lightgray'";
      s += "  <tr bgcolor=" + vBgColor + ">";
      s += "<td align='center' valign='middle'><button name='pin' type='submit' value='";
      s += i<10 ? "0" : "";
      s += i;
      s += "'>GPIO ";
      s += i;
      s += "</button></td>";
      s += "</tr>\r\n";
      client.print(s);
    }
    else {
      client.print("  <tr bgcolor='lightgray'><td align='center' valign='middle'><font face='arial' color='gray' size='-1'>GPIO " + (String)i + "</font></td></tr>\r\n");
    }
  }
  client.print("</table>\r\n");
  client.print("</form>\r\n");
  client.print("<form action='message' method='get'>\r\n");
  client.print("  Message: <input type='text' name='message'><input type='submit' value='Send'>\r\n");
  client.print("</form>\r\n");
  client.print("</html>\n");

  // Send the response to the client
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

