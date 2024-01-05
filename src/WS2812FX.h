/*
  WS2812FX.h - Library for WS2812 LED effects.

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
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#include "Adafruit_NeoPixel.h"

#define MAX_MILLIS (0UL - 1UL) /* ULONG_MAX */



#define DEFAULT_COLOR      (uint32_t)0xFF0000
#define DEFAULT_COLORS     { RED, GREEN, BLUE }
#define COLORS(...)        (const uint32_t[]){__VA_ARGS__}

#define SPEED_MIN (uint16_t)10
#define SPEED_MAX (uint16_t)65535

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255

/* each segment uses 36 bytes of SRAM memory, so if you're compile fails
  because of insufficient flash memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS         1
#define MAX_NUM_ACTIVE_SEGMENTS  1
#define INACTIVE_SEGMENT        255 /* max uint_8 */
#define MAX_NUM_COLORS            3 /* number of colors per segment */
#define MAX_CUSTOM_MODES          8

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define GRAY       (uint32_t)0x101010
#define ULTRAWHITE (uint32_t)0xFFFFFFFF
#define DIM(c)     (uint32_t)((c >> 2) & 0x3f3f3f3f) // color at 25% intensity
#define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f) // color at  6% intensity

// segment options
// bit    7: reverse animation
// bits 4-6: fade rate (0-7)
// bit    3: gamma correction
// bits 1-2: size
// bits   0: TBD
#define NO_OPTIONS   (uint8_t)0b00000000
#define REVERSE      (uint8_t)0b10000000
#define IS_REVERSE   ((_seg->options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)0b00010000
#define FADE_FAST    (uint8_t)0b00100000
#define FADE_MEDIUM  (uint8_t)0b00110000
#define FADE_SLOW    (uint8_t)0b01000000
#define FADE_XSLOW   (uint8_t)0b01010000
#define FADE_XXSLOW  (uint8_t)0b01100000
#define FADE_GLACIAL (uint8_t)0b01110000
#define FADE_RATE    ((_seg->options >> 4) & 7)
#define GAMMA        (uint8_t)0b00001000
#define IS_GAMMA     ((_seg->options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)0b00000000
#define SIZE_MEDIUM  (uint8_t)0b00000010
#define SIZE_LARGE   (uint8_t)0b00000100
#define SIZE_XLARGE  (uint8_t)0b00000110
#define SIZE_OPTION  ((_seg->options >> 1) & 3)

// segment runtime options (aux_param2)
#define FRAME           (uint8_t)0b10000000
#define SET_FRAME       (_seg_rt->aux_param2 |=  FRAME)
#define CLR_FRAME       (_seg_rt->aux_param2 &= ~FRAME)
#define CYCLE           (uint8_t)0b01000000
#define SET_CYCLE       (_seg_rt->aux_param2 |=  CYCLE)
#define CLR_CYCLE       (_seg_rt->aux_param2 &= ~CYCLE)
#define CLR_FRAME_CYCLE (_seg_rt->aux_param2 &= ~(FRAME | CYCLE))

// class WS2812FX : public Adafruit_NeoPixel {

  // public:
  //   typedef uint16_t (WS2812FX_*mode_ptr)(void);

// segment parameters
typedef struct WS2812FX_Segment { // 20 bytes
  uint16_t start;
  uint16_t stop;
  uint16_t speed;
  uint8_t  mode;
  uint8_t  options;
  uint32_t colors[MAX_NUM_COLORS];
} WS2812FX_segment;

// segment runtime parameters
typedef struct WS2812FX_Segment_runtime { // 20 bytes for Arduino, 24 bytes for ESP
  unsigned long next_time;
  uint32_t counter_mode_step;
  uint32_t counter_mode_call;
  uint8_t  aux_param;   // auxilary param (usually stores a color_wheel index)
  uint8_t  aux_param2;  // auxilary param (usually stores bitwise options)
  uint16_t aux_param3;  // auxilary param (usually stores a segment index)
  uint8_t* extDataSrc = NULL; // external data array
  uint16_t extDataCnt = 0;    // number of elements in the external data array
} WS2812FX_segment_runtime;

extern WS2812FX_segment* _seg;
extern WS2812FX_segment_runtime* _seg_rt;
extern uint16_t _seg_len;
extern bool _triggered;
extern uint16_t (*customModes[MAX_CUSTOM_MODES])(void);

