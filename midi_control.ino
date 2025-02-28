/*
midi_control.ini
this version uses pot for pitchwheel data
*/

//////////////////////////////////////
// If using Fast Led
//#include "FastLED.h"

//FASTLED_USING_NAMESPACE

//#define DATA_PIN    5
//#define CLK_PIN   4
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    16
#define HUE_OFFSET 90
//CRGB leds[NUM_LEDS];
byte ledIndex[NUM_LEDS] = {15, 8, 7, 0, 14, 9, 6, 1, 13, 10, 5, 2, 12, 11, 4, 3};

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

byte ch1Hue = 135;
byte maxHue = 240;

//////////////////////////////////////

/////////////////////////////////////////////
// Choosing your board
// Define your board, choose:
// "ATMEGA328" if using ATmega328 - Uno, Mega, Nano...
// "ATMEGA32U4" if using with ATmega32U4 - Micro, Pro Micro, Leonardo...
// "TEENSY" if using a Teensy board
// "DEBUG" if you just want to debug the code in the serial monitor
// you don't need to comment or uncomment any MIDI library below after you define your board

#define ATMEGA32U4 1//* put here the uC you are using, like in the lines above followed by "1", like "ATMEGA328 1", "DEBUG 1", etc.

/////////////////////////////////////////////
// Are you using a multiplexer?
#define USING_MUX 1 //* comment if not using a multiplexer, uncomment if using it.

/////////////////////////////////////////////
// Are you using encoders?
// #define USING_ENCODER 1 //* comment if not using encoders, uncomment if using it.

/////////////////////////////////////////////
// LIBRARIES
// -- Defines the MIDI library -- //

// if using with ATmega328 - Uno, Mega, Nano...
#ifdef ATMEGA328
#include <MIDI.h>
//MIDI_CREATE_DEFAULT_INSTANCE();

// if using with ATmega32U4 - Micro, Pro Micro, Leonardo...
#elif ATMEGA32U4
#include "MIDIUSB.h"

#endif
// ---- //

//////////////////////
// Add this lib if using a cd4067 multiplexer
#ifdef USING_MUX
#include <Multiplexer4067.h> // Multiplexer CD4067 library >> https://github.com/sumotoy/Multiplexer4067
#endif

//////////////////////
// Threads
#include <Thread.h> // Threads library >> https://github.com/ivanseidel/ArduinoThread
#include <ThreadController.h> // Same as above

//////////////////////
// Encoder
#ifdef USING_ENCODER
// In the downloads manager download the Encoder lib from Paul Stoffregen (it comes with the Teensy)
#include <Encoder.h>  // makes all the work for you on reading the encoder
#endif

///////////////////////////////////////////
// MULTIPLEXERS
#ifdef USING_MUX

#define N_MUX 1 //* number of multiplexers
//* Define s0, s1, s2, s3, and x pins
#define s0 16
#define s1 14
#define s2 15
#define s3 2
#define x1 A10 // analog pin of the first mux
// add more #define and the x number if you need

// Initializes the multiplexer
Multiplexer4067 mux[N_MUX] = {
  Multiplexer4067(s0, s1, s2, s3, x1) // The SIG pin where the multiplexer is connnected
  // ...
};

#endif



/////////////////////////////////////////////
// BUTTONS
const int N_BUTTONS = 12; //*  total numbers of buttons. Number of buttons in the Arduino + number of buttons on multiplexer 1 + number of buttons on multiplexer 2...
const int N_BUTTONS_ARDUINO = 0; //* number of buttons connected straight to the Arduino (in order)
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = {0,2,3,5,7,16,14,15}; //* pins of each button connected straight to the Arduino

const int CHANNEL_BUTTON_PIN = 1;
const int NOTE_RANGE_BUTTON_PIN = 0; // Future option to add note offset to buttons 

int NOTE_OFFSET;  // used to expans note range of buttons

