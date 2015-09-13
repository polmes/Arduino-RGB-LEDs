#include <DmxMaster.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

#define N 24

// Ethernet controller
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xCD, 0xB1 };
IPAddress ip(192, 168, 1, 52);
EthernetServer server(80);

// GET variables
char c;
String url;
File web;

void setup() {
  url.reserve(N);
  
  Ethernet.begin(mac, ip);
  delay(1000);
  server.begin();
  
  SD.begin(4);
}

void loop() {
  EthernetClient client = server.available();
  if (client) { // neat!
    boolean newLine = true;
    while (client.connected()) {
      if (client.available()) {
        c = client.read(); // will read char-by-char to check for the newline ending
        if (url.length() < N) url += c;
        
        if (c == '\n' && newLine) { // the HTTP request has ended, send a reply
          // GET
          if (url.indexOf('?') >= 0) {
            DmxMaster.write(1, url.substring(8, url.indexOf('&', 9)).toInt()); // R
            DmxMaster.write(2, url.substring(url.indexOf('g', 10) + 2, url.indexOf('&', 13)).toInt()); // G
            DmxMaster.write(3, url.substring(url.indexOf('b', 14) + 2, url.indexOf(' ', 17)).toInt()); // B
          }
          
          // HTTP header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          
          // HTML from SD
          web = SD.open("index.htm");
          if (web) {
            while (web.available()) {
              client.write(web.read());
            }
            web.close();
          }
          
          break; // from while
        }
        
        if (c == '\n') {
          newLine = true;
        } else if (c != '\r') {
          newLine = false;
        }
      }
    }
    delay(1);
    url = "";
    client.stop();
  }
}