void
//    timer(void),
  WS2812FX_init(uint16_t num_leds, neoPixelType type,
                    uint8_t max_num_segments,// uint8_t max_num_segments=MAX_NUM_SEGMENTS
                    uint8_t max_num_active_segments),
  WS2812FX_start(void),
  WS2812FX_stop(void),
  WS2812FX_pause(void),
  WS2812FX_resume(void),
  WS2812FX_strip_off(void),
  WS2812FX_fade_out(void),
  WS2812FX_fade_out_targetColor(uint32_t),
  WS2812FX_setMode_m(uint8_t m),
  WS2812FX_setMode_seg_m(uint8_t seg, uint8_t m),
  WS2812FX_setOptions(uint8_t seg, uint8_t o),
  WS2812FX_setCustomMode_vp(uint16_t (*p)()),
  WS2812FX_setCustomShow(void (*p)()),
  WS2812FX_setSpeed_s(uint16_t s),
  WS2812FX_setSpeed_seg_s(uint8_t seg, uint16_t s),
  WS2812FX_increaseSpeed(uint8_t s),
  WS2812FX_decreaseSpeed(uint8_t s),
  WS2812FX_setColor_rgb(uint8_t r, uint8_t g, uint8_t b),
  WS2812FX_setColor_rgbw(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
  WS2812FX_setColor_c(uint32_t c),
  WS2812FX_setColor_seg_c(uint8_t seg, uint32_t c),
  WS2812FX_setColors_seg_pc(uint8_t seg, uint32_t* c),
  WS2812FX_fill(uint32_t c, uint16_t f, uint16_t cnt),
  WS2812FX_setBrightness(uint8_t b),
  WS2812FX_increaseBrightness(uint8_t s),
  WS2812FX_decreaseBrightness(uint8_t s),
  WS2812FX_setLength(uint16_t b),
  WS2812FX_increaseLength(uint16_t s),
  WS2812FX_decreaseLength(uint16_t s),
  WS2812FX_trigger(void),
  WS2812FX_setCycle(void),
  WS2812FX_setNumSegments(uint8_t n),

  WS2812FX_setSegment(),
  WS2812FX_setSegment_n(uint8_t n),
  WS2812FX_setSegment_n_start(uint8_t n, uint16_t start),
  WS2812FX_setSegment_n_start_stop(uint8_t n, uint16_t start, uint16_t stop),
  WS2812FX_setSegment_n_start_stop_mode(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode),
  WS2812FX_setSegment_n_start_stop_mode_color(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color),
  WS2812FX_setSegment_n_start_stop_mode_color_speed(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed),
  WS2812FX_setSegment_n_start_stop_mode_color_speed_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse),
  WS2812FX_setSegment_n_start_stop_mode_color_speed_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options),

  WS2812FX_setSegment_n_start_stop_mode_colors(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[]),
  WS2812FX_setSegment_n_start_stop_mode_colors_speed(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed),
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_reverse(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse),
  WS2812FX_setSegment_n_start_stop_mode_colors_speed_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),

  WS2812FX_setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed),
  WS2812FX_setIdleSegment_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
  WS2812FX_setIdleSegment_colors_options(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
  WS2812FX_addActiveSegment(uint8_t seg),
  WS2812FX_removeActiveSegment(uint8_t seg),
  WS2812FX_swapActiveSegment(uint8_t oldSeg, uint8_t newSeg),

  WS2812FX_resetSegments(void),
  WS2812FX_resetSegmentRuntimes(void),
  WS2812FX_resetSegmentRuntime(uint8_t),
  WS2812FX_setPixelColor_nc(uint16_t n, uint32_t c),
  WS2812FX_setPixelColor_nrgb(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
  WS2812FX_setPixelColor_nrgbw(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
  WS2812FX_setRawPixelColor(uint16_t n, uint32_t c),
  WS2812FX_copyPixels(uint16_t d, uint16_t s, uint16_t c),
  WS2812FX_setPixels(uint16_t, uint8_t*),
  WS2812FX_setRandomSeed(uint16_t),
  WS2812FX_setExtDataSrc(uint8_t seg, uint8_t *src, uint8_t cnt),
  WS2812FX_show(void);

bool
  WS2812FX_service(void),
  WS2812FX_isRunning(void),
  WS2812FX_isTriggered(void),
  WS2812FX_isFrame(void),
  WS2812FX_isFrame_seg(uint8_t),
  WS2812FX_isCycle(void),
  WS2812FX_isCycle_seg(uint8_t),
  WS2812FX_isActiveSegment(uint8_t seg);

uint8_t
  WS2812FX_random8(void),
  WS2812FX_random8_lim(uint8_t),
  WS2812FX_random(uint8_t, uint8_t),
  WS2812FX_getMode(void),
  WS2812FX_getMode_seg(uint8_t),
  WS2812FX_getModeCount(void),
  WS2812FX_setCustomMode_p(uint16_t (*p)()),
  WS2812FX_setCustomMode_index_p(uint8_t i, uint16_t (*p)()),
  WS2812FX_getNumSegments(void),
  WS2812FX_get_random_wheel_index(uint8_t),
  WS2812FX_getOptions(uint8_t),
  WS2812FX_getNumBytesPerPixel(void);

uint16_t
  WS2812FX_random16(void),
  WS2812FX_random16_lim(uint16_t),
  WS2812FX_getSpeed(void),
  WS2812FX_getSpeed_seg(uint8_t),
  WS2812FX_getLength(void),
  WS2812FX_getNumBytes(void);

uint32_t
  WS2812FX_color_wheel(uint8_t),
  WS2812FX_getColor(void),
  WS2812FX_getColor_seg(uint8_t),
  WS2812FX_intensitySum(void);

uint32_t* getColors(uint8_t);
uint32_t* intensitySums(void);
uint8_t*  getActiveSegments(void);
uint8_t*  blend(uint8_t*, uint8_t*, uint8_t*, uint16_t, uint8_t);

WS2812FX_Segment* WS2812FX_getSegment(void);

WS2812FX_Segment* WS2812FX_getSegment_seg(uint8_t);

WS2812FX_Segment* WS2812FX_getSegments(void);

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntime(void);

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntime_seg(uint8_t);

WS2812FX_Segment_runtime* WS2812FX_getSegmentRuntimes(void);

// mode helper functions
uint16_t
  WS2812FX_blink(uint32_t, uint32_t, bool strobe),
  WS2812FX_color_wipe(uint32_t, uint32_t, bool),
  WS2812FX_twinkle(uint32_t, uint32_t),
  WS2812FX_twinkle_fade(uint32_t),
  WS2812FX_sparkle(uint32_t, uint32_t),
  WS2812FX_chase(uint32_t, uint32_t, uint32_t),
  WS2812FX_chase_flash(uint32_t, uint32_t),
  WS2812FX_running(uint32_t, uint32_t),
  WS2812FX_fireworks(uint32_t),
  WS2812FX_fire_flicker(int),
  WS2812FX_tricolor_chase(uint32_t, uint32_t, uint32_t),
  WS2812FX_scan(uint32_t, uint32_t, bool);

uint32_t
  WS2812FX_color_blend(uint32_t, uint32_t, uint8_t),
  WS2812FX_getRawPixelColor(uint16_t n);

// builtin modes
uint16_t
  WS2812FX_mode_static(void),
  WS2812FX_mode_blink(void),
  WS2812FX_mode_blink_rainbow(void),
  WS2812FX_mode_strobe(void),
  WS2812FX_mode_strobe_rainbow(void),
  WS2812FX_mode_color_wipe(void),
  WS2812FX_mode_color_wipe_inv(void),
  WS2812FX_mode_color_wipe_rev(void),
  WS2812FX_mode_color_wipe_rev_inv(void),
  WS2812FX_mode_color_wipe_random(void),
  WS2812FX_mode_color_sweep_random(void),
  WS2812FX_mode_random_color(void),
  WS2812FX_mode_single_dynamic(void),
  WS2812FX_mode_multi_dynamic(void),
  WS2812FX_mode_breath(void),
  WS2812FX_mode_fade(void),
  WS2812FX_mode_scan(void),
  WS2812FX_mode_dual_scan(void),
  WS2812FX_mode_theater_chase(void),
  WS2812FX_mode_theater_chase_rainbow(void),
  WS2812FX_mode_rainbow(void),
  WS2812FX_mode_rainbow_cycle(void),
  WS2812FX_mode_running_lights(void),
  WS2812FX_mode_twinkle(void),
  WS2812FX_mode_twinkle_random(void),
  WS2812FX_mode_twinkle_fade(void),
  WS2812FX_mode_twinkle_fade_random(void),
  WS2812FX_mode_sparkle(void),
  WS2812FX_mode_flash_sparkle(void),
  WS2812FX_mode_hyper_sparkle(void),
  WS2812FX_mode_multi_strobe(void),
  WS2812FX_mode_chase_white(void),
  WS2812FX_mode_chase_color(void),
  WS2812FX_mode_chase_random(void),
  WS2812FX_mode_chase_rainbow(void),
  WS2812FX_mode_chase_flash(void),
  WS2812FX_mode_chase_flash_random(void),
  WS2812FX_mode_chase_rainbow_white(void),
  WS2812FX_mode_chase_blackout(void),
  WS2812FX_mode_chase_blackout_rainbow(void),
  WS2812FX_mode_running_color(void),
  WS2812FX_mode_running_red_blue(void),
  WS2812FX_mode_running_random(void),
  WS2812FX_mode_larson_scanner(void),
  WS2812FX_mode_comet(void),
  WS2812FX_mode_fireworks(void),
  WS2812FX_mode_fireworks_random(void),
  WS2812FX_mode_merry_christmas(void),
  WS2812FX_mode_halloween(void),
  WS2812FX_mode_fire_flicker(void),
  WS2812FX_mode_fire_flicker_soft(void),
  WS2812FX_mode_fire_flicker_intense(void),
  WS2812FX_mode_circus_combustus(void),
  WS2812FX_mode_bicolor_chase(void),
  WS2812FX_mode_tricolor_chase(void),
  WS2812FX_mode_twinkleFOX(void),
  WS2812FX_mode_rain(void),
  WS2812FX_mode_block_dissolve(void),
  WS2812FX_mode_icu(void),
  WS2812FX_mode_dual_larson(void),
  WS2812FX_mode_running_random2(void),
  WS2812FX_mode_filler_up(void),
  WS2812FX_mode_rainbow_larson(void),
  WS2812FX_mode_rainbow_fireworks(void),
  WS2812FX_mode_trifade(void),
  WS2812FX_mode_vu_meter(void),
  WS2812FX_mode_heartbeat(void),
  WS2812FX_mode_bits(void),
  WS2812FX_mode_multi_comet(void),
  WS2812FX_mode_flipbook(void),
  WS2812FX_mode_popcorn(void),
  WS2812FX_mode_oscillator(void),
  WS2812FX_mode_custom_0(void),
  WS2812FX_mode_custom_1(void),
  WS2812FX_mode_custom_2(void),
  WS2812FX_mode_custom_3(void),
  WS2812FX_mode_custom_4(void),
  WS2812FX_mode_custom_5(void),
  WS2812FX_mode_custom_6(void),
  WS2812FX_mode_custom_7(void);

// class WS2812FXT {
//   public:
//     WS2812FXT(uint16_t num_leds, uint8_t pin, neoPixelType type,
//       uint8_t max_num_segments=MAX_NUM_SEGMENTS,
//       uint8_t max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS) {
//         v1 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
//         v2 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
//         dest = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments); 
//     };

//     void init(void) {
//       v1->init();
//       v2->init();
//       v1->setCustomShow([]{ return; });
//       v2->setCustomShow([]{ return; });
//     }

//     void start(void) {
//       v1->start();
//       v2->start();
//     }

//     void service(void) {
//       bool doShow = v1->service() || v2->service();
//       if(doShow) {
//         _show();
//       }
//     }

//     void startTransition(uint16_t duration, bool direction = true) {
//       transitionStartTime = millis();
//       transitionDuration = duration;
//       transitionDirection = direction;
//     }

//   private:
//     void _show(void) {
//       unsigned long now = millis();

//       uint8_t *dest_p = dest->getPixels();
//       uint8_t *vstart_p = transitionDirection ? v1->getPixels() : v2->getPixels();
//       uint8_t *vstop_p  = transitionDirection ? v2->getPixels() : v1->getPixels();
//       uint16_t numBytes = dest->getNumBytes();

//       if(now < transitionStartTime) {
//         memmove(dest_p, vstart_p, numBytes);
//       } else if(now > transitionStartTime + transitionDuration) {
//         memmove(dest_p, vstop_p, numBytes);
//       } else {
//         uint8_t blendAmt = map(now, transitionStartTime, transitionStartTime + transitionDuration, 0, 255);
//         dest->blend(dest_p, vstart_p, vstop_p, numBytes, blendAmt);
//       }

//       dest->Adafruit_NeoPixel_show();
//     }

//   public:
//     WS2812FX* v1 = NULL;
//     WS2812FX* v2 = NULL;
//     WS2812FX* dest = NULL;
//     unsigned long transitionStartTime = MAX_MILLIS;
//     uint16_t transitionDuration = 5000;
//     bool transitionDirection = true;
// };

// data struct used by the flipbook effect
struct Flipbook {
  int8_t   numPages;
  int8_t   numRows;
  int8_t   numCols;
  uint32_t* colors;
};

// data struct used by the popcorn effect
struct Popcorn {
  float position;
  float velocity;
  int32_t color;
};

// data struct used by the oscillator effect
struct Oscillator {
  uint8_t size;
  int16_t pos;
  int8_t  speed;
};

#endif
