
#include "../includes/cfrtil.h"

void
_CfrTil_Word_Disassemble ( Word * word )
{
    byte * start ;
    if ( word )
    {
        if ( CfrTil_GetState ( _CfrTil_, INLINE_ON ) )
        {
            start = ( byte* ) word->Definition ;
        }
        else start = word->CodeStart ;
        //_Debugger_Disassemble ( _CfrTil_->Debugger0, start, word->S_CodeSize ? ( word->S_CodeSize < 512 ? word->S_CodeSize : 512 ) : 96, word->CType & (CPRIMITIVE|DLSYM_WORD)? 1 : 0  ) ;
        _Debugger_Disassemble ( _CfrTil_->Debugger0, start, word->S_CodeSize ? ( word->S_CodeSize < 512 ? word->S_CodeSize : 512 ) : 96, 1 ) ;
    }
}

void
CfrTil_Word_Disassemble ( )
{
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    Word * word = Word_GetFromCodeAddress_NoAlias ( address ) ;
    if ( word )
    {
        Printf ( ( byte* ) "\nWord : %s : disassembly :> \n", c_dd ( word->Name ) ) ;
        _CfrTil_Word_Disassemble ( word ) ;
        Printf ( ( byte* ) "\n" ) ;
    }
    else
    {
        Printf ( ( byte* ) "\n%s : WordDisassemble : Can't find word code at this (0x%x) address.\n", c_dd ( Context_Location ( ) ), ( uint ) address ) ;
    }
}

void
CfrTil_Disassemble ( )
{
    uint number = _DataStack_Pop ( ) ;
    byte * address = ( byte* ) _DataStack_Pop ( ) ;
    _Debugger_Disassemble ( _CfrTil_->Debugger0, address, number, 0 ) ;
    Printf ( ( byte* ) "\n" ) ;
}


