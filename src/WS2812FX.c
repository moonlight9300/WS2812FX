/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library

  NOTES
    * Uses the Adafruit NeoPixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel



  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.


  CHANGELOG

  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-26   implemented segment and reverse features
  2017-11-16   changed speed calc, reduced memory footprint
  2018-02-24   added hooks for user created custom effects
*/

#include "WS2812FX.h"
#include "WS2812FX_modes_defines.h"

uint16_t _rand16seed;

void (*customShow)(void) = NULL;

bool
  _running,
  _triggered;

WS2812FX_Segment* _segments;                 // array of segments (20 bytes per element)
WS2812FX_Segment_runtime* _segment_runtimes; // array of segment runtimes (16 bytes per element)
uint8_t* _active_segments;          // array of active segments (1 bytes per element)

uint8_t _segments_len = 0;          // size of _segments array
uint8_t _active_segments_len = 0;   // size of _segments_runtime and _active_segments arrays
uint8_t _num_segments = 0;          // number of configured segments in the _segments array

WS2812FX_Segment* _seg;                      // currently active segment (20 bytes)
WS2812FX_Segment_runtime* _seg_rt;           // currently active segment runtime (16 bytes)

uint16_t _seg_len;                  // num LEDs in the currently active segment

uint32_t (*WS2812FX_millis)(void);

void WS2812FX_init(uint16_t num_leds, neoPixelType type,
                    uint8_t max_num_segments,// uint8_t max_num_segments=MAX_NUM_SEGMENTS
                    uint8_t max_num_active_segments) {// max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS
  uint8_t new_pixls[128];// TODO
  Adafruit_NeoPixel_init(new_pixls, num_leds, type);

  WS2812FX_resetSegmentRuntimes();
  Adafruit_NeoPixel_begin();
  Adafruit_NeoPixel_brightness = DEFAULT_BRIGHTNESS + 1; // Adafruit_NeoPixel internally offsets brightness by 1
  _running = false;

  _segments_len = max_num_segments;
  _active_segments_len = max_num_active_segments;

  // create all the segment arrays and init to zeros
  WS2812FX_Segment _segments[MAX_NUM_SEGMENTS];
  uint8_t _active_segments[MAX_NUM_ACTIVE_SEGMENTS];
  WS2812FX_Segment_runtime _segment_runtimes[MAX_NUM_ACTIVE_SEGMENTS];

  // init segment pointers
  _seg     = _segments;
  _seg_rt  = _segment_runtimes;

  WS2812FX_resetSegments();
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(0, 0, num_leds - 1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

// void WS2812FX_timer() {
//   for (int j=0; j < 1000; j++) {
//     uint16_t delay = (MODE_PTR(_seg->mode))();
//   }
// }

bool WS2812FX_service() {
  bool doShow = false;
  if(_running || _triggered) {
    unsigned long now = WS2812FX_millis(); // Be aware, millis() rolls over every 49 days
    for(uint8_t i=0; i < _active_segments_len; i++) {
      if(_active_segments[i] != INACTIVE_SEGMENT) {
        _seg     = &_segments[_active_segments[i]];
        _seg_len = (uint16_t)(_seg->stop - _seg->start + 1);
        _seg_rt  = &_segment_runtimes[i];
        CLR_FRAME_CYCLE;
        if(now > _seg_rt->next_time || _triggered) {
          SET_FRAME;
          doShow = true;
          uint16_t delay = _modes[_seg->mode]();
          _seg_rt->next_time = now + max(delay, SPEED_MIN);
          _seg_rt->counter_mode_call++;
        }
      }
    }
    if(doShow) {
      WS2812FX_show();
    }
    _triggered = false;
  }
  return doShow;
}

// overload setPixelColor() functions so we can use gamma correction
// (see https://learn.adafruit.com/led-tricks-gamma-correction/the-issue)
void WS2812FX_setPixelColor_nc(uint16_t n, uint32_t c) {
  uint8_t w = (c >> 24) & 0xFF;
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >>  8) & 0xFF;
  uint8_t b =  c        & 0xFF;
  WS2812FX_setPixelColor_nrgbw(n, r, g, b, w);
}

void WS2812FX_setPixelColor_nrgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  WS2812FX_setPixelColor_nrgbw(n, r, g, b, 0);
}

