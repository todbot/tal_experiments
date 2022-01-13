/**
 * arpy.ino -- 
 *  
 *  Tested on ItsyBitsy M4 and NeoTrellis M4
 *  
 *  Libraries:
 *  - Uses Adafruit fork of Teensy Audio Library
 *  - Uses Adafruit_TinyUSB library and TinyUSB USB stack ("Tools" -> "USB Stack")
 *  
 *  6 Jan 2022 - @todbot Tod Kurt
 */
 
#include <Audio.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <Bounce2.h>

#include "ADT.h"
#include "midi_to_freq.h"
#include "Arpy.h"

const int dacLPin  = 0;  // A0 // audio out assumed by TAL
const int dacRPin  = 1;  // A1 // audio out assumed by TAL
const int knobAPin = 2;  // A2
const int knobBPin = 3;  // A3
const int butBPin  = 7;  // 
const int butAPin  = 9;  // 

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDIusb); // USB MIDI

#define NUM_VOICES 4

AudioSynthWaveform *waves[] = {
  &wave0, &wave1, &wave2, &wave3
};

int filterf_max = 4000;
int filterf = 2000;
uint32_t lastControlMillis=0;
uint8_t arp_octaves = 1;

Arpy arp = Arpy();
Bounce butA = Bounce();
Bounce butB = Bounce();

void setup() {
  MIDIusb.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  MIDIusb.turnThruOff();
  delay(1000); // it's hard getting started i the morning
  
  pinMode( knobAPin, INPUT);
  pinMode( knobBPin, INPUT);
  butA.attach( butAPin, INPUT_PULLUP);
  butB.attach( butBPin, INPUT_PULLUP);
  
  arp.setNoteOnHandler(noteOn);
  arp.setNoteOffHandler(noteOff);
  arp.setRootNote( 60 );
  arp.setBPM( 100 );
  arp.on();
  
  AudioMemory(120);

  filter0.frequency(4000);
  
  env0.attack(20);
  env0.hold(200);
  env0.decay(200);
  env0.sustain(0.1);
  env0.release(200);

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  Serial.println("arpy test setup done");
}

void noteOn(uint8_t note) {
  Serial.printf("noteOn: note:%d\n", note);
  waves[0]->begin( 0.9, tune_frequencies2_PGM[note], WAVEFORM_SAWTOOTH);
  waves[1]->begin( 0.9, tune_frequencies2_PGM[note] * 1.01, WAVEFORM_SAWTOOTH); // detune
  waves[2]->begin( 0.9, tune_frequencies2_PGM[note] * 1.02, WAVEFORM_SAWTOOTH); // detune
  waves[3]->begin( 0.9, tune_frequencies2_PGM[note] * 1.03, WAVEFORM_SAWTOOTH); // detune
  filterf = filterf_max;
  filter0.frequency(filterf);
  env0.noteOn();
}

void noteOff(uint8_t note) {
  Serial.printf("noteOff: note:%d\n", note);
  env0.noteOff();
}

void loop() {

  if (MIDIusb.read()) {     // If we have received a message
    Serial.print("MIDIusb received");
    digitalWrite(LED_BUILTIN, HIGH);
    uint8_t msg_type = MIDIusb.getType();
    if( msg_type == midi::NoteOn ) { 
      uint8_t note = MIDIusb.getData1();
      arp.setRootNote( note ); // this is quickly overridden by below
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
 
  arp.update();
  butA.update();
  butB.update();
 
  if( butA.fell() ) { // pressed
    arp.nextArpId();
    Serial.printf("picking next arp: %d\n", arp.getArpId());
  }

  if( butB.fell() ) {
    arp_octaves = (arp_octaves+1) % 3 + 1;
    arp.setTransposeSteps( arp_octaves );
    Serial.printf("arp steps:%d\n",arp_octaves);
  }
  
  if( millis() - lastControlMillis > 20 ) { 
    lastControlMillis = millis();
    
    int knobA = analogRead(knobAPin);
    int knobB = analogRead(knobBPin);
    uint8_t root_note = map( knobA, 0,1023, 24, 84);
    int bpm = map( knobB, 0,1023, 100, 5000 );
    
    arp.setRootNote( root_note );
    arp.setBPM( bpm );
    
    //Serial.printf("knobA:%d knobB:%d \t root:%d bpm:%d\n", knobA, knobB, root_note, bpm);
    
    // Filter "envelope"
    // simple ramp down LFO on frequency
    filter0.frequency(filterf);
    if( filterf>30) { filterf = filterf * 0.98; };

  }
  
}
