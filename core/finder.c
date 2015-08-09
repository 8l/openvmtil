#include "../includes/cfrtil.h"


// we could make this a SymbolList function if we refactor State field

Word *
_WordList_DescendMap_1 ( Word * word, int32 state, int32 stayInOneNamespaceFlag, MapFunction_Cell_1 mf, int32 one )
{
    Word * word2, *nextWord ;
    for ( ; word ; word = nextWord )
    {
        nextWord = ( Word* ) DLNode_Next ( ( Node* ) word ) ;
        if ( mf ( ( Symbol* ) word, one ) ) return word ;
        else if ( ! stayInOneNamespaceFlag )
        {
            if ( Is_NamespaceType (word) )
            {
                if ( word->State & state )
                {
                    if ( ( word2 = _WordList_DescendMap_1 ( ( Word* ) DLList_First ( word->Lo_List ), state, stayInOneNamespaceFlag, mf, one ) ) ) return word2 ;
                }
            }
        }
    }
    return 0 ;
}

Word *
Word_FindInOneNamespace ( Namespace * ns, byte * name )
{
    if ( ns && name )
    {
        _Context_->Finder0->FoundWord = 0 ;
        _Context_->Finder0->w_Word = 0 ;
        return _Context_->Finder0->w_Word = _WordList_DescendMap_1 ( ( Word* ) DLList_First ( ns->Lo_List ), USING, 1, ( MapFunction_Cell_1 ) _Symbol_CompareName, ( int32 ) name ) ;
    }
    return 0 ;
}

Word *
_Word_Find_Minimal ( DLList * list, int32 state, byte * name )
{
    Word * word = _WordList_DescendMap_1 ( ( Word* ) DLList_First ( list ), state, 1, ( MapFunction_Cell_1 ) _aSymbol_CompareName, ( int32 ) name ) ;
    return word ;
}

Word *
_Word_Find ( int32 state, byte * name )
{
    _Context_->Finder0->FoundWord = 0 ;
    _Context_->Finder0->w_Word = 0 ;
    return _WordList_DescendMap_1 ( _CfrTil_->Namespaces, state, 0, ( MapFunction_Cell_1 ) _Symbol_CompareName, ( int32 ) name ) ;
}

Word *
Word_FindUsing ( byte * name )
{
    return _Word_Find ( USING, name ) ;
}

Word *
_Word_FindAny ( byte * name )
{
    return _Word_Find ( ANY, name ) ;
}

Word *
Word_Find ( byte * name )
{
    return _Word_Find ( ANY, name ) ;
}

void
Finder_Init ( Finder * finder )
{
#if 0    
    finder->FinderFlags = 0 ;
    finder->FoundWord = 0 ;
    finder->w_Word = 0 ;
    finder->LastWord = 0 ;
#else
    memset ( finder, 0, sizeof (Finder ) ) ;
#endif    
}

Finder *
Finder_New ( int32 allocationType )
{
    Finder * finder = ( Finder * ) _Mem_Allocate ( sizeof (Finder ), allocationType ) ;
    //Finder_Init ( finder ) ; // not needed assuming _Mem_Allocate returns clear mem
    return finder ;
}

#if 0

void
Finder_Delete ( Finder * finder )
{
    Mem_FreeItem ( _Q_->PermanentMemList, ( byte* ) finder ) ;
}
#endif

Symbol *
_Finder_CompareDefinitionAddress ( Symbol * symbol, byte * address )
{
    Word * word = ( Word * ) symbol ;
    //if ( word->CType & BLOCK )
    //if ( word->W_WordData ) 
    {
        //byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
        byte * codeStart = word->CodeStart ;
        if ( codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) )
        {
            return symbol ;
        }
        return 0 ;
    }
}

Symbol *
_Finder_CompareDefinitionAddress_NoAlias ( Symbol * symbol, byte * address )
{
    Word * word = ( Word * ) symbol ;
    //byte * codeStart = ( byte* ) word->Definition ; // nb. this maybe more accurate ??
    byte * codeStart = word->CodeStart ;
    if ( ( ! ( word->CType & ALIAS ) ) && codeStart && ( address >= codeStart ) && ( address <= ( codeStart + word->S_CodeSize ) ) )
    {
        return symbol ;
    }
    else return 0 ;
}

