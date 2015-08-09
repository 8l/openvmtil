
#include "../includes/cfrtil.h"
LambdaCalculus * _LC_ ;
// --------------------------------------------------------
// LC  : an abstract core of a prefix language related to Lambda Calculus with list objects
//       (it seems neither lisp nor scheme as they exist meet my more abstract intent so it's "Lambda Calculus")
// LO  : ListObject
// nb. : a Word is a ListObject is a Namespace is a DObject 
//     : a ListObject is a Word is a Namespace is a DObject 
// ---------------------------------------------------------

// lisp type lists can maybe be thought of as not rpn - not reverse polish notation, 
// not postfix but prefix - cambridge polish/prefix notation blocks

// sections 
// _LO_Eval
// _LO_Apply
// _LO_Read 
// _LO_Print
// _LO_SpecialFunctions
// LO_Repl
// LO_misc : _L0_New _L0_Copy
// LC_x

//===================================================================================================================
//| LO_Eval
//===================================================================================================================
// #define EVAL(x)         (isNum(x)? x : isSym(x)? val(x) : evList(x)) // picolisp

ListObject *
_LO_Eval ( ListObject * l0, ListObject * locals, int32 applyFlag )
{
    Compiler * compiler = _Context_->Compiler0 ;
    ListObject *lfunction, *ldata = 0, *lfirst ;
    Word * w ;
start:
    if ( l0 )
    {
        if ( LO_IsQuoted ( l0 ) ) return l0 ;
        else if ( ( l0->LType & T_LISP_SYMBOL ) )
        {
            w = _LO_FindWord ( l0->Lo_Name, locals ) ;
            if ( w )
            {
                if ( w->CType & ( CPRIMITIVE | CFRTIL_WORD | LOCAL_VARIABLE | STACK_VARIABLE | CATEGORY_RECURSIVE | T_LISP_COMPILED_WORD ) )
                {
                    if ( ! _LC_->DontCopyFlag ) l0 = LO_CopyOne ( l0 ) ;
                    l0->Lo_Value = w->Lo_Value ;
                    l0->Lo_CfrTilWord = w ;
                    l0->CType |= w->CType ;
                    l0->LType |= w->LType ;
                }
                else if ( w->LType & T_LISP_DEFINE ) // after macro because a macro is also a define
                {
                    l0 = ( ListObject * ) w->Lo_Value ;
                }
                else
                {
                    l0 = w ;
                }
                if ( CompileMode )
                {
                    // this maybe could be improved to be a list-block ?!?
                    if ( LO_CheckBeginBlock ( ) )
                    {
                        _LO_CompileOrInterpret_One ( l0 ) ;
                    }
                }
                if ( w->CType & COMBINATOR )
                {
                    Compiler_SetState ( compiler, LISP_COMBINATOR_MODE, true ) ;
                    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
                    ci.BlockLevel = compiler->BlockLevel ;
                    ci.ParenLevel = compiler->LispParenLevel ;
                    _Stack_Push ( compiler->CombinatorInfoStack, ( int32 ) ci.CI_i32_Info ) ; // this stack idea works because we can only be in one combinator at a time
                }
            }
        }
        else if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            if ( CompileMode )
            {
                LO_CheckEndBlock ( ) ;
                LO_CheckBeginBlock ( ) ;
            }
            compiler->LispParenLevel ++ ;
            lfirst = _LO_First ( l0 ) ;
            if ( lfirst )
            {
                if ( lfirst->LType & T_LISP_SPECIAL )
                {
                    l0 = LO_SpecialFunction ( l0, locals ) ;
                    compiler->LispParenLevel -- ;
                    goto done ;
                }
                lfunction = _LO_Eval ( lfirst, locals, applyFlag ) ;
                ldata = _LO_EvalList ( _LO_Next ( lfirst ), locals, applyFlag ) ;
                if ( applyFlag && lfunction && (
                    ( lfunction->CType & ( CPRIMITIVE | CFRTIL_WORD | CATEGORY_RECURSIVE ) ) ||
                    ( lfunction->LType & ( T_LISP_COMPILED_WORD ) )
#if LISP_NEW                    
                    || ( ( lfunction->LType & T_LISP_SYMBOL ) && ldata && GetState ( _LC_, LISP_COMPILE_MODE ) )
#endif                    
                    ) )
                {
                    l0 = _LO_Apply ( l0, lfunction, ldata ) ;
                }
                else if ( lfunction && ( lfunction->LType & T_LAMBDA ) && lfunction->Lo_CfrTilWord->Lo_LambdaFunctionBody )
                {
                    locals = ldata ;
                    // LambdaArgs, the formal args, are not changed by LO_Substitute (locals - lvals are just essentially renamed) and thus don't need to be copied
                    LO_Substitute ( _LO_First ( ( ListObject * ) lfunction->Lo_CfrTilWord->Lo_LambdaFunctionParameters ), _LO_First ( locals ) ) ;
                    //l0 = LO_Copy ( ( ListObject * ) lproc->Lo_CfrTilWord->Lo_LambdaBody ) ; 
                    // nb. this logic (below) is to prevent having to copy the whole LambdaBody ; saving a lot of unneeded memory use
                    if ( ! _LC_->CurrentFunction )
                    {
                        _LC_->CurrentFunction = lfunction ;
                    }
                    else if ( _LC_->CurrentFunction == lfunction )
                    {
                        if ( _LC_->Loop ++ > 1 ) _LC_->DontCopyFlag = 1 ;
                    }
                    else
                    {
                        _LC_->CurrentFunction = 0 ;
                        _LC_->DontCopyFlag = 0 ;
                    }
                    l0 = ( ListObject * ) lfunction->Lo_CfrTilWord->Lo_LambdaFunctionBody ;
                    goto start ;
                }
                else
                {
                    SetState ( _LC_, LISP_COMPILE_MODE, false ) ;
                    if ( ldata )
                    {
                        // return the evaluated list
                        DLList_AddNodeToHead ( ldata->W_dll_SymbolList, ( DLNode* ) lfunction ) ;
                        return ldata ;
                    }
                    else
                    {
                        return lfunction ; //seems more common sense for this !?!? ...
                        //return l0 ; //... also a possibility - how should this be defined/is it standardly defined?
                    }
                }
            }
        }
    }
done:
    return l0 ;
}

ListObject *
_LO_EvalList ( ListObject * lorig, ListObject * locals, int32 applyFlag )
{
    ListObject * lnew = 0, *lnode ;
    if ( lorig )
    {
        lnew = LO_New ( LIST, 0 ) ;
        for ( lnode = lorig ; lnode ; lnode = _LO_Next ( lnode ) ) // eval each node
        {
            lnode->State |= LISP_EVALLIST_ARG ;
            LO_AddToTail ( lnew, LO_CopyOne ( _LO_Eval ( lnode, locals, applyFlag ) ) ) ; // this should be optimizable
        }
    }
    return lnew ;
}

// untested

ListObject *
_LO_MacroPreprocess ( ListObject * l0 )
{
    if ( LO_IsQuoted ( l0 ) ) return l0 ;
    while ( _LO_First ( l0 )->LType & T_LISP_MACRO ) l0 = _LO_Eval ( l0, 0, 0 ) ;
    return l0 ;
}

//===================================================================================================================
//| _LO_Apply 
//===================================================================================================================

// assumes list contains only one application 

