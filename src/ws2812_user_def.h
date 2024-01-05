#ifndef _WX2812_USER_DEF_H_
#define _WX2812_USER_DEF_H_

#define DEFAULT_BRIGHTNESS (uint8_t)50
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint16_t)1000

typedef struct _WS2812_User_Ctl_Hdl
{
    uint32_t (*user_millis)(void);
}WS2812_User_Ctl_Hdl;




#endif