void WS2812FX_setPixelColor_nrgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  if(IS_GAMMA) {
    Adafruit_NeoPixel_setPixelColor_nrgbw(n, Adafruit_NeoPixel_gamma8(r), Adafruit_NeoPixel_gamma8(g), Adafruit_NeoPixel_gamma8(b), Adafruit_NeoPixel_gamma8(w));
  } else {
    Adafruit_NeoPixel_setPixelColor_nrgbw(n, r, g, b, w);
  }
}

// custom setPixelColor() function that bypasses the Adafruit_Neopixel global brightness rigmarole
void WS2812FX_setRawPixelColor(uint16_t n, uint32_t c) {
  if (n < Adafruit_NeoPixel_numLEDs) {
    uint8_t *p = (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) ? &Adafruit_NeoPixel_pixels[n * 3] : &Adafruit_NeoPixel_pixels[n * 4]; 
    uint8_t w = (uint8_t)(c >> 24), r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;

    p[Adafruit_NeoPixel_wOffset] = w;
    p[Adafruit_NeoPixel_rOffset] = r;
    p[Adafruit_NeoPixel_gOffset] = g;
    p[Adafruit_NeoPixel_bOffset] = b;
  }
}

// custom getPixelColor() function that bypasses the Adafruit_Neopixel global brightness rigmarole
uint32_t WS2812FX_getRawPixelColor(uint16_t n) {
  if (n >= Adafruit_NeoPixel_numLEDs) return 0; // Out of bounds, return no color.

  if(Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) { // RGB
    uint8_t *p = &Adafruit_NeoPixel_pixels[n * 3]; 
    return ((uint32_t)p[Adafruit_NeoPixel_rOffset] << 16) | ((uint32_t)p[Adafruit_NeoPixel_gOffset] << 8) | (uint32_t)p[Adafruit_NeoPixel_bOffset];
  } else { // RGBW
    uint8_t *p = &Adafruit_NeoPixel_pixels[n * 4];
    return ((uint32_t)p[Adafruit_NeoPixel_wOffset] << 24) | ((uint32_t)p[Adafruit_NeoPixel_rOffset] << 16) | ((uint32_t)p[Adafruit_NeoPixel_gOffset] << 8) | (uint32_t)p[Adafruit_NeoPixel_bOffset];
  }
}

void WS2812FX_copyPixels(uint16_t dest, uint16_t src, uint16_t count) {
  uint8_t *pixels = Adafruit_NeoPixel_getPixels();
  uint8_t bytesPerPixel = WS2812FX_getNumBytesPerPixel(); // 3=RGB, 4=RGBW

  Adafruit_NeoPixel_memmove(pixels + (dest * bytesPerPixel), pixels + (src * bytesPerPixel), count * bytesPerPixel);
}

// overload show() functions so we can use custom show()
void WS2812FX_show(void) {
  customShow == NULL ? Adafruit_NeoPixel_show() : customShow();
}

void WS2812FX_start() {
  WS2812FX_resetSegmentRuntimes();
  _running = true;
}

void WS2812FX_stop() {
  _running = false;
  WS2812FX_strip_off();
}

void WS2812FX_pause() {
  _running = false;
}

void WS2812FX_resume() {
  _running = true;
}

void WS2812FX_trigger() {
  _triggered = true;
}

void WS2812FX_setMode_m(uint8_t m) {
  WS2812FX_setMode_seg_m(0, m);
}

void WS2812FX_setMode_seg_m(uint8_t seg, uint8_t m) {
  WS2812FX_resetSegmentRuntime(seg);
  _segments[seg].mode = Adafruit_NeoPixel_constrain(m, 0, MODE_COUNT - 1);
}

void WS2812FX_setOptions(uint8_t seg, uint8_t o) {
  _segments[seg].options = o;
}

void WS2812FX_setSpeed_s(uint16_t s) {
  WS2812FX_setSpeed_seg_s(0, s);
}

void WS2812FX_setSpeed_seg_s(uint8_t seg, uint16_t s) {
  _segments[seg].speed = Adafruit_NeoPixel_constrain(s, SPEED_MIN, SPEED_MAX);
}