//#define USING_CUSTOM_NN 1 //* comment if not using CUSTOM NOTE NUMBERS (scales), uncomment if using it.
#ifdef USING_CUSTOM_NN
int BUTTON_NN[N_BUTTONS] = {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

//* Add the NOTE NUMBER of each button/switch you want
#endif

//#define USING_BUTTON_CC_N 1 //* comment if not using BUTTON CC, uncomment if using it.
#ifdef USING_BUTTON_CC_N // if using button with CC
int BUTTON_CC_N[N_BUTTONS] = {12, 16}; //* Add the CC NUMBER of each button/switch you want
#endif

//#define USING_TOGGLE 1 //* comment if not using BUTTON TOGGLE mode, uncomment if using it.
// With toggle mode on, when you press the button once it sends a note on, when you press it again it sends a note off

#ifdef USING_MUX // Fill if you are using mux, otherwise just leave it
const int N_BUTTONS_PER_MUX[N_MUX] = {16}; //* number of buttons in each mux (in order)
const int BUTTON_MUX_PIN[N_MUX][16] = { //* pin of each button of each mux in order
{
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
}, //* pins of the first mux
// ...
};
#endif

int buttonMuxThreshold = 300;

int buttonCState[N_BUTTONS] = {0};        // stores the button current value
int buttonPState[N_BUTTONS] = {0};        // stores the button previous value

//#define pin13 1 // uncomment if you are using pin 13 (pin with led), or comment the line if it is not
byte pin13index = 12; //* put the index of the pin 13 of the buttonPin[] array if you are using, if not, comment

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = {0};  // the last time the output pin was toggled
unsigned long debounceDelay = 5;    //* the debounce time; increase if the output flickers

// velocity
byte velocity[N_BUTTONS] = {127};

/////////////////////////////////////////////
// POTENTIOMETERS
const int N_POTS = 8; //* total numbers of pots (slide & rotary). Number of pots in the Arduino + number of pots on multiplexer 1 + number of pots on multiplexer 2...
const int N_POTS_ARDUINO = 8; //* number of pots connected straight to the Arduino
const int POT_ARDUINO_PIN[N_POTS_ARDUINO] = {A0, A1, A2, A3, A6, A7, A8, A9}; //* pins of each pot connected straight to the Arduino
const int PITCHWHEEL_CH = 8; // number of analog channel in array above ( starting with 1) that we want to be pitch change command

#define USING_CUSTOM_CC_N 1 //* comment if not using CUSTOM CC NUMBERS, uncomment if using it.
#ifdef USING_CUSTOM_CC_N
int POT_CC_N[N_POTS] = {1, 2,  4, 5, 6, 7, 3, 8}; // Add the CC NUMBER of each pot you want
#endif

#ifdef USING_MUX
const int N_POTS_PER_MUX[N_MUX] = {0}; //* number of pots in each multiplexer (in order)
const int POT_MUX_PIN[N_MUX][16] = { //* pins of each pot of each mux in the order you want them to be
// nothing
};
#endif

int potCState[N_POTS] = {0}; // Current state of the pot
int potPState[N_POTS] = {0}; // Previous state of the pot
int potVar = 0; // Difference between the current and previous state of the pot

int potMidiCState[N_POTS] = {0}; // Current state of the midi value
int potMidiPState[N_POTS] = {0}; // Previous state of the midi value

const int TIMEOUT = 30; //* Amount of time the potentiometer will be read after it exceeds the varThreshold 300
const int varThreshold = 1; //* Threshold for the potentiometer signal variation 10
boolean potMoving = true; // If the potentiometer is moving
unsigned long PTime[N_POTS] = {0}; // Previously stored time
unsigned long timer[N_POTS] = {0}; // Stores the time that has elapsed since the timer was reset

// one pole filter
// y[n] = A0 * x[n] + B1 * y[n-1];
// A = 0.15 and B = 0.85
float filterA = 0.3;
float filterB = 0.7;

/////////////////////////////////////////////
// ENCODERS
#ifdef USING_ENCODER
// You can add as many encoders you want separated in many encoderChannels you want
//#define TRAKTOR 1 // uncomment if using with traktor, comment if not
const int N_ENCODERS = 2; //* number of encoders
const int N_ENCODER_CHANNELS = 1; //* number of encoderChannels
const int N_ENCODER_PINS = N_ENCODERS * 2; //number of pins used by the encoders

Encoder encoder[N_ENCODERS] = {{9, 8}, {11, 10}}; // the two pins of each encoder (backwards) -  Use pins with Interrupts!

int encoderMinVal = 0; //* encoder minimum value

int preset[N_ENCODER_CHANNELS][N_ENCODERS] = { //stores presets to start your encoders
  {64, 64},
};

int lastEncoderValue[N_ENCODER_CHANNELS][N_ENCODERS] = {127};
int encoderValue[N_ENCODER_CHANNELS][N_ENCODERS] = {127};

// for the encoder Channels - Used for different banks
int encoderChannel = 0;
int lastEncoderChannel = 0;

#endif
/////////////////////////////////////////////

/////////////////////////////////////////////
// CHANNEL
byte MIDI_CH = 0; //* MIDI channel to be used
byte BUTTON_MIDI_CH = 0; //* MIDI channel to be used
byte NOTE = 36; //* Lowest NOTE to be used - if not using custom NOTE NUMBER
byte CC = 1; //* Lowest MIDI CC to be used - if not using custom CC NUMBER

boolean channelMenuOn = false;
boolean rangeMenuOn = false;
byte midiChMenuColor = 200;

/////////////////////////////////////////////
// THREADS
// This libs create a "fake" thread. This means you can make something happen every x milisseconds
// We can use that to read something in an interval, instead of reading it every single loop
// In this case we'll read the potentiometers in a thread, making the reading of the buttons faster
ThreadController cpu; //thread master, where the other threads will be added
Thread threadPotentiometers; // thread to control the pots
Thread threadChannelMenu; // thread to control the pots
Thread threadRangeMenu;

void setup() {

  // Baud Rate
  // use if using with ATmega328 (uno, mega, nano...)
  // 31250 for MIDI class compliant | 115200 for Hairless MIDI
  //Serial.begin(115200); //*


#ifdef DEBUG
Serial.println("Debug mode");
Serial.println();
#endif


  ////////////////////////////////////
  //FAST LED
//  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  //FastLED.setBrightness(BRIGHTNESS);

//  setAllLeds(ch1Hue, 30);// set all leds at once with a hue (hue, randomness)

//  FastLED.show();

  //////////////////////////////////////

  //////////////////////////////////////
  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < N_BUTTONS_ARDUINO; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

  pinMode(CHANNEL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NOTE_RANGE_BUTTON_PIN , INPUT_PULLUP);

#ifdef pin13 // Initialize pin 13 as an input
pinMode(BUTTON_ARDUINO_PIN[pin13index], INPUT);
#endif
////////////////////////////////////

  //////////////////////////////////////
  // Potentiometer
  // Initialize pots with pull up resistors
  for (int i = 0; i < N_POTS_ARDUINO; i++) {
    pinMode(POT_ARDUINO_PIN[i], INPUT_PULLUP);
  }
  ////////////////////////////////////



  /////////////////////////////////////////////
  // Multiplexers

#ifdef USING_MUX

  // Initialize the multiplexers
  for (int i = 0; i < N_MUX; i++) {
    mux[i].begin();
  }
  //* Set each X pin as input_pullup (avoid floating behavior)
  pinMode(x1, INPUT_PULLUP);

#endif

  /////////////////////////////////////////////
  // Threads
  // Potentiometers
  threadPotentiometers.setInterval(15); // every how many millisiconds
  threadPotentiometers.onRun(potentiometers); // the function that will be added to the thread
  // Channel Menu
  threadChannelMenu.setInterval(40); // every how many millisiconds
  threadChannelMenu.onRun(channelMenu); // the function that will be added to the thread
  //Range Menu
  threadRangeMenu.setInterval(40); // every how many millisiconds
  threadRangeMenu.onRun(noterangeMenu);

  cpu.add(&threadPotentiometers); // add every thread here
  cpu.add(&threadChannelMenu); // add every thread here
  cpu.add(&threadRangeMenu); // add every thread here

  /////////////////////////////////////////////
  // Encoders
#ifdef USING_ENCODER

  for (int i = 0; i < N_ENCODERS; i++) { // if you want to start with a certain value use presets
    encoder[i].write(preset[0][i]);
  }

  for (int z = 0; z < N_ENCODER_CHANNELS; z++) {
    for (int i = 0; i < N_ENCODERS; i++) {
      lastEncoderValue[z][i] = preset[z][i];
      encoderValue[z][i] = preset[z][i];
    }
  }

#endif
/////////////////////////////////////////////



}

void loop() {

  MIDIread();

  buttons();

#ifdef USING_ENCODER
encoders();
#endif

  cpu.run(); // for threads

  //FastLED.show();
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000 / FRAMES_PER_SECOND);

}

