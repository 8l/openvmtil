
#include "../includes/cfrtil.h"

void
_Compile_C_Call_1_Arg ( byte* function, int32 arg )
{
    _Compile_Esp_Push ( arg ) ;
    _Compile_Call ( function ) ;
    _Compile_Rsp_Drop ( ) ;
}

void
_Compile_CallFunctionWithArg ( byte * function, int32 arg )
{
    _Compile_C_Call_1_Arg ( function, arg ) ;
}

void
_CfrTil_Do_ClassField ( Word * word )
{
    Compiler * compiler = _Context_->Compiler0 ;
    _Context_->Interpreter0->ObjectField = word ;
    compiler->ArrayEnds = 0 ;
    uint32 accumulatedAddress ;

    if ( Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) ) Finder_SetQualifyingNamespace ( _Context_->Finder0, word->ClassFieldNamespace ) ;
    if ( CompileMode )
    {
        if ( word->Offset )
        {
            IncrementCurrentAccumulatedOffset ( word->Offset ) ;
            //word->Value =  *_Context_->Compiler0->AccumulatedOffsetPointer ;
        }
    }
    else
    {

        accumulatedAddress = _DataStack_Pop ( ) ;
        accumulatedAddress += word->Offset ;
        if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_RHS ) && (Lexer_NextNonDelimiterChar ( _Context_->Lexer0 ) != '.') ) _Push ( * (int32*) accumulatedAddress ) ;
        else _Push ( accumulatedAddress ) ;
    }
}

// nb. 'word' is the previous word to the '.' (dot) cf. CfrTil_Dot so it can be recompiled, a little different maybe, as an object

void
_CfrTil_Do_Object ( Word * word )
{
    Compiler * compiler = _Context_->Compiler0 ;
    _Context_->Interpreter0->BaseObject = word ;
    _Context_->Interpreter0->ObjectField = TypeNamespace_Get ( word ) ;

    if ( ( ! _Context_->Finder0->QualifyingNamespace ) && Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) ) Finder_SetQualifyingNamespace ( _Context_->Finder0, word->ContainingNamespace ) ;
    if ( CompileMode )
    {
        byte * saveHere = word->Coding ; // Here : instead overwrite the previous word's Code as Here since 'word' is the word before the '.'
        SetHere ( word->Coding ) ; // we don't need the word's code if compiling -- this is an optimization though
        _Compile_VarConstOrLit_RValue_To_Reg ( word, EAX ) ;
        compiler->AddOffsetCodePointer = ( int32* ) Here ;
        Compile_ADDI ( REG, EAX, 0, 0, CELL ) ;
        // !! this block could (and should) be moved to overwrite the above ADDI if the AccumulatedOffset remains 0 !!
        compiler->AccumulatedOffsetPointer = ( int32* ) ( Here - CELL ) ; // offset will be calculated as we go along by ClassFields and Array accesses
        //if ( ( ! GetState ( _Context_, ADDRESS_OF_MODE ) ) && GetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID ) && ( word->CType & ( LOCAL_VARIABLE | STACK_VARIABLE ) ) && ( ! word->TypeNamespace ) )
        //if ( ( ! GetState ( _Context_, ADDRESS_OF_MODE ) ) && GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_RHS ) )
        if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_RHS ) )
            _Compile_Move_Rm_To_Reg ( EAX, EAX, 0 ) ;
        _Word_CompileAndRecord_PushEAX ( word ) ;
        if ( GetState ( _CfrTil_, OPTIMIZE_ON ) )
        {
            // we sometimes refer to more than one field of the same object, eg. 'this' in a block
            // each reference may be to a different labeled field each with a different offset so we must 
            // create copies of the multiply referenced word to hold the referenced offsets for the optimizer
            // 'word' is the 'baseObject' word. If it is already on the Object word Stack certain optimizations can be made.
            int32 i, stackSize = Stack_Depth ( compiler->ObjectStack ) ;
            for ( i = 0 ; i < stackSize ; i ++ )
            {
                if ( word == ( Word* ) ( Stack_OffsetValue ( compiler->ObjectStack, - i ) ) )
                {
                    word = Word_Copy ( word, SESSION ) ; // especially for "this" so we can use a different Code & AccumulatedOffsetPointer not the existing 
                    //word = wordCopy ; // this line is only needed for debugging
                    break ;
                }
            }
            _Stack_Push ( compiler->ObjectStack, ( int32 ) word ) ; // push the copy, so both are now on the stack
            word->ObjectCode = saveHere ; // the new word's code pointer
        }
        word->AccumulatedOffset = 0 ;
        compiler->AccumulatedOptimizeOffsetPointer = & word->AccumulatedOffset ;
    }
    else
    {
        compiler->AccumulatedOffsetPointer = 0 ; // ?? used as a flag for non compile mode ??
        _Push ( ( int32 ) word->bp_WD_Object ) ;
    }
}

// rvalue - rhs value - right hand side of '=' - the actual value, used on the right hand side of C statements

