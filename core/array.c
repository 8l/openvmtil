
#include "../includes/cfrtil.h"

byte *
_ByteArray_AppendSpace ( ByteArray * array, int32 size ) // size in bytes
{
    NamedByteArray * nba = array->OurNBA ;
tryAgain:
    array->StartIndex = array->EndIndex ; // move index to end of the last append
    array->EndIndex += size ;
    if ( array->EndIndex >= array->bp_Last )
    {
        if ( nba )
        {
            if ( nba->NBA_AType == CODE )
            {
                Error_Abort ( ( byte* ) "\nOut of Code Memory : Set Code Memory size higher at startup.\n" ) ;
            }
            _NamedByteArray_AddNewByteArray ( nba, size ) ;
            array = nba->ba_ByteArray ;
            goto tryAgain ;
        }
    }
    if ( nba ) nba->MemRemaining -= size ; //nb. debugger->StepInstructionBA doesn't have an nba
    return array->StartIndex ;
}

void
_ByteArray_UnAppendSpace ( ByteArray * array, int32 size ) // size in bytes
{
    // ?? no error checking ??
    array->EndIndex -= size ;
    array->StartIndex -= size ;
}

void
_ByteArray_DataClear ( ByteArray * array )
{
    Mem_Clear ( array->BA_Data, array->BA_Size ) ;
}

void
_ByteArray_Init ( ByteArray * array, int32 size )
{
    array->BA_Data = ( byte* ) ( array + 1 ) ;
    array->BA_Size = size ;
    array->StartIndex = array->BA_Data ;
    array->EndIndex = array->StartIndex ;
    array->bp_Last = & array->BA_Data [ size ] ;
    //array->LastAllocationSize = size ;
    _ByteArray_DataClear ( array ) ;
}

void
_ByteArray_ReInit ( ByteArray * array )
{
    _ByteArray_Init ( array, array->BA_Size ) ;
}

ByteArray *
_ByteArray_AllocateNew_ ( DLList * list, int32 size, int64 type )
{
    ByteArray * ba = ( ByteArray* ) _MemChunk_Allocate ( sizeof ( ByteArray ) + size, type ) ;
    DLList_AddNodeToHead ( list, ( DLNode* ) ba ) ;
    ba->BA_AType = type ;
    _ByteArray_Init ( ba, size ) ;
    return ba ;
}

ByteArray *
_ByteArray_AllocateNew ( NamedByteArray * nba, int32 size )
{
    ByteArray * ba = _ByteArray_AllocateNew_ ( nba->BA_MemoryList, size, nba->NBA_AType ) ;
    ba->OurNBA = nba ;
    return ba ;
}

byte *
_ByteArray_GetEndIndex ( ByteArray * array )
{
    return array->EndIndex ;
}

byte *
_ByteArray_Here ( ByteArray * array )
{
    return array->EndIndex ;
}

void
_ByteArray_SetEndIndex ( ByteArray * array, byte * index )
{
    array->EndIndex = index ;
}

void
_ByteArray_SetHere ( ByteArray * array, byte * index )
{
    array->EndIndex = index ;
}

void
_ByteArray_SetHere_AndForDebug ( ByteArray * array, byte * index )
{
    _ByteArray_SetEndIndex ( array, index ) ;
    _CfrTil_->Debugger0->OptimizedCodeAffected = index ;
}

byte *
_ByteArray_GetStartIndex ( ByteArray * array )
{
    return array->StartIndex ;
}

void
_ByteArray_SetStartIndex ( ByteArray * array, byte * index )
{
    array->StartIndex = index ;
}

// ! TODO : should be macros here !

void
ByteArray_AppendCopyItem ( ByteArray * array, int32 size, int32 data ) // size in bytes
{
    _ByteArray_AppendSpace ( array, size ) ; // size in bytes
    byte * index = array->StartIndex ;
    switch ( size )
    {
        case 1:
        {
            *( ( byte* ) index ) = ( byte ) data ;
            break ;
        }
        case 2:
        {
            *( ( short* ) index ) = ( short ) data ;
            break ;
        }
        case 4:
        {
            *( ( int* ) index ) = ( int ) data ;
            break ;
        }
        case 8:
        {
            *( ( long int* ) index ) = ( long int ) data ;
            break ;
        }
    }
}

void
ByteArray_AppendCopy ( ByteArray * array, int32 size, byte * data ) // size in bytes
{
    _ByteArray_AppendSpace ( array, size ) ; // size in bytes
    memcpy ( array->StartIndex, data, size ) ;
}

void
ByteArray_AppendCopyUpToRET ( ByteArray * array, byte * data ) // size in bytes
{
    int32 i ;
    for ( i = 0 ; 1 ; i ++ )
    {
        if ( data [ i ] == _RET ) break ;
    }
    ByteArray_AppendCopy ( array, i, data ) ; // ! after we find out how big 'i' is
}

#if 0
// insert size data into array atPosition

void
ByteArray_InsertData ( ByteArray * array, byte * data, int32 atPosition, int32 size ) // size in bytes
{
    _ByteArray_AppendSpace ( array, size ) ; // size in bytes
    Mem_Insert ( array->BA_Data, data, atPosition, size ) ; // size in bytes
}

#endif