void
_LO_CompileOrInterpret_One ( ListObject * l0 )
{
    // just interpret the non-nil, non-list objects
    // nil means that it doesn't need to be interpreted any more
    if ( ( l0 ) && ( ! ( l0->LType & ( LIST | LIST_NODE | T_NIL ) ) ) )
    {
        Word * word = l0->Lo_CfrTilWord ;
        if ( GetState ( _CfrTil_, DEBUG_MODE ) )
        {
            DebugColors ;
            Printf ( ( byte* ) "\n_LO_CompileOrInterpret_One : \n\tl0 =%s, l0->Lo_CfrTilWord = %s", _LO_PRINT ( l0 ), word ? word->Name : ( byte* ) "" ) ;
            DefaultColors ;
        }
        if ( word &&
            ( !
            ( l0->LType & ( LITERAL | T_LISP_SYMBOL ) ) ||
            ( l0->CType & ( BLOCK | CFRTIL_WORD | CPRIMITIVE ) ) ||
            ( CompileMode && ( l0->CType & ( LOCAL_VARIABLE | STACK_VARIABLE ) ) ) )
            )
        {
            _Word_Eval ( word ) ;
        }
        else
        {
            _Compiler_WordStack_PushWord ( _Context_->Compiler0, word ? word : l0 ) ; // ? l0 or word ?
            if ( word ) DataObject_Run ( word ) ; //Do_VariableOrLiteral ( word ) ;
            else DataObject_Run ( l0 ) ; //Do_VariableOrLiteral ( l0 ) ; _Do_Literal ( ( int32 ) l0->Lo_Value ) ;
        }
    }
}

void
_LO_CompileOrInterpret ( ListObject * lfunction, ListObject * ldata )
{
    if ( GetState ( _CfrTil_, DEBUG_MODE ) )
    {
        DebugColors ;
        Printf ( ( byte* ) "\n_LO_CompileOrInterpret : \n\tlfunction =%s\n\tldata =%s", _LO_PRINT ( lfunction ), _LO_PRINT ( ldata ) ) ;
        DefaultColors ;
    }
    ListObject * lfword = lfunction->Lo_CfrTilWord ;

    if ( ldata && lfword && ( lfword->CType & ( CATEGORY_OP_ORDERED | CATEGORY_OP_UNORDERED ) ) ) // ?!!? 2 arg op with multi-args : this is not a precise distinction yet : need more types ?!!? 
    {
        _LO_CompileOrInterpret_One ( ldata ) ;
        while ( ( ldata = _LO_Next ( ldata ) ) )
        {
            _LO_CompileOrInterpret_One ( ldata ) ; // two args first then op, then after each arg the operator : nb. assumes word can take unlimited args 2 at a time
            _LO_CompileOrInterpret_One ( lfword ) ;
        }
    }
    else
    {
        // interpret them in forward polish notation as opposed to RPN
        // nb. uses the optimizer as in non lisp mode 
        for ( ; ldata ; ldata = _LO_Next ( ldata ) )
        {
            _LO_CompileOrInterpret_One ( ldata ) ;
        }
        _LO_CompileOrInterpret_One ( lfword ) ;
    }
}
// for calling 'C' functions such as printf or other system functions
// where the arguments are pushed first from the end of the list like 'C' arguments

ListObject *
_LO_Apply ( ListObject * l0, ListObject * lfunction, ListObject * ldata )
{
    ListObject * lfdata = _LO_First ( ldata ) ;
    if ( GetState ( _CfrTil_, DEBUG_MODE ) )
    {
        DebugColors ;
        Printf ( ( byte* ) "\n_LO_Apply : \n\tl0 =%s", _LO_PRINT ( l0 ) ) ;
        DefaultColors ;
    }
    if ( lfunction->LType & LIST_FUNCTION ) return ( ( ListFunction ) lfunction->Lo_CfrTilWord->Definition ) ( l0 ) ;
    else if ( lfdata )
    {
        _LO_CompileOrInterpret ( lfunction, lfdata ) ;
        _Context_->Compiler0->LispParenLevel -- ;
        if ( CompileMode ) LO_CheckEndBlock ( ) ;
        return LO_PrepareReturnObject ( ) ;
    }
    else
    {
        _Context_->Compiler0->LispParenLevel -- ;
        if ( CompileMode ) LO_CheckEndBlock ( ) ;
        SetState ( _LC_, LISP_COMPILE_MODE, false ) ;
        return lfunction ;
    }
}

// subst : lisp 1.5

void
LO_Substitute ( ListObject *args, ListObject * vals )
{
    while ( args && vals )
    {
        // just preserve the name of the arg for the finder
        vals->Lo_Name = args->Lo_Name ;
        args = _LO_Next ( args ) ;
        vals = _LO_Next ( vals ) ;
    }
}

//===================================================================================================================
//| LO_SpecialFunction(s) 
//===================================================================================================================

block
CompileLispBlock ( ListObject *args, ListObject * body )
{
    Compiler *compiler = _Context_->Compiler0 ;
    block code ;
    byte * here = Here ;
    LO_BeginBlock ( ) ; // must have a block before local variables
    Namespace * locals = _CfrTil_Parse_LocalsAndStackVariables ( 1, 0, 1, args ) ;
    Word * word = compiler->CurrentWord ;
    compiler->RecursiveWord = word ;
    // ?!? : consider all lisp compiled words as possibly recursive
    word->CType = BLOCK | CATEGORY_RECURSIVE ;
    word->LType |= T_LISP_COMPILED_WORD ;
    SetState ( _LC_, LISP_COMPILE_MODE, true ) ;
    _LO_Eval ( body, locals, 1 ) ;
    LO_EndBlock ( ) ;
    if ( GetState ( _LC_, LISP_COMPILE_MODE ) )
    {
        code = ( block ) _DataStack_Pop ( ) ;
    }
    else // nb. LISP_COMPILE_MODE : this state can change with some functions that can't be compiled yet
    {
        SetHere ( here ) ; //recover the unused code space
        code = 0 ;
        word->LType &= ~ T_LISP_COMPILED_WORD ;
        if ( _Q_->Verbosity > 1 )
        {
            AlertColors ;
            Printf ( "\nLisp can not compile this word yet : %s : -- interpreting ...\n ", _Word_Location_pbyte ( word ) ) ;
            DefaultColors ;
        }
    }
    _Word ( word, ( byte* ) code ) ; // nb. LISP_COMPILE_MODE is reset by _Word_Finish
    SetState ( word, NOT_COMPILED, false ) ;

    return code ;
}

