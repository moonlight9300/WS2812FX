/*
  modes_esp.h - WS2812FX header file for ESP8266 and ESP32 microprocessors

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

  2022-03-23   Separated from the original WS2812FX.h file
*/
#ifndef mode_defines_h
#define mode_defines_h

#include "Adafruit_NeoPixel_defines.h"
#include "WS2812FX.h"

#define MODE_COUNT 80

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_INV           4 
#define FX_MODE_COLOR_WIPE_REV           5
#define FX_MODE_COLOR_WIPE_REV_INV       6
#define FX_MODE_COLOR_WIPE_RANDOM        7
#define FX_MODE_RANDOM_COLOR             8
#define FX_MODE_SINGLE_DYNAMIC           9
#define FX_MODE_MULTI_DYNAMIC           10
#define FX_MODE_RAINBOW                 11
#define FX_MODE_RAINBOW_CYCLE           12
#define FX_MODE_SCAN                    13
#define FX_MODE_DUAL_SCAN               14
#define FX_MODE_FADE                    15
#define FX_MODE_THEATER_CHASE           16
#define FX_MODE_THEATER_CHASE_RAINBOW   17
#define FX_MODE_RUNNING_LIGHTS          18
#define FX_MODE_TWINKLE                 19
#define FX_MODE_TWINKLE_RANDOM          20
#define FX_MODE_TWINKLE_FADE            21
#define FX_MODE_TWINKLE_FADE_RANDOM     22
#define FX_MODE_SPARKLE                 23
#define FX_MODE_FLASH_SPARKLE           24
#define FX_MODE_HYPER_SPARKLE           25
#define FX_MODE_STROBE                  26
#define FX_MODE_STROBE_RAINBOW          27
#define FX_MODE_MULTI_STROBE            28
#define FX_MODE_BLINK_RAINBOW           29
#define FX_MODE_CHASE_WHITE             30
#define FX_MODE_CHASE_COLOR             31
#define FX_MODE_CHASE_RANDOM            32
#define FX_MODE_CHASE_RAINBOW           33
#define FX_MODE_CHASE_FLASH             34
#define FX_MODE_CHASE_FLASH_RANDOM      35
#define FX_MODE_CHASE_RAINBOW_WHITE     36
#define FX_MODE_CHASE_BLACKOUT          37
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  38
#define FX_MODE_COLOR_SWEEP_RANDOM      39
#define FX_MODE_RUNNING_COLOR           40
#define FX_MODE_RUNNING_RED_BLUE        41
#define FX_MODE_RUNNING_RANDOM          42
#define FX_MODE_LARSON_SCANNER          43
#define FX_MODE_COMET                   44
#define FX_MODE_FIREWORKS               45
#define FX_MODE_FIREWORKS_RANDOM        46
#define FX_MODE_MERRY_CHRISTMAS         47
#define FX_MODE_FIRE_FLICKER            48
#define FX_MODE_FIRE_FLICKER_SOFT       49
#define FX_MODE_FIRE_FLICKER_INTENSE    50
#define FX_MODE_CIRCUS_COMBUSTUS        51
#define FX_MODE_HALLOWEEN               52
#define FX_MODE_BICOLOR_CHASE           53
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TWINKLEFOX              55
#define FX_MODE_RAIN                    56
#define FX_MODE_BLOCK_DISSOLVE          57
#define FX_MODE_ICU                     58
#define FX_MODE_DUAL_LARSON             59
#define FX_MODE_RUNNING_RANDOM2         60
#define FX_MODE_FILLER_UP               61
#define FX_MODE_RAINBOW_LARSON          62
#define FX_MODE_RAINBOW_FIREWORKS       63
#define FX_MODE_TRIFADE                 64
#define FX_MODE_VU_METER                65
#define FX_MODE_HEARTBEAT               66
#define FX_MODE_BITS                    67
#define FX_MODE_MULTI_COMET             68
#define FX_MODE_FLIPBOOK                69
#define FX_MODE_POPCORN                 70
#define FX_MODE_OSCILLATOR              71
#define FX_MODE_CUSTOM                  72  // keep this for backward compatiblity
#define FX_MODE_CUSTOM_0                72  // custom modes need to go at the end
#define FX_MODE_CUSTOM_1                73
#define FX_MODE_CUSTOM_2                74
#define FX_MODE_CUSTOM_3                75
#define FX_MODE_CUSTOM_4                76
#define FX_MODE_CUSTOM_5                77
#define FX_MODE_CUSTOM_6                78
#define FX_MODE_CUSTOM_7                79

