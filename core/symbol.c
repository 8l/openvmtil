
#include "../includes/cfrtil.h"

void
_Symbol_Init ( Symbol * symbol, byte * name )
{
    symbol->S_Name = name ;
}

void
_Symbol_Init_AllocName ( Symbol * symbol, byte * name, int32 allocType )
{
    if ( name )
    {
        byte* sname = name ? String_New ( name, allocType ) : 0 ;
        _Symbol_Init ( symbol, sname ) ;
    }
}

// doesn't allocate name

Symbol *
__Symbol_New ( int32 allocType )
{
    return ( Symbol * ) _Mem_Allocate ( sizeof (Symbol ), allocType ) ;
}

Symbol *
_Symbol_New ( byte * name, int32 allocType )
{
    Symbol * symbol = __Symbol_New ( allocType ) ;
    _Symbol_Init_AllocName ( symbol, name, allocType ) ;
    return symbol ;
}

// doesn't allocate name

Symbol *
Symbol_New ( byte * name )
{
    return _Symbol_New ( name, DICTIONARY ) ;
}

Symbol *
Symbol_NewValue ( int32 value, int32 allocType )
{
    Symbol * sym = __Symbol_New ( allocType ) ;
    sym->S_Value = (byte*) value ;
    return sym ;
}

Symbol *
_aSymbol_CompareName ( Symbol * symbol, byte * name )
{
    if ( name && symbol->S_Name && ( strcmp ( ( char* ) symbol->S_Name, ( char* ) name ) == 0 ) ) return symbol ;
    else return 0 ;
}

Symbol *
_Symbol_CompareName ( Symbol * symbol, byte * name )
{
    //if ( (!(symbol->S_CType & WORD_CREATE)) && ( symbol = _aSymbol_CompareName ( symbol, name ) ) )
    if ( symbol = _aSymbol_CompareName ( symbol, name ) )
    {
        _Context_->Finder0->w_Word = ( Word * ) symbol ;
        _Context_->Finder0->FoundWord = _Context_->Finder0->w_Word ;
        _Context_->Finder0->FoundWordNamespace = ( ( Word * ) symbol )->ContainingNamespace ;
        return symbol ;
    }
    return 0 ;
}
