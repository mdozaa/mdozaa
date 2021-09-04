
/*
 * Animated GIFs Display Code for SmartMatrix and HUB75 RGB LED Panels
 *
 * Uses SmartMatrix Library written by Louis Beaudoin at pixelmatix.com
 *
 * Written by: Craig A. Lindley
 *
 * Copyright (c) 2014 Craig A. Lindley
 * Refactoring by Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This SmartMatrix Library example displays GIF animations loaded from a SD Card connected to the Teensy 3/4 and ESP32
 *
 * This example requires SmartMatrix Library 4.0 and AnimatedGIF Library to be installed, you can do this from Arduino Library Manager
 *   - https://github.com/pixelmatix/SmartMatrix
 *   - https://github.com/bitbank2/AnimatedGIF
 *
 * The example can be modified to drive displays other than SmartMatrix by replacing SmartMatrix Library calls in setup() and
 * the *Callback() functions with calls to a different library (look for the USE_SMARTMATRIX and ENABLE_SCROLLING blocks and replace)
 *
 * Wiring is on the default Teensy 3.2 SPI pins, and chip select can be on any GPIO,
 * set by defining SD_CS in the code below.  For Teensy 3.5/3.6/4.1 with the onboard SDIO, SD_CS should be the default BUILTIN_SDCARD
 * Function     | Pin
 * DOUT         |  11
 * DIN          |  12
 * CLK          |  13
 * CS (default) |  15
 *
 * Wiring for ESP32 follows the default for the ESP32 SD Library, see: https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 *
 * This code first looks for .gif files in the /gifs/ directory
 * (customize below with the GIF_DIRECTORY definition) then plays random GIFs in the directory,
 * looping each GIF for DISPLAY_TIME_SECONDS
 *
 * This example is meant to give you an idea of how to add GIF playback to your own sketch.
 * For a project that adds GIF playback with other features, take a look at
 * Light Appliance and Aurora:
 * https://github.com/CraigLindley/LightAppliance
 * https://github.com/pixelmatix/aurora
 *
 * If you find any GIFs that won't play properly, please attach them to a new
 * Issue post in the GitHub repo here:
 * https://github.com/pixelmatix/AnimatedGIFs/issues
 */

// Including IRremote so I can switch between the gifs being displayed - Miguel Pacheco
#include <boarddefs.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
#include <SmartMatrix.h>
#include <SD.h>
#include <GifDecoder.h>
#include "FilenameFunctions.h"

#define DISPLAY_TIME_SECONDS 5
#define NUMBER_FULL_CYCLES   1

#define USE_SMARTMATRIX         1
#define ENABLE_SCROLLING        1

// range 0-255
//Originally a constant, but this program takes advantage of the use of an IR remote to change panel brightness on the fly
static int defaultBrightness = 50;

const rgb24 COLOR_BLACK = {
    0, 0, 0 };

#if (USE_SMARTMATRIX == 1)
/* SmartMatrix configuration and memory allocation */
#define COLOR_DEPTH 24                 // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 64;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 64;      // Set to the height of your display
const uint8_t kRefreshDepth = 24;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;  // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
#if (ENABLE_SCROLLING == 1)
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
#endif
#endif

/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 * 
 * lzwMaxBits is included for backwards compatibility reasons, but isn't used anymore
 */
GifDecoder<kMatrixWidth, kMatrixHeight, 12> decoder;

// Chip select for SD card
#if defined(ESP32)
    #define SD_CS 5
#elif defined (ARDUINO)
    #define SD_CS BUILTIN_SDCARD
    //#define SD_CS 15
#endif

#if defined(ESP32)
    // ESP32 SD Library can't handle a trailing slash in the directory name
    #define GIF_DIRECTORY "/gifs"
#else
    // Teensy SD Library requires a trailing slash in the directory name
    #define GIF_DIRECTORY "/gifs/"
#endif

int num_files;

void screenClearCallback(void) {
#if (USE_SMARTMATRIX == 1)
  backgroundLayer.fillScreen({0,0,0});
#endif
}

void updateScreenCallback(void) {
#if (USE_SMARTMATRIX == 1)
  backgroundLayer.swapBuffers();
#endif
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
#if (USE_SMARTMATRIX == 1)
    backgroundLayer.drawPixel(x, y, {red, green, blue});
#endif
}


int RECV_PIN = 23; //Assigns pin 23 of the teensy to be the receiving pin for the IR receiver, reassign as needed. - Miguel P.
IRrecv irrecv(RECV_PIN);

decode_results results;