void WS2812FX_increaseSpeed(uint8_t s) {
  uint16_t newSpeed = Adafruit_NeoPixel_constrain(_seg->speed + s, SPEED_MIN, SPEED_MAX);
  WS2812FX_setSpeed_s(newSpeed);
}

void WS2812FX_decreaseSpeed(uint8_t s) {
  uint16_t newSpeed = Adafruit_NeoPixel_constrain(_seg->speed - s, SPEED_MIN, SPEED_MAX);
  WS2812FX_setSpeed_s(newSpeed);
}

void WS2812FX_setColor_rgb(uint8_t r, uint8_t g, uint8_t b) {
  WS2812FX_setColor_c(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX_setColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  WS2812FX_setColor_c((((uint32_t)w << 24)| ((uint32_t)r << 16) | ((uint32_t)g << 8)| ((uint32_t)b)));
}

void WS2812FX_setColor_c(uint32_t c) {
  WS2812FX_setColor_seg_c(0, c);
}

void WS2812FX_setColor_seg_c(uint8_t seg, uint32_t c) {
  _segments[seg].colors[0] = c;
}

void WS2812FX_setColors(uint8_t seg, uint32_t* c) {
  for(uint8_t i=0; i<MAX_NUM_COLORS; i++) {
    _segments[seg].colors[i] = c[i];
  }
}

void WS2812FX_setBrightness(uint8_t b) {
//b = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  Adafruit_NeoPixel_setBrightness(b);
  WS2812FX_show();
}

void WS2812FX_increaseBrightness(uint8_t s) {
//s = constrain(getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  WS2812FX_setBrightness(Adafruit_NeoPixel_getBrightness() + s);
}

void WS2812FX_decreaseBrightness(uint8_t s) {
//s = constrain(getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  WS2812FX_setBrightness(Adafruit_NeoPixel_getBrightness() - s);
}


void WS2812FX_increaseLength(uint16_t s) {
  uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
  WS2812FX_setLength(seglen + s);
}

void WS2812FX_decreaseLength(uint16_t s) {
  uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
  WS2812FX_fill(BLACK, _segments[0].start, seglen);
  WS2812FX_show();

  if (s < seglen) WS2812FX_setLength(seglen - s);
}

bool WS2812FX_isRunning() {
  return _running;
}

bool WS2812FX_isTriggered() {
  return _triggered;
}

bool WS2812FX_isFrame(void) {
  return WS2812FX_isFrame_seg(0);
}

bool WS2812FX_isFrame_seg(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return false; // segment not active
  return (_segment_runtimes[ptr - _active_segments].aux_param2 & FRAME);
}

bool WS2812FX_isCycle() {
  return WS2812FX_isCycle_seg(0);
}

bool WS2812FX_isCycle_seg(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return false; // segment not active
  return (_segment_runtimes[ptr - _active_segments].aux_param2 & CYCLE);
}

void WS2812FX_setCycle() {
  SET_CYCLE;
}

uint8_t WS2812FX_getMode(void) {
  return WS2812FX_getMode(0);
}

uint8_t WS2812FX_getMode(uint8_t seg) {
  return _segments[seg].mode;
}

uint16_t WS2812FX_getSpeed(void) {
  return WS2812FX_getSpeed(0);
}

uint16_t WS2812FX_getSpeed(uint8_t seg) {
  return _segments[seg].speed;
}

uint8_t WS2812FX_getOptions(uint8_t seg) {
  return _segments[seg].options;
}

uint16_t WS2812FX_getLength(void) {
  return Adafruit_NeoPixel_numPixels();
}

uint16_t WS2812FX_getNumBytes(void) {
  return Adafruit_NeoPixel_numBytes;
}

uint8_t WS2812FX_getNumBytesPerPixel(void) {
  return (Adafruit_NeoPixel_wOffset == Adafruit_NeoPixel_rOffset) ? 3 : 4; // 3=RGB, 4=RGBW
}

uint8_t WS2812FX_getModeCount(void) {
  return MODE_COUNT;
}

uint8_t WS2812FX_getNumSegments(void) {
  return _num_segments;
}

void WS2812FX_setNumSegments(uint8_t n) {
  _num_segments = n;
}

uint32_t WS2812FX_getColor(void) {
  return WS2812FX_getColor(0);
}

uint32_t WS2812FX_getColor(uint8_t seg) {
  return _segments[seg].colors[0];
}

uint32_t* WS2812FX_getColors(uint8_t seg) {
  return _segments[seg].colors;
}

WS2812FX_Segment* WS2812FX_getSegment(void) {
  return _seg;
}

WS2812FX_Segment* WS2812FX_getSegment(uint8_t seg) {
  return &_segments[seg];
}

WS2812FX_Segment* WS2812FX_getSegments(void) {
  return _segments;
}

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntime(void) {
  return _seg_rt;
}

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntime(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return NULL; // segment not active
  return &_segment_runtimes[ptr - _active_segments];
}

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntimes(void) {
  return _segment_runtimes;
}

uint8_t* WS2812FX_getActiveSegments(void) {
  return _active_segments;
}

void WS2812FX_setSegment() {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(0, 0, WS2812FX_getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n(uint8_t n) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, 0, WS2812FX_getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start(uint8_t n, uint16_t start) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, WS2812FX_getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop(uint8_t n, uint16_t start, uint16_t stop) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, stop, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, stop, mode, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode_color(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, stop, mode, color, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode_color_speed(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, stop, mode, color, speed, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode_color_speed_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse) {
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(n, start, stop, mode, color, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX_setSegment_n_start_stop_mode_color_speed_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options) {
  uint32_t colors[] = {color, 0, 0};
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(n, start, stop, mode, colors, speed, options);
}

void WS2812FX_setSegment_n_start_stop_mode_colors(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[]) {
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(n, start, stop, mode, colors, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode_colors_speed(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed) {
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(n, start, stop, mode, colors, speed, NO_OPTIONS);
}

void WS2812FX_setSegment_n_start_stop_mode_colors_speed_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse) {
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(n, start, stop, mode, colors, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options) {
  if(n < _segments_len) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].options = options;

    WS2812FX_setColors_seg_pc(n, (uint32_t*)colors);

    if(n < _active_segments_len) WS2812FX_addActiveSegment(n);
  }
}

void WS2812FX_setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed) {
  WS2812FX_setIdleSegment_options(n, start, stop, mode, color, speed, NO_OPTIONS);
}

void WS2812FX_setIdleSegment_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options) {
  uint32_t colors[] = {color, 0, 0};
  WS2812FX_setIdleSegment_colors_options(n, start, stop, mode, colors, speed, options);
}

void WS2812FX_setIdleSegment_colors_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options) {
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(n, start, stop, mode, colors, speed, options);
  if(n < _active_segments_len) WS2812FX_removeActiveSegment(n);;
}

void WS2812FX_addActiveSegment(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr != NULL) return; // segment already active
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == INACTIVE_SEGMENT) {
      _active_segments[i] = seg;
      WS2812FX_resetSegmentRuntime(seg);
      break;
    }
  }
}

