#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define APPLEMIDI_INITIATOR
#include <AppleMidi.h>
USING_NAMESPACE_APPLEMIDI

#include <NeoPixelBus.h>


char ssid[] = "WIFI_SSID"; //  your network SSID (name)
char pass[] = "WIFI_PASSWORD";    // your network password (use for WPA, or use as key for WEP)

int show_debug = 1;

/* Avoid these pins:
ESP32 has 6 strapping pins:
 MTDI/GPIO12: internal pull-down
 GPIO0: internal pull-up
 GPIO2: internal pull-down
 GPIO4: internal pull-down
 MTDO/GPIO15: internal pull-up
 GPIO5: internal pull-up
*/


// This pin drives a 2N2222A transistor (1k base resitor, 100Ohm Collector resitor)
//#define TRANSISTOR_PIN 23
//int transistorstate = 0;

// NEOPIXEL SETUP
#define PIN            22

#define NUMPIXELS      19

//C1
#define LOWEST_NOTE   24


#include <ETH.h>
#include <ESPmDNS.h>

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

bool ETH_startup()
{
  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  Serial.println(F("Getting IP address..."));

  while (!eth_connected)
    delay(100);

  return true;
}

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> pixels (NUMPIXELS, PIN);

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "ESP32", DEFAULT_CONTROL_PORT);


void setAllLeds(int v)
{
  for(int i=0; i<NUMPIXELS;i++)
  {
    pixels.SetPixelColor(i, RgbColor(v,v,v));

  }
  pixels.Show();
}



// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
  isConnected = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  Serial.println(F("Disconnected"));
}

void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
  Serial.print  (F("Exception "));
  Serial.print  (err);
  Serial.print  (F(" from ssrc 0x"));
  Serial.println(ssrc, HEX);

  switch (err)
  {
    case Exception::NoResponseFromConnectionRequestException:
      Serial.println(F("xxx:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
      break;
  }
}




void OnAppleMidiControlChange(byte channel, byte number, byte value){
  /*
  Serial.print(F("Incoming ControlChange from channel:"));
  Serial.print(channel);
  Serial.print(F(" number:"));
  Serial.print(number);
  Serial.print(F(" value:"));
  Serial.print(value);
  Serial.println();
  */

  if(number == 31)
  {
    setAllLeds(value);
  }
  else if(number == 1)
  {
    show_debug = (value>0);

  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  /*
  Serial.print(F("Incoming NoteOn from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
*/


    int i = note - LOWEST_NOTE;
    if(i<0)
    {
      i=0;
    }

    if(show_debug)
    {
      Serial.println(i);
    }

    if(i<NUMPIXELS)
    {
      pixels.SetPixelColor(i, RgbColor(100,100,100));
      pixels.Show();
    }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  /*
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
*/

  int i = note - LOWEST_NOTE;
  if(i<0)
  {
    i=0;
  }

  if(i<NUMPIXELS)
    {
      pixels.SetPixelColor(i, RgbColor(0,0,0));
      pixels.Show();
    }
}


void OnAppleMidiByte(const ssrc_t & ssrc, byte data) {
  Serial.print(F("raw MIDI: "));
  Serial.println(data);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //pinMode(TRANSISTOR_PIN, OUTPUT);


  Serial.print(F("Getting IP address..."));


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));


  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

    // SETUP NeoPixel
  pixels.Begin();
  delay(10);
  pixels.Show();


  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named ESP32 with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));


  MDNS.begin(AppleMIDI.getName());

  MIDI.begin(1); // listen on channel 1

  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleError(OnAppleMidiError);


  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
  MDNS.addService("http", "tcp", 80);

  MIDI.setHandleNoteOn(OnAppleMidiNoteOn);
  MIDI.setHandleNoteOff(OnAppleMidiNoteOff);

  AppleMIDI.setHandleReceivedMidi(OnAppleMidiByte);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
 MIDI.read();

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  //if (isConnected && (millis() - t0) > 1000)
  //{
    //t0 = millis();
    //digitalWrite(TRANSISTOR_PIN, transistorstate);
    //transistorstate = !transistorstate;
    //   Serial.print(".");

    //byte note = 45;
    //byte velocity = 55;
    //byte channel = 1;

    //AppleMIDI.sendNoteOn(note, velocity, channel);
    //AppleMIDI.sendNoteOff(note, velocity, channel);
  //}
}
