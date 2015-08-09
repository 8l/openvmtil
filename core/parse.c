#include "../includes/cfrtil.h"

// assuming we are using "Class" namespace
// syntax : ':{' ( classId identifer ( '[' integer ']' )* ';' ? )* '};'

void
_CfrTil_Parse_ClassStructure ( int32 cloneFlag )
{
    int32 size = 0, offset = 0, sizeOf = 0, i, arrayDimensionSize ;
    Namespace *ns, *inNs = _CfrTil_Namespace_InNamespaceGet ( ) ;
    byte * token ;
    int32 arrayDimensions [ 32 ] ;
    memset ( arrayDimensions, 0, sizeof (arrayDimensions ) ) ;
    if ( cloneFlag )
    {
        offset = _Namespace_VariableValueGet ( inNs, ( byte* ) "size" ) ; // allows for cloning - prototyping
        sizeOf = offset ;
    }
    while ( 1 )
    {
        // each name/word is an increasing offset from object address on stack
        // first name is at 0 offset
        // token = Lexer_NextToken ( _Context_->Lexer0 ) ;
        _CfrTil_Namespace_InNamespaceSet ( inNs ) ; // parsing arrays changes namespace so reset it here
        token = _Lexer_ReadToken ( _Context_->Lexer0, ( byte* ) " \n\r\t" ) ;
gotNextToken:
        if ( String_Equal ( ( char* ) token, "};" ) ) break ;
        if ( ( String_Equal ( ( char* ) token, "}" ) ) && GetState ( _Context_, C_SYNTAX ) )
        {
            CfrTil_TypedefStructEnd ( ) ;
            break ;
        }
        if ( String_Equal ( ( char* ) token, ";" ) ) continue ;
        if ( String_Equal ( ( char* ) token, "//" ) )
        {
            ReadLiner_CommentToEndOfLine ( _Context_->ReadLiner0 ) ;
            continue ;
        }
        ns = _Namespace_Find ( token, 0, 0 ) ;
        if ( ! ns ) CfrTil_Exception ( NAMESPACE_ERROR, 1 ) ;
        size = _Namespace_VariableValueGet ( ns, ( byte* ) "size" ) ;
        if ( ns && size )
        {
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
#if OLD            
            _CfrTil_ClassField_New ( token, ns, size, offset ) ;
#else
            _CfrTil_ClassField_New ( token, inNs, size, offset ) ;
#endif            
            byte * token = Lexer_PeekNextNonDebugTokenWord ( _Context_->Lexer0 ) ;
            if ( token [0] != '[' )
            {
                offset += size ;
                sizeOf += size ;
            }
        }
        else
            CfrTil_Exception ( NAMESPACE_ERROR, 1 ) ; // else structure component size error
        for ( i = 0 ; 1 ; )
        {
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
            if ( String_Equal ( ( char* ) token, "[" ) )
            {
                CfrTil_InterpretNextToken ( ) ; // next token must be an integer for the array dimension size
                arrayDimensionSize = _DataStack_Pop ( ) ;
                size = size * arrayDimensionSize ;
                offset += size ;
                sizeOf += size ;
                token = Lexer_ReadToken ( _Context_->Lexer0 ) ;
                arrayDimensions [ i ] = arrayDimensionSize ;
                if ( ! String_Equal ( ( char* ) token, "]" ) ) CfrTil_Exception ( SYNTAX_ERROR, 1 ) ;
                i ++ ;
            }
            else
            {
                if ( i )
                {
                    ns->ArrayDimensions = ( int32 * ) _Mem_Allocate ( i * sizeof (int32 ), DICTIONARY ) ;
                    memcpy ( ns->ArrayDimensions, arrayDimensions, i * sizeof (int32 ) ) ;
                }
                goto gotNextToken ;
            }
        }
    }
    _CfrTil_VariableValueSet ( inNs, ( byte* ) "size", sizeOf ) ;
}