// mode categories
// const char cat_simple[]  PROGMEM = "Simple";
// const char cat_wipe[]    PROGMEM = "Wipe";
// const char cat_sweep[]   PROGMEM = "Sweep";
// const char cat_special[] PROGMEM = "Special";
// const char cat_custom[]  PROGMEM = "Custom";

// create GLOBAL names to allow WS2812FX to compile with sketches and other libs
// that store strings in PROGMEM (get rid of the "section type conflict with __c"
// errors once and for all. Amen.)
const char name_0[] = "Static";
const char name_1[] = "Blink";
const char name_2[] = "Breath";
const char name_3[] = "Color Wipe";
const char name_4[] = "Color Wipe Inverse";
const char name_5[] = "Color Wipe Reverse";
const char name_6[] = "Color Wipe Reverse Inverse";
const char name_7[] = "Color Wipe Random";
const char name_8[] = "Random Color";
const char name_9[] = "Single Dynamic";
const char name_10[] = "Multi Dynamic";
const char name_11[] = "Rainbow";
const char name_12[] = "Rainbow Cycle";
const char name_13[] = "Scan";
const char name_14[] = "Dual Scan";
const char name_15[] = "Fade";
const char name_16[] = "Theater Chase";
const char name_17[] = "Theater Chase Rainbow";
const char name_18[] = "Running Lights";
const char name_19[] = "Twinkle";
const char name_20[] = "Twinkle Random";
const char name_21[] = "Twinkle Fade";
const char name_22[] = "Twinkle Fade Random";
const char name_23[] = "Sparkle";
const char name_24[] = "Flash Sparkle";
const char name_25[] = "Hyper Sparkle";
const char name_26[] = "Strobe";
const char name_27[] = "Strobe Rainbow";
const char name_28[] = "Multi Strobe";
const char name_29[] = "Blink Rainbow";
const char name_30[] = "Chase White";
const char name_31[] = "Chase Color";
const char name_32[] = "Chase Random";
const char name_33[] = "Chase Rainbow";
const char name_34[] = "Chase Flash";
const char name_35[] = "Chase Flash Random";
const char name_36[] = "Chase Rainbow White";
const char name_37[] = "Chase Blackout";
const char name_38[] = "Chase Blackout Rainbow";
const char name_39[] = "Color Sweep Random";
const char name_40[] = "Running Color";
const char name_41[] = "Running Red Blue";
const char name_42[] = "Running Random";
const char name_43[] = "Larson Scanner";
const char name_44[] = "Comet";
const char name_45[] = "Fireworks";
const char name_46[] = "Fireworks Random";
const char name_47[] = "Merry Christmas";
const char name_48[] = "Fire Flicker";
const char name_49[] = "Fire Flicker (soft)";
const char name_50[] = "Fire Flicker (intense)";
const char name_51[] = "Circus Combustus";
const char name_52[] = "Halloween";
const char name_53[] = "Bicolor Chase";
const char name_54[] = "Tricolor Chase";
const char name_55[] = "TwinkleFOX";
const char name_56[] = "Rain";
const char name_57[] = "Block Dissolve";
const char name_58[] = "ICU";
const char name_59[] = "Dual Larson";
const char name_60[] = "Running Random2";
const char name_61[] = "Filler Up";
const char name_62[] = "Rainbow Larson";
const char name_63[] = "Rainbow Fireworks";
const char name_64[] = "Trifade";
const char name_65[] = "VU Meter";
const char name_66[] = "Heartbeat";
const char name_67[] = "Bits";
const char name_68[] = "Multi Comet";
const char name_69[] = "Flipbook";
const char name_70[] = "Popcorn";
const char name_71[] = "Oscillator";
const char name_72[] = "Custom 0"; // custom modes need to go at the end
const char name_73[] = "Custom 1";
const char name_74[] = "Custom 2";
const char name_75[] = "Custom 3";
const char name_76[] = "Custom 4";
const char name_77[] = "Custom 5";
const char name_78[] = "Custom 6";
const char name_79[] = "Custom 7";