ListObject *
_LO_MakeLambda ( ListObject * l0 )
{
    // lambdaSignature is "lambda" or "/.", etc.
    ListObject *args, *body, *word = 0, *signature = _LO_First ( l0 ), *lnew, *last ;
    // allow args to be optionally an actual parenthesized list or just vars after the lambda
    args = _LO_Next ( signature ) ;
    last = _LO_Last ( l0 ) ;
    if ( args->LType & ( LIST | LIST_NODE ) )
    {
        args = _LO_Copy ( args, LispAllocType ) ;
    }
    else
    {
        lnew = LO_New ( LIST, 0 ) ;
        do
        {
            LO_AddToTail ( lnew, _LO_CopyOne ( args, LispAllocType ) ) ;
        }
        while ( ( args = _LO_Next ( args ) ) != last ) ;
        args = lnew ;
    }
    if ( ! ( last->LType & ( LIST | LIST_NODE ) ) )
    {
        lnew = LO_New ( LIST, 0 ) ;
        LO_AddToTail ( lnew, _LO_CopyOne ( last, LispAllocType ) ) ;
        body = lnew ;
    }
    else body = _LO_Copy ( last, LispAllocType ) ;
    if ( GetState ( _LC_, LC_DEFINE_MODE ) ) word = _Context_->Compiler0->CurrentWord ;
    if ( ! word ) word = _Word_New ( "anonymous", WORD_CREATE, 0, DICTIONARY ) ;
    if ( GetState ( _LC_, LISP_COMPILE_MODE ) )
    {
        SetState ( _LC_, LISP_LAMBDA_MODE, true ) ;
        block codeBlk = CompileLispBlock ( args, body ) ;
        word->Lo_Value = ( Object* ) codeBlk ;
    }
    if ( ! GetState ( _LC_, LISP_COMPILE_MODE ) ) // nb. this needs to be 'if' not 'else if' because the state is sometimes changed, eg. for function parameters
    {
        word->Lo_CfrTilWord = word ;
        word->Lo_LambdaFunctionParameters = args ;
        word->Lo_LambdaFunctionBody = body ;
        word->LType = T_LAMBDA ;
        word->CType = 0 ;
    }
    return word ;
}

ListObject *
_LO_Define0 ( byte * sname, ListObject * nameNode, ListObject * locals )
{
    Compiler * compiler = _Context_->Compiler0 ;
    byte * b = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    ListObject *value, *l1 ;
    value = _LO_Next ( nameNode ) ;
    Word * word = nameNode->Lo_CfrTilWord ;
    compiler->CurrentWord = word ;
    word->Lo_CfrTilWord = word ;
    sprintf ( ( char* ) b, " ( %s %s )", sname, ( byte* ) _LO_PrintList ( nameNode, 0, 0, 0 ) ) ;
    word->SourceCode = String_New ( b, DICTIONARY ) ;
    _Namespace_DoAddWord ( _LC_->LispNamespace, word ) ; // put it at the beginning of the list to be found first
    word->CType = VARIABLE ; // nb. !
    SetState ( word, NOT_COMPILED, true ) ; // a needed property to compile recursive words 
    //SetState ( _LC_, ( LC_DEFINE_MODE | PRINT_VALUE ), true ) ;
    SetState ( _LC_, ( LC_DEFINE_MODE ), true ) ;
    value = _LO_Eval ( value, locals, 0 ) ; // 0 : don't apply
    // traverse tree, move all to LispNamespace - we want to keep these as a non-temporary part of Lisp
    if ( value->LType & ( T_LAMBDA ) ) // | T_LISP_MACRO ) )
    {
        value->Lo_LambdaFunctionParameters = _LO_Copy ( value->Lo_LambdaFunctionParameters, LISP ) ;
        value->Lo_LambdaFunctionBody = _LO_Copy ( value->Lo_LambdaFunctionBody, LISP ) ;
    }
    else value = _LO_Copy ( value, LISP ) ; // this object now becomes part of LISP permanent memory - not a temp
    word->Lo_Value = ( Object* ) value ; // Lo_Value = Lo_Object
    word->LType |= ( T_LISP_DEFINE | T_LISP_SYMBOL ) ; //| value->LType) ; // Lo_Value = Lo_Object
    // the value was entered into the LISP memory, now we need a temporary carrier to print
    l1 = _LO_New ( word->LType, word->CType, ( Object* ) value, word, 0, 0, LispAllocType ) ; // all words are symbols
    CfrTil_NewLine ( ) ; // always print nl before a define to make easier reading
    return l1 ;
}

ListObject *
_LO_Define ( ListObject * l0, ListObject * locals )
{
    ListObject * nameNode = _LO_Next ( l0 ) ;
    return _LO_Define0 ( "define", nameNode, locals ) ;
}

ListObject *
_LO_Compile ( ListObject * l0, ListObject * locals )
{
    Compiler_SetState ( _Context_->Compiler0, RETURN_TOS, true ) ;
    SetState ( _LC_, LISP_COMPILE_MODE, true ) ;
    l0 = _LO_Define ( l0, locals ) ;
    SetState ( _LC_, LISP_COMPILE_MODE, false ) ;

    return l0 ;
}

ListObject *
LO_Define ( ListObject * l0, ListObject * locals )
{
    return _LO_Compile ( l0, locals ) ;
}

ListObject *
LO_MakeLambda ( ListObject * l0 )
{
    Word * word = _LO_MakeLambda ( l0 ) ;
    word->LType |= T_LAMBDA ;

    return word ;
}

ListObject *
_LO_Cons ( ListObject *first, ListObject * second, uint64 allocType )
{
    ListObject * l0 = _LO_New ( LIST, 0, 0, 0, 0, 0, allocType ) ;
    LO_AddToTail ( l0->Lo_List, first ) ;
    LO_AddToTail ( l0->Lo_List, second ) ;

    return l0 ;
}

ListObject *
LO_If ( ListObject * l0, ListObject * locals )
{
    ListObject * tf, *test, *trueList, *elseList, *value ;
    test = _LO_Next ( l0 ) ;
    trueList = _LO_Next ( test ) ;
    elseList = _LO_Next ( trueList ) ;
    tf = _LO_Eval ( test, locals, 1 ) ;
    if ( tf->Lo_Value && ( tf != nil ) ) value = _LO_Eval ( trueList, locals, 1 ) ;
    else value = _LO_Eval ( elseList, locals, 1 ) ;

    return value ;
}

// lisp cond : conditional

ListObject *
LO_Cond ( ListObject * l0, ListObject * locals )
{
    ListObject * tf, *trueNode, * elseNode = nil ;
    // 'cond' is first node ; skip it.
    l0 = _LO_First ( l0 ) ;
    tf = _LO_Next ( l0 ) ;
    while ( ( trueNode = _LO_Next ( tf ) ) )
    {
        tf = _LO_Eval ( tf, locals, 1 ) ;
        if ( ( tf != nil ) && ( tf->Lo_Value ) ) return _LO_Eval ( LO_CopyOne ( trueNode ), locals, 1 ) ;
            // nb we have to copy one here else we return the whole rest of the list 
            //and we can't remove it else it could break a LambdaBody, etc.

        else tf = elseNode = _LO_Next ( trueNode ) ;
    }
    return _LO_Eval ( elseNode, locals, 1 ) ; // last one no need to copy
}

// lisp 'list' function

ListObject *
LO_List ( ListObject * l0 )
{
    ListObject * lnew ;
    if ( l0 )
    {
        lnew = LO_New ( LIST, 0 ) ;
        l0 = _LO_First ( l0 ) ; // first is the 'list' token

        while ( l0 = _LO_Next ( l0 ) ) LO_AddToTail ( lnew, LO_Copy ( l0 ) ) ;
    }
    else lnew = 0 ;
    return lnew ;
}

ListObject *
LO_Begin ( ListObject * l0, ListObject * locals )
{
    ListObject * leval ;
    // 'begin' is first node ; skip it.
    if ( l0 )
    {
        for ( l0 = _LO_Next ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            leval = _LO_Eval ( l0, locals, 1 ) ;
        }
    }
    else leval = 0 ;
    return leval ;
}

