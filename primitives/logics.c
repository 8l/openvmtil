
#include "../includes/cfrtil.h"

// ( b top | b < top ) dpans

void
CfrTil_LessThan ( ) // <
{
    if ( CompileMode )
    {
        Compile_LessThan ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b < top ) ) ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () < _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_LessThanOrEqual ( ) // <
{
    if ( CompileMode )
    {
        Compile_LessThanOrEqual ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b <= top ) ) ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () < _DataStack_GetTop ( Dsp ) ) ;
    }
}

// ( b top | b > top ) dpans

void
CfrTil_GreaterThan ( ) // >
{
    if ( CompileMode )
    {
        Compile_GreaterThan ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b > top ) ) ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () > _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_GreaterThanOrEqual ( ) // >
{
    if ( CompileMode )
    {
        Compile_GreaterThanOrEqual ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b >= top ) ) ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () > _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_Equals ( ) // == 
{
    if ( CompileMode )
    {
        Compile_Equals ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b == top ) ) ;
        //_DataStack_SetTop ( _DataStack_Pop () == _DataStack_GetTop () ) ;
    }
}

void
CfrTil_DoesNotEqual ( ) // !=
{
    if ( CompileMode )
    {
        Compile_DoesNotEqual ( _Context_->Compiler0 ) ;
    }
    else
    {
        int32 top, b ;
        top = _DataStack_Pop ( ) ;
        b = _DataStack_GetTop ( ) ;
        _DataStack_SetTop ( ( int32 ) ( b != top ) ) ;
        //_DataStack_SetTop ( _DataStack_Pop () > _DataStack_GetTop () ) ;
    }
}

void
CfrTil_LogicalNot ( ) // not
{
    if ( CompileMode )
    {
        Compile_LogicalNot ( _Context_->Compiler0 ) ;
    }
    else
    {
        TOS = ! ( TOS ) ;
    }
}

void
CfrTil_LogicalAnd ( ) // and
{
    if ( CompileMode )
    {
        Compile_Logical_X ( _Context_->Compiler0, AND ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] && TOS ;
        Dsp -- ;
    }
}

void
CfrTil_LogicalOr ( ) // or
{
    if ( CompileMode )
    {
        Compile_Logical_X ( _Context_->Compiler0, OR ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] || TOS ;
        Dsp -- ;
    }
}

void
CfrTil_LogicalXor ( ) // xor
{
    if ( CompileMode )
    {
        Compile_Logical_X ( _Context_->Compiler0, XOR ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] ^ TOS ;
        Dsp -- ;
    }
}

