#include "../includes/cfrtil.h"

void
_Word_Location_Printf ( Word * word )
{
    if ( word ) Printf ( "\n%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->Filename, word->LineNumber, word->CursorPosition ) ;
}

byte *
_Word_Location_pbyte ( Word * word )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
    byte * b = Buffer_Data ( buffer ) ;
    if ( word ) sprintf ( b, "%s.%s : %s %d.%d", word->ContainingNamespace->Name, word->Name, word->Filename, word->LineNumber, word->CursorPosition ) ;
    return b ;
}

void
Word_PrintName ( Word * word )
{
    if ( word ) Printf ( ( byte* ) "%s ", word->Name ) ;
}

void
_Word_Print ( Word * word )
{
    Printf ( ( byte* ) c_ud ( " %s" ), word->Name ) ;
}

void
__Word_ShowSourceCode ( Word * word )
{
    if ( word && word->W_pwd_WordData && word->SourceCode ) //word->CType & ( CPRIMITIVE | BLOCK ) )
    {
#if 1        
        Buffer *dstb = Buffer_NewLocked ( BUFFER_SIZE ) ;
        byte * dst = dstb->B_Data ;
        _String_ConvertStringToBackSlash ( dst, word->SourceCode ) ;
        char * name = c_dd ( word->Name ), *dest = c_dd ( String_FilterForHistory ( dst ) ) ;
        Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", name, dest ) ;
        Buffer_Unlock ( dstb ) ;
        Buffer_SetAsUnused ( dstb ) ;
#else
        Printf ( ( byte* ) "\nSourceCode for ""%s"" :> \n%s", c_dd ( word->Name ), c_dd ( word->SourceCode ) ) ;
#endif        
    }
}

void
_Word_ShowSourceCode ( Word * word )
{
    _CfrTil_Source ( word, 0 ) ;
}

Word *
Word_GetFromCodeAddress ( byte * address )
{
    return Finder_Address_FindAny ( _Context_->Finder0, address ) ;
}

Word *
Word_GetFromCodeAddress_NoAlias ( byte * address )
{
    return Finder_Address_FindAny_NoAlias ( _Context_->Finder0, address ) ;
}

void
_CfrTil_WordName_Run ( byte * name )
{
    _Block_Eval ( Finder_Word_FindUsing ( _Context_->Finder0, name )->Definition ) ;
}

#if NO_GLOBAL_REGISTERS  // NGR NO_GLOBAL_REGISTERS

void
_Compile_Dsp_To_ESI ( )
{
    _Compile_C_Var_To_Reg ( ESI, ( int32 * ) Dsp ) ;
}

void
_Compile_ESI_To_Dsp ( )
{
    _Compile_Reg_To_C_Var ( ESI, ( int32 * ) Dsp ) ;
}
#endif

void
_Word_Run ( Word * word )
{
    _CfrTil_->CurrentRunWord = word ;
    word->Definition ( ) ;
}

void
_Word_Eval ( Word * word )
{
    if ( word )
    {
        if ( word->CType & DEBUG_WORD ) DebugColors ;
        Debugger * debugger = _CfrTil_->Debugger0 ;
        int32 dm = GetState ( _CfrTil_, DEBUG_MODE ) && ( ! GetState ( debugger, DBG_STEPPING ) ) ;
        if ( dm ) _Debugger_PreSetup ( debugger, 0, word ) ;
        if ( ! ( dm && GetState ( debugger, DBG_DONE ) ) )
        {
            word->StackPushRegisterCode = 0 ; // nb. used! by the rewriting optimizer
            if ( ( word->CType & IMMEDIATE ) || ( ! CompileMode ) )
            {
                _Word_Run ( word ) ;
            }
            else
            {
                _CompileWord ( word ) ;
            }
        }
        if ( dm ) _Debugger_PostShow ( debugger, 0, word ) ; // a literal could have been created and shown by _Word_Run
        if ( word->CType & DEBUG_WORD ) DefaultColors ; // reset colors after a debug word
    }
}

Namespace *
_Word_Namespace ( Word * word )
{
    if ( word->CType & NAMESPACE ) return ( Namespace * ) word ;
    else return word->ContainingNamespace ;
}

void
_CfrTil_AddSymbol ( Symbol * symbol )
{
    _Namespace_DoAddSymbol ( _CfrTil_Namespace_InNamespaceGet ( ), symbol ) ;
}

void
_CfrTil_AddWord ( Word * word )
{
    Namespace_AddWord ( _CfrTil_Namespace_InNamespaceGet ( ), word ) ;
}

