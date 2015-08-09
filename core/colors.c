#include "../includes/cfrtil.h"

#if 0

void
_OpenVmTil_ColorsInit ( OpenVmTil * ovt )
{
    Colors_Init6 ( &_Q_->Alert, 0, 0, 0, 255, 0, 0 ) ;
    Colors_Init6 ( &_Q_->Debug, 255, 255, 255, 0, 0, 255 ) ;
    Colors_Init6 ( &_Q_->Default, 0, 0, 0, 64, 64, 64 ) ;
    Colors_Init6 ( &_Q_->Notice, 255, 255, 255, 0, 255, 0 ) ; // grey, cyan
    Colors_Init6 ( &_Q_->User, 0, 0, 0, 255, 255, 255 ) ; // grey, cyan

    _Colors_Init2 ( &_Q_->Alert, Color_Black, Color_Red ) ;
    _Colors_Init2 ( &_Q_->Debug, Color_Black, Color_Blue ) ;
    _Colors_Init2 ( &_Q_->Default, Color_Default, Color_Default ) ;
    _Colors_Init2 ( &_Q_->Notice, Color_Green, Color_Black ) ;
    _Colors_Init2 ( &_Q_->User, Color_Black, Color_White ) ;
}
#else

void
_OpenVmTil_ColorsInit ( OpenVmTil * ovt )
{
    Colors_Init6 ( &ovt->Alert, 255, 0, 0, 0, 0, 0 ) ;
    Colors_Init6 ( &ovt->Debug, 0, 0, 255, 0, 0, 0 ) ;
    Colors_Init6 ( &ovt->Default, 0, 255, 0, 0, 0, 0 ) ;
    Colors_Init6 ( &ovt->Notice, 127, 127, 0, 255, 255, 255 ) ;
    Colors_Init6 ( &ovt->User, 0, 255, 0, 255, 255, 255 ) ;

    _Colors_Init2 ( &ovt->Alert, Color_Red, Color_Default ) ;
    _Colors_Init2 ( &ovt->Debug, Color_Blue, Color_Default ) ;
    _Colors_Init2 ( &ovt->Default, Color_Default, Color_Default ) ;
    _Colors_Init2 ( &ovt->Notice, Color_Yellow, Color_Black ) ;
    _Colors_Init2 ( &ovt->User, Color_Green, Color_Default ) ;
    ovt->Current = & ovt->Default ;
}
#endif

void
Console ( )
{
    _DataStack_Push ( ( int32 ) & _Q_->Console ) ;
}

void
_ShowRGB ( int fr, int fg, int fb, int br, int bg, int bb )
{
    if ( _Q_->Console == 1 ) _Printf ( "%c[38;2;%d;%d;%dm %c[48;2;%d;%d;%dm", ESC, fr, fg, fb, ESC, br, bg, bb ) ; // Konsole mode
    else if ( _Q_->Console == 2 ) _Printf ( "%c[38;5;%d;%d;%dm%c[48;5;%d;%d;%dm", ESC, fr, fg, fb, ESC, br, bg, bb ) ; // 'xterm' mode
}

void
_String_ShowRGB ( char * buf, int fr, int fg, int fb, int br, int bg, int bb )
{
    if ( _Q_->Console == 1 ) sprintf ( buf, "%c[38;2;%d;%d;%dm %c[48;2;%d;%d;%dm", ESC, fr, fg, fb, ESC, br, bg, bb ) ; // Konsole mode
    else if ( _Q_->Console == 2 ) sprintf ( buf, "%c[38;5;%d;%d;%dm%c[48;5;%d;%d;%dm", ESC, fr, fg, fb, ESC, br, bg, bb ) ; // 'xterm' mode
}

void
_ShowRgbColors ( Colors *c )
{
    Colors_Setup6 ( c, fr, fg, fb, br, bg, bb ) ;
    _ShowRGB ( fr, fg, fb, br, bg, bb ) ;
}