void
Compile_InitRegisterVariables ( Compiler * compiler )
{
    int32 initVars = compiler->NumberOfRegisterVariables - compiler->NumberOfLocals ; // we only initialize, from the incoming stack, the stack variables
    int32 regOrder [ 4 ] = { EBX, ECX, EDX, EAX }, fpIndex = - 1, regIndex = 0 ; // -1 : cf locals.c 
    for ( ; initVars -- > 0 ; regIndex ++, fpIndex -- )
    {
        if ( compiler->State & ( RETURN_TOS | RETURN_EAX ) ) _Compile_Move_StackN_To_Reg ( regOrder [ regIndex ], DSP, 0 ) ;
        else _Compile_Move_StackN_To_Reg ( regOrder [ regIndex ], FP, fpIndex ) ; //

    }
}
// old docs :
// parse local variable notation to a temporary "_locals_" namespace
// calculate necessary frame size
// the stack frame (Fsp) will be just above TOS -- at higer addresses
// save entry Dsp in a CfrTil variable (or at Fsp [ 0 ]). Dsp will be reset to just
// above the framestack during duration of the function and at the end of the function
// will be reset to its original value on entry stored in the CfrTil variable (Fsp [ 0 ])
// so that DataStack pushes and pops during the function will be accessing above the top of the new Fsp
// initialize the words to access a slot in the framesize so that the
// compiler can use the slot number in the function being compiled
// compile a local variable such that when used at runtime it pushes
// the slot address on the DataStack

