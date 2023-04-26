#ifndef _DASH_LOGO_H
#define _DASH_LOGO_H

#include <Arduino.h>

#define DASH_MINI_LOGO_MIME         "image/svg+xml"
#define DASH_MINI_LOGO_GZIPPED      1
#define DASH_MINI_LOGO_WIDTH        32
#define DASH_MINI_LOGO_HEIGHT       41

extern const uint8_t DASH_MINI_LOGO[215];


#define DASH_LARGE_LOGO_MIME        "image/png"
#define DASH_LARGE_LOGO_GZIPPED     1
#define DASH_LARGE_LOGO_WIDTH       162
#define DASH_LARGE_LOGO_HEIGHT      28

extern const uint8_t DASH_LARGE_LOGO[7655];

#endif