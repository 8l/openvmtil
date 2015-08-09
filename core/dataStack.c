
#include "../includes/cfrtil.h"

// these functions are part of the C vm and can't be compiled
// ! they are should only be called in C functions !

int32
_DataStack_Pop ( )
{
#if NO_GLOBAL_REGISTERS  // NGR NO_GLOBAL_REGISTERS
    NGR_ESI_To_Dsp ;
    int32 top = Dsp [ 0 ] ;  Dsp -- ;
    NGR_Dsp_To_ESI ;
    return top ;
#else
    return Dsp -- [ 0 ] ;
#endif    
}

void
_DataStack_Push ( int32 value )
{
    NGR_ESI_To_Dsp ;
    _Push ( value ) ;
    NGR_Dsp_To_ESI ;
}

void
_DataStack_Dup ( )
{
    NGR_ESI_To_Dsp ;
    _Dup ( ) ;
    NGR_Dsp_To_ESI ;
}

void
_DataStack_DropN ( int n )
{
    NGR_ESI_To_Dsp ;
    Dsp -= n ;
    NGR_Dsp_To_ESI ;
}

void
_DataStack_Drop ( )
{
    NGR_ESI_To_Dsp ;
    _Drop ( ) ;
    NGR_Dsp_To_ESI ;
}

int32
DataStack_Overflow ( )
{
    return ( Dsp >= _DataStack_->StackMax ) ;
}

int32
DataStack_Underflow ( )
{
    return ( Dsp < _DataStack_->InitialTosPointer ) ;
}

void
DataStack_Check ( )
{
    if ( ( Dsp < _DataStack_->InitialTosPointer ) || ( Dsp >= _DataStack_->StackMax ) )
    {
        CfrTil_Exception ( STACK_OVERFLOW, QUIT ) ;
    }
}

int32
DataStack_Depth ( )
{
    if ( _DataStack_ )
    {
        _DataStack_->StackPointer = Dsp ;
        return Stack_Depth ( _DataStack_ ) ;
    }
    return 0 ;
}

// safe form with stack checking

int32
DataStack_Pop ( )
{
    _DataStack_->StackPointer = Dsp ;
    _DataStack_Pop ( ) ;
    int32 top = Stack_Pop_WithExceptionOnEmpty ( _DataStack_ ) ;
    return top ;
}

