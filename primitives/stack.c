#include "../includes/cfrtil.h"

void
CfrTil_Dsp ( )
{
    _DataStack_Push ( ( int32 ) Dsp ) ;
}

#if 1 //use macros
void
Drop ( )
{
    Dsp -- ;
}

void
DropN ( int n )
{
    Dsp -= n ;
}
#endif

void
CfrTil_Drop ( )
{
    if ( CompileMode )
    {
        _Compile_Stack_Drop ( DSP ) ;
    }
    else
    {
        _Drop ( ) ;
    }
}

void
CfrTil_DropN ( )
{
    if ( CompileMode ) _Compile_Stack_DropN ( DSP, _DataStack_Pop ( ) ) ;
    else _DataStack_DropN ( TOS + 1 ) ; 
}

void
_CfrTil_Push ( int32 value )
{
    if ( CompileMode )
    {
        _Compile_Stack_Push ( DSP, value ) ;
    }
    else
    {
        _DataStack_Push ( value ) ;
    }
}

void
CfrTil_Dup ( )
{
    if ( CompileMode )
    {
        _Compile_Stack_Dup ( DSP ) ;
    }
    else
    {
        _DataStack_Dup ( ) ;
    }
}

// result is as if one did n dups in a row 

void
CfrTil_NDup ( )
{
    int32 n = TOS ;
    int32 value = * -- Dsp ; // -1 : n now occupies 1 to be also used slot
    while ( n -- )
    {
        * ++ Dsp = value ;
    }
}

// pick is from stack below top index

void
CfrTil_Pick ( ) // pick
{
    if ( CompileMode )
    {
        _Compile_Stack_Pick ( DSP ) ;
    }
    else
    {
        * Dsp = ( * ( Dsp - * ( Dsp ) - 1 ) ) ;
    }
}

void
CfrTil_Swap ( )
{
    if ( CompileMode )
    {
        _Compile_Stack_Swap ( DSP ) ;
    }
    else
    {
        int32 a = TOS ;
        TOS = Dsp [ - 1 ] ;
        Dsp [ - 1 ] = a ;
    }
}