/////////////////////////////////////////////
// BUTTONS
void buttons() {

  // read pins from arduino
  for (int i = 0; i < N_BUTTONS_ARDUINO; i++) {
    buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);
  }

#ifdef USING_MUX

  // It will happen if you are using MUX
  int nButtonsPerMuxSum = N_BUTTONS_ARDUINO; // offsets the buttonCState at every mux reading

  // read the pins from every mux
  for (int j = 0; j < N_MUX; j++) {
    for (int i = 0; i < N_BUTTONS_PER_MUX[j]; i++) {
      buttonCState[i + nButtonsPerMuxSum] = mux[j].readChannel(BUTTON_MUX_PIN[j][i]);
      // Scale values to 0-1
      if (buttonCState[i + nButtonsPerMuxSum] > buttonMuxThreshold) {
        buttonCState[i + nButtonsPerMuxSum] = HIGH;
      }
      else {
        buttonCState[i + nButtonsPerMuxSum] = LOW;
      }
    }
    nButtonsPerMuxSum += N_BUTTONS_PER_MUX[j];
  }
#endif

  for (int i = 0; i < N_BUTTONS; i++) { // Read the buttons connected to the Arduino

#ifdef pin13

    // It will happen if you are using pin 13
    if (i == pin13index) {
      buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down resistor instead of a pull up
    }
#endif

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {

      if (buttonPState[i] != buttonCState[i]) {
        lastDebounceTime[i] = millis();

        if (buttonCState[i] == LOW) {

#ifndef USING_TOGGLE // if NOT using button toggle mode

          velocity[i] = 127; // if button is pressed velocity is 127
#else

          velocity[i] = !velocity[i] * 127; // changes the velocity between 0 and 127 each time one press a button
#endif

        }
        else {

#ifndef USING_TOGGLE // if NOT using button toggle mode

          velocity[i] = 0; // if button is released velocity is 0
#endif
}


          // Sends the MIDI NOTE ON accordingly to the chosen board

#ifdef USING_TOGGLE

          if (buttonCState[i] == LOW) { // only when button is pressed
#endif

            /////////////////////////////////////////////
#ifdef ATMEGA328

            // it will happen if using with ATmega328 (uno, mega, nano...)

#ifndef USING_BUTTON_CC_N // if NOT using button CC

#ifdef USING_CUSTOM_NN

            // if using custom NOTE numbers
            MIDI.sendNoteOn(BUTTON_NN[i]+ NOTE_OFFSET, velocity[i], BUTTON_MIDI_CH); // note, velocity, channel
#else

            // if not using custom NOTE numbers
            MIDI.sendNoteOn(NOTE + i + NOTE_OFFSET, velocity[i], BUTTON_MIDI_CH); // note, velocity, channel
#endif

#else // if USING button CC

            if (buttonCState[i] == LOW) { // only sends note on when button is pressed, nothing when released
              MIDI.sendControlChange(BUTTON_CC_N[i], velocity[i], BUTTON_MIDI_CH); // note, velocity, channel
            }
#endif

            /////////////////////////////////////////////
#elif ATMEGA32U4

            // it will happen if using with ATmega32U4 (micro, pro micro, leonardo...)

#ifndef USING_BUTTON_CC_N // if NOT using button CC

#ifdef USING_CUSTOM_NN

            // if using custom NOTE numbers

            noteOn(BUTTON_MIDI_CH, BUTTON_NN[i] + NOTE_OFFSET, velocity[i]);  // channel, note, velocity
            MidiUSB.flush();
#else

            // if not using custom NOTE
            noteOn(BUTTON_MIDI_CH, NOTE + i + NOTE_OFFSET, velocity[i]);  // channel, note, velocity
            MidiUSB.flush();
#endif

#else // if USING button CC

            if (velocity[i] > 0) { // only sends note on when button is pressed, nothing when released
              controlChange(BUTTON_MIDI_CH, BUTTON_CC_N[i], velocity[i]); //  (channel, CC number,  CC value)
              MidiUSB.flush();
            }
#endif


            /////////////////////////////////////////////
#elif TEENSY
//do usbMIDI.sendNoteOn if using with Teensy

#ifndef USING_BUTTON_CC_N // if NOT using button CC

#ifdef USING_CUSTOM_NN

            // if using custom NOTE numbers
            usbMIDI.sendNoteOn(BUTTON_NN[i] + NOTE_OFFSET, velocity[i], BUTTON_MIDI_CH); // note, velocity, channel
#else

            // if not using custom NOTE
            usbMIDI.sendNoteOn(NOTE + i + NOTE_OFFSET, velocity[i], BUTTON_MIDI_CH); // note, velocity, channel
#endif

#else // if USING button CC

            if (velocity[i] > 0) { // only sends note on when button is pressed, nothing when released
              usbMIDI.sendControlChange(BUTTON_CC_N[i], velocity[i], BUTTON_MIDI_CH); // CC number, CC value, midi channel
            }
#endif

            /////////////////////////////////////////////
#elif DEBUG

#ifndef USING_BUTTON_CC_N // print if not using button cc number

            Serial.print("Button: ");
            Serial.print(i);
            Serial.print("  | ch: ");
            Serial.print(BUTTON_MIDI_CH);
            Serial.print("  | nn: ");

#ifdef USING_CUSTOM_NN

            Serial.print(BUTTON_NN[i]);
#else

            Serial.print(NOTE + i);
#endif

            Serial.print("  | velocity: ");
            Serial.println(velocity[i]);
#else

            Serial.print("Button: ");
            Serial.print(i);
            Serial.print("  | ch: ");
            Serial.print(BUTTON_MIDI_CH);
            Serial.print("  | cc: ");
            Serial.print(BUTTON_CC_N[i]);
            Serial.print("  | value: ");
            Serial.println(velocity[i]);
#endif

#endif

            /////////////////////////////////////////////

#ifdef USING_TOGGLE

          }
#endif

          buttonPState[i] = buttonCState[i];
        }

      }
    }
  }

  /////////////////////////////////////////////
  // POTENTIOMETERS
  void potentiometers() {

    // reads the pins from arduino
    for (int i = 0; i < N_POTS_ARDUINO; i++) {
      //potCState[i] = analogRead(POT_ARDUINO_PIN[i]);

      // one pole filter
      // y[n] = A0 * x[n] + B1 * y[n-1];
      // A = 0.15 and B = 0.85
      int reading = analogRead(POT_ARDUINO_PIN[i]);
      float filteredVal = filterA * reading + filterB * potPState[i]; // filtered value
      potCState[i] = filteredVal;
    }

#ifdef USING_MUX
//It will happen if using a mux
int nPotsPerMuxSum = N_POTS_ARDUINO; //offsets the buttonCState at every mux reading

    // reads the pins from every mux
    for (int j = 0; j < N_MUX; j++) {
      for (int i = 0; i < N_POTS_PER_MUX[j]; i++) {
        potCState[i + nPotsPerMuxSum] = mux[j].readChannel(POT_MUX_PIN[j][i]);
      }
      nPotsPerMuxSum += N_POTS_PER_MUX[j];
    }
#endif

    //Debug only
    //    for (int i = 0; i < nPots; i++) {
    //      Serial.print(potCState[i]); Serial.print(" ");
    //    }
    //    Serial.println();

    for (int i = 0; i < N_POTS; i++) { // Loops through all the potentiometers

      potMidiCState[i] = map(potCState[i], 0, 1023, 0, 127); // Maps the reading of the potCState to a value usable in midi

      potVar = abs(potCState[i] - potPState[i]); // Calculates the absolute value between the difference between the current and previous state of the pot

      if (potVar > varThreshold) { // Opens the gate if the potentiometer variation is greater than the threshold
        PTime[i] = millis(); // Stores the previous time
      }

      timer[i] = millis() - PTime[i]; // Resets the timer 11000 - 11000 = 0ms

      if (timer[i] < TIMEOUT) { // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
        potMoving = true;
      }
      else {
        potMoving = false;
      }

      if (potMoving == true) { // If the potentiometer is still moving, send the change control
        if (potMidiPState[i] != potMidiCState[i]) {

          // Sends the MIDI CC accordingly to the chosen board
#ifdef ATMEGA328
// use if using with ATmega328 (uno, mega, nano...)

#ifdef USING_CUSTOM_CC_N
MIDI.sendControlChange(POT_CC_N[i], potMidiCState[i], MIDI_CH); // CC number, CC value, midi channel - custom cc
#else
MIDI.sendControlChange(CC + i, potMidiCState[i], MIDI_CH); // CC number, CC value, midi channel
#endif

#elif ATMEGA32U4
//use if using with ATmega32U4 (micro, pro micro, leonardo...)

#ifdef USING_CUSTOM_CC_N

if (POT_CC_N[i]==PITCHWHEEL_CH){
  int temp = (int) map(potCState[i],32, 997, 0, 16383); // scale actual pot values for accurate pitch extremes
  temp = constrain( temp, 0, 16383); // constrain map expansion to valid midi limits
  if ( (temp < 9017) && (temp > 8262)) temp = 8192;   // make dead zone default to center pitch value 8192
  pitchBendChange(MIDI_CH,temp ) ;
  
} else {
  controlChange(MIDI_CH, POT_CC_N[i], potMidiCState[i]); //  (channel, CC number,  CC value)
}
MidiUSB.flush();
#else
controlChange(MIDI_CH, CC + i, potMidiCState[i]); //  (channel, CC number,  CC value)
MidiUSB.flush();
#endif

#elif TEENSY
//do usbMIDI.sendControlChange if using with Teensy

#ifdef USING_CUSTOM_CC_N
usbMIDI.sendControlChange(POT_CC_N[i], potMidiCState[i], MIDI_CH); // CC number, CC value, midi channel
#else
usbMIDI.sendControlChange(CC + i, potMidiCState[i], MIDI_CH); // CC number, CC value, midi channel
#endif

#elif DEBUG
Serial.print("Pot: ");
Serial.print(i);
Serial.print("  |  ch: ");
Serial.print(MIDI_CH);
Serial.print("  |  cc: ");
#ifdef USING_CUSTOM_NN
Serial.print(POT_CC_N[i]);
#else
Serial.print(CC + i);
#endif
Serial.print("  |  value: ");
Serial.println(potMidiCState[i]);
#endif


          potPState[i] = potCState[i]; // Stores the current reading of the potentiometer to compare with the next
          potMidiPState[i] = potMidiCState[i];
        }
      }
    }
  }

  /////////////////////////////////////////////
  // ENCODERS