Namespace *
_CfrTil_Parse_LocalsAndStackVariables ( int32 svf, int32 debugFlag, int32 lispMode, ListObject * args ) // stack variables flag
{
    // number of stack variables, number of locals, stack variable flag
    Compiler * compiler = _Context_->Compiler0 ;
    if ( GetState ( compiler, VARIABLE_FRAME ) && GetState ( _Context_, INFIX_MODE ) )
    {
        // ?!? fix me : this logic ?!? -- just allow namespace change to handle this !!!!
        Interpret_DoParenthesizedRValue ( ) ;
        return ;
    }
    Lexer * lexer = _Context_->Lexer0 ;
    Finder * finder = _Context_->Finder0 ;
    byte * svDelimiters = lexer->TokenDelimiters ;
    DLList *locals ;
    Word * word, *lWord ;
    DLNode * node ;
    int64 ctype ;
    int32 i, svff = 0, nosv = 0, nol = 0, addWords, getReturn = 0, getReturnFlag = 0, regVars = 0, regToUse = 0 ;
    Boolean regFlag = false ;
    int32 regOrder [ 4 ] = { EBX, ECX, EDX, EAX }, regIndex = 0 ;
    byte *token, *returnVariable = 0 ;
    Namespace *typeNamespace = 0, *saveInNs = _CfrTil_->InNamespace, *localsNs = debugFlag ? _CfrTil_->Debugger0->Locals : Namespace_FindOrNew_Local ( ) ;

    if ( svf ) svff = 1 ;
    locals = _DLList_New ( SESSION ) ;
    addWords = 1 ;
    if ( lispMode ) args = ( ListObject * ) args->Lo_List->Head ;
    while ( ( lispMode ? ( int32 ) _LO_Next ( args ) : 1 ) )
    {
        if ( lispMode )
        {
            args = _LO_Next ( args ) ;
            token = ( byte* ) args->Lo_Name ;
        }
        else token = _Lexer_ReadToken ( lexer, ( byte* ) " ,\n\r\t" ) ;
        word = Finder_Word_FindUsing ( finder, token ) ; // ?? find after Literal - eliminate make strings or numbers words ??
        if ( word && ( word->CType & ( NAMESPACE | CLASS ) ) && (CharTable_IsCharType ( ReadLine_PeekNextChar ( lexer->ReadLiner ), CHAR_ALPHA ) ) )
        {
            typeNamespace = word ;
            continue ;
        }
        if ( strcmp ( ( char* ) token, "|" ) == 0 )
        {
            svff = 0 ; // set stack variable flag to off -- no more stack variables ; begin local variables
            continue ; // don't add a node to our temporary list for this token
        }
        if ( strcmp ( ( char* ) token, "--" ) == 0 ) // || ( strcmp ( ( char* ) token, "|-" ) == 0 ) || ( strcmp ( ( char* ) token, "|--" ) == 0 ) )
        {
            if ( ! svf )
                break ;
            else
            {
                addWords = 0 ;
                getReturnFlag = 1 ;
                continue ;
            }
        }
        if ( strcmp ( ( char* ) token, ")" ) == 0 )
        {
            break ;
        }
        if ( strcmp ( ( char* ) token, "REG:" ) == 0 )
        {
            regFlag = true ;
            continue ;
        }
        if ( strcmp ( ( char* ) token, "EAX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 3 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "ECX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 1 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "EDX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 2 ;
            continue ;
        }
        else if ( strcmp ( ( char* ) token, "EBX:" ) == 0 )
        {
            regFlag = true ;
            regToUse = 0 ;
            continue ;
        }
        if ( ( strcmp ( ( char* ) token, "{" ) == 0 ) || ( strcmp ( ( char* ) token, ";" ) == 0 ) )
        {
            Printf ( ( byte* ) "\nLocal variables syntax error : no close parenthesis ')' found" ) ;
            CfrTil_Exception ( SYNTAX_ERROR, 1 ) ;
        }
        if ( getReturnFlag )
        {
            addWords = 0 ;
            if ( stricmp ( token, ( byte* ) "EAX" ) == 0 )
                getReturn = RETURN_EAX ;
            else if ( stricmp ( token, ( byte* ) "TOS" ) == 0 )
                getReturn = RETURN_TOS ;
            else if ( stricmp ( token, ( byte* ) "0" ) == 0 )
                getReturn = DONT_REMOVE_STACK_VARIABLES ;
            else returnVariable = token ;
            continue ;
        }
        if ( addWords )
        {
            if ( svff )
            {

                nosv ++ ;
                if ( lispMode ) ctype = T_LISP_SYMBOL | STACK_VARIABLE ;
                else ctype = STACK_VARIABLE ;
            }
            else
            {
                nol ++ ;
                if ( lispMode ) ctype = T_LISP_SYMBOL | LOCAL_VARIABLE ;
                else ctype = LOCAL_VARIABLE ;
            }
            if ( regFlag == true )
            {
                ctype |= REGISTER_VARIABLE ;
                regVars ++ ;
            }
            word = _Word_New ( token, ctype, 0, 0 ) ;
            if ( regFlag == true )
            {
                if ( regToUse )
                {
                    word->RegToUse = regOrder [ regToUse ] ;
                    regToUse = 0 ;
                }
                else
                {
                    word->RegToUse = regOrder [ regIndex ++ ] ;
                    if ( regIndex == 3 ) regIndex = 0 ;
                }
            }
            regFlag = false ;
            DLList_AddNodeToTail ( locals, ( DLNode* ) word ) ;
            word->TypeNamespace = typeNamespace ;
            typeNamespace = 0 ;
        }
    }
    compiler->NumberOfLocals += nol ;
    compiler->NumberOfStackVariables += nosv ;
    compiler->NumberOfRegisterVariables += regVars ;
    compiler->State |= getReturn ;
    _CfrTil_->InNamespace = localsNs ;

    // we support nested locals and may have locals in other blocks so the indexes are cumulative
    nol = compiler->NumberOfLocals ;
    nosv = compiler->NumberOfStackVariables ;

    compiler->FunctionTypesArray = ( Namespace** ) _Mem_Allocate ( ( compiler->NumberOfStackVariables + 1 ) * sizeof ( Namespace * ), DICTIONARY ) ;

    uint64 ltype = lispMode ? T_LISP_SYMBOL : 0 ;
    for ( i = 1, node = DLList_First ( locals ) ; node ; node = DLNode_Next ( node ) )
    {
        word = ( Word * ) node ;
        //if ( word->CType & LOCAL_VARIABLE ) lWord = _CfrTil_LocalWord ( word->Name, -- nol, word->CType, ltype ) ;
        if ( word->CType & LOCAL_VARIABLE ) lWord = _DataObject_New ( word->CType, word->Name, word->CType, ltype, -- nol, 0 ) ;
        //else lWord = _CfrTil_LocalWord ( word->Name, -- nosv, word->CType, ltype ) ; 
        else lWord = _DataObject_New ( word->CType, word->Name, word->CType, ltype, -- nosv, 0 ) ;
        lWord->RegToUse = word->RegToUse ;
        lWord->TypeNamespace = word->TypeNamespace ;
        if ( nosv ) compiler->FunctionTypesArray [ i ++ ] = word->TypeNamespace ; // nosv : check not to exceed array bounds
    }
    if ( ! debugFlag ) Compile_InitRegisterVariables ( compiler ) ;
    if ( returnVariable ) compiler->ReturnVariableWord = Word_FindInOneNamespace ( localsNs, returnVariable ) ;

    _CfrTil_->InNamespace = saveInNs ;
    Stack_Init ( compiler->WordStack ) ;
    finder->w_Word = 0 ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, SESSION ) ;
    SetState ( compiler, VARIABLE_FRAME, true ) ;
    return localsNs ;
}

