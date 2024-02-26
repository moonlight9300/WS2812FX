/*!
 * @file Adafruit_NeoPixel.c
 *
 * @mainpage Arduino Library for driving Adafruit NeoPixel addressable LEDs,
 * FLORA RGB Smart Pixels and compatible devicess -- WS2811, WS2812, WS2812B,
 * SK6812, etc.
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's NeoPixel library for the
 * Arduino platform, allowing a broad range of microcontroller boards
 * (most AVR boards, many ARM devices, ESP8266 and ESP32, among others)
 * to control Adafruit NeoPixels, FLORA RGB Smart Pixels and compatible
 * devices -- WS2811, WS2812, WS2812B, SK6812, etc.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing products
 * from Adafruit!
 *
 * @section author Author
 *
 * Written by Phil "Paint Your Dragon" Burgess for Adafruit Industries,
 * with contributions by PJRC, Michael Miller and other members of the
 * open source community.
 *
 * @section license License
 *
 * This file is part of the Adafruit_NeoPixel library.
 *
 * Adafruit_NeoPixel is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Adafruit_NeoPixel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoPixel. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "Adafruit_NeoPixel.h"
#include "ws2812_user_def.h"

static bool begun;         ///< true if begin() previously called
uint16_t Adafruit_NeoPixel_numLEDs;   ///< Number of RGB LEDs in strip
uint16_t Adafruit_NeoPixel_numBytes;  ///< Size of 'pixels' buffer below
static int16_t pin;        ///< Output pin number (-1 if not yet set)
uint8_t Adafruit_NeoPixel_brightness; ///< Strip brightness 0-255 (stored as +1)
uint8_t *Adafruit_NeoPixel_pixels;    ///< Holds LED color values (3 or 4 bytes each)
uint8_t Adafruit_NeoPixel_rOffset;    ///< Red index within each 3- or 4-byte pixel
uint8_t Adafruit_NeoPixel_gOffset;    ///< Index of green byte
uint8_t Adafruit_NeoPixel_bOffset;    ///< Index of blue byte
uint8_t Adafruit_NeoPixel_wOffset;    ///< Index of white (==rOffset if no white)
static uint32_t endTime;   ///< Latch timing reference

/*!
  @brief   NeoPixel constructor when length, pin and pixel type are known
           at compile-time.
  @param   length  Number of NeoPixels in strand.
  @param   new_pixels  buffer of pixels.
  @param   t  Pixel type -- add together NEO_* constants defined in
              Adafruit_NeoPixel.h, for example NEO_GRB+NEO_KHZ800 for
              NeoPixels expecting an 800 KHz (vs 400 KHz) data stream
              with color bytes expressed in green, red, blue order per
              pixel.
  @return  Adafruit_NeoPixel object. Call the begin() function before use.
*/
void Adafruit_NeoPixel_init(uint8_t *new_pixels, uint16_t length, neoPixelType t) {
  begun = false;
  Adafruit_NeoPixel_brightness = 0;
  endTime = 0 ;
  Adafruit_NeoPixel_wOffset = (t >> 6) & 0b11; // See notes in header file
  Adafruit_NeoPixel_rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  Adafruit_NeoPixel_gOffset = (t >> 2) & 0b11;
  Adafruit_NeoPixel_bOffset = t & 0b11;
  Adafruit_NeoPixel_numBytes = length * ((Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) ? 3 : 4);
  Adafruit_NeoPixel_pixels = new_pixels;
  Adafruit_NeoPixel_memset(Adafruit_NeoPixel_pixels, 0, Adafruit_NeoPixel_numBytes);
  Adafruit_NeoPixel_numLEDs = length;
}

/*!
  @brief   Deallocate Adafruit_NeoPixel object, set data pin back to INPUT.
*/
void Adafruit_NeoPixel_deinit() {
#if SUPPORT_MALLOC
  free(pixels);
#endif
}