ListObject *
LO_SpecialFunction ( ListObject * l0, ListObject * locals )
{
    ListObject * lfirst = _LO_First ( l0 ) ;
    switch ( lfirst->LType & ( ~ ( T_LISP_SYMBOL | T_LISP_SPECIAL | LISP_VOID_RETURN ) ) )
    {
        case T_LISP_BEGIN:
        {
            return LO_Begin ( lfirst, locals ) ;
        }
        case T_LISP_DEFINE:
        {
            return LO_Define ( lfirst, locals ) ; // same as compile
        }
        case T_LISP_MACRO:
        {
            return _LO_Macro ( lfirst, locals ) ; // same as compile
        }
        case T_LISP_COMPILE:
        {
            return _LO_Compile ( lfirst, locals ) ;
        }
        case T_LAMBDA:
        {
            return LO_MakeLambda ( l0 ) ;
        }
        case T_LISP_IF:
        {
            return LO_Cond ( lfirst, locals ) ;
        }
        case T_LISP_SET: return LO_Set ( lfirst, locals ) ;
        case T_LISP_LET: return LO_Let ( lfirst, locals ) ;
        case LISP_C_RTL_ARGS:
        {
            // ?!? LISP_C_RTL_ARGS here is hasn't been maintained ?!?
            //return _LO_CompileRun_C_Rtl_ArgList ( l0, lfirst ) ;
            return _LO_Apply_C_Rtl_ArgList ( l0, lfirst ) ;
        }
        default:
        {

            return (( LispFunction2 ) ( lfirst->Lo_CfrTilWord->Definition ) ) ( lfirst, locals ) ;
        }
    }
}

ListObject *
_LO_Macro ( ListObject * l0, ListObject * locals )
{
    ListObject * nameNode = _LO_Next ( l0 ) ;
    l0 = _LO_Define0 ( "macro", nameNode, locals ) ; //nameNode, locals ) ;
    l0->LType |= T_LISP_MACRO ;

    return l0 ;
}

// setq

ListObject *
_LO_Set ( ListObject * lfirst, Namespace * ns )
{
    ListObject *l0, *lnext, * lsymbol, *lvalue ;
    // lfirst is the 'set' signature
    for ( l0 = lfirst ; ( lnext = _LO_Next ( l0 ) ) && ( lvalue = _LO_Next ( lnext ) ) ; l0 = lvalue )
    {
        lsymbol = lnext ; // we want to return the last symbol
        LO_Print ( _LO_Define0 ( "set", lsymbol, ns ) ) ;
    }
    return _LC_->True ;
}

ListObject *
LO_Set ( ListObject * lfirst, ListObject * locals )
{

    return _LO_Set ( lfirst, locals ? locals : _LC_->LispNamespace ) ;
}

ListObject *
LO_Let ( ListObject * lfirst, ListObject * locals )
{
    return _LO_Set ( lfirst, locals ) ;
}

//
// nb. we must not use the word as the ListObject because it is on its own namespace list
// so we create a new ListObject with a pointer to the word for its values/properties

// can we just use the 'word' instead of this

//===================================================================================================================
//| _LO_Read 
//===================================================================================================================

// remember a Word is a union with ListObject 

ListObject *
LO_New_ParseRawStringOrLiteral ( byte * token, int32 parseFlag )
{
    Lexer * lexer = _Context_->Lexer0 ;
    if ( parseFlag ) Lexer_ParseObject ( lexer, token ) ; // same as Lexer_Literal except also consider a raw string as a token
    Word * word ;

    if ( Lexer_GetState ( lexer, KNOWN_OBJECT ) )
    {
        uint64 ctokenType = lexer->TokenType | LITERAL ;
        //word = _DObject_New ( lexer->OriginalToken, lexer->Literal, ctokenType, ctokenType, LITERAL, ( byte* ) DataObject_Run, 0, 0, 0, LispAllocType ) ;
        word = _Word_New ( lexer->OriginalToken, ctokenType, LITERAL, LispAllocType ) ;
        word->RunType = LITERAL ;
        if ( lexer->TokenType & T_RAW_STRING )
        {
            // nb. we don't want to do this block with literals it slows down the eval and is wrong
            word->LType |= T_LISP_SYMBOL ;
            _Namespace_DoAddWord ( _LC_->LispTemporariesNamespace, word ) ; // put it at the beginning of the list to be found first
            word->Lo_Value = ( Object* ) nil ;
        }
        else
        {
            word->Lo_Value = ( Object* ) lexer->Literal ;
            word->LType |= ctokenType ;
        }
        word->Lo_CfrTilWord = word ;
        _DObject_Init ( word, lexer->Literal, ctokenType, LITERAL, ( byte* ) DataObject_Run, 0, 0, 0 ) ; // nb! after the above initialization 
        return word ; // remember a Word is ListObject is a Namespace is a DObject is a Class is a DynamicObject.
    }
    else
    {
        Printf ( ( byte* ) "\n%s ?\n", ( char* ) token ) ;
        CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
        return 0 ;
    }
}

void
LO_Quote ( )
{
    if ( _LC_ ) _LC_->ItemQuoteState |= QUOTED ;
    else Lexer_CheckMacroRepl ( _Context_->Lexer0 ) ;
}

void
LO_QuasiQuote ( )
{
    if ( _LC_ ) _LC_->ItemQuoteState |= QUASIQUOTED ;
    else Lexer_CheckMacroRepl ( _Context_->Lexer0 ) ;
}

void
LO_UnQuoteSplicing ( )
{
    _LC_->ItemQuoteState |= UNQUOTE_SPLICE ;
}

void
LO_UnQuote ( )
{
    _LC_->ItemQuoteState |= UNQUOTED ;
}

ListObject *
_LO_Read ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    Lexer * lexer = _Context_->Lexer0 ;
    ListObject *l0, *lreturn, *lnew = compiler->LispParenLevel ? LO_New ( LIST, 0 ) : 0 ;
    byte * token, *token1 ;
    if ( ! _LC_ ) LC_New ( 1 ) ;
    lreturn = lnew ;
    do
    {
next:
        if ( Lexer_IsTokenQualifiedID ( lexer ) ) SetState ( compiler, PARSING_QUALIFIED_ID, true ) ;
        else SetState ( compiler, PARSING_QUALIFIED_ID, false ) ;
        token = _Lexer_ReadToken ( lexer, ( byte* ) " ,;\n\r\t" ) ;
        if ( token )
        {
            if ( String_Equal ( ( char* ) token, "(" ) )
            {
                Stack_Push ( _LC_->QuoteStateStack, _LC_->QuoteState ) ;
                _LC_->QuoteState = _LC_->ItemQuoteState ;
                compiler->LispParenLevel ++ ;
                l0 = _LO_Read ( ) ;
                l0 = LO_New ( LIST_NODE, l0 ) ;
                _LC_->QuoteState = Stack_Pop ( _LC_->QuoteStateStack ) ;
            }
            else if ( String_Equal ( ( char* ) token, ")" ) ) break ;
            else
            {
                Word * word = _LO_FindWord ( token, 0 ) ;
                if ( word )
                {
                    if ( word->LType & T_LISP_READ_MACRO )
                    {
                        word->Definition ( ) ; // scheme read macro preprocessor 
                        goto next ;
                    }
                    if ( word->LType & T_LISP_TERMINATING_MACRO )
                    {
                        word->Definition ( ) ; // scheme read macro preprocessor 
                        token1 = ( byte* ) _DataStack_Pop ( ) ;
                        l0 = LO_New_ParseRawStringOrLiteral ( token1, 0 ) ; //don't parse a string twice; but other macros may need to be adjusted 
                    }
                    else
                    {
                        l0 = _LO_New ( T_LISP_SYMBOL | word->LType, word->CType, word->Lo_Value, word, 0, 0, LispAllocType ) ; // all words are symbols
                    }
                }
                else
                {
                    l0 = LO_New_ParseRawStringOrLiteral ( token, 1 ) ;
                    if ( GetState ( compiler, PARSING_QUALIFIED_ID ) ) SetState ( l0, QUALIFIED_ID, true ) ;
                }
            }
            l0->State |= ( _LC_->ItemQuoteState | _LC_->QuoteState ) ;
            if ( ( l0->State & UNQUOTE_SPLICE ) && ( ! ( l0->State & QUOTED ) ) ) //&& ( l0->LType & ( LIST_NODE | LIST ) ) )
            {
                l0 = LO_Eval ( LO_Copy ( l0 ) ) ;
                if ( lnew )
                {
                    LO_SpliceAtTail ( lnew, l0 ) ;
                }
                else
                {
                    lreturn = l0 ;
                    break ;
                }
            }
            else if ( lnew )
            {
                //if ( ! GetState ( compiler, LC_PREFIX_ARG_PARSING ) ) 
                LO_AddToTail ( lnew, l0 ) ;
            }
            else
            {
                lreturn = l0 ; // did i break this by the "if ( l0 )" ?
                break ;
            }
        }
        else Error ( "\n_LO_Read : Syntax error : no token?\n", QUIT ) ;
        _LC_->ItemQuoteState = 0 ;
    }
    while ( compiler->LispParenLevel ) ;
    return lreturn ;
}

