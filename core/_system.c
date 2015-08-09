
#include "../includes/cfrtil.h"

void
_System_TimerInit ( System * system, int32 i )
{
    clock_gettime ( CLOCK_REALTIME, &system->Timers [ i ] ) ;
    //clock_gettime ( CLOCK_MONOTONIC, &system->Timers [ i ] ) ;
}

void
_System_Time ( System * system, uint timer, char * format, byte * toString )
{
    if ( timer < 8 )
    {
        __time_t seconds, seconds2 ;
        long int nseconds, nseconds2 ;
        seconds = system->Timers [ timer ].tv_sec ;
        nseconds = system->Timers [ timer ].tv_nsec ;
        _System_TimerInit ( system, timer ) ;
        seconds2 = system->Timers [ timer ].tv_sec ;
        nseconds2 = system->Timers [ timer ].tv_nsec ;
        //clock_settime ( CLOCK_MONOTONIC, &system->Timers [ timer ] ) ;
        //clock_settime ( CLOCK_REALTIME, &system->Timers [ timer ] ) ;
        if ( nseconds > nseconds2 )
        {
            seconds2 -- ;
            nseconds2 += 1000000000 ;
        }
        sprintf ( ( char* ) toString, format, seconds2 - seconds, nseconds2 - nseconds ) ;
    }
    else Throw ( ( byte* ) "Error: timer index must be less than 8", QUIT ) ;
}

void
System_Time ( System * system, uint timer, char * string, int tflag )
{
    byte buffer [ 64 ] ;
    _System_Time ( system, timer, ( char* ) "%ld.%09ld", buffer ) ;
    if ( tflag && ( _Q_->Verbosity ) ) Printf ( ( byte* ) "\n%s [ %d ] : elapsed time = %s seconds", string, timer, buffer ) ;
    //if ( tflag ) Printf ( ( byte* ) "\n%s [ %d ] : elapsed time = %s seconds", string, timer, buffer ) ;
}

void
System_InitTime ( System * system )
{
    int i ;
    for ( i = 0 ; i < 8 ; i ++ )
    {
        _System_TimerInit ( system, i ) ;
    }
}

void
System_RunInit ( System * system )
{
    ConserveNewlines  ;
    if ( _Q_->Signal > QUIT )
    {
        CfrTil_DataStack_Init () ;
    }
    else
    {
        CfrTil_SetDspFromStackPointer ( _CfrTil_ ) ; // for preserving stack on "quit"
        if ( DataStack_Underflow ( ) || ( DataStack_Overflow ( ) ) )
        {
            CfrTil_PrintDataStack ( ) ;
        }
    }
}

void
_System_Copy ( System * system, System * system0, int32 type )
{
    memcpy ( system, system0, sizeof (System ) ) ;
}

System *
System_Copy ( System * system0, int32 type )
{
    System * system = ( System * ) _Mem_Allocate ( sizeof ( System ), type ) ;
    _System_Copy ( system, system0, type ) ;
    return system ;
}

#if 0
void
System_Delete ( System * system )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) system ) ;
}
#endif

void
_System_Init ( System * system )
{
    system->NumberBase = 10 ;
    system->BigNumPrecision = 16 ;
    //system->ExceptionFlag = 0 ;
    system->IncludeFileStackNumber = 0 ;
}

void
System_Init ( System * system )
{
    system->State = 0 ;
    System_SetState ( system, ADD_READLINE_TO_HISTORY | ALWAYS_ABORT_ON_EXCEPION, true ) ;
    _System_Init ( system ) ;
}

System *
System_New ( int32 type )
{
    System * system = ( System * ) _Mem_Allocate ( sizeof (System ), type ) ;
    System_Init ( system ) ;
    System_InitTime ( system ) ;
    return system ;
}

// ( address number -- )