void WS2812FX_removeActiveSegment(uint8_t seg) {
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == seg) {
      _active_segments[i] = INACTIVE_SEGMENT;
    }
  }
}

void WS2812FX_swapActiveSegment(uint8_t oldSeg, uint8_t newSeg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, newSeg, _active_segments_len);
  if(ptr != NULL) return; // if newSeg is already active, don't swap
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == oldSeg) {
      _active_segments[i] = newSeg;

      // reset all runtime parameters EXCEPT next_time,
      // allowing the current animation frame to complete
      WS2812FX_Segment_runtime seg_rt = _segment_runtimes[i];
      seg_rt.counter_mode_step = 0;
      seg_rt.counter_mode_call = 0;
      seg_rt.aux_param = 0;
      seg_rt.aux_param2 = 0;
      seg_rt.aux_param3 = 0;
      break;
    }
  }
}

bool WS2812FX_isActiveSegment(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr != NULL) return true;
  return false;
}

void WS2812FX_resetSegments() {
  WS2812FX_resetSegmentRuntimes();
  Adafruit_NeoPixel_memset(_segments, 0, _segments_len * sizeof(WS2812FX_Segment));
  Adafruit_NeoPixel_memset(_active_segments, INACTIVE_SEGMENT, _active_segments_len);
  _num_segments = 0;
}

