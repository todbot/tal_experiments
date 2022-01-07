/**
 * ominous_foreboding.ino -- 
 *  - “It’s learning, acquiring new skills” “Can we stop it?” “I don’t know.”
 *  - The sound of things to come. 
 *  Tested on ItsyBitsy M4 and NeoTrellis M4
 *  
 *  6 Jan 2022 - @todbot Tod Kurt
 */
 
#include <Audio.h>

// from Audio/examples/Synthesis/PlaySynthMusic
// This is for the Teensy Audio library which specifies
// frequencies as floating point. See make_notetab.xlsx
const float tune_frequencies2_PGM[128] =
{
    8.1758,    8.6620,    9.1770,    9.7227,    10.3009,    10.9134,    11.5623,    12.2499,
    12.9783,   13.7500,   14.5676,   15.4339,   16.3516,    17.3239,    18.3540,    19.4454,
    20.6017,   21.8268,   23.1247,   24.4997,   25.9565,    27.5000,    29.1352,    30.8677,
    32.7032,   34.6478,   36.7081,   38.8909,   41.2034,    43.6535,    46.2493,    48.9994,
    51.9131,   55.0000,   58.2705,   61.7354,   65.4064,    69.2957,    73.4162,    77.7817,
    82.4069,   87.3071,   92.4986,   97.9989,   103.8262,   110.0000,   116.5409,   123.4708,
    130.8128,  138.5913,  146.8324,  155.5635,  164.8138,   174.6141,   184.9972,   195.9977,
    207.6523,  220.0000,  233.0819,  246.9417,  261.6256,   277.1826,   293.6648,   311.1270,
    329.6276,  349.2282,  369.9944,  391.9954,  415.3047,   440.0000,   466.1638,   493.8833,
    523.2511,  554.3653,  587.3295,  622.2540,  659.2551,   698.4565,   739.9888,   783.9909,
    830.6094,  880.0000,  932.3275,  987.7666,  1046.5023,  1108.7305,  1174.6591,  1244.5079,
    1318.5102, 1396.9129, 1479.9777, 1567.9817, 1661.2188,  1760.0000,  1864.6550,  1975.5332,
    2093.0045, 2217.4610, 2349.3181, 2489.0159, 2637.0205,  2793.8259,  2959.9554,  3135.9635,
    3322.4376, 3520.0000, 3729.3101, 3951.0664, 4186.0090,  4434.9221,  4698.6363,  4978.0317,
    5274.0409, 5587.6517, 5919.9108, 6271.9270, 6644.8752,  7040.0000,  7458.6202,  7902.1328,
    8372.0181, 8869.8442, 9397.2726, 9956.0635, 10548.0818, 11175.3034, 11839.8215, 12543.8540
};

// GUItool: begin automatically generated code
AudioSynthWaveform       wave0;          //xy=502.74795150756836,82.7552137374878
AudioSynthWaveform       wave1;      //xy=504.28649139404297,117.86524295806885
AudioSynthWaveform       wave2;      //xy=503.2865982055664,153.0081024169922
AudioSynthWaveform       wave3;      //xy=502.8580284118653,188.86524295806885
AudioMixer4              mixer0;         //xy=633.7151184082031,100.00811004638672
AudioEffectEnvelope      env0;           //xy=758.612813949585,54.04482841491699
AudioFilterStateVariable filter0;        //xy=888.6010780334473,60.850419998168945
AudioMixer4              mixerA;         //xy=1010.7359161376953,171.30673599243164
AudioMixer4              mixerL;      //xy=1196.8192749023438,210.86235809326172
AudioMixer4              mixerR;     //xy=1198.2637329101562,277.8345947265625
AudioOutputAnalogStereo  audioOut;       //xy=1360.3193969726562,250.61236572265625
AudioConnection          patchCord1(wave0, 0, mixer0, 0);
AudioConnection          patchCord2(wave3, 0, mixer0, 3);
AudioConnection          patchCord3(wave2, 0, mixer0, 2);
AudioConnection          patchCord4(wave1, 0, mixer0, 1);
AudioConnection          patchCord5(mixer0, env0);
AudioConnection          patchCord6(env0, 0, filter0, 0);
AudioConnection          patchCord7(filter0, 0, mixerA, 0);
AudioConnection          patchCord8(mixerA, 0, mixerL, 0);
AudioConnection          patchCord9(mixerA, 0, mixerR, 0);
AudioConnection          patchCord10(mixerL, 0, audioOut, 0);
AudioConnection          patchCord11(mixerR, 0, audioOut, 1);
// GUItool: end automatically generated code


#define NUM_VOICES 4

AudioSynthWaveform *waves[] = {
  &wave0, &wave1, &wave2, &wave3
};

uint8_t notes[] = {43, 43, 44, 41, }; // possible notes to play MIDI G2, G2#, F2
uint8_t note_id=0;
int filterf_max = 2000;
int filterf = 2000;

int16_t noteMillis = 2000;
uint32_t lastMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("AdaTAL_testing hello");

  AudioMemory(120);

  filter0.frequency(4000);
  
  // set envelope parameters, for pleasing sound :-)
  env0.attack(100);
  env0.hold(2);
  env0.decay(300);
  env0.sustain(0.6);
  env0.release(1200);

  Serial.println("setup done");

  // Initialize processor and memory measurements
  AudioProcessorUsageMaxReset();
  AudioMemoryUsageMaxReset();
}

void noteOn(int channel, int note) {
  Serial.printf("noteOn: channel:%d note:%d\n", channel, note);
  waves[0]->begin( 0.9, tune_frequencies2_PGM[note], WAVEFORM_SAWTOOTH);
  waves[1]->begin( 0.9, tune_frequencies2_PGM[note] * 1.01, WAVEFORM_SAWTOOTH); // detune
  waves[2]->begin( 0.9, tune_frequencies2_PGM[note] * 1.02, WAVEFORM_SAWTOOTH); // detune
  waves[3]->begin( 0.9, tune_frequencies2_PGM[note] / 2, WAVEFORM_SAWTOOTH); // octave down

  env0.noteOn();
}

void noteOff(int voice_num) {
  Serial.printf("noteOff: voice:%d\n", voice_num);
  env0.noteOff();
}

void loop() { 
  if( millis() - lastMillis > noteMillis ) { 
    lastMillis = millis();
    Serial.println("Note!");
    noteOn(0, notes[note_id]);
    note_id = (note_id+1) % sizeof(notes);
    filterf = filterf_max;
  }
  // simple ramp down LFO on frequency
  filter0.frequency(filterf);
  if( filterf>30) { filterf = filterf * 0.99; };
  
  delay(10);
}
