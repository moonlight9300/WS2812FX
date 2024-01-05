#include "ws2812_user_def.h"
#include "WS2812FX.h"

WS2812_User_Ctl_Hdl ws2812_hdl;

void user_ws2812_init(WS2812_User_Ctl_Hdl ws2812_hdl)
{
    WS2812FX_init(ws2812_hdl.user_millis)
}


