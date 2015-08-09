
#include "../../includes/cfrtil.h"

int32
_CheckArrayDimensionForVariables ( )
{
    Compiler *compiler = _Context_->Compiler0 ;
    //if ( Lexer_CheckArrayDimensionForVariables ( _Context_->Lexer0 ) )
    if ( _Readline_CheckArrayDimensionForVariables ( _Context_->ReadLiner0 ) )
    {
        compiler->AccumulatedOffsetPointer = 0 ; //controls whether we can/should use IncrementCurrentAccumulatedOffset ( increment ) ;
        Compiler_SetState ( compiler, COMPILE_MODE, compiler->SaveCompileMode ) ;
        return true ;
    }
    return false ;
}

// ?!?
// offset is calculated using this formula :
// d1 + d2*(D1) + d3*(D2*D1) + d4*(D3*D2*D1) ...
// where d1, d2, d3, ... are the dimension variables and D1, D2, D3, ... are the Dimension sizes
// ?!? this formula needs a proof but it has been working ?!?

void
CfrTil_ArrayBegin ( void )
{
    Interpreter * interpreter = _Context_->Interpreter0 ;
    Word * baseObject = interpreter->BaseObject ;
    Compiler *compiler = _Context_->Compiler0 ;
    Lexer * lexer = _Context_->Lexer0 ;
    byte * token = lexer->OriginalToken ;
    int32 objSize = 0, arrayIndex, increment = 0, localVariableFlag = false ;
    Namespace * ns = 0 ;
    if( interpreter->ObjectField ) ns = TypeNamespace_Get ( interpreter->ObjectField ) ;
    if ( ns && ( ! ns->ArrayDimensions ) ) ns = TypeNamespace_Get ( baseObject ) ;
    if ( ns && ( ! ns->ArrayDimensions ) ) CfrTil_Exception ( ARRAY_DIMENSION_ERROR, QUIT ) ;
    compiler->SaveCompileMode = compiler->State & COMPILE_MODE ;
    Compiler_SetState ( compiler, COMPILE_MODE, false ) ;
    if ( interpreter->ObjectField ) objSize = interpreter->ObjectField->Size ; //_CfrTil_VariableValueGet ( _Context_->Interpreter0->CurrentClassField, ( byte* ) "size" ) ; 
    if ( ! objSize )
    {
        CfrTil_Exception ( OBJECT_SIZE_ERROR, QUIT ) ;
    }
    localVariableFlag = _CheckArrayDimensionForVariables ( ) ;
    do
    {
        token = Lexer_ReadToken ( lexer ) ;
        if ( token [0] == '[' )
        {
            localVariableFlag = _CheckArrayDimensionForVariables ( ) ;
            continue ;
        }
        if ( token [0] == ']' ) // ']' == an "array end"
        {
            Debugger * debugger = _CfrTil_->Debugger0 ;
            int32 dm = GetState ( _CfrTil_, DEBUG_MODE ) ; //&& ( ! GetState ( debugger, DBG_STEPPING ) ) ;
            Word * word = Finder_Word_FindUsing ( _Context_->Finder0, token ) ;
            if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;

            // d1 + d2*(D1) + d3*(D2*D1) + d4*(D3*D2*D1) ...
            int32 dimNumber = compiler->ArrayEnds, dimSize = 1 ;
            while ( -- dimNumber >= 0 ) // -- : zero based ns->ArrayDimensions
            {
                dimSize *= ns->ArrayDimensions [ dimNumber ] ; // the parser created and populated this array in _CfrTil_Parse_ClassStructure 
            }
            if ( localVariableFlag == false )
            {
                arrayIndex = _DataStack_Pop ( ) ;
                increment += arrayIndex * dimSize * objSize ; // keep a running total of 
                if ( _Context_StrCmpNextToken ( _Context_, "[" ) )
                {
                    if ( compiler->SaveCompileMode & COMPILE_MODE )
                    {
                        IncrementCurrentAccumulatedOffset ( increment ) ;
                    }
                    else _DataStack_SetTop ( _DataStack_GetTop ( ) + increment ) ;
                    if ( dm ) _Debugger_PostShow ( debugger, 0, word ) ;
                    break ;
                }
            }
            else
            {
                // assume arrayIndex has just been pushed to TOS
                //_Compile_IMULI ( int32 mod, int32 reg, int32 rm, int32 sib, int32 disp, int32 imm, int32 size )
                _Compile_IMULI ( MEM, EAX, DSP, 0, 0, dimSize * objSize, 0 ) ;
                _Compile_Stack_DropN ( DSP, 1 ) ; // drop the array index
                //Compile_ADD( toRegOrMem, mod, reg, rm, sib, disp, isize ) 
                Compile_ADD ( MEM, MEM, EAX, DSP, 0, 0, CELL ) ;
                if ( dm ) _Debugger_PostShow ( debugger, 0, word ) ;
                if ( _Context_StrCmpNextToken ( _Context_, "[" ) ) break ;
            }
            localVariableFlag = false ;
            compiler->ArrayEnds ++ ;

            continue ;
        }
        _Interpreter_InterpretAToken ( interpreter, token ) ;

    }
    while ( 1 ) ;
    compiler->ArrayEnds = 0 ; // reset for next array word in the current word being compiled
    interpreter->BaseObject = baseObject ; // _Context_->Interpreter0->baseObject is reset by the interpreter by the types of words between array brackets
    compiler->State |= compiler->SaveCompileMode ; // before _CalculateOffset
}

void
CfrTil_ArrayEnd ( void )
{
    // for the debugger
}