//===================================================================================================================
//| LO Misc : _LO_FindWord _LO_New _LO_Copy
//===================================================================================================================

ListObject *
_LO_First ( ListObject * l0 )
{
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) return ( ListObject* ) DLList_First ( ( DLList * ) l0->Lo_List ) ;

        else return l0 ;
    }
    return 0 ;
}

ListObject *
_LO_Last ( ListObject * l0 )
{
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) return ( ListObject* ) DLList_Last ( ( DLList * ) l0->Lo_List ) ;

        else return l0 ;
    }
    return 0 ;
}

ListObject *
_LO_Next ( ListObject * l0 )
{
    return ( ListObject* ) DLNode_Next ( ( DLNode* ) l0 ) ;
}

Word *
_LO_FindWord ( byte * name, ListObject * locals )
{
    Word * word = 0 ;
    if ( locals )
    {
        word = Word_FindInOneNamespace ( locals, name ) ;
        //if ( word ) word->LType |= T_LISP_LOCALS_ARG ;
    }
    if ( ! word )
    {
        word = Word_FindInOneNamespace ( _LC_->LispTemporariesNamespace, name ) ;
        if ( ! word )
        {
            word = Word_FindInOneNamespace ( _LC_->LispNamespace, name ) ; // prefer Lisp namespace
            if ( ! word )
            {

                word = Finder_Word_FindUsing ( _Context_->Finder0, name ) ;
            }
        }
    }
    return word ;
}

ListObject *
_LO_New ( uint64 lType, uint64 ctype, Object * value, Word * word, int32 addFlag, byte * name, uint64 allocType )
{
#if 0    
    ListObject * l0 = _Word_Allocate ( T_LISP_SYMBOL ) ; //( ListObject * ) _Object_Allocate ( sizeof (Word ), allocType ) ;
    if ( lType & T_LISP_SYMBOL ) l0->RunType = word->RunType ;
#else    
    ListObject * l0 = ( ListObject * ) _Object_Allocate ( sizeof (Word ), allocType ) ;
#endif    
    l0->CType = ctype ;
    l0->LType = lType ;
    l0->Lo_Value = value ; // Lo_Value = Lo_Object = Lo_List : is a union with SymbolList in case the object is a List
    if ( lType & LIST ) _LO_ListInit ( l0, allocType ) ;
    else if ( ctype & LIST_NODE ) l0->Lo_List = ( DLList* ) l0->Lo_Value ;
    if ( word )
    {
        l0->Lo_Name = word->Name ;
        l0->Lo_CfrTilWord = word ;
        word->Lo_CfrTilWord = word ;
        word->Lo_Value = value ;
        if ( addFlag )
        {
            _Namespace_DoAddWord ( _LC_->LispTemporariesNamespace, word ) ; // put it at the beginning of the list to be found first
        }
    }
    else if ( name && ( allocType != LispAllocType ) ) l0->Lo_Name = String_New ( name, allocType ) ;
    else l0->Name = name ;

    return l0 ;
}

void
LO_SpliceAtTail ( ListObject * lnew, ListObject * l0 ) // ltbat : list to be added to ; ltbat : list to be added 
{
    if ( lnew )
    {
        for ( l0 = _LO_First ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            LO_AddToTail ( lnew, LO_CopyOne ( l0 ) ) ;
        }
    }
}

ListObject *
_LO_AllocCopyOne ( ListObject * l0, uint64 allocType )
{
    ListObject * l1 = 0 ;
    if ( l0 )
    {
        l1 = ( ListObject * ) _Object_Allocate ( sizeof ( ListObject ), allocType ) ; //Mem_Allocate ( ( sizeof (ListObject ) + ((slots-1) * CELL), AllocType ) ;
        memcpy ( l1, l0, sizeof ( ListObject ) ) ;
        // nb. since we are coping the car/cdr are the same as the original so we must clear them else when try to add to the list and remove first it will try to remove from a wrong list so ...
        l1->Lo_Car = 0 ;
        l1->Lo_Cdr = 0 ;
    }
    return l1 ;
}

void
_LO_ListInit ( ListObject * l0, uint64 allocType )
{

    l0->Lo_Head = _DLNode_New ( allocType ) ;
    l0->Lo_Tail = _DLNode_New ( allocType ) ;
    _DLList_Init ( ( DLList * ) l0 ) ;
    l0->Lo_List = ( DLList* ) l0 ;
    l0->LType |= LIST ; // a LIST_NODE needs to be initialized also to be also a LIST
}

ListObject *
_LO_ListNode_Copy ( ListObject * l0, uint64 allocType )
{
    ListObject * l1 = _LO_AllocCopyOne ( l0, allocType ) ;
    _LO_ListInit ( l1, allocType ) ;

    return l1 ;
}

ListObject *
_LO_CopyOne ( ListObject * l0, uint64 allocType )
{
    ListObject *l1 = 0 ;
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            l1 = _LO_Copy ( l0, allocType ) ;
            if ( l0->LType & LIST_NODE ) l1 = _LO_New ( LIST_NODE, 0, ( Object * ) l1, 0, 0, 0, allocType ) ;
        }

        else l1 = _LO_AllocCopyOne ( l0, allocType ) ;
    }
    return l1 ;
}

// copy a whole list or a single node

ListObject *
_LO_Copy ( ListObject * l0, uint64 allocType )
{
    ListObject * lnew = 0, *l1 ;
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) lnew = _LO_ListNode_Copy ( l0, allocType ) ;
        for ( l0 = _LO_First ( l0 ) ; l0 ; l0 = _LO_Next ( l0 ) )
        {
            l1 = _LO_CopyOne ( l0, allocType ) ;
            if ( lnew ) LO_AddToTail ( lnew, l1 ) ;

            else return l1 ;
        }
    }
    return lnew ;
}

Boolean
LO_strcat ( char * buffer, char * buffer2 )
{
    if ( strlen ( buffer2 ) + strlen ( buffer ) >= BUFFER_SIZE ) return false ;
    else strcat ( buffer, buffer2 ) ;
    buffer2 [0] = 0 ;
    return true ;
}