void
_Compile_VarConstOrLit_LValue_To_Reg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & LOCAL_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( LocalVarOffset ( word ) ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CType & STACK_VARIABLE )
    {
        _Compile_LEA ( reg, FP, 0, LocalVarIndex_Disp ( StackVarOffset ( word ) ) ) ;
    }
    else if ( word->CType & ( VARIABLE | OBJECT | THIS ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->PtrObject ) ;
    }
    else
    {

        CfrTil_Exception ( SYNTAX_ERROR, QUIT ) ;
    }
}

void
_Compile_VarConstOrLit_RValue_To_Reg ( Word * word, int32 reg )
{
    if ( word->CType & REGISTER_VARIABLE )
    {
        if ( word->RegToUse == reg ) return ;
        else _Compile_Move_Reg_To_Reg ( reg, word->RegToUse ) ;
    }
    else if ( word->CType & LOCAL_VARIABLE )
    {
        //_Compile_Move_LocalVarRValue_To_Reg ( word, reg ) ;
        _Compile_Move_StackN_To_Reg ( reg, FP, LocalVarOffset ( word ) ) ; // 2 : account for saved fp and return slot
    }
    else if ( word->CType & STACK_VARIABLE )
    {
        //_Compile_Move_StackVarRValue_To_Reg ( word, reg ) ;
        _Compile_Move_StackN_To_Reg ( reg, FP, StackVarOffset ( word ) ) ; // account for stored bp and return value
    }
    else if ( word->CType & ( VARIABLE | THIS ) )
    {
        if ( reg == EAX ) _Compile_Move_AddressValue_To_EAX ( ( int32 ) word->PtrObject ) ;
        else
        {
            _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->PtrObject ) ; // remember we want be getting runtime value not the compile time value
            _Compile_Move_Rm_To_Reg ( reg, reg, 0 ) ;
        }
    }
    else if ( word->CType & ( LITERAL | CONSTANT | OBJECT ) )
    {
        _Compile_Move_Literal_Immediate_To_Reg ( reg, ( int32 ) word->bp_WD_Object ) ;
    }
    else
    {

        CfrTil_Exception ( SYNTAX_ERROR, QUIT ) ;
    }
}

_Word_CompileAndRecord_PushEAX ( Word * word )
{

    word->StackPushRegisterCode = Here ; // nb. used! by the rewriting optimizer
    Compile_Stack_PushEAX ( DSP ) ;
}

void
_Do_Literal ( int32 value )
{
    if ( CompileMode )
    {
        //_Compile_MoveImm_To_Reg ( EAX, value, CELL ) ;
        //_Word_CompileAndRecord_PushEAX ( word ) ;
        _Compile_Stack_Push ( DSP, value ) ;
    }

    else _Push ( value ) ;
}

void
Do_Variable ( Word * word )
{
    int32 value ;
    if ( word->CType & VARIABLE )
    {
        if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_RHS ) )
        {
            value = ( int32 ) word->Value ;
        }
        else value = ( int32 ) & word->Value ;
    }

    else value = ( int32 ) word->Value ;
    _Do_Literal ( value ) ;
}

void
_Namespace_DoNamespace ( Namespace * ns )
{
    if ( CompileMode && ( Lexer_NextNonDelimiterChar ( _Context_->Lexer0 ) != '.' ) ) // ( ! GetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID ) ) ) //&& ( ! GetState ( _Context_, C_SYNTAX ) ) )
    {
        _Compile_C_Call_1_Arg ( ( byte* ) _Namespace_DoNamespace, ( int32 ) ns ) ;
    }
    if ( ! Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) )
    {
        _Namespace_ActivateAsPrimary ( ns ) ;
    }
    else
    {

        Finder_SetQualifyingNamespace ( _Context_->Finder0, ns ) ;
        //_Namespace_AddToNamespacesHead_SetAsInNamespace ( ns ) ;
    }
}

void
_CfrTil_Do_DObject ( DObject * dobject )
{
    Lexer * lexer = _Context_->Lexer0 ;
    DObject * ndobject ;
    byte * token ;
    int32 result ;
    while ( Lexer_IsTokenForwardDotted ( lexer ) )
    {
        Lexer_ReadToken ( lexer ) ; // the '.'
        token = Lexer_ReadToken ( lexer ) ;
        if ( String_Equal ( "prototype", ( char* ) token ) )
        {
            dobject = dobject->ContainingNamespace ;
            continue ;
        }
        if ( ! ( ndobject = _DObject_FindSlot_BottomUp ( dobject, token ) ) )
        {
            _CfrTil_->InNamespace = dobject ; // put the new dynamic object in the namespace of it's mother object
            dobject = _DObject_NewSlot ( dobject, token, 0 ) ;
        }
        else dobject = ndobject ;
    }
    //result = ( int32 ) & dobject->bp_WD_Object ; // nb : lvalue
    result = ( int32 ) dobject ; // nb : lvalue
    _Context_->Interpreter0->ObjectField = TypeNamespace_Get ( dobject ) ;
    if ( CompileMode )
    {
        _Compile_DataStack_Push ( result ) ;
    }
    else
    {

        _Push ( result ) ;
    }
}