Word *
_Word_Allocate ( uint64 category )
{
    Word * word ;
    int32 atype ;
    if ( category & ( LOCAL_VARIABLE | STACK_VARIABLE | REGISTER_VARIABLE | SESSION ) ) atype = SESSION ;
    else atype = DICTIONARY ;
    word = ( Word* ) _Mem_Allocate ( sizeof ( Word ) + sizeof ( WordData ), atype ) ;
    word->W_pwd_WordData = ( WordData * ) ( word + 1 ) ; // nb. "pointer arithmetic"
    return word ;
}

// deep copy from word0 to word

void
_Word_Copy ( Word * word, Word * word0 )
{
    WordData * swdata = word->W_pwd_WordData ;
    memcpy ( word, word0, sizeof (Word ) ) ;
    word->W_pwd_WordData = swdata ; // restore the WordData pointer we overwrote by the above memcpy
    memcpy ( word->W_pwd_WordData, word0->W_pwd_WordData, sizeof (WordData ) ) ;
}

Word *
Word_Copy ( Word * word0, int32 type )
{
    Word * word = _Word_Allocate ( type ) ;
    _Word_Copy ( word, word0 ) ;
    return word ;
}

Word *
_Word_Init ( Word * word, uint64 ctype, uint64 ltype )
{
    word->CType = ctype ;
    word->LType = ltype ;
    if ( Is_NamespaceType ( word ) ) word->Lo_List = DLList_New ( ) ;
    return word ;
}

Word *
_Word_New ( byte * name, uint64 ctype, uint64 ltype, int32 allocType )
{
    Word * word = _Word_Allocate ( allocType ? allocType : DICTIONARY ) ;
    if ( allocType != EXISTING ) _Symbol_Init_AllocName ( ( Symbol* ) word, name, DICTIONARY ) ;
    else _Symbol_Init ( ( Symbol * ) word, name ) ;
    _Word_Init ( word, ctype, ltype ) ;
    return word ;
}

void
_Word_Finish ( Word * word )
{
    Compiler * compiler = _Context_->Compiler0 ;
    _DObject_Finish ( word ) ;
    _CfrTil_FinishSourceCode ( word ) ;
    word->W_FunctionTypesArray = compiler->FunctionTypesArray ;
    Compiler_Init ( compiler, 0 ) ; // after every word
}

void
_Word_Add ( Word * word, int32 addToInNs, Namespace * addToNs )
{
    uint64 ctype = word->CType ;
    Namespace * ins = addToInNs ? _CfrTil_Namespace_InNamespaceGet ( ) : 0 ;
    if ( Is_NamespaceType ( word ) && _CfrTil_->Namespaces )
    {
        _Namespace_DoAddWord ( _CfrTil_->Namespaces, word ) ;
        if ( addToNs ) word->ContainingNamespace = addToNs ;
        else if ( addToInNs ) word->ContainingNamespace = ins ;
    }
    else if ( addToNs ) _Namespace_DoAddWord ( addToNs, word ) ;
    else if ( addToInNs ) _CfrTil_AddWord ( word ) ;
    if ( addToInNs && ( ! CompileMode ) && ( _Q_->Verbosity > 2 ) && ( ! ( ctype & ( SESSION | LOCAL_VARIABLE | STACK_VARIABLE ) ) ) )
    {
        if ( ctype & BLOCK ) Printf ( ( byte* ) "\nnew Word :: %s.%s\n", ins->Name, word->Name ) ;
        else Printf ( ( byte* ) "\nnew DObject :: %s.%s\n", ins->Name, word->Name ) ;
    }
}

void
_Word_DefinitionStore ( Word * word, block code )
{
    _DObject_Definition_EvalStore ( word, ( int32 ) code, word->CType | BLOCK, word->LType | BLOCK, 0, 0 ) ;
}

void
_Word ( Word * word, byte * code )
{
    _Word_DefinitionStore ( word, ( block ) code ) ;
    _Word_Add ( word, ( ! _Context_->Compiler0->RecursiveWord ), 0 ) ; // don't re-add if it is a recursive word cf. CfrTil_BeginRecursiveWord
    _Word_Finish ( word ) ;
}

Word *
_Word_Create ( byte * name )
{
    Word * word = _Word_New ( name, CFRTIL_WORD | WORD_CREATE, 0, DICTIONARY ) ; //CFRTIL_WORD : cfrTil compiled words
    _Context_->Compiler0->CurrentCreatedWord = word ;
    return word ;
}

