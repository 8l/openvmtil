#include "../includes/cfrtil.h"

void
CfrTil_Peek ( ) // @
{
    if ( CompileMode )
    {
        Compile_Peek ( _Context_->Compiler0, DSP ) ;
    }
    else
    {
        TOS = *(int32*) TOS ;
    }
}

void
CfrTil_Poke ( ) // =
{
    if ( CompileMode )
    {
        Compile_Poke ( _Context_->Compiler0, DSP ) ;
    }
    else
    {
        * (int32*) ( Dsp [ -1 ] ) = TOS ;
        Dsp -= 2 ;
    }
}

void
CfrTil_AtEqual ( ) // !
{
    if ( CompileMode )
    {
        Compile_AtEqual ( DSP ) ;
    }
    else
    {
        *(int32*) Dsp [ -1 ] = *(int32*) TOS ;
        Dsp -= 2 ;
    }
}

// ( n addr -- ) // (*addr) = n

void
CfrTil_Store ( ) // !
{
    if ( CompileMode )
    {
        Compile_Store ( _Context_->Compiler0, DSP ) ;
    }
    else
    {
        * (int32*) ( TOS ) = Dsp [ -1 ] ;
        Dsp -= 2 ;
    }
}