// 'Run' :: this is part of runtime in the interpreter/compiler for data objects
// they are compiled to much more optimized native code

void
DataObject_Run ( Word * word )
{
    Debugger * debugger = _CfrTil_->Debugger0 ;
    int32 dm = GetState ( _CfrTil_, DEBUG_MODE ) && ( ! GetState ( debugger, DBG_STEPPING ) ) ;
    if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;
    if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
    {
#if 1
        if ( word->CType & T_LISP_SYMBOL )
        {
            _Compile_Move_StackN_To_Reg ( EAX, FP, StackVarOffset ( word ) ) ; // account for stored bp and return value
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
        else if ( word->CType & DOBJECT )
        {
            _CfrTil_Do_DObject ( word ) ;
        }
        else if ( word->CType & OBJECT )
        {
            _CfrTil_Do_Object ( word ) ;
        }
        else if ( word->CType & CLASS_MEMBER_ACCESS )
        {
            _CfrTil_Do_ClassField ( word ) ;
        }
        else if ( word->CType & ( C_TYPE | C_CLASS ) )
        {
            _Namespace_Do_C_Type ( word ) ;
        }
        else if ( word->CType & ( NAMESPACE | CLASS | CLASS_CLONE ) )
        {
            _Namespace_DoNamespace ( word ) ;
        }
        else if ( word->CType & ( LOCAL_VARIABLE | STACK_VARIABLE ) )
        {
            if ( GetState ( _Context_, C_SYNTAX ) )
            {
                if ( GetState ( _Context_, C_RHS ) )
                {
                    _Compile_VarConstOrLit_RValue_To_Reg ( word, EAX ) ;
                    _Word_CompileAndRecord_PushEAX ( word ) ;
                }
                else if GetState ( _Context_, C_LHS )
                {
                    SetState ( _Context_, C_LHS, false ) ;
                    _Context_->Compiler0->LHS_Word = word ;
                }
            }
            else
            {
                _Compile_VarConstOrLit_LValue_To_Reg ( word, EAX ) ;
                _Word_CompileAndRecord_PushEAX ( word ) ;
            }
        }
        else if ( word->CType & VARIABLE )
        {
            if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_LHS ) )
            {
                SetState ( _Context_, C_LHS, false ) ;
                _Context_->Compiler0->LHS_Word = word ;
            }
            else Do_Variable ( word ) ;
        }
        else if ( word->CType & ( CONSTANT | LITERAL ) )
        {
            if ( word->LType & ( LITERAL | T_LISP_SYMBOL ) ) _Do_Literal ( ( int32 ) word->Lo_Value ) ;
            else _Do_Literal ( ( int32 ) word->Value ) ;
        }
#else          
        switch ( word->RunType )
        {
#if 1            
            case T_LISP_SYMBOL:
            {
                _Compile_Move_StackN_To_Reg ( EAX, FP, StackVarOffset ( word ) ) ; // account for stored bp and return value
                _Word_CompileAndRecord_PushEAX ( word ) ;
                break ;
            }
#endif            
            case DOBJECT:
            {
                _CfrTil_Do_DObject ( word ) ;
                break ;
            }
            case OBJECT:
            {
                _CfrTil_Do_Object ( word ) ;
                break ;
            }
            case CLASS_MEMBER_ACCESS:
            {
                _CfrTil_Do_ClassField ( word ) ;
                break ;
            }
            case NAMESPACE:
            case CLASS:
            case CLASS_CLONE:
            {
                _Namespace_DoNamespace ( word ) ;
                break ;
            }
            case C_TYPE:
            case C_CLASS:
            {
                _Namespace_Do_C_Type ( word ) ;
                break ;
            }
            case LOCAL_VARIABLE:
            case STACK_VARIABLE:
            {
                if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_RHS ) ) //( word != _Context_->Compiler0->LHS_Word ) )
                {
                    _Compile_VarConstOrLit_RValue_To_Reg ( word, EAX ) ;
                    _Word_CompileAndRecord_PushEAX ( word ) ;
                }
                else
                {
                    _Compile_VarConstOrLit_LValue_To_Reg ( word, EAX ) ;
                    _Word_CompileAndRecord_PushEAX ( word ) ;
                }
                break ;
            }
            case VARIABLE:
            {
                if ( GetState ( _Context_, C_SYNTAX ) && GetState ( _Context_, C_LHS ) && ( Lexer_NextNonDelimiterChar ( _Context_->Lexer0 ) != ')' ) )
                {
                    SetState ( _Context_, C_LHS, false ) ;
                    _Context_->Compiler0->LHS_Word = word ;
                }
                else Do_Variable ( word ) ;
                break ;
            }
            case CONSTANT:
            case LITERAL:
            {
                if ( word->LType & ( LITERAL | T_LISP_SYMBOL ) ) _Do_Literal ( ( int32 ) word->Lo_Value ) ;
                else _Do_Literal ( ( int32 ) word->Value ) ;
                break ;
            }
        }
#endif         
    }
    if ( dm ) _Debugger_PostShow ( debugger, 0, _Context_->Interpreter0->w_Word ) ; // a literal could have been created and shown by _Word_Run
}