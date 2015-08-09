
#include "../includes/cfrtil.h"

Boolean
_Interpreter_IsPrefixWord ( Interpreter * interp, Word * word )
{
    if ( ( GetState ( _Context_, PREFIX_MODE ) ) && ( ! _Namespace_IsUsing ( _CfrTil_->LispNamespace ) ) )
        //if ( ! _Namespace_IsUsing ( _CfrTil_->LispNamespace ) ) 
    {
        // with this any postfix word that is not a keyword or a c rtl arg word can now be used prefix with parentheses 
        byte c = Lexer_NextNonDelimiterChar ( interp->Lexer ) ;
        if ( ( c == '(' ) && ( ! ( word->CType & KEYWORD ) ) && ( ! ( word->WType & _C_PREFIX_RTL_ARGS ) ) )
        {
            return true ;
        }
    }
    return false ;
}

// TODO : this ... well just look at it... 

void
_Interpreter_Do_MorphismWord ( Interpreter * interp, Word * word )
{
    int32 svs_c_rhs, svs_c_lhs ;
    if ( word )
    {
        if ( _Interpreter_IsPrefixWord ( interp, word ) )
        {
            _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
        }
        else
        {
            switch ( word->WType )
            {
                case _PREFIX:
                {
                    _Interpret_PrefixFunction_Until_RParen ( interp, word ) ;
                    break ;
                }
                case _C_PREFIX_RTL_ARGS:
                {
                    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
                    if ( GetState ( _Context_, C_SYNTAX ) )
                    {
                        svs_c_rhs = GetState ( _Context_, C_RHS ) ;
                        svs_c_lhs = GetState ( _Context_, C_LHS ) ;
                        SetState ( _Context_, C_RHS, true ) ;
                    }
                    LC_CompileRun_ArgList ( word ) ;
                    if ( GetState ( _Context_, C_SYNTAX ) )
                    {
                        SetState ( _Context_, C_RHS, svs_c_rhs ) ;
                        SetState ( _Context_, C_LHS, svs_c_lhs ) ;
                    }
                    break ;
                }
                case _POSTFIX:
                case _INFIXABLE: // cf. also _Interpreter_SetupFor_MorphismWord
                default:
                {
                    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
                    _Word_Eval ( word ) ;
                    break ;
                }
            }
        }
    }
}

void
_Interpreter_SetupFor_MorphismWord ( Interpreter * interp, Word * word )
{
    if ( ( word->CType & INFIXABLE ) && ( GetState ( _Context_, INFIX_MODE ) ) ) // nb. INFIX_MODE must be in Interpreter because it is effective for more than one word
    {
        Interpreter_InterpretNextToken ( interp ) ;
        // then continue and interpret this 'word' - just one out of lexical order
    }
    interp->w_Word = word ;
    if ( ! ( word->CType & PREFIX ) ) interp->CurrentPrefixWord = 0 ; // prefix words are now processed in _Interpreter_DoMorphismToken
    if ( ( ! GetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID ) ) && ( ! ( word->CType & ( OBJECT | CLASS_MEMBER_ACCESS | DEBUG_WORD ) ) ) )
    {
        interp->BaseObject = 0 ;
        Finder_SetQualifyingNamespace ( interp->Finder, 0 ) ;
    }
    // keep track in the word itself where the machine code is to go if this word is compiled or causes compiling code - used for optimization
    word->Coding = Here ;
    if ( ! ( word->CType & ( DEBUG_WORD ) ) ) Stack_Push ( _Context_->Compiler0->WordStack, ( int32 ) word ) ;
}

void
_Interpret_MorphismWord_Default ( Interpreter * interp, Word * word )
{
    _Interpreter_SetupFor_MorphismWord ( interp, word ) ;
    _Word_Eval ( word ) ;
}

void
Interpret_MorphismWord_Default ( Word * word )
{
    _Interpreter_SetupFor_MorphismWord ( _Context_->Interpreter0, word ) ;
    _Word_Eval ( word ) ;
}

// interpret with find after parse for known objects
// !! this version eliminates the possiblity of numbers being used as words !!
// objects and morphismsm - terms from category theory

Word *
_Interpreter_InterpretAToken ( Interpreter * interp, byte * token )
{
    Word * word = 0 ;
    if ( token )
    {
        interp->Token = token ;
        word = Finder_Word_FindUsing ( interp->Finder, token ) ; // ?? find after Literal - eliminate make strings or numbers words ??
        if ( word )
        {
            _Interpreter_Do_MorphismWord ( interp, word ) ;
        }
        else word = Lexer_ObjectToken_New ( interp->Lexer, token, 1 ) ;
        interp->w_Word = word ;
    }
    return word ;
}

void
Interpreter_InterpretNextToken ( Interpreter * interp )
{
    _Interpreter_InterpretAToken ( interp, Lexer_ReadToken ( interp->Lexer ) ) ;
}

void
_Interpret_PrefixFunction_Until_Token ( Interpreter * interp, Word * prefixFunction, byte * end, byte * delimiters )
{
    byte * token ;
    while ( 1 )
    {
        token = _Lexer_ReadToken ( interp->Lexer, delimiters ) ;
        if ( String_Equal ( token, end ) ) break ;
        else _Interpreter_InterpretAToken ( interp, token ) ;
    }
    if ( prefixFunction ) _Interpret_MorphismWord_Default ( interp, prefixFunction ) ;
}

void
_Interpret_PrefixFunction_Until_RParen ( Interpreter * interp, Word * prefixFunction )
{
    Word * word ;
    byte * token ;
    int32 svs_c_rhs ;
    while ( 1 )
    {
        token = Lexer_ReadToken ( interp->Lexer ) ; // skip the opening left paren
        if ( ! String_Equal ( token, "(" ) )
        {
            if ( word = Finder_Word_FindUsing ( interp->Finder, token ) )
            {
                if ( word->CType & DEBUG_WORD )
                {
                    continue ;
                }
            }
            Error ( "\nSyntax Error : Prefix function with no opening left parenthesis!\n", QUIT ) ;
        }
        else break ;
    }
    if ( GetState ( _Context_, C_SYNTAX ) )
    {

        svs_c_rhs = GetState ( _Context_, C_RHS ) ;
        SetState ( _Context_, C_LHS, false ) ;
        SetState ( _Context_, C_RHS, true ) ;
    }
    _Interpret_PrefixFunction_Until_Token ( interp, prefixFunction, ")", ( byte* ) " ,\n\r\t" ) ;
    SetState ( _Context_, C_RHS, svs_c_rhs ) ;
}

void
_Interpret_UntilFlagged ( Interpreter * interp, int32 doneFlags )
{
    while ( ( ! Interpreter_IsDone ( interp, doneFlags | INTERPRETER_DONE ) ) )
    {
        Interpreter_InterpretNextToken ( interp ) ;
    }
}

void
_Interpret_ToEndOfLine ( Interpreter * interp )
{
    while ( 1 )
    {
        Interpreter_InterpretNextToken ( interp ) ;
        if ( GetState ( interp->Lexer, END_OF_LINE ) ) break ; // either the lexer with get a newline or the readLiner
        if ( ReadLine_PeekNextChar ( interp->ReadLiner ) == '\n' ) break ;
    }
}

void
Interpret_UntilFlaggedWithInit ( Interpreter * interp, int32 doneFlags )
{
    Interpreter_Init ( interp ) ;
    _Interpret_UntilFlagged ( interp, doneFlags ) ;
}


