/**
 * arpy.ino -- Play with arps
 * 
 *  
 *  Tested on ItsyBitsy M4 and NeoTrellis M4
 *  
 *  Controls:
 *   - knobA on A2 controls root note
 *   - knobB on A3 controls BPM
 *   - knobC on A4 controls filter frequency
 *   - buttonA on D9 controls arp pattern switching
 *   - buttonB on D7 controls arp octave count
 *  
 *  Outputs:
 *   - Audio on A0 & A1
 *   - MIDI USB of notes played
 * 
 *  Libraries:
 *  - Uses Adafruit fork of Teensy Audio Library
 *  - Uses Adafruit_TinyUSB library and TinyUSB USB stack ("Tools" -> "USB Stack")
 *  
 *  12 Jan 2022 - @todbot Tod Kurt
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
const int knobCPin = 4;  // A4
const int butBPin  = 7;  // 
const int butAPin  = 9;  // 

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDIusb); // USB MIDI

#define NUM_VOICES 4

AudioSynthWaveform *waves[] = {
  &wave0, &wave1, &wave2, &wave3
};

int filterf_max = 6000;
int filterf = filterf_max;

uint32_t lastControlMillis=0;

uint8_t arp_octaves = 1;
uint8_t root_note;

Arpy arp = Arpy();
Bounce butA = Bounce();
Bounce butB = Bounce();

void setup() {
  MIDIusb.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  MIDIusb.turnThruOff();
  delay(1000); // it's hard getting started in the morning

  pinMode( knobAPin, INPUT);
  pinMode( knobBPin, INPUT);
  pinMode( knobCPin, INPUT);
  butA.attach( butAPin, INPUT_PULLUP);
  butB.attach( butBPin, INPUT_PULLUP);
  
  arp.setNoteOnHandler(noteOn);
  arp.setNoteOffHandler(noteOff);
  arp.setRootNote( 60 );
  arp.setBPM( 100 );
  arp.setGateTime( 0.75 ); // percentage of bpm
  arp.on();
  
  AudioMemory(120);

  filter0.frequency(filterf_max);
  filter0.resonance(0.5);
  
  env0.attack(10);
  env0.hold(2);
  env0.decay(100);
  env0.sustain(0.5);
  env0.release(100);

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();

  Serial.println("arpy test setup done");
}

int waveform = WAVEFORM_SAWTOOTH;

void noteOn(uint8_t note) {
  Serial.printf("noteOn: note:%d\n", note);
  digitalWrite(LED_BUILTIN, HIGH);
  waves[0]->begin( 0.9, tune_frequencies2_PGM[note], waveform);
  waves[1]->begin( 0.9, tune_frequencies2_PGM[note] * 1.01, waveform); // detune
  waves[2]->begin( 0.9, tune_frequencies2_PGM[note] * 1.005, waveform); // detune
  waves[3]->begin( 0.9, tune_frequencies2_PGM[note] * 1.015, waveform); // detune
  filterf = filterf_max;
  filter0.frequency(filterf);
  env0.noteOn();
}

void noteOff(uint8_t note) {
  //Serial.printf("noteOff: note:%d\n", note);
  digitalWrite(LED_BUILTIN, HIGH);
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
    arp_octaves = arp_octaves + 1; if( arp_octaves==4) { arp_octaves=1; }
    arp.setTransposeSteps( arp_octaves );
    Serial.printf("arp steps:%d\n",arp_octaves);
  }
  
  if( millis() - lastControlMillis > 20 ) { 
    lastControlMillis = millis();
    
    int knobA = analogRead(knobAPin);
    int knobB = analogRead(knobBPin);
    int knobC = analogRead(knobCPin);
    
    root_note = map( knobA, 0,1023, 36, 72);
    int bpm = map( knobB, 0,1023, 100, 3000 );
    filterf_max = map( knobC, 0,1023, 30, 8000);   
    //float note_duration =  (float)knobC / 1023;

    filter0.frequency(filterf_max);
    
    arp.setRootNote( root_note );
    arp.setBPM( bpm );
    
    //Serial.printf("knobs:%d %d %d   \n", knobA,knobB,knobC);
    
//     Filter "envelope"
//     simple ramp down LFO on frequency
//    filter0.frequency(filterf);
//    if( filterf>30) { filterf = filterf * 0.80; };

  }
  
}