// Setup method runs once, when the sketch starts
void setup() {
   
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
    
    // NOTE: new callback function required after we moved to using the external AnimatedGIF library to decode GIFs
    decoder.setFileSizeCallback(fileSizeCallback);

    Serial.begin(115200);

    // give time for USB Serial to be ready
    delay(1000);

    Serial.println("Enabling IRin");
    Serial.println("Starting AnimatedGIFs Sketch");
    irrecv.enableIRIn(); //Start the receiver
    Serial.println("Enabled IRin");

#if (USE_SMARTMATRIX == 1)
    // Initialize matrix
    matrix.addLayer(&backgroundLayer); 
#if (ENABLE_SCROLLING == 1)
    matrix.addLayer(&scrollingLayer); 
#endif

    matrix.setBrightness(defaultBrightness);

    // for large panels, may want to set the refresh rate lower to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly)
    //matrix.setRefreshRate(90);

#if !defined(ESP32)
    matrix.begin();
#endif

#if defined(ESP32)
    // for large panels on ESP32, may want to set the max percentage time dedicated to updating the refresh frames lower, to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly)
    //matrix.setMaxCalculationCpuPercentage(50);

    // alternatively, for large panels on ESP32, may want to set the calculation refresh rate divider lower to leave more CPU time to decoding GIFs (needed if GIFs are playing back slowly) - this has the same effect as matrix.setMaxCalculationCpuPercentage() but is set with a different parameter
    //matrix.setCalcRefreshRateDivider(4);

    // The ESP32 SD Card library is going to want to malloc about 28000 bytes of DMA-capable RAM, make sure at least that much is left free
    matrix.begin(28000);
#endif

    // Clear screen
    backgroundLayer.fillScreen(COLOR_BLACK);
    backgroundLayer.swapBuffers();
#endif

    if(initFileSystem(SD_CS) < 0) {
#if (ENABLE_SCROLLING == 1)
        scrollingLayer.start("No SD card", -1);
#endif
        Serial.println("No SD card");
        while(1);
    }

    // Determine how many animated GIF files exist
    num_files = enumerateGIFFiles(GIF_DIRECTORY, true);
    Serial.println(num_files);

    if(num_files < 0) {
#if (ENABLE_SCROLLING == 1)
        scrollingLayer.start("No gifs directory", -1);
#endif
        Serial.println("No gifs directory");
        while(1);
    }

    if(!num_files) {
#if (ENABLE_SCROLLING == 1)
        scrollingLayer.start("Empty gifs directory", -1);
#endif
        Serial.println("Empty gifs directory");
        while(1);
    }
}

/*Modified loop - Edited by Miguel Pacheco
Original loop functioned by running gifs on a timer, however my implementation allows the user to display a GIF of their choice
until they decide that they want to change it

Note this requires an IR receiver and remote.
*/
void loop() {
    static int maxIndex = num_files - 1;
    static int nextGIF = 1;     
    static int index = 0;
    
    if(nextGIF)
    {
        nextGIF = 0;

        if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0) {
            if(decoder.startDecoding() < 0) {
                nextGIF = 1;
                return;
            } 
        }
    }

    if(decoder.decodeFrame() < 0) {
        // There's an error with this GIF, go to the next one
        nextGIF = 1;
    }

    //Use serial monitor with the IR receiver attached to see what codes the remote fires off for each button, assign as required.
    if (irrecv.decode(&results)) {
      Serial.println(results.value);

      //Go to the next GIF, wrapping around to the front if we are already at the end.
      if (results.value == 16601263) {
        if (++index >= num_files) {
            index = 0;
        }
        nextGIF = 1;
      }

      //Go to the previous GIF, wrapping around to the end if we are already at the beginning.
      if (results.value == 16584943) {
        if (--index < 0) {
            index = maxIndex;
        }
        nextGIF = 1;
      }

      //Here begins preset buttons
      //Note that because these are values that are surely found in the array, no checks need to be made for the index
      //1 - Gojo dancing
      if (results.value == 16582903) {
        index = 5;
        nextGIF = 1;
      }

      //2 - Giyuu butterflies 
      if (results.value == 16615543) {
        index = 7;
        nextGIF = 1;
      }
      
      //3 - Shinobu dancing
      if (results.value == 16599223) {
        index = 8;
        nextGIF = 1;
      }

      //4 - Cool squirtles
      if (results.value == 16591063) {
        index = 2;
        nextGIF = 1;
      }

      //5 - Pikachu Trippy
      if (results.value == 16623703) {
        index = 11;
        nextGIF = 1;
      }

      //6 - Pikachu cheeks
      if (results.value == 16607383) {
        index = 39;
        nextGIF = 1;
      }

      //7 - Squidward dancing
      if (results.value == 16586983) {
        index = 4;
        nextGIF = 1;
      }

      //8 - Kirby
      if (results.value == 16619623) {
        index = 33;
        nextGIF = 1;
      }

      //9 - Cat trippy
      if (results.value == 16603303) {
        index = 15;
        nextGIF = 1;
      }
      
      //Up button - Put brightness UP
      if (results.value == 16621663) {
        defaultBrightness = defaultBrightness + 10;
         matrix.setBrightness(defaultBrightness);
      }

      //Down button - Put brightness DOWN
      if (results.value == 16625743) {
        defaultBrightness = defaultBrightness - 10;
         matrix.setBrightness(defaultBrightness);
      }
    
      irrecv.resume(); // Receive the next value
    }
    delay(25); //note that delay will affect GIF speed. Found that some GIFS were being played back too slowly when @ 100 (as originally programmed).
}
