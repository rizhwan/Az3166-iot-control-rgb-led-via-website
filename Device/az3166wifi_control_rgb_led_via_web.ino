#include "AZ3166WiFi.h"
#include "RGB_LED.h"

static bool hasWifi = false;

WiFiServer server(80);

// Variable to store the HTTP request
String header;

// variables to store the current output state
String output4State = "off";

// Assign output variables 
const byte output4 = 4;

// Current time
unsigned long currentTime = millis();

// Previous time
unsigned long previousTime = 0;

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Utilities
RGB_LED rgbLed;

static void InitWifi()
{
  Screen.print(2, "Connecting...");

  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(1, ip.get_address());
    hasWifi = true;
    Screen.print(2, "Running... \r\n");
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

void setup()
{
  Screen.init();
  Screen.print(0, "Rizhwan Kit");
  Screen.print(2, "Initializing...");

  Screen.print(3, " > Serial");
  Serial.begin(115200);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWifi();
  if (!hasWifi)
  {
    return;
  }

  server.begin();
}

void loop()
{
  WiFiClient client = server.available(); // Listen for incoming clients
  if (client)
  {
    String currentLine = ""; // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /4/on") >= 0)
            {
              rgbLed.setColor(255, 100, 100);
              output4State = "on";
              digitalWrite(output4, HIGH);
            }
            else if (header.indexOf("GET /4/off") >= 0)
            {
              output4State = "off";
              rgbLed.turnOff();
              digitalWrite(output4, LOW);
            }
            else if (header.indexOf("GET /red") >= 0)
            {
              output4State = "on";
              rgbLed.setColor(255, 0, 0);
            }
            else if (header.indexOf("GET /green") >= 0)
            {
              output4State = "on";
              rgbLed.setColor(0, 255, 0);
            }
            else if (header.indexOf("GET /blue") >= 0)
            {
              output4State = "on";
              rgbLed.setColor(0, 0, 255);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Rizhwan Web Server</h1>");

            client.println("<p><a href=\"/red\"><button class=\"button\">Red</button></a></p>");
            client.println("<p><a href=\"/green\"><button class=\"button\">Green</button></a></p>");
            client.println("<p><a href=\"/blue\"><button class=\"button\">Blue</button></a></p>");

            // Display current state, and ON/OFF buttons
            client.println("<p>State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button
            if (output4State == "off")
            {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
}