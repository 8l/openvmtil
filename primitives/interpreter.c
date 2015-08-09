#include "../includes/cfrtil.h"

void
CfrTil_CommentToEndOfLine ( )
{
    ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
    String_RemoveEndWhitespaceAndAddNewline ( _CfrTil_->SourceCodeScratchPad ) ;
    _CfrTil_UnAppendTokenFromSourceCode ( _Context_->Lexer0->OriginalToken ) ;
    SC_ScratchPadIndex_Init ( ) ;
    SetState ( _Context_->Lexer0, END_OF_LINE, true ) ;
}

void
CfrTil_ParenthesisComment ( )
{
    while ( 1 )
    {
        int inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
        if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;
        char * token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ;
        if ( strcmp ( token, "*/" ) == 0 ) return ;
    }
}

void
CfrTil_If_ConditionalInterpret ( )
{
    _CfrTil_ConditionalInterpret ( 1 ) ;
}

void
CfrTil_Else_ConditionalInterpret ( )
{
    _CfrTil_ConditionalInterpret ( 0 ) ;
}

void
CfrTil_Interpreter_IsDone ( )
{
    _DataStack_Push ( Interpreter_GetState ( _Context_->Interpreter0, END_OF_FILE | END_OF_STRING | INTERPRETER_DONE ) ) ;
}

void
CfrTil_Interpreter_Done ( )
{
    Interpreter_SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
}

void
CfrTil_Interpreter_Init ( )
{
    Interpreter_Init ( _Context_->Interpreter0 ) ;
}

void
CfrTil_InterpretNextToken ( )
{
    Interpreter_InterpretNextToken ( _Context_->Interpreter0 ) ;
}

void
CfrTil_Interpret ( )
{
    _Context_InterpretFile ( _Context_ ) ;
}

void
CfrTil_InterpretPromptedLine ( )
{
    _DoPrompt ( ) ;
    Context_Interpret ( _CfrTil_->Context0 ) ;
}

void
CfrTil_InterpretString ( )
{
    _InterpretString ( ( byte* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_Interpreter_EvalWord ( )
{
    _Interpreter_Do_MorphismWord ( _Context_->Interpreter0, ( Word* ) _DataStack_Pop ( ) ) ;
}

void
CfrTil_InterpretALiteralToken ( )
{
    Lexer_ObjectToken_New ( _Context_->Lexer0, ( byte* ) _DataStack_Pop ( ), 1 ) ;
}

void
_CfrTil_Interpret ( CfrTil * cfrTil )
{
    do
    {
        _CfrTil_Init_SessionCore ( cfrTil, 1, 1 ) ;
        Context_Interpret ( cfrTil->Context0 ) ;
    }
    while ( GetState ( cfrTil, CFRTIL_RUN ) ) ;
}

void
CfrTil_InterpreterRun ( )
{
    _CfrTil_Interpret ( _CfrTil_ ) ;
}

void
CfrTil_InterpreterStop ( )
{
    SetState ( _Context_->Interpreter0, INTERPRETER_DONE, true ) ;
    SetState ( _CfrTil_, CFRTIL_RUN, false ) ;
}