ListObject *
LO_PrepareReturnObject ( )
{
    uint64 type = 0 ;
    byte * name ;
    if ( ! CompileMode )
    {
        Namespace * ns = _CfrTil_InNamespace ( ) ;
        name = ns->Name ;
        if ( String_Equal ( ( char* ) name, "BigInt" ) )
        {
            type = T_BIG_INT ;
        }
        else if ( String_Equal ( ( char* ) name, "BigFloat" ) )
        {
            type = T_BIG_FLOAT ;
        }
        return _LO_New ( LITERAL | type, LITERAL | type, ( Object* ) _DataStack_Pop ( ), 0, 0, 0, LispAllocType ) ;
    }
    else return nil ;
}

void
LO_BeginBlock ( )
{

    if ( ! _Context_->Compiler0->BlockLevel ) _LC_->SavedCodeSpace = CompilerMemByteArray ;
    _Compiler_SetCompilingSpace ( CompileMode ? ( byte* ) "CodeSpace" : ( byte* ) "TempObjectSpace" ) ;
    CfrTil_BeginBlock ( ) ;
}

void
LO_EndBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( _LC_ && _LC_->SavedCodeSpace )
    {
        NGR_Dsp_To_ESI ;
        CfrTil_EndBlock ( ) ;
        if ( ! GetState ( _LC_, LISP_COMPILE_MODE ) ) CfrTil_BlockRun ( ) ;

        if ( ! compiler->BlockLevel ) Set_CompilerSpace ( _LC_->SavedCodeSpace ) ;
    }
}

void
LO_CheckEndBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    if ( Compiler_GetState ( compiler, LISP_COMBINATOR_MODE ) )
    {
        int32 cii = _Stack_Top ( compiler->CombinatorInfoStack ) ;
        CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
        ci.CI_i32_Info = cii ;
        if ( ( compiler->LispParenLevel == ci.ParenLevel ) && ( compiler->BlockLevel > ci.BlockLevel ) )
        {
            LO_EndBlock ( ) ;
        }
    }
}

int32
LO_CheckBeginBlock ( )
{
    Compiler * compiler = _Context_->Compiler0 ;
    int32 cii = _Stack_Top ( compiler->CombinatorInfoStack ) ;
    CombinatorInfo ci ; // remember sizeof of CombinatorInfo = 4 bytes
    ci.CI_i32_Info = cii ;
    if ( ( Compiler_GetState ( compiler, LISP_COMBINATOR_MODE ) ) && ( compiler->LispParenLevel == ci.ParenLevel ) && ( compiler->BlockLevel == ci.BlockLevel ) )
    {
        LO_BeginBlock ( ) ;

        return true ;
    }
    return false ;
}

// for calling 'C' functions such as printf or other system functions
// where the arguments are pushed first from the end of the list like 'C' arguments

ListObject *
_LO_Apply_C_Rtl_ArgList ( ListObject * l0, Word * word )
{
    ListObject *l1, *l2 ;
    Word * word1 ;
    ByteArray * scs ;
    Compiler * compiler = _Context_->Compiler0 ;
    int32 i, svcm0 = GetState ( _LC_, LISP_COMPILE_MODE ), svcm = Compiling ;

    Debugger * debugger = _CfrTil_->Debugger0 ;
    int32 dm = GetState ( _CfrTil_, DEBUG_MODE ) && ( ! GetState ( debugger, DBG_STEPPING ) ) ;
    if ( l0 )
    {
        if ( ! svcm )
        {
            scs = CompilerMemByteArray ;
            _Compiler_SetCompilingSpace ( ( byte* ) "SessionObjectsSpace" ) ;
            CfrTil_BeginBlock ( ) ;
        }
        for ( i = 0, l1 = LO_Last ( l0 ) ; l1 ; i ++, l1 = LO_Previous ( l1 ) )
        {
            if ( dm ) _Debugger_PreSetup ( debugger, 0, word1 = l1->Lo_CfrTilWord ) ;
            if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
            {
                if ( l1->LType & ( LIST | LIST_NODE ) )
                {
                    int32 svlcms2 = GetState ( _LC_, LISP_COMPILE_MODE ) ;
                    SetState ( _LC_, LISP_COMPILE_MODE, false ) ;
                    l2 = LO_Eval ( l1 ) ;
                    SetState ( _LC_, LISP_COMPILE_MODE, svlcms2 ) ;
                    if ( ! l2 || ( l2->LType & T_NIL ) )
                    {
                        Compile_DspPop_EspPush ( ) ;
                    }
                    else
                    {
                        _Compile_PushEspImm ( ( int32 ) l2->Lo_Value ) ;
                    }
                }
                else if ( l1->CType & LOCAL_VARIABLE )
                {
                    //_Compile_Move_LocalVarRValue_To_Reg ( l1->Lo_CfrTilWord, EAX ) ;
                    _Compile_Move_StackN_To_Reg ( EAX, FP, LocalVarOffset ( l1->Lo_CfrTilWord ) ) ; // 2 : account for saved fp and return slot
                    _Compile_PushReg ( EAX ) ;
                }
                else if ( l1->CType & STACK_VARIABLE )
                {
                    _Compile_Move_StackN_To_Reg ( EAX, FP, StackVarOffset ( l1->Lo_CfrTilWord ) ) ; // account for stored bp and return value
                    _Compile_PushReg ( EAX ) ;
                }
                else if ( GetState ( l1, QUALIFIED_ID ) )
                {
                    if ( dm ) _Debugger_PreSetup ( debugger, 0, l1 ) ;
                    _Namespace_AddToNamespacesHead_SetAsInNamespace ( _LC_->LispTemporariesNamespace ) ;
                    SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, true ) ;
                    //_Context_->Interpreter0->BaseObject = 0 ;
                    Interpreter_EvalQualifiedID ( l1 ) ;
                    SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, false ) ;
                    SetHere ( _Context_->Interpreter0->BaseObject->StackPushRegisterCode ) ;
                    _Compile_PushReg ( EAX ) ;
                    _DataStack_Pop ( ) ;
                    if ( dm ) _Debugger_PostShow ( debugger, 0, l1 ) ; // a literal could have been created and shown by _Word_Run
                }
                else
                {
                    _Compile_Esp_Push ( _DataStack_Pop ( ) ) ;
                }
            }
            if ( dm ) _Debugger_PostShow ( debugger, 0, word1 ) ; // a literal could have been created and shown by _Word_Run

        }
        // keep the optimizer informed ...
        //Stack_Push ( compiler->WordStack, ( int32 ) word ) ;
    }
    if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;
    if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
    {
        _Compiler_WordStack_PushWord ( compiler, word ) ; // ? l0 or word ?
        //_Compile_Call_NoOptimize ( ( byte* ) word->Definition ) ;
        _Compile_Call ( ( byte* ) word->Definition ) ;
        if ( i > 0 ) Compile_ADDI ( REG, ESP, 0, i * sizeof (int32 ), 0 ) ;
        if ( ! svcm )
        {
            CfrTil_EndBlock ( ) ;
            CfrTil_BlockRun ( ) ;
            Set_CompilerSpace ( scs ) ;
        }
        else if ( word->CType & C_RETURN )
        {
            _Word_CompileAndRecord_PushEAX ( word ) ;
        }
    }
    if ( dm ) _Debugger_PostShow ( debugger, 0, word ) ; // a literal could have been created and shown by _Word_Run
    SetState ( compiler, COMPILE_MODE, svcm ) ;
    SetState ( _LC_, LISP_COMPILE_MODE, svcm0 ) ;
    return nil ;
}