bool Adafruit_NeoPixel_canShow(void) {
  // It's normal and possible for endTime to exceed micros() if the
  // 32-bit clock counter has rolled over (about every 70 minutes).
  // Since both are uint32_t, a negative delta correctly maps back to
  // positive space, and it would seem like the subtraction below would
  // suffice. But a problem arises if code invokes show() very
  // infrequently...the micros() counter may roll over MULTIPLE times in
  // that interval, the delta calculation is no longer correct and the
  // next update may stall for a very long time. The check below resets
  // the latch counter if a rollover has occurred. This can cause an
  // extra delay of up to 300 microseconds in the rare case where a
  // show() call happens precisely around the rollover, but that's
  // neither likely nor especially harmful, vs. other code that might
  // stall for 30+ minutes, or having to document and frequently remind
  // and/or provide tech support explaining an unintuitive need for
  // show() calls at least once an hour.
  uint32_t now = micros();
  if (endTime > now) {
    endTime = now;
  }
  return (now - endTime) >= 300L;
}

uint8_t *Adafruit_NeoPixel_getPixels(void) { 
  return Adafruit_NeoPixel_pixels;
}

uint16_t Adafruit_NeoPixel_numPixels(void) { 
  return Adafruit_NeoPixel_numLEDs; 
}

static uint8_t Adafruit_NeoPixel_sine8(uint8_t x) {
  return _NeoPixelSineTable[x]; // 0-255 in, 0-255 out
}

static uint8_t Adafruit_NeoPixel_gamma8(uint8_t x) {
  return _NeoPixelGammaTable[x]; // 0-255 in, 0-255 out
}

static uint32_t Adafruit_NeoPixel_Color_rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static uint32_t Adafruit_NeoPixel_Color_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void Adafruit_NeoPixel_port_init() {
  // #error "delete this comment and write your port init code."
}

/*!
  @brief   Configure NeoPixel pin for output.
*/
void Adafruit_NeoPixel_begin(void) {
  Adafruit_NeoPixel_port_init();
  begun = true;
}

/*!
  @brief   Transmit pixel data in RAM to NeoPixels.
  @note    On most architectures, interrupts are temporarily disabled in
           order to achieve the correct NeoPixel signal timing. This means
           that the Arduino millis() and micros() functions, which require
           interrupts, will lose small intervals of time whenever this
           function is called (about 30 microseconds per RGB pixel, 40 for
           RGBW pixels). There's no easy fix for this, but a few
           specialized alternative or companion libraries exist that use
           very device-specific peripherals to work around it.
*/
void Adafruit_NeoPixel_show(void) {

  if (!Adafruit_NeoPixel_pixels)
    return;

  // Data latch = 300+ microsecond pause in the output stream. Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed. This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while (!Adafruit_NeoPixel_canShow())
    ;
    // endTime is a private member (rather than global var) so that multiple
    // instances on different pins can be quickly issued in succession (each
    // instance doesn't delay the next).

    // In order to make this code runtime-configurable to work with any pin,
    // SBI/CBI instructions are eschewed in favor of full PORT writes via the
    // OUT or ST instructions. It relies on two facts: that peripheral
    // functions (such as PWM) take precedence on output pins, so our PORT-
    // wide writes won't interfere, and that interrupts are globally disabled
    // while data is being issued to the LEDs, so no other code will be
    // accessing the PORT. The code takes an initial 'snapshot' of the PORT
    // state, computes 'pin high' and 'pin low' values, and writes these back
    // to the PORT register as needed.

    // NRF52 may use PWM + DMA (if available), may not need to disable interrupt
    // ESP32 may not disable interrupts because espShow() uses RMT which tries to acquire locks
#if INTERRUPT_WHEN_SHOWING
  noInterrupts(); // Need 100% focus on instruction timing
#endif


#if INTERRUPT_WHEN_SHOWING
  interrupts();
#endif

  endTime = micros(); // Save EOD time for latch on next call
}