#ifdef USING_ENCODER

  void encoders() {

    for (int i = 0; i < N_ENCODERS; i++) {
      encoderValue[encoderChannel][i] = encoder[i].read(); // reads each encoder and stores the value
    }

    for (int i = 0; i < N_ENCODERS; i++) {

      if (encoderValue[encoderChannel][i] != lastEncoderValue[encoderChannel][i]) {

#ifdef TRAKTOR // to use with Traktor
if (encoderValue[encoderChannel][i] > lastEncoderValue[encoderChannel][i]) {
encoderValue[encoderChannel][i] = 127;
} else {
encoderValue[encoderChannel][i] = 0;
}
#endif

        clipEncoderValue(i, encoderMinVal, encoderMaxVal); // checks if it's greater than the max value or less than the min value

        // Sends the MIDI CC accordingly to the chosen board
#ifdef ATMEGA328
// if using with ATmega328 (uno, mega, nano...)
MIDI.sendControlChange(i, encoderValue[encoderChannel][i], encoderChannel);

#elif ATMEGA32U4
// if using with ATmega32U4 (micro, pro micro, leonardo...)
controlChange(i, encoderValue[encoderChannel][i], encoderChannel);
MidiUSB.flush();

#elif TEENSY
// if using with Teensy
usbMIDI.sendControlChange(i, encoderValue[encoderChannel][i], encoderChannel);

#elif DEBUG
Serial.print("encoder channel: "); Serial.print(encoderChannel); Serial.print("  ");
Serial.print("Encoder "); Serial.print(i); Serial.print(": ");
Serial.println(encoderValue[encoderChannel][i]);
#endif

        lastEncoderValue[encoderChannel][i] = encoderValue[encoderChannel][i];
      }
    }
  }


  ////////////////////////////////////////////
  // checks if it's greater than maximum value or less than then the minimum value
  void clipEncoderValue(int i, int minVal, int maxVal) {

    if (encoderValue[encoderChannel][i] > maxVal - 1) {
      encoderValue[encoderChannel][i] = maxVal;
      encoder[i].write(maxVal);
    }
    if (encoderValue[encoderChannel][i] < minVal + 1) {
      encoderValue[encoderChannel][i] = minVal;
      encoder[i].write(minVal);
    }
  }