void
LC_Interpret_MorphismWord ( Word * word )
{
    _Interpret_MorphismWord_Default ( _Context_->Interpreter0, word ) ;
}

void
LC_Interpret_AListObject ( ListObject * l0 )
{
    Word * word = l0->Lo_CfrTilWord ;
    if ( ! ( word->CType & LITERAL ) )
    {
        LC_Interpret_MorphismWord ( word ) ;
    }
        //else LC_Interpret_ObjectToken ( _Context_->Interpreter0, word ) ;
    else _Context_->Interpreter0->w_Word = Lexer_ObjectToken_New ( _Context_->Lexer0, word->Name, 1 ) ;
}

void
LC_CompileRun_ArgList ( Word * word ) // C protocol : right to left arguments from a list pushed on the stack
{
    Lexer * lexer = _Context_->Lexer0 ;
    Compiler * compiler = _Context_->Compiler0 ;
    byte *svDelimiters = lexer->TokenDelimiters ;
    ListObject * l0 ;
    byte * token = _Lexer_ReadToken ( lexer, ( byte* ) " ,;\n\r\t" ) ;
    if ( word->CType & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        if ( ! token || strcmp ( "(", ( char* ) token ) ) Error ( "Syntax error : C RTL Args : no '('", ABORT ) ; // should be '('
    }
    compiler->LispParenLevel = 1 ;
    SetState ( compiler, LC_ARG_PARSING, true ) ;
    if ( word->CType & ( C_PREFIX | C_PREFIX_RTL_ARGS ) )
    {
        //SetState ( compiler, LC_C_RTL_ARG_PARSING, true ) ;
        int32 svcm = Compiling ;
        SetState ( compiler, COMPILE_MODE, false ) ; // we must have the arguments pushed and not compiled for _LO_Apply_C_Rtl_ArgList which will compile them for a C_Rtl function
        l0 = _LO_Read ( ) ;
        SetState ( compiler, COMPILE_MODE, svcm ) ;
        //SetState ( compiler, LC_C_RTL_ARG_PARSING, false ) ;
        _LO_Apply_C_Rtl_ArgList ( l0, word ) ;
    }
    SetState ( compiler, LC_ARG_PARSING | LC_C_RTL_ARG_PARSING, false ) ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, SESSION ) ;
}

//===================================================================================================================
//| LO_Print
//===================================================================================================================

char *
_LO_Print ( ListObject * l0, char * buffer, int lambdaFlag, int printValueFlag )
{
    char * format ;
    if ( ! buffer )
    {
        buffer = ( char* ) _CfrTil_->LispPrintBuffer ;
        buffer [0] = 0 ;
    }
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) )
        {
            byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
            _LO_PrintList ( l0, buffer2, lambdaFlag, printValueFlag ) ;
            if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
        }
        else if ( ( l0 == nil ) || ( l0->LType == T_NIL ) )
        {
            if ( _AtCommandLine ( ) )
            {
                sprintf ( buffer, " nil" ) ;
            }
        }
        else if ( l0->LType == true )
        {
            if ( _AtCommandLine ( ) )
            {
                sprintf ( buffer, " T" ) ;
            }
        }
        else if ( l0->LType & T_STRING )
        {
            if ( l0->State & UNQUOTED ) sprintf ( buffer, " %s", ( char* ) l0->Lo_String ) ;
            else sprintf ( buffer, " \"%s\"", ( char* ) l0->Lo_String ) ;
        }
        else if ( l0->LType & T_LISP_COMPILED_WORD )
        {
            sprintf ( buffer, " %s", ( char* ) l0->Lo_CfrTilWord->SourceCode ) ;
        }
        else if ( l0->LType & T_RAW_STRING )
        {
            //if ( LO_IsQuoted ( l0 ) ) sprintf ( buffer, " \'%s", ( char* ) l0->Lo_Name ) ;
            //else 
            sprintf ( buffer, " %s", ( char* ) l0->Lo_Name ) ;
        }
        else if ( l0->LType & T_LISP_SYMBOL )
            // how this block works still seems a little unclear and needs to be rethought
        {
            if ( LO_IsQuoted ( l0 ) ) sprintf ( buffer, " %s", l0->Lo_Name ) ;
                //else if ( lambdaFlag && l0->Lo_CfrTilWord->Lo_LambdaFunctionParameters && l0->Lo_CfrTilWord && ( l0->LType & T_LAMBDA ) ) //&& l0->Lo_CfrTilWord->Lo_LambdaFunctionParameters ) 
            else if ( lambdaFlag && l0->Lo_CfrTilWord && ( l0->LType & T_LAMBDA ) ) //&& l0->Lo_CfrTilWord->Lo_LambdaFunctionParameters ) 
            {
                byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
                sprintf ( buffer2, " ( define %s ( lambda", l0->Lo_Name ) ;
                //sprintf ( buffer2, " ( %s", l0->Lo_Name ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                _LO_PrintList ( ( ListObject * ) l0->Lo_CfrTilWord->Lo_LambdaFunctionParameters, buffer2, lambdaFlag = 1, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                _LO_PrintList ( ( ListObject * ) l0->Lo_CfrTilWord->Lo_LambdaFunctionBody, buffer2, lambdaFlag = 1, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
                sprintf ( buffer2, " ) )" ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
            }
            else if ( lambdaFlag && l0->Lo_Value )
            {
                if ( l0->Lo_CfrTilWord && l0->Lo_CfrTilWord->SourceCode ) sprintf ( buffer, " %s", l0->Lo_CfrTilWord->SourceCode ) ;
                else return _LO_Print ( ( ListObject * ) l0->Lo_Value, buffer, lambdaFlag, printValueFlag ) ;
            }
            else if ( printValueFlag ) //&& GetState ( _LC_, ( PRINT_VALUE ) ) )
            {
                if ( l0->Lo_Value != ( Object* ) nil )
                {
                    if ( ( ! l0->Lo_Value ) && l0->Lo_CfrTilWord )
                    {
                        if ( _Q_->Verbosity > 2 ) sprintf ( buffer, " %s = 0x%08x", l0->Lo_CfrTilWord->Lo_Name, ( int32 ) l0->Lo_CfrTilWord ) ;
                        else sprintf ( buffer, " %s", l0->Lo_Name ) ;
                    }
                    else
                    {
                        return _LO_Print ( ( ListObject * ) l0->Lo_Value, buffer, ( l0->LType & T_LAMBDA ), printValueFlag ) ; //lambdaFlag, printValueFlag ) ;
                    }
                }
            }
            else sprintf ( buffer, " %s", l0->Lo_Name ) ;
        }
        else if ( l0->LType & BLOCK )
        {
            sprintf ( buffer, " %s:#<BLOCK>:0x%08x", l0->Lo_Name, ( uint ) l0->Lo_UInteger ) ;
        }
        else if ( l0->LType & T_BIG_INT )
        {
            gmp_sprintf ( buffer, " %Zd\n", l0->Lo_Value ) ;
        }
        else if ( l0->LType & T_BIG_FLOAT )
        {
            gmp_sprintf ( buffer, " %*.*Ff\n", _Context_->System0->BigNumWidth, _Context_->System0->BigNumPrecision, l0->Lo_Value ) ;
        }
        else if ( l0->LType & T_INT )
        {
            if ( _Context_->System0->NumberBase == 16 ) sprintf ( buffer, " 0x%08x", ( uint ) l0->Lo_UInteger ) ;
            else
            {
                format = ( ( ( int32 ) l0->Lo_Integer ) < 0 ) ? ( char* ) " 0x%08x" : ( char* ) " %d" ;
                sprintf ( buffer, format, l0->Lo_Integer ) ;
            }
        }
        else if ( l0->LType & T_LAMBDA )
        {
            sprintf ( buffer, " #<LAMBDA>" ) ;
        }
        else if ( l0->LType & LITERAL )
        {
            format = ( ( ( int32 ) l0->Lo_Integer ) < 0 ) ? ( char* ) " 0x%08x" : ( char* ) " %d" ;
            if ( ( l0->Lo_Integer < 0 ) || ( _Context_->System0->NumberBase == 16 ) ) sprintf ( buffer, " 0x%08x", ( uint ) l0->Lo_UInteger ) ;
            else sprintf ( buffer, format, l0->Lo_Integer ) ;
        }
        else if ( l0->LType & ( CPRIMITIVE | CFRTIL_WORD ) )
        {
            sprintf ( buffer, " %s", l0->Lo_Name ) ;
        }
        else if ( l0->LType & ( T_HEAD | T_TAIL ) )
        {
            ; //break ;
        }
        else
        {
            if ( l0->Lo_CfrTilWord ) sprintf ( buffer, " %s", l0->Lo_CfrTilWord->Lo_Name ) ;
        }
    }
