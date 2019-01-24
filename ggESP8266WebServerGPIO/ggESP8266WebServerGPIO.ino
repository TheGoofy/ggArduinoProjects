
#include <ESP8266WiFi.h>


const char* ssid = "Gardunkel";
const char* password = "goofy123";


// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);


/*
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
*/

void setup()
{
  // configure GPIO pins as output
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // connect
  WiFi.begin(ssid, password);

  // wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  // Start the server
  server.begin();
}


void loop()
{  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  while (!client.available()) {
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
  else {
    // unknown request
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
  for (int i=0; i<4; i++) {
    String s = "";
    String vBgColor = digitalRead(i) ? "'orange'" : "'skyblue'";
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
  client.print("</table>\r\n");
  client.print("</form>\r\n");
  client.print("</html>\n");
}