/*!
  @brief   Set a pixel's color using separate red, green and blue
           components. If using RGBW pixels, white will be set to 0.
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
*/
void Adafruit_NeoPixel_setPixelColor_nrgb(uint16_t n, uint8_t r, uint8_t g,
                                      uint8_t b) {

  if (n < Adafruit_NeoPixel_numLEDs) {
    if (Adafruit_NeoPixel_brightness) { // See notes in setBrightness()
      r = (r * Adafruit_NeoPixel_brightness) >> 8;
      g = (g * Adafruit_NeoPixel_brightness) >> 8;
      b = (b * Adafruit_NeoPixel_brightness) >> 8;
    }
    uint8_t *p;
    if (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) { // Is an RGB-type strip
      p = &Adafruit_NeoPixel_pixels[n * 3];     // 3 bytes per pixel
    } else {                  // Is a WRGB-type strip
      p = &Adafruit_NeoPixel_pixels[n * 4];     // 4 bytes per pixel
      p[Adafruit_NeoPixel_wOffset] = 0;         // But only R,G,B passed -- set W to 0
    }
    p[Adafruit_NeoPixel_rOffset] = r; // R,G,B always stored
    p[Adafruit_NeoPixel_gOffset] = g;
    p[Adafruit_NeoPixel_bOffset] = b;
  }
}

/*!
  @brief   Set a pixel's color using separate red, green, blue and white
           components (for RGBW NeoPixels only).
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
  @param   w  White brightness, 0 = minimum (off), 255 = maximum, ignored
              if using RGB pixels.
*/
void Adafruit_NeoPixel_setPixelColor_nrgbw(uint16_t n, uint8_t r, uint8_t g,
                                      uint8_t b, uint8_t w) {

  if (n < Adafruit_NeoPixel_numLEDs) {
    if (Adafruit_NeoPixel_brightness) { // See notes in setBrightness()
      r = (r * Adafruit_NeoPixel_brightness) >> 8;
      g = (g * Adafruit_NeoPixel_brightness) >> 8;
      b = (b * Adafruit_NeoPixel_brightness) >> 8;
      w = (w * Adafruit_NeoPixel_brightness) >> 8;
    }
    uint8_t *p;
    if (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) { // Is an RGB-type strip
      p = &Adafruit_NeoPixel_pixels[n * 3];     // 3 bytes per pixel (ignore W)
    } else {                  // Is a WRGB-type strip
      p = &Adafruit_NeoPixel_pixels[n * 4];     // 4 bytes per pixel
      p[Adafruit_NeoPixel_wOffset] = w;         // Store W
    }
    p[Adafruit_NeoPixel_rOffset] = r; // Store R,G,B
    p[Adafruit_NeoPixel_gOffset] = g;
    p[Adafruit_NeoPixel_bOffset] = b;
  }
}

/*!
  @brief   Set a pixel's color using a 32-bit 'packed' RGB or RGBW value.
  @param   n  Pixel index, starting from 0.
  @param   c  32-bit color value. Most significant byte is white (for RGBW
              pixels) or ignored (for RGB pixels), next is red, then green,
              and least significant byte is blue.
*/
void Adafruit_NeoPixel_setPixelColor_nc(uint16_t n, uint32_t c) {
  if (n < Adafruit_NeoPixel_numLEDs) {
    uint8_t *p, r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
    if (Adafruit_NeoPixel_brightness) { // See notes in setBrightness()
      r = (r * Adafruit_NeoPixel_brightness) >> 8;
      g = (g * Adafruit_NeoPixel_brightness) >> 8;
      b = (b * Adafruit_NeoPixel_brightness) >> 8;
    }
    if (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) {
      p = &Adafruit_NeoPixel_pixels[n * 3];
    } else {
      p = &Adafruit_NeoPixel_pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[Adafruit_NeoPixel_wOffset] = Adafruit_NeoPixel_brightness ? ((w * Adafruit_NeoPixel_brightness) >> 8) : w;
    }
    p[Adafruit_NeoPixel_rOffset] = r;
    p[Adafruit_NeoPixel_gOffset] = g;
    p[Adafruit_NeoPixel_bOffset] = b;
  }
}

