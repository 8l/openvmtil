#include "../includes/cfrtil.h"

// we don't want the classes semantics when interpreting

void
CfrTil_ClassStructureEnd ( void )
{
    Namespace_RemoveFromUsingList_WithCheck ( ( byte* ) "Class" ) ;
}

void
CfrTil_ClassStructureBegin ( void )
{
    _CfrTil_Parse_ClassStructure ( 0 ) ;
}

void
CfrTil_CloneStructureBegin ( void )
{
    _CfrTil_Parse_ClassStructure ( 1 ) ;
}