done:

    return buffer ;
}

char *
_LO_PrintList ( ListObject * l0, char * buffer, int lambdaFlag, int printValueFlag )
{
    byte * buffer2 = Buffer_New_pbyte ( BUFFER_SIZE ) ;
    ListObject * l1 ;
    if ( ! buffer )
    {
        buffer = ( char* ) _CfrTil_->LispPrintBuffer ;
        buffer [0] = 0 ;
    }
    if ( l0 )
    {
        if ( l0->LType & ( LIST | LIST_NODE ) ) sprintf ( buffer2, " (" ) ;
        if ( ! LO_strcat ( buffer, buffer2 ) ) goto done ;
        for ( l1 = _LO_First ( l0 ) ; l1 ; l1 = _LO_Next ( l1 ) ) //lnext )
        {
            if ( l1->LType & ( LIST | LIST_NODE ) )
            {
                _LO_PrintList ( l1, buffer2, lambdaFlag, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) return buffer ;
            }
            else
            {
                _LO_Print ( l1, buffer2, lambdaFlag, printValueFlag ) ;
                if ( ! LO_strcat ( buffer, buffer2 ) ) goto done ;
            }
        }
        if ( l0->LType & ( LIST | LIST_NODE ) ) sprintf ( buffer2, " )" ) ;
        LO_strcat ( buffer, buffer2 ) ;
    }
done:

    return buffer ;
}

void
LO_Print ( ListObject * l0 )
{
    DefaultColors ;
    ConserveNewlines ;
    SetState ( _LC_, ( PRINT_VALUE ), true ) ;
    Printf ( ( byte* ) "%s", _LO_PRINT ( l0 ) ) ;
    SetState ( _LC_, PRINT_VALUE, false ) ;
    AllowNewlines ;
}

//===================================================================================================================
//| LO_Repl
//===================================================================================================================

void
_LO_EvalPrint ( ListObject * l0, int32 * saveDsp )
{
    Compiler * compiler = _Context_->Compiler0 ;
    _LC_->ListFirst = _LO_First ( l0 ) ;
    l0 = _LO_MacroPreprocess ( l0 ) ;
    l0 = LO_Eval ( l0 ) ;
    LO_PrintWithValue ( l0 ) ;
    _Namespace_Clear ( _LC_->LispTemporariesNamespace ) ;
    //if ( ! Compiling ) 
    Compiler_Init ( compiler, 0 ) ; // we could be compiling a cfrTil word as in oldLisp.cft
    if ( saveDsp ) RestoreStackPointer ( saveDsp ) ; // ?!? maybe we should do this stuff differently
    SetBuffersUnused ;
    AllowNewlines ;
}

void
_LO_ReadEvalPrint_ListObject ( ListObject * l0, int32 parenLevel )
{
    Lexer * lexer = _Context_->Lexer0 ;
    Compiler * compiler = _Context_->Compiler0 ;
    SetState ( compiler, LISP_MODE, true ) ;
    byte *svDelimiters = lexer->TokenDelimiters ;
    compiler->LispParenLevel = parenLevel ;
    compiler->BlockLevel = 0 ;
    _LC_ = LC_New ( 1 ) ;
    _LC_->QuoteState = 0 ;
    int32 *saveDsp = SaveStackPointer ; // ?!? maybe we should do this stuff differently : literals are pushed on the stack by the interpreter
    if ( ! l0 ) l0 = _LO_Read ( ) ;
    _LO_EvalPrint ( l0, saveDsp ) ;
    SetState ( compiler, LISP_MODE, false ) ;
    Lexer_SetTokenDelimiters ( lexer, svDelimiters, 0 ) ;
    _LC_ = 0 ;
}

void
_LO_ReadEvalPrint0 ( int32 parenLevel )
{
    _LO_ReadEvalPrint_ListObject ( 0, parenLevel ) ;
}

void
LO_ReadEvalPrint1 ( )
{
    CfrTil_SourceCode_Init ( ) ;
    _LO_ReadEvalPrint0 ( 1 ) ;
    SetState ( _CfrTil_, SOURCE_CODE_INITIALIZED, false ) ;
}

void
LO_ReadEvalPrint ( )
{
    CfrTil_InitSourceCode_WithCurrentInputChar ( ) ;
    Namespace_ActivateAsPrimary ( ( byte* ) "Lisp" ) ;
    _LO_ReadEvalPrint0 ( 0 ) ;
}

void
LO_Repl ( )
{
    Printf ( ( byte* ) "\ncfrTil lisp : (type ';;' to exit)\n" ) ;
    _Repl ( LO_ReadEvalPrint ) ;
}

//===================================================================================================================
//| LC_ : lambda calculus
//===================================================================================================================

void
_LC_Init ( LambdaCalculus * lc )
{

    lc->LispNamespace = Namespace_Find ( ( byte* ) "Lisp" ) ;
    //lc->LispTemporariesNamespace = _Namespace_New ( ( byte* ) "LispTemporaries", lc->LispNamespace ) ;
    lc->LispTemporariesNamespace = Namespace_FindOrNew_SetUsing ( ( byte* ) "LispTemporaries", lc->LispNamespace, 0 ) ;
    lc->SavedCodeSpace = 0 ;
    lc->Nil = _LO_New ( T_NIL, 0, 0, 0, 0, 0, LISP ) ;
    lc->True = _LO_New ( ( uint64 ) true, 0, 0, 0, 0, 0, LISP ) ;
    lc->OurCfrTil = _CfrTil_ ;
    lc->QuoteState = 0 ;
    lc->QuoteStateStack = Stack_New ( 64, LISP_TEMP ) ;
    _CfrTil_->LC = lc ;
}

LambdaCalculus *
LC_New ( int32 initFlag )
{
    // there remain some problems with LC initialization logic and working with the rest of the system, 
    // and regarding the LispTemporariesNamespace
    //if ( ! _LC_ )
    {
        _LC_ = ( LambdaCalculus * ) _Mem_Allocate ( sizeof (LambdaCalculus ), LISP_TEMP ) ;
    }
    //if ( initFlag) 
    _LC_Init ( _LC_ ) ;

    return _LC_ ;
}