void
_Lexer_ParseString ( Lexer * lexer, int32 allocType )
{
    byte *s0 ;
    if ( ! ( s0 = _String_UnBox ( lexer->OriginalToken, allocType ) ) )
    {
        Lexer_SetState ( lexer, KNOWN_OBJECT, false ) ;
        return ;
    }
    lexer->Literal = ( int32 ) s0 ;
    if ( lexer->OriginalToken [ 0 ] == '"' ) lexer->TokenType = T_STRING ;
    else lexer->TokenType = T_RAW_STRING ;
    Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;

}

void
Lexer_ParseString ( Lexer * lexer )
{
    _Lexer_ParseString ( lexer, 0 ) ;
}

void
Lexer_ParseBinary ( Lexer * lexer, int offset )
{
    byte * token = & lexer->OriginalToken [offset] ;
    int32 cc = 0, i, l = strlen ( ( char* ) token ) ; // 8 bits/byte
    byte current ;
    for ( i = 0 ; i < l ; i ++ )
    {
        current = token [ l - i - 1 ] ; // 1 : remember zero based array indexing
        if ( current == '1' )
            cc += ( 1 << i ) ;
        else if ( current == '0' )
            continue ;
        else if ( current == ' ' )
            continue ;
        else
        {
            Lexer_SetState ( lexer, KNOWN_OBJECT, false ) ;
        }
    }
    lexer->Literal = cc ;
    Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
}

void
Lexer_ParseBigNum ( Lexer * lexer, byte * token )
{
    _CfrTil_Namespace_InNamespaceGet ( ) ;
    {
        byte * name = _CfrTil_InNamespace ( )->Name ;
        if ( strcmp ( ( char* ) name, "BigInt" ) == 0 )
        {
            mpz_t *bi = ( mpz_t* ) _CfrTil_BigInt_New ( 0 ) ;
            if ( token )
            {
                gmp_sscanf ( ( char* ) token, "%Zd", *bi ) ;
            }
            lexer->Literal = ( int32 ) bi ;
            lexer->TokenType = T_BIG_INT ;
            Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
        }
        else if ( strcmp ( ( char* ) name, "BigFloat" ) == 0 )
        {
            mpf_t *bf = ( mpf_t* ) _CfrTil_BigFloat_New ( 0 ) ;
            if ( token )
            {

                gmp_sscanf ( ( char* ) token, "%Fd", *bf ) ;
            }
            lexer->Literal = ( int32 ) bf ;
            lexer->TokenType = T_BIG_FLOAT ;
            Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
        }
    }
}
// return boolean 0 or 1 if lexer->Literal value is pushed

