#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <AppleMidi.h>
#include <NeoPixelBus.h>


char ssid[] = "SSID"; //  your network SSID (name)
char pass[] = "PASSWORD";    // your network password (use for WPA, or use as key for WEP)

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
#define PIN            25

#define NUMPIXELS      60

//C1
#define LOWEST_NOTE   24

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt800KbpsMethod> pixels (NUMPIXELS, PIN);

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

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

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected  = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected  = false;
  Serial.println(F("Disconnected"));
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
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  AppleMIDI.OnReceiveControlChange(OnAppleMidiControlChange);

  //Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();

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
