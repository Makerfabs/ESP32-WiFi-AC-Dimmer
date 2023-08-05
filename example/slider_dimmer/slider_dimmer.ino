#include <arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

// s3
// #define SCR_Pin 9
// #define RELAY_PIN 20
// #define LED_PIN 19
// #define ZCD_PIN 10

// ESP32
#define SCR_Pin 25
#define RELAY_PIN 26
#define LED_PIN 32
#define ZCD_PIN 33

#define AC_CTRL_OFF digitalWrite(SCR_Pin, LOW)
#define AC_CTRL_ON digitalWrite(SCR_Pin, HIGH)

#define RELAY_OFF digitalWrite(RELAY_PIN, LOW)
#define RELAY_ON digitalWrite(RELAY_PIN, HIGH)

#define LED_OFF digitalWrite(LED_PIN, HIGH)
#define LED_ON digitalWrite(LED_PIN, LOW)

unsigned char dim = 0;

WiFiServer server(80);

const char mainPage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP WiFi Dimmer</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  <p><span id="textSliderValue">%SLIDERVALUE%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="100" value="%SLIDERVALUE%" step="1" class="slider"></p>
<script>
function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/update?value="+sliderValue, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(SCR_Pin, OUTPUT);
    pinMode(ZCD_PIN, INPUT);
    LED_OFF;
    RELAY_OFF;
    AC_CTRL_OFF;

    Serial.begin(115200); // initialize the serial communication:
    delay(1000);

    RELAY_ON;
    AC_CTRL_ON;
    delay(2000);
    RELAY_OFF;
    AC_CTRL_OFF;

    server_setup();

    attachInterrupt(ZCD_PIN, zero_cross_int, RISING); // CHANGE FALLING RISING

    Serial.println("Test begin");
    set_power(5);
}

void loop()
{
    dimmer_server();

    // for (i = 1; i < 10; i++)
    // {
    //     set_power(i);
    //     delay(500);
    // }
    // set_power(0);
    // delay(1000);
}

void zero_cross_int() // function to be fired at the zero crossing to dim the light
{
    if (dim < 5)
        return;
    if (dim > 90)
        return;

    int dimtime = (100 * dim);
    delayMicroseconds(dimtime); // Off cycle
    AC_CTRL_ON;                 // triac firing
    delayMicroseconds(500);     // triac On propagation delay
    AC_CTRL_OFF;                // triac Off
}

void set_power(int level)
{
    dim = map(level, 0, 100, 95, 5);
    if (level == 0)
    {
        RELAY_OFF;
    }
    else
        RELAY_ON;
}

void server_setup()
{
    WiFi.disconnect();

    WiFi.begin("Makerfabs", "20160704");

    int connect_count = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        Serial.print(".");
        connect_count++;
        if (connect_count > 20)
        {
            Serial.println("Wifi error");
            break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    server.begin();
}

void dimmer_server()
{

    WiFiClient client = server.available(); // listen for incoming clients

    if (client) // if you get a client,
    {
        Serial.println("---------------------------------------------------");
        Serial.println("New Client.");
        String currentLine = "";
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {
                char c = client.read();
                Serial.write(c);

                // PAGE:192.168.4.1
                if (c == '\n')
                { // if the byte is a newline character

                    if (currentLine.length() == 0)
                    {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        client.println(mainPage);
                        client.stop();

                        return;
                    }
                    else
                    {
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {
                    currentLine += c;
                }

                // API:保存设置
                if (currentLine.endsWith("GET /update"))
                {
                    String get_request = "";
                    // read GET next line
                    while (1)
                    {
                        char c_get = client.read();
                        Serial.write(c_get);
                        if (c_get == '\n')
                        {
                            break;
                        }
                        else
                        {
                            get_request += c_get;
                        }
                    }

                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();
                    // client.println(mainPage);
                    client.println("Update Over");
                    client.println();
                    client.stop();

                    Serial.println(get_request);
                    req_explain(get_request);

                    return;
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
    return;
}

void req_explain(String str)
{
    char temp[50];

    str.replace("&", " ");
    sprintf(temp, "%s", str.c_str());
    Serial.println(temp);

    int var_1 = 0;

    sscanf(temp, "?value=%d HTTP", &var_1);
    Serial.println(var_1);

    set_power(var_1);
}