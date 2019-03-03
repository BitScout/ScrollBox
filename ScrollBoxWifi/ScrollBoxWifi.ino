/*
  ScrollBox
  by Christian Kollross (BitScout)

  Based on the example sketch "WiFiAccessPoint.ino" by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define DEBUG 1 // 0 = Default debug level, 1 = full debugging

const char *ssid     = "ScrollBox";
const char *password = "scrollbox"; // At least 8 characters

WiFiServer server(80);

struct formField {
   String key   = "";
   String value = "";
};

void setup() {
  Serial2.begin(9600);
  
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println();
    Serial.print("Debug level: ");
    Serial.println(DEBUG);
    Serial.println("Configuring access point...");
  #endif

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  server.begin();

  #ifdef DEBUG
    Serial.print("IP address: ");
    Serial.println(myIP);
    Serial.println("Server started");
  #endif
}

void loop() {
  WiFiClient client = server.available(); // listen for incoming clients

  if (client) {
    #ifdef DEBUG
      Serial.println("\nNew connection");
    #endif
    
    processConnection(client);
    Serial.println("\tTerminating connection");
    client.stop();
  }
}

void processConnection(WiFiClient client) {
  char   c; // Incoming character
  String line = "";
  long   contentLength = 0;
  bool   isPost = false;
  bool   postBodyStarted = false;
  
  while (client.connected()) {
    if (!client.available()) {
      continue;
    }
    
    c = client.read();
    
    #ifdef DEBUG
      if(DEBUG > 1) Serial.write(c);
    #endif

    if(isPost && postBodyStarted) {
      line += c;
      contentLength--;

      if(contentLength <= 0) {
        processBody(client, line);
      }
    } else if (c == '\n') {
      if(line.indexOf("GET / HTTP") == 0) {
        printFormResponse(client);
        
        return;
      } else if(line.indexOf("POST / HTTP") == 0) {
        isPost = true;
      } else if(line.indexOf("GET /favicon.ico HTTP") == 0) {
        printHeader(client, 404, "Not Found");
        
        return;
      } else if(line == "") {
        if(!postBodyStarted) {
          postBodyStarted = true;
        }
      } else if(line.indexOf("Content-Length: ") == 0) {
        contentLength = line.substring(line.indexOf(' ') + 1).toInt();
      }

      line = "";
    } else if (c != '\r') {
      line += c;
    }
  }
}

void processBody(WiFiClient client, String& body) {
  #ifdef DEBUG
    Serial.print("Processing query: ");
    Serial.println(body);
  #endif

  String color;
  String text;
  struct formField field;
  int i = 0;

  do{
    field = getUrlField(body, i);
    i++;

    if(field.key == "cl") {
        color = field.value;
    } else if(field.key == "txt") {
        text = field.value;
    }
  
  } while(field.key != "");

  String messageString = color + text;
  
  char message[messageString.length() + 1];
  messageString.toCharArray(message, messageString.length() + 1);

  #ifdef DEBUG
    Serial.print("Sending message to Arduino Nano: ");
    Serial.println(message);
  #endif
  
  Serial2.write(message);

  printFormResponse(client);
}

void printFormResponse(WiFiClient client) {
  printHeader(client, 200, "OK");
  printFormHtml(client);
  client.println();
}

void printFormHtml(WiFiClient client) {
  // the content of the HTTP response follows the header:
  client.print("<html><body style=\"font-size: 3em;\">");
  client.print("<br><center><form method=\"post\">");
  printColorPicker(client, "cl");
  client.print("<input type=\"text\" name=\"txt\" maxlength=\"30\">");
  client.print("<input type=\"submit\" value=\"Set\">");
  client.print("</form></center>");
  client.print("</body></html>");
}

void printColorPicker(WiFiClient client, char field[]) {
  client.print(" <input type=\"color\" name=\"");
  client.print(field);
  client.print("\" value=\"#ffffff\">");
}

void printHeader(WiFiClient client, int code, String message) {
  client.print("HTTP/1.1 ");
  client.print(code);
  client.println(' ');
  client.print(message);
  client.println("Content-type:text/html");
  client.println();
}

// Based on: https://stackoverflow.com/questions/29671455/how-to-split-a-string-using-a-specific-delimiter-in-arduino
struct formField getUrlField(String data, int index)
{
  char separator = '&';
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  struct formField result;

  if(found<=index) {
    return result;
  }

  String field = data.substring(strIndex[0], strIndex[1]);
  int equalsPosition = field.indexOf("=");

  result.key   = urldecode(field.substring(0, equalsPosition));
  result.value = urldecode(field.substring(equalsPosition + 1));

  return result;
}

