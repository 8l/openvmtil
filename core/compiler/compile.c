#include "../../includes/cfrtil.h"

void
_Compile_CallEAX ( void )
{
    //_Compile_Group5 ( int32 code, int32 mod, int32 rm, int32 sib, int32 disp, int32 size )
    _Compile_Group5 ( CALL, 3, 0, 0, 0, 0 ) ;
}

void
Compile_DataStack_PopAndCall ( void )
{
    Compile_Pop_ToEAX_AndCall ( DSP ) ;
}

// >R - Rsp to

void
_Compile_Rsp_To ( ) // data stack pop to rsp
{
    Compile_DspPop_EspPush ( ) ;
}

// rdrop

void
_Compile_Rsp_Drop ( )
{
    _Compile_DropN_ESP ( 1 ) ;
}

// rsp

void
_Compile_Rsp_Get ( )
{
    _Compile_Stack_PushReg ( DSP, ESP ) ;
}

// rsp@

void
_Compile_Rsp_Fetch ( )
{
    _Compile_Move_Reg_To_Reg ( EAX, ESP ) ;
    _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 * CELL ) ;
    _Compile_Stack_PushReg ( DSP, EAX ) ;
}

// r< - r from

void
_Compile_Rsp_From ( )
{
    _Compile_Rsp_Fetch ( ) ;
    _Compile_Rsp_Drop ( ) ;
}

// Rsp! - Rsp store

void
_Compile_Rsp_Store ( ) // data stack pop to rsp [0] !
{
    _Compile_Rsp_From ( ) ;
    Compile_Store ( _Context_->Compiler0, DSP ) ;
}

Word *
_CfrTil_VariableGet ( Namespace * ns, byte * name )
{
    Word * word = Word_FindInOneNamespace ( ns, name ) ;
    return word ;
}

void
_CfrTil_VariableValueSet ( Namespace * ns, byte * name, int32 value )
{
    Word * word = _CfrTil_VariableGet ( ns, name ) ;
    if ( word ) word->bp_WD_Object = ( byte* ) value ; // value of variable
}

int32
_CfrTil_VariableValueGet ( byte* namespace, byte * name )
{
    return _Namespace_VariableValueGet ( Namespace_Find ( namespace ), name ) ;
}

void
_Compile_C_Var_To_Reg ( int32 reg, int32 * ptrToCVar )
{
    D0 ( byte *here = Here ; )
        _Compile_Move_Literal_Immediate_To_Reg ( EAX, ( int32 ) ptrToCVar ) ; // use pointer because value has to be taken at run time not compile time
    D0 ( if ( _CfrTil_->Debugger0 ) Debugger_UdisOneInstruction ( _CfrTil_->Debugger0, here, ( byte* ) "", ( byte* ) "" ) ; )
        D0 ( here = Here ; )
        _Compile_Move_Rm_To_Reg ( reg, EAX, 0 ) ;
    D0 ( if ( _CfrTil_->Debugger0 ) Debugger_UdisOneInstruction ( _CfrTil_->Debugger0, here, ( byte* ) "", ( byte* ) "" ) ; )
    }

void
_Compile_Reg_To_C_Var ( int32 reg, int32 * ptrToCVar )
{
    D0 ( byte *here = Here ; )
        _Compile_Move_Literal_Immediate_To_Reg ( EAX, ( int32 ) ptrToCVar ) ; // use pointer because value has to be taken at run time not compile time
    D0 ( if ( _CfrTil_->Debugger0 ) Debugger_UdisOneInstruction ( _CfrTil_->Debugger0, here, ( byte* ) "", ( byte* ) "" ) ; )
        D0 ( here = Here ; )
        _Compile_Move_Reg_To_Rm ( EAX, 0, reg ) ;
    D0 ( if ( _CfrTil_->Debugger0 ) Debugger_UdisOneInstruction ( _CfrTil_->Debugger0, here, ( byte* ) "", ( byte* ) "" ) ; )
    }

void
_Compile_Move_Literal_Immediate_To_Reg ( int32 reg, int32 value )
{
    _Compile_MoveImm_To_Reg ( reg, value, CELL ) ;
}

void
_Compile_LValue_ClassFieldToReg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        //_Compile_Move_Rm_To_Reg ( reg, word->RegToUse, 0 ) ; 
        _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & ( STACK_VARIABLE | LOCAL_VARIABLE ) )
    {
        _Compile_VarConstOrLit_RValue_To_Reg ( word, reg ) ; // nb.!! : compile rvalue for stack/local variables !!
    }
    else if ( word->CType & ( THIS | VARIABLE | OBJECT ) )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->PtrObject ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) word->PtrObject ) ; // intel insn set doesnt include move [mem immediate] to ecx like eax
            _Compile_Move_Rm_To_Reg ( ECX, ECX, 0 ) ;
        }
    }
    else SyntaxError ( 1 ) ;
    int32 offset = word->AccumulatedOffset ;
    if ( offset ) Compile_ADDI ( REG, reg, 0, offset, CELL ) ; // nb : rpn 
}

void
_Compile_RValue_ClassFieldToReg ( Word * word, int32 reg )
{
    if ( word->CType & ( STACK_VARIABLE | LOCAL_VARIABLE ) )
    {
        _Compile_VarConstOrLit_RValue_To_Reg ( word, reg ) ;
    }
    else if ( word->CType & ( THIS | VARIABLE | OBJECT ) )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->bp_WD_Object ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( ECX, ( int32 ) word->bp_WD_Object ) ; // intel insn set doesnt include move [mem immediate] to ecx like eax
            _Compile_Move_Rm_To_Reg ( ECX, ECX, 0 ) ;
        }
    }
    else SyntaxError ( 1 ) ;
    int32 offset = word->AccumulatedOffset ;
    if ( offset ) Compile_ADDI ( REG, reg, 0, offset, CELL ) ; // nb : rpn 
}