/*!
  @brief   Fill all or part of the NeoPixel strip with a color.
  @param   c      32-bit color value. Most significant byte is white (for
                  RGBW pixels) or ignored (for RGB pixels), next is red,
                  then green, and least significant byte is blue. If all
                  arguments are unspecified, this will be 0 (off).
  @param   first  Index of first pixel to fill, starting from 0. Must be
                  in-bounds, no clipping is performed. 0 if unspecified.
  @param   count  Number of pixels to fill, as a positive value. Passing
                  0 or leaving unspecified will fill to end of strip.
*/
void Adafruit_NeoPixel_fill(uint32_t c, uint16_t first, uint16_t count) {
  uint16_t i, end;

  if (first >= Adafruit_NeoPixel_numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if (count == 0) {
    // Fill to end of strip
    end = Adafruit_NeoPixel_numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if (end > Adafruit_NeoPixel_numLEDs)
      end = Adafruit_NeoPixel_numLEDs;
  }

  for (i = first; i < end; i++) {
    Adafruit_NeoPixel_setPixelColor_nc(i, c);
  }
}

/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB with the most significant byte set to 0 -- the
           white element of WRGB pixels is NOT utilized. Result is linearly
           but not perceptually correct, so you may want to pass the result
           through the gamma32() function (or your own gamma-correction
           operation) else colors may appear washed out. This is not done
           automatically by this function because coders may desire a more
           refined gamma-correction function than the simplified
           one-size-fits-all operation of gamma32(). Diffusing the LEDs also
           really seems to help when using low-saturation colors.
*/
uint32_t Adafruit_NeoPixel_ColorHSV(uint16_t hue, uint8_t sat, uint8_t val) {

  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if (hue < 510) { // Red to Green-1
    b = 0;
    if (hue < 255) { //   Red to Yellow-1
      r = 255;
      g = hue;       //     g = 0 to 254
    } else {         //   Yellow to Green-1
      r = 510 - hue; //     r = 255 to 1
      g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    r = 0;
    if (hue < 765) { //   Green to Cyan-1
      g = 255;
      b = hue - 510;  //     b = 0 to 254
    } else {          //   Cyan to Blue-1
      g = 1020 - hue; //     g = 255 to 1
      b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    g = 0;
    if (hue < 1275) { //   Blue to Magenta-1
      r = hue - 1020; //     r = 0 to 254
      b = 255;
    } else { //   Magenta to Red-1
      r = 255;
      b = 1530 - hue; //     b = 255 to 1
    }
  } else { // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
  uint16_t s1 = 1 + sat;  // 1 to 256; same reason
  uint8_t s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
         (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
         (((((b * s1) >> 8) + s2) * v1) >> 8);
}

/*!
  @brief   Query the color of a previously-set pixel.
  @param   n  Index of pixel to read (0 = first).
  @return  'Packed' 32-bit RGB or WRGB value. Most significant byte is white
           (for RGBW pixels) or 0 (for RGB pixels), next is red, then green,
           and least significant byte is blue.
  @note    If the strip brightness has been changed from the default value
           of 255, the color read from a pixel may not exactly match what
           was previously written with one of the setPixelColor() functions.
           This gets more pronounced at lower brightness levels.
*/
uint32_t Adafruit_NeoPixel_getPixelColor(uint16_t n) {
  if (n >= Adafruit_NeoPixel_numLEDs)
    return 0; // Out of bounds, return no color.

  uint8_t *p;

  if (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) { // Is RGB-type device
    p = &Adafruit_NeoPixel_pixels[n * 3];
    if (Adafruit_NeoPixel_brightness) {
      // Stored color was decimated by setBrightness(). Returned value
      // attempts to scale back to an approximation of the original 24-bit
      // value used when setting the pixel color, but there will always be
      // some error -- those bits are simply gone. Issue is most
      // pronounced at low brightness levels.
      return (((uint32_t)(p[Adafruit_NeoPixel_rOffset] << 8) / Adafruit_NeoPixel_brightness) << 16) |
             (((uint32_t)(p[Adafruit_NeoPixel_gOffset] << 8) / Adafruit_NeoPixel_brightness) << 8) |
             ((uint32_t)(p[Adafruit_NeoPixel_bOffset] << 8) / Adafruit_NeoPixel_brightness);
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[Adafruit_NeoPixel_rOffset] << 16) | ((uint32_t)p[Adafruit_NeoPixel_gOffset] << 8) |
             (uint32_t)p[Adafruit_NeoPixel_bOffset];
    }
  } else { // Is RGBW-type device
    p = &Adafruit_NeoPixel_pixels[n * 4];
    if (Adafruit_NeoPixel_brightness) { // Return scaled color
      return (((uint32_t)(p[Adafruit_NeoPixel_wOffset] << 8) / Adafruit_NeoPixel_brightness) << 24) |
             (((uint32_t)(p[Adafruit_NeoPixel_rOffset] << 8) / Adafruit_NeoPixel_brightness) << 16) |
             (((uint32_t)(p[Adafruit_NeoPixel_gOffset] << 8) / Adafruit_NeoPixel_brightness) << 8) |
             ((uint32_t)(p[Adafruit_NeoPixel_bOffset] << 8) / Adafruit_NeoPixel_brightness);
    } else { // Return raw color
      return ((uint32_t)p[Adafruit_NeoPixel_wOffset] << 24) | ((uint32_t)p[Adafruit_NeoPixel_rOffset] << 16) |
             ((uint32_t)p[Adafruit_NeoPixel_gOffset] << 8) | (uint32_t)p[Adafruit_NeoPixel_bOffset];
    }
  }
}

/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. The next call to show() will
           refresh the LEDs at this level.
  @param   b  Brightness setting, 0=minimum (off), 255=brightest.
  @note    This was intended for one-time use in one's setup() function,
           not as an animation effect in itself. Because of the way this
           library "pre-multiplies" LED colors in RAM, changing the
           brightness is often a "lossy" operation -- what you write to
           pixels isn't necessary the same as what you'll read back.
           Repeated brightness changes using this function exacerbate the
           problem. Smart programs therefore treat the strip as a
           write-only resource, maintaining their own state to render each
           frame of an animation, not relying on read-modify-write.
*/
void Adafruit_NeoPixel_setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB. 'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if (newBrightness != Adafruit_NeoPixel_brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM,
    // This process is potentially "lossy," especially when increasing
    // brightness. The tight timing in the WS2811/WS2812 code means there
    // aren't enough free cycles to perform this scaling on the fly as data
    // is issued. So we make a pass through the existing color data in RAM
    // and scale it (subsequent graphics commands also work at this
    // brightness level). If there's a significant step up in brightness,
    // the limited number of steps (quantization) in the old data will be
    // quite visible in the re-scaled version. For a non-destructive
    // change, you'll need to re-render the full strip data. C'est la vie.
    uint8_t c, *ptr = Adafruit_NeoPixel_pixels,
               oldBrightness = Adafruit_NeoPixel_brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if (oldBrightness == 0)
      scale = 0; // Avoid /0
    else if (b == 255)
      scale = 65535 / oldBrightness;
    else
      scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for (uint16_t i = 0; i < Adafruit_NeoPixel_numBytes; i++) {
      c = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    Adafruit_NeoPixel_brightness = newBrightness;
  }
}

/*!
  @brief   Retrieve the last-set brightness value for the strip.
  @return  Brightness value: 0 = minimum (off), 255 = maximum.
*/
uint8_t Adafruit_NeoPixel_getBrightness(void) { return Adafruit_NeoPixel_brightness - 1; }

/*!
  @brief   Fill the whole NeoPixel strip with 0 / black / off.
*/
void Adafruit_NeoPixel_clear(void) { Adafruit_NeoPixel_memset(Adafruit_NeoPixel_pixels, 0, Adafruit_NeoPixel_numBytes); }

// A 32-bit variant of gamma8() that applies the same function
// to all components of a packed RGB or WRGB value.
static uint32_t Adafruit_NeoPixel_gamma32(uint32_t x) {
  uint8_t *y = (uint8_t *)&x;
  // All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
  // to avoid a bunch of shifting and masking that would be necessary for
  // properly handling different endianisms (and each byte is a fairly
  // trivial operation, so it might not even be wasting cycles vs a check
  // and branch for the RGB case). In theory this might cause trouble *if*
  // someone's storing information in the unused most significant byte
  // of an RGB value, but this seems exceedingly rare and if it's
  // encountered in reality they can mask values going in or coming out.
  for (uint8_t i = 0; i < 4; i++)
    y[i] = Adafruit_NeoPixel_gamma8(y[i]);
  return x; // Packed 32-bit return
}

/*!
  @brief   Fill NeoPixel strip with one or more cycles of hues.
           Everyone loves the rainbow swirl so much, now it's canon!
  @param   first_hue   Hue of first pixel, 0-65535, representing one full
                       cycle of the color wheel. Each subsequent pixel will
                       be offset to complete one or more cycles over the
                       length of the strip.
  @param   reps        Number of cycles of the color wheel over the length
                       of the strip. Default is 1. Negative values can be
                       used to reverse the hue order.
  @param   saturation  Saturation (optional), 0-255 = gray to pure hue,
                       default = 255.
  @param   brightness  Brightness/value (optional), 0-255 = off to max,
                       default = 255. This is distinct and in combination
                       with any configured global strip brightness.
  @param   gammify     If true (default), apply gamma correction to colors
                       for better appearance.
*/
void Adafruit_NeoPixel_rainbow(uint16_t first_hue, int8_t reps,
  uint8_t saturation, uint8_t brightness, bool gammify) {
  for (uint16_t i=0; i<Adafruit_NeoPixel_numLEDs; i++) {
    uint16_t hue = first_hue + (i * reps * 65536) / Adafruit_NeoPixel_numLEDs;
    uint32_t color = Adafruit_NeoPixel_ColorHSV(hue, saturation, brightness);
    if (gammify) color = Adafruit_NeoPixel_gamma32(color);
    Adafruit_NeoPixel_setPixelColor_nc(i, color);
  }
}

/*!
  @brief  Convert pixel color order from string (e.g. "BGR") to NeoPixel
          color order constant (e.g. NEO_BGR). This may be helpful for code
          that initializes from text configuration rather than compile-time
          constants.
  @param   v  Input string. Should be reasonably sanitized (a 3- or 4-
              character NUL-terminated string) or undefined behavior may
              result (output is still a valid NeoPixel order constant, but
              might not present as expected). Garbage in, garbage out.
  @return  One of the NeoPixel color order constants (e.g. NEO_BGR).
           NEO_KHZ400 or NEO_KHZ800 bits are not included, nor needed (all
           NeoPixels actually support 800 KHz it's been found, and this is
           the default state if no KHZ bits set).
  @note    This function is declared static in the class so it can be called
           without a NeoPixel object (since it's not likely been declared
           in the code yet). Use Adafruit_NeoPixel::str2order().
*/
char Adafruit_NeoPixel_tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        // 如果是大写字母，则转换为小写字母
        return c + ('a' - 'A');
    } else {
        // 如果不是大写字母，则保持原样
        return c;
    }
}
static neoPixelType Adafruit_NeoPixel_str2order(const char *v) {
  int8_t r = 0, g = 0, b = 0, w = -1;
  if (v) {
    char c;
    for (uint8_t i=0; ((c = Adafruit_NeoPixel_tolower(v[i]))); i++) {
      if (c == 'r') r = i;
      else if (c == 'g') g = i;
      else if (c == 'b') b = i;
      else if (c == 'w') w = i;
    }
    r &= 3;
  }
  if (w < 0) w = r; // If 'w' not specified, duplicate r bits
  return (w << 6) | (r << 4) | ((g & 3) << 2) | (b & 3);
} 