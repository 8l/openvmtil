
#include "../includes/cfrtil.h"

void
_InterpretString_InContext ( byte *str )
{
    _Context_InterpretString ( _Context_, str ) ;
}

void
Interpreter_EvalQualifiedID ( Word * qid )
{
    _InterpretString_InContext ( qid->Name ) ;
}

void
_InterpretString ( byte *str )
{
    _CfrTil_ContextNew_InterpretString ( _CfrTil_, str, SESSION ) ;
}

// #if
// "#if" stack pop is 'true' interpret until "#else" and this does nothing ; if stack pop 'false' skip to "#else" token skip those tokens and continue interpreting

void
_CfrTil_ConditionalInterpret ( int32 ifFlag )
{
    char * token ;
    ;
    int32 ifStack = 1, status ;
    int32 svcm = Compiling ;
    SetState ( _Context_->Compiler0, COMPILE_MODE, false ) ;
    if ( ifFlag )
    {
        Finder_SetNamedQualifyingNamespace ( _Context_->Interpreter0->Finder, "PreProcessor" ) ; // so we can properly deal with parenthesized values here
        _Interpret_ToEndOfLine ( _Context_->Interpreter0 ) ;
        status = _DataStack_Pop ( ) ;
    }
    else status = 0 ;
    //nb : if condition is not true we skip interpreting with this block until "#else" 
    if ( ( ! ifFlag ) || ( ! status ) )
    {
        SetState ( _Context_->Compiler0, COMPILE_MODE, svcm ) ;
        while ( 1 )
        {
            int inChar = ReadLine_PeekNextChar ( _Context_->ReadLiner0 ) ;
            if ( ( inChar == - 1 ) || ( inChar == eof ) ) break ;

            if ( ( token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ) )
            {
                if ( String_Equal ( token, "//" ) ) CfrTil_CommentToEndOfLine ( ) ;
                else if ( String_Equal ( token, "/*" ) ) CfrTil_ParenthesisComment ( ) ;
                else if ( String_Equal ( token, "#" ) )
                {
                    if ( ( token = ( char* ) Lexer_ReadToken ( _Context_->Lexer0 ) ) )
                    {
                        if ( String_Equal ( token, "endif" ) )
                        {
                            if ( -- ifStack == 0 )
                            {
                                break ;
                            }
                        }
                        else if ( String_Equal ( token, "if" ) ) ifStack ++ ;
                        else if ( String_Equal ( token, "else" ) )
                        {
                            if ( ifStack == 1 )
                            {
                                break ;
                            }
                        }
                    }
                }
            }
        }
    }
    SetState ( _Context_->Compiler0, COMPILE_MODE, svcm ) ;
}

void
Interpreter_Init ( Interpreter * interp )
{
    if ( _CfrTil_->Debugger0 ) SetState ( _CfrTil_->Debugger0, DBG_AUTO_MODE, false ) ;
    interp->State = 0 ;
    //interp->BaseObject = 0 ;
    //interp->ParenLevel = 0 ;
}

Interpreter *
Interpreter_New ( int32 type )
{
    Interpreter * interp = ( Interpreter * ) _Mem_Allocate ( sizeof (Interpreter ), type ) ;

    interp->Lexer = Lexer_New ( type ) ;
    interp->ReadLiner = interp->Lexer->ReadLiner ;
    interp->Lexer->OurInterpreter = interp ;
    interp->Finder = Finder_New ( type ) ;
    interp->Compiler = Compiler_New ( type ) ;

    Interpreter_Init ( interp ) ;
    _Interpreter_ = interp ;
    return interp ;
}

void
_Interpreter_Copy ( Interpreter * interp, Interpreter * interp0 )
{
    memcpy ( interp, interp0, sizeof (Interpreter ) ) ;
}

Interpreter *
Interpreter_Copy ( Interpreter * interp0, int32 type )
{
    Interpreter * interp = ( Interpreter * ) _Mem_Allocate ( sizeof (Interpreter ), type ) ;
    _Interpreter_Copy ( interp, interp0 ) ;
    Interpreter_Init ( interp ) ;
    _Interpreter_ = interp ;
    return interp ;
}

#if 0

void
Interpreter_Delete ( Interpreter * interp )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) interp ) ;
}
#endif

int32
Interpreter_IsDone ( Interpreter * interp, int32 flags )
{
    return Interpreter_GetState ( interp, flags | INTERPRETER_DONE ) ;
}