#endif

  /////////////////////////////////////////////
  // if using with ATmega32U4 (micro, pro micro, leonardo...)
#ifdef ATMEGA32U4

  // Arduino (pro)micro midi functions MIDIUSB Library
  void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
  }

  void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
  }

  void controlChange(byte channel, byte control, byte value) {
    midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
    MidiUSB.sendMIDI(event);
  }
  void pitchBendChange(byte channel,  int value) {
    byte lowValue = value & 0x7F;
    byte highValue = value >> 7;
    midiEventPacket_t event = {0x0E, 0xE0 | channel, lowValue, highValue};
    MidiUSB.sendMIDI(event);
  }
  

#endif


  void MIDIread() {

    midiEventPacket_t rx = MidiUSB.read();
    switch (rx.header) {
      case 0:
        break; //No pending events

      case 0x9:
        handlennOn(
          rx.byte1 & 0xF,  //channel
          rx.byte2,        //pitch
          rx.byte3         //velocity
        );
        break;

      case 0x8:
        handlennOff(
          rx.byte1 & 0xF,  //channel
          rx.byte2,        //pitch
          rx.byte3         //velocity
        );
        break;
    }

    if (rx.header != 0) {
      //Serial.print("Unhandled MIDI message: ");
      //      Serial.print(rx.byte1 & 0xF, DEC);
      //      Serial.print("-");
      //      Serial.print(rx.byte1, DEC);
      //      Serial.print("-");
      //      Serial.print(rx.byte2, DEC);
      //      Serial.print("-");
      //      Serial.println(rx.byte3, DEC);
    }

  }

  void handleControlChange(byte channel, byte number, byte value)
  {

  }

  void handlennOn(byte channel, byte number, byte value) {

    if (channel == BUTTON_MIDI_CH) {

      int ledN = number - NOTE;
      byte tempHue = map(value, 0, 127, ch1Hue, maxHue);

 //     leds[ledIndex[ledN]].setHue(tempHue);

 //     FastLED.show();
      //      // insert a delay to keep the framerate modest
      //      FastLED.delay(1000 / FRAMES_PER_SECOND);
    }

  }

  void handlennOff(byte channel, byte number, byte value) {

    if (channel == BUTTON_MIDI_CH) {
      int ledN = number - NOTE;

      byte offset = random(0, 20);
 //     leds[ledIndex[ledN]].setHue(ch1Hue  + offset);

//      FastLED.show();
      //      // insert a delay to keep the framerate modest
      //      FastLED.delay(1000 / FRAMES_PER_SECOND);
    }

  }
  ////////////////////////////////////////////

  ////////////////////////////////////////////
  // Channel Menu
  void channelMenu() {

    while ( digitalRead(CHANNEL_BUTTON_PIN)== LOW ){  // digitalRead(NOTE_RANGE_BUTTON_PIN) == LOW) {

//      setAllLeds(midiChMenuColor, 0); // turn all lights into the menu lights
//      leds[ledIndex[BUTTON_MIDI_CH]].setHue(midiChMenuColor + 60); // turn the specific channel light on
      channelMenuOn = true;
      
      // read pins from arduino
      for (int i = 0; i < N_BUTTONS_ARDUINO; i++) {
        buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);
      }

#ifdef USING_MUX

      // It will happen if you are using MUX
      int nButtonsPerMuxSum = N_BUTTONS_ARDUINO; // offsets the buttonCState at every mux reading

      // read the pins from every mux
      for (int j = 0; j < N_MUX; j++) {
        for (int i = 0; i < N_BUTTONS_PER_MUX[j]; i++) {
          buttonCState[i + nButtonsPerMuxSum] = mux[j].readChannel(BUTTON_MUX_PIN[j][i]);
          // Scale values to 0-1
          if (buttonCState[i + nButtonsPerMuxSum] > buttonMuxThreshold) {
            buttonCState[i + nButtonsPerMuxSum] = HIGH;
          }
          else {
            buttonCState[i + nButtonsPerMuxSum] = LOW;
          }
        }
        nButtonsPerMuxSum += N_BUTTONS_PER_MUX[j];
      }
#endif

      for (int i = 0; i < N_BUTTONS; i++) { // Read the buttons connected to the Arduino

#ifdef pin13

        // It will happen if you are using pin 13
        if (i == pin13index) {
          buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down resistor instead of a pull up
        }
#endif

        if ((millis() - lastDebounceTime[i]) > debounceDelay) {

          if (buttonPState[i] != buttonCState[i]) {
            lastDebounceTime[i] = millis();

        
              // DO STUFF

              BUTTON_MIDI_CH = i;
            buttonPState[i] = buttonCState[i];
          }
        }
      }
 //     FastLED.show();
      //      // insert a delay to keep the framerate modest
      //      FastLED.delay(1000 / FRAMES_PER_SECOND);
    }

    if (channelMenuOn == true) {
 //     setAllLeds(ch1Hue, 30);
      //Serial.println("menu lights off");

    }
    channelMenuOn = false;
    //
