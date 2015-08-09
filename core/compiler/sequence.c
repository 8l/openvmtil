
#include "../../includes/cfrtil.h"

/*
struct _Sequence
{
    cell_t  ItemSize ; // bytes
    cell_t  NumberOfItems ;
    byte *  Index ;
    byte *  Last ;
    byte    Data [ ] ;
};
typedef struct _Sequence Sequence ;


inline Sequence *
_Sequence_Create ( cell_t itemSize, cell_t numberOfItems )
{
    return (Sequence*) ByteArray_Allocate ( sizeof (Sequence) + itemSize * numberOfItems ) ;
}

inline void
_Sequence_Init ( Sequence * array, cell_t itemSize, cell_t numberOfItems )
{
    array->ItemSize = itemSize ;
    array->NumberOfItems = numberOfItems ;
    array->Index = &array->Data [0];
    array->Last = (byte*) array->Data + ( array->ItemSize * array->NumberOfItems ) ;
}

Sequence *
Sequence_New ( cell_t itemSize, cell_t numberOfItems )
{
    Sequence * array = _Sequence_Create ( itemSize, numberOfItems ) ;
    _Sequence_Init ( array, itemSize, numberOfItems ) ;
    return array ;
}

byte *
Sequence_Append ( Sequence * array, cell_t size ) // size in bytes
{
    byte * index = array->Index ;
    if ( ( array->Index += size ) > array->Last )
    {
        array->Index -= size ;
        CfrTil_Exception ( MEMORY_ALLOCATION_ERROR, 1 ) ;
    }
    return index ;
}

// private !!
byte *
_Sequence_DataPointerGet ( Sequence * array )
{
    return & array->Data [ array->Index * array->ItemSize ] ;
}

cell_t
_Sequence_Index ( Sequence * array )
{
    return array->Index ;
}

void
_Sequence_IndexSet ( Sequence * array, cell_t index )
{
    array->Index = index ;
}


*/