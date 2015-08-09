#include "../includes/cfrtil.h"

// ( n ttt -- )

void
CfrTil_JMP ( ) 
{
    if ( CompileMode )
    {
        _Compile_UninitializedJump ( ) ; // at the end of the 'if block' we need to jmp over the 'else block'
        CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
        _Stack_PointerToJmpOffset_Set ( ) ;
    }
    else
    {
        Error_Abort ( ( byte* ) "\njmp : can only be used in compile mode." ) ;
    }
}

void
CfrTil_Compile_Jcc ( )
{
    int32 ttt = _DataStack_Pop ( ) ;
    int32 n = _DataStack_Pop ( ) ;
    _Compile_JCC ( n, ttt, 0 ) ; // we do need to store and get this logic set by various conditions by the compiler : _Compile_SET_tttn_REG
    _Stack_PointerToJmpOffset_Set ( ) ;
}

void
CfrTil_Jcc_Label ( )
{
    int32 ttt = _DataStack_Pop ( ) ;
    int32 n = _DataStack_Pop ( ) ;
    GotoInfo * gotoInfo = ( GotoInfo * ) _GotoInfo_Allocate ( ) ;
    _Compile_JCC ( n, ttt, 0 ) ; 
    gotoInfo->pb_JmpOffsetPointer = Here - 4 ; // after the call opcode
    gotoInfo->GI_CType = GI_CALL_LABEL ;
    gotoInfo->pb_LabelName = (byte*) _DataStack_Pop () ;
    DLList_AddNodeToTail ( _Context_->Compiler0->GotoList, ( DLNode* ) gotoInfo ) ;
}

void
CfrTil_JmpToHere ( ) 
{
    CfrTil_CalculateAndSetPreviousJmpOffset_ToHere ( ) ;
}

void
CfrTil_BitWise_NOT ( ) // xor
{
    if ( CompileMode )
    {
        Compile_BitWise_NOT ( _Context_->Compiler0 ) ;
    }
    else
    {
        TOS = ~ TOS ;
    }
}

void
CfrTil_BitWise_NEG ( ) // xor
{
    if ( CompileMode )
    {
        Compile_BitWise_NEG ( _Context_->Compiler0 ) ;
    }
    else
    {
        TOS = - TOS ;
    }
}

void
CfrTil_BitWise_OR ( ) // xor
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Context_->Compiler0, OR, ZERO, N ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] | TOS ;
        Dsp -- ;
    }
}

void
CfrTil_BitWise_OrEqual ( ) // -=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Context_->Compiler0, OR ) ;
    }
    else
    {
        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = ( * x ) | n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_BitWise_AND ( ) // xor
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Context_->Compiler0, AND, ZERO, N ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] & TOS ;
        Dsp -- ;
    }
}

void
CfrTil_BitWise_AndEqual ( ) // -=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Context_->Compiler0, AND ) ;
    }
    else
    {
        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = ( * x ) & n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_BitWise_XOR ( ) // xor
{
    if ( CompileMode )
    {
        Compile_X_Group1 ( _Context_->Compiler0, XOR, ZERO, N ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] ^ TOS ;
        Dsp -- ;
    }
}

void
CfrTil_BitWise_XorEqual ( ) // -=
{
    if ( CompileMode )
    {
        Compile_Group1_X_OpEqual ( _Context_->Compiler0, XOR ) ;
    }
    else
    {
        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = ( * x ) ^ n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_ShiftLeft ( ) // lshift
{
    if ( CompileMode )
    {
        Compile_ShiftLeft ( ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] << TOS ;
        Dsp -- ;
    }
}

void
CfrTil_ShiftRight ( ) // rshift
{
    if ( CompileMode )
    {
        Compile_ShiftRight ( ) ;
    }
    else
    {
        Dsp [ - 1 ] = Dsp [ - 1 ] >> TOS ;
        Dsp -- ;
    }
}

void
CfrTil_ShiftLeft_Equal ( ) // +=
{
    if ( Compiler_GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        Compile_X_Shift ( _Context_->Compiler0, SHL, 0 ) ;
    }
    else
    {
        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x << n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}

void
CfrTil_ShiftRight_Equal ( ) // +=
{
    if ( Compiler_GetState( _Context_->Compiler0, BLOCK_MODE ) )
    {
        Compile_X_Shift ( _Context_->Compiler0, SHR, 0 ) ;
    }
    else
    {
        int32 *x, n ;
        n = _DataStack_Pop ( ) ;
        x = ( int32* ) _DataStack_Pop ( ) ;
        *x = * x >> n ;
        //_DataStack_SetTop ( Dsp, _DataStack_Pop () + _DataStack_GetTop ( Dsp ) ) ;
    }
}