//    FastLED.show();
    //    // insert a delay to keep the framerate modest
    //    FastLED.delay(1000 / FRAMES_PER_SECOND);

  } //end

////////////////////////////////////////////
  // Note Range Menu
  void noterangeMenu() {

    while ( digitalRead(NOTE_RANGE_BUTTON_PIN) == LOW ){  

//      setAllLeds(midiChMenuColor, 0); // turn all lights into the menu lights
//      leds[ledIndex[BUTTON_MIDI_CH]].setHue(midiChMenuColor + 60); // turn the specific channel light on
      rangeMenuOn = true;
      
      // read pins from arduino
      for (int i = 0; i < N_BUTTONS_ARDUINO; i++) {
        buttonCState[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);
      }

#ifdef USING_MUX

      // It will happen if you are using MUX
      int nButtonsPerMuxSum = N_BUTTONS_ARDUINO; // offsets the buttonCState at every mux reading

      // read the pins from every mux
      for (int j = 0; j < N_MUX; j++) {
        for (int i = 0; i < N_BUTTONS_PER_MUX[j]; i++) {
          buttonCState[i + nButtonsPerMuxSum] = mux[j].readChannel(BUTTON_MUX_PIN[j][i]);
          // Scale values to 0-1
          if (buttonCState[i + nButtonsPerMuxSum] > buttonMuxThreshold) {
            buttonCState[i + nButtonsPerMuxSum] = HIGH;
          }
          else {
            buttonCState[i + nButtonsPerMuxSum] = LOW;
          }
        }
        nButtonsPerMuxSum += N_BUTTONS_PER_MUX[j];
      }
#endif

      for (int i = 0; i < N_BUTTONS; i++) { // Read the buttons connected to the Arduino

#ifdef pin13

        // It will happen if you are using pin 13
        if (i == pin13index) {
          buttonCState[i] = !buttonCState[i]; // inverts the pin 13 because it has a pull down resistor instead of a pull up
        }
#endif

        if ((millis() - lastDebounceTime[i]) > debounceDelay) {

          if (buttonPState[i] != buttonCState[i]) {
            lastDebounceTime[i] = millis();

        
              // DO STUFF

             NOTE_OFFSET = i * N_BUTTONS;
             if (NOTE_OFFSET>84) NOTE_OFFSET=84; // keep within playable notes
              Serial.print("NOTE_OFFSET ");
              Serial.println(NOTE_OFFSET);
            

           
            buttonPState[i] = buttonCState[i];
          }
        }
      }
 //     FastLED.show();
      //      // insert a delay to keep the framerate modest
      //      FastLED.delay(1000 / FRAMES_PER_SECOND);
    }

    if (channelMenuOn == true) {
 //     setAllLeds(ch1Hue, 30);
      //Serial.println("menu lights off");

    }
    rangeMenuOn = false;
    //
//    FastLED.show();
    //    // insert a delay to keep the framerate modest
    //    FastLED.delay(1000 / FRAMES_PER_SECOND);

  } //end
/*  void setAllLeds(byte hue_, byte randomness_) {

    for (int i = 0; i < NUM_LEDS; i++) {
      byte offset = random(0, randomness_);
      leds[i].setHue(hue_  + offset);
    }
  }*/
