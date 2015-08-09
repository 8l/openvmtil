
#include "../includes/cfrtil.h"

void
CfrTil_Kbhit ( void )
{
    _DataStack_Push ( ( int32 ) kbhit ( ) ) ;
}

void
_CfrTil_PrintString ( byte * string ) //  '."'
{
    Printf ( ( byte* ) string ) ;
}

void
CfrTil_PrintString ( )
{
    _CfrTil_PrintString ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_NewLine ( )
{
    //ConserveNewlines ;
    int32 sstate = GetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE ) ;
    AllowNewlines ;
    Printf ( ( byte* ) "\n" ) ;
    _Q_->psi_PrintStateInfo->State = sstate ;
}

void
CfrTil_CarriageReturn ( )
{
    Printf ( ( byte* ) "\r" ) ;
}

void
CfrTil_SPACE ( ) // '.'
{
    _CfrTil_PrintString ( ( byte* ) " " ) ;
}

void
CfrTil_TAB ( ) // '.'
{
    _CfrTil_PrintString ( ( byte* ) "\t" ) ;
}

void
_Print_Binary ( byte* buffer, int32 n )
{
    int i, size = 42 ; // 8 - bits/byte ; 4 - spacing
    byte * ptr = & buffer [ size - 1 ] ;
    buffer [ size ] = 0 ;
    for ( i = 0 ; i < size ; i ++ ) buffer [ i ] = ' ' ;
    for ( i = 0 ; i < 32 ; ptr -- )
    {
        if ( n & ( 1 << i ) )
        {
            *ptr = '1' ;
        }
        else
        {
            *ptr = '0' ;
        }
        i ++ ;
        if ( ! ( i % 4 ) ) ptr -- ;
        if ( ! ( i % 8 ) ) ptr -- ;
        //if ( ! ( i % 16 ) ) ptr --;
    }
}

void
Print_Binary ( int32 n, int32 min, int32 max )
{
    if ( n )
    {
    int32 chars, modulo, rem, adj, size = 42 ;
    byte * endOfBuffer, *ptr, buffer [ size ] ; // 8 - bits/byte ; 4 - spacing
    _Print_Binary ( buffer, n ) ;
    endOfBuffer = & buffer [ size ] ; // 1 : dont count final null
    for ( ptr = buffer ; ( * ptr == '0' ) || ( * ptr == ' ' ) ; ptr ++ ) ;
    chars = endOfBuffer - ptr ;
    if ( chars < 5 ) modulo = 4 ;
    else if ( chars < 10 ) modulo = 9 ;
    else if ( chars < 21 ) modulo = 20 ;
    else modulo = 42 ;
    rem = chars % modulo ;
    if ( rem )
    {
        adj = modulo - rem ;
        ptr -= adj ;
    }
    Printf ( ptr ) ;
    }
    else Printf ( "%d", n ) ;
}

void
PrintfInt ( int32 n )
{
    byte buffer [ 1024 ] ;
    if ( _Context_->System0->NumberBase == 10 ) sprintf ( ( char* ) buffer, INT_FRMT, n ) ;
    else if ( _Context_->System0->NumberBase == 2 )
    {
        Print_Binary ( n, 4, 46 ) ;
        return ;
    }
    else /* if ( _Context->System0->NumberBase == 16 ) */ sprintf ( ( char* ) buffer, UINT_FRMT_0x09, n ) ; // hex
    // ?? any and all other number bases ??
    Printf ( ( byte* ) buffer ) ;
}

void
CfrTil_PrintInt ( )
{
    PrintfInt ( _DataStack_Pop ( ) ) ;
}

void
CfrTil_Emit ( )
{
    int32 c = _DataStack_Pop ( ) ;
    if ( ( c >= 0 ) && ( c < 256 ) ) Printf ( ( byte* ) "%c", c ) ;
    else Printf ( ( byte* ) "%c", ( ( CString ) c )[0] ) ;
}

void
CfrTil_Key ( )
{
#if 0    
    ReadLine_Key ( _Context_->ReadLiner0 ) ;
    _DataStack_Push ( _Context_->ReadLiner0->InputKeyedCharacter ) ;
#else
        _DataStack_Push ( Key() ) ;

#endif    
}

void
CfrTil_Prompt ( )
{
    if ( System_GetState ( _Context_->System0, DO_PROMPT ) ) // && ( ( _Context->OutputLineCharacterNumber == 0 ) || ( _Context->OutputLineCharacterNumber > 3 ) ) )
    {
        _CfrTil_Prompt ( 1 ) ;
    }
}

void
_CfrTil_Ok ( int32 promptFlag )
{
    if ( _Q_->Verbosity > 2 )
    {
        _CfrTil_SystemState_Print ( 0 ) ;
        //CfrTil_MemorySpaceAllocated ( ( byte* ) "SessionObjectsSpace" ) ;
        Printf ( ( byte* ) "\n<Esc> - break, <Ctrl-C> - quit, <Ctrl-D> - restart, \"exit\" - leave.\n ok " ) ;
    }
    _CfrTil_Prompt ( _Q_->Verbosity && promptFlag ) ;
}

void
CfrTil_Ok ( )
{
    _CfrTil_Ok ( 1 ) ;
    //_CfrTil_Prompt ( _Q_->Verbosity && ( ( _Q_->RestartCondition < RESET_ALL ) || _Q_->StartTimes > 1 ) ) ;
}

void
CfrTil_LogOn ( )
{
    _CfrTil_->LogFlag = true ;
}

void
CfrTil_LogAppend ( )
{
    byte * logFilename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_->LogFILE = fopen ( ( char* ) logFilename, "a" ) ;
    CfrTil_LogOn ( ) ;
}

void
CfrTil_LogWrite ( )
{
    byte * logFilename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_->LogFILE = fopen ( ( char* ) logFilename, "w" ) ;
    CfrTil_LogOn ( ) ;
}

void
CfrTil_LogOff ( )
{
    fflush ( _CfrTil_->LogFILE ) ;
    _CfrTil_->LogFlag = false ;
}