void
_Lexer_ParseObject ( Lexer * lexer, byte * token, int32 allocType )
{
    int32 offset = 0 ;
    float f ;
    lexer->OriginalToken = token ;
    lexer->Literal = 0 ;
    if ( token )
    {
        if ( token [0] == '#' ) // following scheme notation
        {
            char c ;
            if ( ( c = tolower ( token [1] ) ) == 'x' )
            {
                token [1] = c ;
                goto doHex ; // #x
            }
            if ( ( c = tolower ( token [1] ) ) == 'b' )
            {
                token [1] = c ;
                offset = 2 ;
                goto doBinary ; // #b
            }
            if ( tolower ( token [1] ) == 'd' ) goto doDecimal ; // #d
            //if ( tolower ( token [1] ) == 'o' ) goto doOctal ; // #o
        }
        if ( _Context_->System0->NumberBase == 10 )
        {
            doDecimal :
            if ( sscanf ( ( char* ) token, INT_FRMT, ( int* ) &lexer->Literal ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            if ( sscanf ( ( char* ) token, LISP_DECIMAL_FRMT, ( int* ) &lexer->Literal ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            else if ( sscanf ( ( char* ) token, "%f", &f ) )
            {
                lexer->TokenType = T_FLOAT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            else _Lexer_ParseString ( lexer, allocType ) ;
        }
        else if ( _Context_->System0->NumberBase == 2 )
        {
doBinary:
            Lexer_ParseBinary ( lexer, offset ) ;
            if ( Lexer_GetState ( lexer, KNOWN_OBJECT ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            else _Lexer_ParseString ( lexer, allocType ) ;
        }
        else if ( _Context_->System0->NumberBase == 16 )
        {
doHex:
            if ( sscanf ( ( char* ) token, HEX_INT_FRMT, ( unsigned int* ) &lexer->Literal ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            else if ( sscanf ( ( char* ) token, HEX_UINT_FRMT, ( unsigned int* ) &lexer->Literal ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }
            else if ( sscanf ( ( char* ) token, LISP_HEX_FRMT, ( unsigned int* ) &lexer->Literal ) )
            {
                lexer->TokenType = T_INT ;
                Lexer_SetState ( lexer, KNOWN_OBJECT, true ) ;
                Lexer_ParseBigNum ( lexer, token ) ;
            }

            else _Lexer_ParseString ( lexer, allocType ) ;
        }
    }
}

void
Lexer_ParseObject ( Lexer * lexer, byte * token )
{
    _Lexer_ParseObject ( lexer, token, TEMPORARY ) ;
}

void
CfrTil_Parse ( )
{
    Lexer * lexer = _Context_->Lexer0 ;
    byte * token = ( byte* ) _DataStack_Pop ( ) ;
    Lexer_ParseObject ( lexer, token ) ;
    _DataStack_Push ( lexer->Literal ) ;
}

byte *
Parse_Macro ( int64 type )
{
    byte * value ;
    Lexer * lexer = _Context_->Lexer0 ;
    if ( type == STRING_MACRO )
    {
        value = Lexer_ReadToken ( lexer ) ;
        while ( strcmp ( ";", ( char* ) Lexer_ReadToken ( lexer ) ) ) ; // nb. we take only the first string all else ignored
    }
    else if ( type == TEXT_MACRO )
    {
        int n = 0 ;
        Buffer * b = Buffer_New ( BUFFER_SIZE ) ;
        byte nc, *buffer = Buffer_Data ( b ) ;
        buffer [0] = 0 ;
        do
        {
            nc = _ReadLine_GetNextChar ( _Context_->ReadLiner0 ) ;
            //_Lexer_AppendCharToSourceCode ( lexer, nc ) ;
            if ( nc == ';' )
            {
                buffer [ n ] = 0 ;
                break ;
            }
            buffer [ n ++ ] = nc ;
        }
        while ( nc ) ;
        value = String_New ( ( byte* ) buffer, TEMPORARY ) ;
        Buffer_SetAsUnused ( b ) ;
        ;
    }
    return value ;
}