void WS2812FX_resetSegmentRuntimes() {
  for(uint8_t i=0; i<_segments_len; i++) {
    WS2812FX_resetSegmentRuntime(i);
  };
}

void WS2812FX_resetSegmentRuntime(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)Adafruit_NeoPixel_memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return; // segment not active
  _segment_runtimes[seg].next_time = 0;
  _segment_runtimes[seg].counter_mode_step = 0;
  _segment_runtimes[seg].counter_mode_call = 0;
  _segment_runtimes[seg].aux_param = 0;
  _segment_runtimes[seg].aux_param2 = 0;
  _segment_runtimes[seg].aux_param3 = 0;
  // don't reset any external data source
}

/*
 * Turns everything off. Doh.
 */
void WS2812FX_strip_off() {
  Adafruit_NeoPixel_clear();
  WS2812FX_show();
}

/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX_color_wheel(uint8_t pos) {
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}

/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX_get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t d = 0;

  while(d < 42) {
    r = WS2812FX_random8();
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }

  return r;
}

void WS2812FX_setRandomSeed(uint16_t seed) {
  _rand16seed = seed;
}

// fast 8-bit random number generator shamelessly borrowed from FastLED
uint8_t WS2812FX_random8() {
  _rand16seed = (_rand16seed * 2053) + 13849;
  return (uint8_t)((_rand16seed + (_rand16seed >> 8)) & 0xFF);
}

// note random8(lim) generates numbers in the range 0 to (lim -1)
uint8_t WS2812FX_random8_lim(uint8_t lim) {
  uint8_t r = WS2812FX_random8();
  r = ((uint16_t)r * lim) >> 8;
  return r;
}

uint8_t WS2812FX_random(uint8_t min_value, uint8_t max_value) {
    uint8_t range = max_value - min_value;
    return (WS2812FX_random8() % range) + min_value;
}

uint16_t WS2812FX_random16() {
  return (uint16_t)WS2812FX_random8() * 256 + WS2812FX_random8();
}

// note random16(lim) generates numbers in the range 0 to (lim - 1)
uint16_t WS2812FX_random16_lim(uint16_t lim) {
  uint16_t r = WS2812FX_random16();
  r = ((uint32_t)r * lim) >> 16;
  return r;
}

// Return the sum of all LED intensities (can be used for
// rudimentary power calculations)
uint32_t WS2812FX_intensitySum() {
  uint8_t *pixels = Adafruit_NeoPixel_getPixels();
  uint32_t sum = 0;
  for(uint16_t i=0; i < Adafruit_NeoPixel_numBytes; i++) {
    sum+= pixels[i];
  }
  return sum;
}

// Return the sum of each color's intensity. Note, the order of
// intensities in the returned array depends on the type of WS2812
// LEDs you have. NEO_GRB LEDs will return an array with entries
// in a different order then NEO_RGB LEDs.
uint32_t* WS2812FX_intensitySums() {
  static uint32_t intensities[] = { 0, 0, 0, 0 };
  Adafruit_NeoPixel_memset(intensities, 0, sizeof(intensities));

  uint8_t *pixels = Adafruit_NeoPixel_getPixels();
  uint8_t bytesPerPixel = WS2812FX_getNumBytesPerPixel(); // 3=RGB, 4=RGBW
  for(uint16_t i=0; i < Adafruit_NeoPixel_numBytes; i += bytesPerPixel) {
    intensities[0] += pixels[i];
    intensities[1] += pixels[i + 1];
    intensities[2] += pixels[i + 2];
    if(bytesPerPixel == 4) intensities[3] += pixels[i + 3]; // for RGBW LEDs
  }
  return intensities;
}


/*
 * Custom show helper
 */
void WS2812FX_setCustomShow(void (*p)()) {
  customShow = p;
}

/*
 * set a segment runtime's external data source
 */
void WS2812FX_setExtDataSrc(uint8_t seg, uint8_t *src, uint8_t cnt) {
  _segment_runtimes[seg].extDataSrc = src;
  _segment_runtimes[seg].extDataCnt = cnt;
}