// define static array of member function pointers.
// make sure the order of the _modes array elements matches the FX_MODE_* values
static uint16_t (*_modes[])() = {
  WS2812FX_mode_static,
  WS2812FX_mode_blink,
  WS2812FX_mode_breath,
  WS2812FX_mode_color_wipe,
  WS2812FX_mode_color_wipe_inv,
  WS2812FX_mode_color_wipe_rev,
  WS2812FX_mode_color_wipe_rev_inv,
  WS2812FX_mode_color_wipe_random,
  WS2812FX_mode_random_color,
  WS2812FX_mode_single_dynamic,
  WS2812FX_mode_multi_dynamic,
  WS2812FX_mode_rainbow,
  WS2812FX_mode_rainbow_cycle,
  WS2812FX_mode_scan,
  WS2812FX_mode_dual_scan,
  WS2812FX_mode_fade,
  WS2812FX_mode_theater_chase,
  WS2812FX_mode_theater_chase_rainbow,
  WS2812FX_mode_running_lights,
  WS2812FX_mode_twinkle,
  WS2812FX_mode_twinkle_random,
  WS2812FX_mode_twinkle_fade,
  WS2812FX_mode_twinkle_fade_random,
  WS2812FX_mode_sparkle,
  WS2812FX_mode_flash_sparkle,
  WS2812FX_mode_hyper_sparkle,
  WS2812FX_mode_strobe,
  WS2812FX_mode_strobe_rainbow,
  WS2812FX_mode_multi_strobe,
  WS2812FX_mode_blink_rainbow,
  WS2812FX_mode_chase_white,
  WS2812FX_mode_chase_color,
  WS2812FX_mode_chase_random,
  WS2812FX_mode_chase_rainbow,
  WS2812FX_mode_chase_flash,
  WS2812FX_mode_chase_flash_random,
  WS2812FX_mode_chase_rainbow_white,
  WS2812FX_mode_chase_blackout,
  WS2812FX_mode_chase_blackout_rainbow,
  WS2812FX_mode_color_sweep_random,
  WS2812FX_mode_running_color,
  WS2812FX_mode_running_red_blue,
  WS2812FX_mode_running_random,
  WS2812FX_mode_larson_scanner,
  WS2812FX_mode_comet,
  WS2812FX_mode_fireworks,
  WS2812FX_mode_fireworks_random,
  WS2812FX_mode_merry_christmas,
  WS2812FX_mode_fire_flicker,
  WS2812FX_mode_fire_flicker_soft,
  WS2812FX_mode_fire_flicker_intense,
  WS2812FX_mode_circus_combustus,
  WS2812FX_mode_halloween,
  WS2812FX_mode_bicolor_chase,
  WS2812FX_mode_tricolor_chase,
  WS2812FX_mode_twinkleFOX,
  WS2812FX_mode_rain,
  WS2812FX_mode_block_dissolve,
  WS2812FX_mode_icu,
  WS2812FX_mode_dual_larson,
  WS2812FX_mode_running_random2,
  WS2812FX_mode_filler_up,
  WS2812FX_mode_rainbow_larson,
  WS2812FX_mode_rainbow_fireworks,
  WS2812FX_mode_trifade,
  WS2812FX_mode_vu_meter,
  WS2812FX_mode_heartbeat,
  WS2812FX_mode_bits,
  WS2812FX_mode_multi_comet,
  WS2812FX_mode_flipbook,
  WS2812FX_mode_popcorn,
  WS2812FX_mode_oscillator,
  WS2812FX_mode_custom_0,
  WS2812FX_mode_custom_1,
  WS2812FX_mode_custom_2,
  WS2812FX_mode_custom_3,
  WS2812FX_mode_custom_4,
  WS2812FX_mode_custom_5,
  WS2812FX_mode_custom_6,
  WS2812FX_mode_custom_7
};
#endif