void
_String_ShowRgbColors ( char * buf, Colors *c )
{
    Colors_Setup6 ( c, fr, fg, fb, br, bg, bb ) ;
    _String_ShowRGB ( buf, fr, fg, fb, br, bg, bb ) ;
}

void
Colors_Init6 ( Colors *c, int fr, int fg, int fb, int br, int bg, int bb )
{
    Colors_Get6 ( c, fr, fg, fb, br, bg, bb ) ;
}

void
ShowColors ( Colors * c )
{
    if ( _Q_->Console ) _ShowRgbColors ( c ) ;
    else _ShowColors ( c->ics_IntColors.Fg, c->ics_IntColors.Bg ) ;
    if ( c ) _Q_->Current = c ;
}

void
String_ShowColors ( char * buf, Colors * c )
{
    if ( _Q_->Console ) _String_ShowRgbColors ( buf, c ) ;
    else _String_ShowColors ( buf, c->ics_IntColors.Fg, c->ics_IntColors.Bg ) ;
    if ( c ) _Q_->Current = c ;
}

void
_Colors_Init2 ( Colors * c, int fg, int bg )
{
    c->ics_IntColors.Fg = fg, c->ics_IntColors.Bg = bg ;
}

void
_CfrTil_SetRGBColor ( Colors * c )
{
    int fr, fg, fb, br, bg, bb ;
    bb = _DataStack_Pop ( ) ;
    bg = _DataStack_Pop ( ) ;
    br = _DataStack_Pop ( ) ;
    fb = _DataStack_Pop ( ) ;
    fg = _DataStack_Pop ( ) ;
    fr = _DataStack_Pop ( ) ;
    Colors_Get6 ( c, fr, fg, fb, br, bg, bb ) ;
}

void
_CfrTil_SetColors ( Colors * c )
{
    int fg, bg ;
    bg = _DataStack_Pop ( ) ;
    fg = _DataStack_Pop ( ) ;
    _Colors_Init2 ( c, fg, bg ) ;
}

void
Ovt_UserColors ( )
{
    ShowColors ( &_Q_->User ) ;
}

void
Ovt_AlertColors ( )
{
    ShowColors ( &_Q_->Alert ) ;
}

void
Ovt_DefaultColors ( )
{
    ShowColors ( &_Q_->Default ) ;
}

void
Ovt_DebugColors ( )
{
    ShowColors ( &_Q_->Debug ) ;
}

void
Ovt_NoticeColors ( )
{
    ShowColors ( &_Q_->Notice ) ;
}

void
CfrTil_SetDefaultColors ( )
{
    _CfrTil_SetColors ( &_Q_->Default ) ;
}

void
CfrTil_SetAlertColors ( )
{
    _CfrTil_SetColors ( &_Q_->Alert ) ;
}

void
CfrTil_SetDebugColors ( )
{
    _CfrTil_SetColors ( &_Q_->Debug ) ;
}

void
CfrTil_SetUserColors ( )
{
    _CfrTil_SetColors ( &_Q_->User ) ;
}

void
CfrTil_SetNoticeColors ( )
{
    _CfrTil_SetColors ( &_Q_->Notice ) ;
}

void
CfrTil_SetDefaultRGB ( )
{
    _CfrTil_SetRGBColor ( &_Q_->Default ) ;
}

void
CfrTil_SetUserRGB ( )
{
    _CfrTil_SetRGBColor ( &_Q_->User ) ;
}

void
CfrTil_SetAlertRGB ( )
{
    _CfrTil_SetRGBColor ( &_Q_->Alert ) ;
}

void
CfrTil_SetDebugRGB ( )
{
    _CfrTil_SetRGBColor ( &_Q_->Debug ) ;
}

void
CfrTil_SetNoticeRGB ( )
{
    _CfrTil_SetRGBColor ( &_Q_->Notice ) ;
}