Word *
Finder_Address_FindInOneNamespace ( Finder * finder, Namespace * ns, byte * address )
{
    return finder->w_Word = _WordList_DescendMap_1 ( ns, USING, 1, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
}

Word *
Finder_Address_FindAny ( Finder * finder, byte * address )
{
    return finder->w_Word = _WordList_DescendMap_1 ( _CfrTil_->Namespaces, USING | NOT_USING, 0, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress, ( int32 ) address ) ;
}

Word *
Finder_Address_FindAny_NoAlias ( Finder * finder, byte * address )
{
    return finder->w_Word = _WordList_DescendMap_1 ( _CfrTil_->Namespaces, USING | NOT_USING, 0, ( MapFunction_Cell_1 ) _Finder_CompareDefinitionAddress_NoAlias, ( int32 ) address ) ;
}

void
Finder_SetQualifyingNamespace ( Finder * finder, Namespace * ns )
{
    finder->QualifyingNamespace = ns ;
}

void
Finder_SetNamedQualifyingNamespace ( Finder * finder, byte * name )
{
    finder->QualifyingNamespace = Namespace_Find ( name ) ;
}

Namespace *
Finder_GetQualifyingNamespace ( Finder * finder )
{

    return finder->QualifyingNamespace ;
}

Word *
Finder_Word_FindUsing ( Finder * finder, byte * name )
{
    Word * word = 0 ;
    if ( name )
    {
        if ( finder->QualifyingNamespace )
        {
            if ( String_Equal ( ".", ( char* ) name ) ) word = Word_FindUsing ( name ) ; // keep QualifyingNamespace intact // ?? assumes function of CfrTil_Dot is always and only named "." ??
            else
#if OLD               
            {
                word = Word_FindInOneNamespace ( finder->QualifyingNamespace, name ) ;
                if ( ! GetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID ) ) Finder_SetQualifyingNamespace ( finder, 0 ) ; // nb. QualifyingNamespace is only good for one find
            }
#else            
            {
                word = Word_FindInOneNamespace ( finder->QualifyingNamespace, name ) ;
                if ( ReadLine_IsReverseTokenQualifiedID ( _Context_->ReadLiner0 ) )
                {
                    SetState ( _Context_, CONTEXT_PARSING_QUALIFIED_ID, false ) ;
                    Finder_SetQualifyingNamespace ( finder, 0 ) ; // nb. QualifyingNamespace is only good for approximately one find
                }
            }
#endif            
        }
        if ( ! word ) word = Word_FindUsing ( name ) ;
    }
    return word ;
}

Word *
Finder_FindQualifiedIDWord ( Finder * finder, byte * token )
{
    Word * word ;

    //while ( ( symbol = _Symbol_FindAny ( token ) ) )
    while ( ( word = Finder_Word_FindUsing ( finder, token ) ) )
    {
        if ( word->CType & ( NAMESPACE ) )
        {
            Namespace * ns = ( Namespace * ) word ;
            Finder_SetQualifyingNamespace ( finder, ns ) ;
        }
        else if ( word->CType & ( OBJECT ) )
        {
            Finder_SetQualifyingNamespace ( finder, word->ContainingNamespace ) ;
        }
        else if ( word->CType & ( CLASS_MEMBER_ACCESS ) )
        {
            Finder_SetQualifyingNamespace ( finder, word->ClassFieldNamespace ) ;
        }
        else return word ;
        if ( Lexer_IsTokenForwardDotted ( _Context_->Lexer0 ) )
        {
            Lexer_ReadToken ( _Context_->Lexer0 ) ; // the '.'
            token = Lexer_ReadToken ( _Context_->Lexer0 ) ; // the namespace
            continue ;
        }

        else return word ;
    }
    return 0 ;
}

byte *
Finder_GetTokenDefinitionAddress ( Finder * finder, byte * token )
{
    byte * definitionAddress = 0 ;
    if ( token )
    {
        Word * word = Finder_Word_FindUsing ( finder, token ) ;

        if ( word ) definitionAddress = ( byte* ) word->Definition ;
    }
    return definitionAddress ;
}

Word *
Finder_FindToken_WithException ( Finder * finder, byte * token )
{
    if ( Finder_Word_FindUsing ( finder, token ) == 0 )
    {

        Printf ( ( byte* ) "\n%s ?", ( char* ) token ) ;
        CfrTil_Using ( ) ;
        CfrTil_Exception ( NOT_A_KNOWN_OBJECT, QUIT ) ;
    }
    return finder->w_Word ;
}

Word *
Finder_FindToken ( Finder * finder, byte * token )
{
    return Finder_FindToken_WithException ( finder, token ) ;
}

Word *
_CfrTil_FindInAnyNamespace ( byte * name )
{
    return _Word_FindAny ( name ) ;
}

Word *
_CfrTil_Token_FindUsing ( byte * token )
{
    return Finder_Word_FindUsing ( _Context_->Finder0, token ) ;
}

void
CfrTil_Token_Find ( )
{
    _CfrTil_Token_FindUsing ( _Context_->Lexer0->OriginalToken ) ;
}

void
CfrTil_Find ( )
{
    _DataStack_Push ( ( int32 ) Finder_FindToken ( _Context_->Finder0, _Context_->Lexer0->OriginalToken ) ) ;
}

void
CfrTil_Postfix_Find ( )
{
    Word * word = Finder_Word_FindUsing ( _Context_->Finder0, ( byte* ) _DataStack_Pop ( ) ) ;
    _DataStack_Push ( ( int32 ) word ) ;
}



