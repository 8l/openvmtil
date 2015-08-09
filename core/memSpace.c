
#include "../includes/cfrtil.h"

void
_MemChunk_CheckAndInit ( MemChunk * mchunk, int32 size, uint64 type )
{
    if ( ( mchunk == (MemChunk*) MAP_FAILED ) )
    {
        perror ( "mmap" ) ;
        OpenVmTil_Throw ( ( byte* ) "Memory Allocation Error", FULL_RESTART ) ;
    }
    //Mem_Clear ( ( byte* ) mchunk, size ) ; //?!? not necessary MAP_ANONYMOUS comes cleared
    mchunk->S_AType = type ;
    mchunk->S_ChunkSize = size ; // S_ChunkSize is the total size of the chunk already including any prepended booking structure in that total
    if ( _Q_ )
    {
        _Q_->MmapMemoryAllocated += size ; // added here only for ovt allocation
        if ( ( _Q_->Verbosity > 2 ) && ( size > 10000000 ) )
        {
            _Printf ( "\nAllocate : %s : 0x%lld : %d, ", ( (NamedByteArray*) ( mchunk->S_pb_Data ) )->Name, mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
    mchunk->S_Chunk = (byte*) ( mchunk + 1 ) ; // nb. ptr arithmetic
}

// this version does *not* prepend a MemChunk and the caller has to add it to his list

byte *
_MemChunk_Allocate ( int32 size, uint64 type )
{
    MemChunk * mchunk = (MemChunk*) mmap ( NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
    _MemChunk_CheckAndInit ( mchunk, size, type ) ;
    return (byte*) mchunk->S_Chunk ;
}

// this version prepends a MemChunk and adds it to a list

byte *
MemList_AllocateChunk ( DLList * list, int32 osize, uint64 type )
{
    int32 nsize = osize + sizeof ( MemChunk ) ;
    MemChunk * mchunk = (MemChunk*) mmap ( NULL, nsize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, - 1, 0 ) ;
    _MemChunk_CheckAndInit ( mchunk, nsize, type ) ;
    DLList_AddNodeToHead ( list, (DLNode*) mchunk ) ;
    return (byte*) mchunk->S_Chunk ;
}

#define _Allocate( size, nba ) _ByteArray_AppendSpace ( nba->ba_ByteArray, size ) 

byte *
_Mem_Allocate ( int32 size, uint64 type )
{
    MemorySpace * ms = _Q_->MemorySpace0 ;
    switch ( type )
    {
        case OPENVMTIL:
        {
            if ( ms && ms->OpenVmTilSpace ) return _Allocate ( size, ms->OpenVmTilSpace ) ;
            else return MemList_AllocateChunk ( &_MemList_, size, OPENVMTIL ) ;
        }
        case LISP:
        case OBJECT_MEMORY: return _Allocate ( size, ms->ObjectSpace ) ;
        case TEMP_OBJECT_MEMORY: return _Allocate ( size, ms->TempObjectSpace ) ;
        case DICTIONARY: return _Allocate ( size, ms->DictionarySpace ) ;
        case SESSION: return _Allocate ( size, ms->SessionObjectsSpace ) ;
        case CODE: return _Allocate ( size, ms->CodeSpace ) ;
        case BUFFER: return _Allocate ( size, ms->BufferSpace ) ;
        case HISTORY: return _Allocate ( size, ms->HistorySpace ) ;
        case LISP_TEMP: return _Allocate ( size, ms->LispTempSpace ) ;
        case CONTEXT: return _Allocate ( size, ms->ContextSpace ) ;
        case COMPILER_TEMP_OBJECT_MEMORY: return _Allocate ( size, ms->CompilerTempObjectSpace ) ;
        case CFRTIL:
        case DATA_STACK: return _Allocate ( size, ms->CfrTilInternalSpace ) ;
        default: CfrTil_Exception ( MEMORY_ALLOCATION_ERROR, QUIT ) ;
    }
}

byte *
Mem_Allocate ( DLList * list, int32 size, uint64 type )
{
    DLNode * memNode = (DLNode *) _Mem_Allocate ( size, type ) ;
    DLList_AddNodeToHead ( list, memNode ) ;
}

byte *
_CfrTil_AllocateNew ( int32 size, int32 type )
{
    return _Mem_Allocate ( size, type ) ;
}

void
_Mem_ChunkFree ( MemChunk * mchunk )
{
    int32 size = mchunk->S_ChunkSize ; 
    if ( _Q_ )
    {
        _Q_->MmapMemoryAllocated -= size ;
        if ( ( _Q_->Verbosity > 2 ) && ( size > 10000000 ) )
        {
            Symbol * sym = (Symbol *) ( mchunk + 1 ) ;
            _Printf ( "\nFree : %s : 0x%lld : %d, ", (int) ( sym->S_Name ) > 0x80000000 ? (char*) sym->S_Name : "(null)", mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
    DLNode_Remove ( (DLNode*) mchunk ) ;
    munmap ( mchunk, size ) ; 
}

void
Mem_FreeItem ( DLList * mList, byte * item )
{
    DLNode * node, *nodeNext ;
    for ( node = DLList_First ( mList ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = (MemChunk*) node ;
        nodeNext = DLNode_Next ( node ) ;
        if ( (byte*) mchunk->S_pb_Data == item )
        {
            _Mem_ChunkFree ( mchunk ) ;
            return ;
        }
    }
}

void
FreeChunkList ( DLList * list )
{
    DLNode * node, *nodeNext ;
    for ( node = DLList_First ( list ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = (MemChunk*) node ;
        nodeNext = DLNode_Next ( node ) ;
        _Mem_ChunkFree ( mchunk ) ;
    }
}

void
NBA_FreeChunkType ( NBA * nba, uint64 type, int32 exactFlag )
{
    if ( exactFlag ) if ( nba->NBA_AType != type ) return ;
        else if ( ! ( nba->NBA_AType & type ) ) return ;
    FreeChunkList ( nba->BA_MemoryList ) ;
}

void
_MemList_FreeExactType ( DLList * list, int type )
{
    DLList_Map2_64 ( list, (MapFunction2_64) NBA_FreeChunkType, type, 1 ) ;
}

void
_MemList_FreeVariousTypes ( DLList * list, int type )
{
    DLList_Map2_64 ( list, (MapFunction2_64) NBA_FreeChunkType, type, 0 ) ;
}

void
NBAsMemList_FreeExactType ( int type )
{
    //DLList_Map2_64 ( _Q_->MemorySpace0->NBAs, (MapFunction2_64) NBA_FreeChunkType, type, 1 ) ;
    _MemList_FreeExactType ( _Q_->MemorySpace0->NBAs, type ) ;
}

void
NBAsMemList_FreeVariousTypes ( int type )
{
    //DLList_Map2_64 ( _Q_->MemorySpace0->NBAs, (MapFunction2_64) NBA_FreeChunkType, type, 0 ) ;
    _MemList_FreeVariousTypes ( _Q_->MemorySpace0->NBAs, type ) ;
}

void
_NamedByteArray_AddNewByteArray ( NamedByteArray *nba, int32 size )
{
    if ( size > nba->NBA_Size )
    {
        size += nba->NBA_Size ;
    }
    nba->MemAllocated += size ;
    nba->MemRemaining += size ;
    nba->ba_ByteArray = _ByteArray_AllocateNew ( nba, size ) ; // the whole array itself is allocated as a chunk then we can allocate with its specific type
    nba->NumberOfByteArrays ++ ;
}

NamedByteArray *
_NamedByteArray_Allocate ( )
{
    return (NamedByteArray *) _MemChunk_Allocate ( sizeof (NamedByteArray ), OPENVMTIL ) ; // nb : this is just the nba structure and it must be type OPENVMTIL or ..
}

void
_NamedByteArray_Init ( NamedByteArray * nba, byte * name, int32 size, int32 atype )
{
    _Symbol_Init ( (Symbol*) nba, name ) ;
    nba->NBA_AType = atype ;
    nba->BA_MemoryList = _DLList_New ( OPENVMTIL ) ;
    nba->NBA_Size = size ;
    nba->MemInitial = size ;
    nba->NumberOfByteArrays = 0 ;
    _NamedByteArray_AddNewByteArray ( nba, size ) ;
}

NamedByteArray *
NamedByteArray_New ( byte * name, int32 size, int32 atype )
{
    NamedByteArray * nba = _NamedByteArray_Allocate ( ) ; // else the nba would be deleted with MemList_FreeExactType ( nba->NBA_AType ) ;
    _NamedByteArray_Init ( nba, name, size, atype ) ;
    return nba ;
}

void
OVT_MemList_FreeNBAMemory ( byte * name, uint32 moreThan, int32 always )
{
#if 1 // ?!? how much do we need this at all ?!?
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    if ( always || ( nba->MemAllocated > ( nba->MemInitial + moreThan ) ) )
    {
        NBAsMemList_FreeExactType ( nba->NBA_AType ) ;
        nba->MemAllocated = 0 ;
        nba->MemRemaining = 0 ;
        _NamedByteArray_AddNewByteArray ( nba, nba->NBA_Size ) ;
    }
#endif    
}

void
OVT_MemListFree_Objects ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "ObjectSpace", 20 * M, 0 ) ;
}

void
OVT_MemListFree_TempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "TempObjectSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_ContextMemory ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "ContextSpace", 10 * M, 0 ) ;
}

void
OVT_MemListFree_CompilerTempObjects ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "CompilerTempObjectSpace", 0, 0 ) ;
}

void
OVT_MemListFree_LispTemp ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "LispTempSpace", 2 * M, 0 ) ;
}

void
OVT_MemListFree_Session ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "SessionObjectsSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_CfrTilInternal ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "CfrTilInternalSpace", 1 * M, 0 ) ;
}

void
OVT_MemListFree_HistorySpace ( )
{
    OVT_MemList_FreeNBAMemory ( (byte*) "HistorySpace", 1 * M, 0 ) ;
}

#if 0
void
_CfrTil_MemorySpaceAllocated ( NamedByteArray * nba )
{
    //nba = _Q_->MemorySpace0->SessionObjectsSpace

    int32 dmem = ( nba->ba_ByteArray->EndIndex - nba->ba_ByteArray->BA_Data ) ;
    int32 dmema = nba->NBA_Size - dmem ;
    Printf ( (byte*) "\n%s memory allocated in this session = %d : remaining was %d", nba->Name, dmem, dmema ) ;
}

void
CfrTil_MemorySpaceAllocated ( byte * name )
{
    NamedByteArray *nba = _OVT_Find_NBA ( name ) ;
    _CfrTil_MemorySpaceAllocated ( nba ) ;
}
#endif

void
NBA_Show ( NamedByteArray * nba )
{
    ByteArray * ba = nba->ba_ByteArray ;
    byte * name = nba->s_Symbol.S_Name ;
    Printf ( (byte*) "\n%-28s" "Used = " INT_FRMT_9 " : Available = " INT_FRMT_9, name, nba->MemAllocated - nba->MemRemaining, nba->MemRemaining ) ;
}

int32
_MemList_GetCurrentMemAllocated ( DLList * list, int32 flag )
{
    DLNode * node, *nodeNext ;
    int32 memAllocated = 0 ;
    if ( flag ) Printf ( c_dd ( "\nformat :: Type Name or Chunk Pointer : Type : Size, ...\n" ) ) ;
    for ( node = DLList_First ( list ) ; node ; node = nodeNext )
    {
        MemChunk * mchunk = (MemChunk*) node ;
        nodeNext = DLNode_Next ( node ) ;
        if ( mchunk->S_ChunkSize )
        {
            memAllocated += mchunk->S_ChunkSize ;
            if ( flag ) _Printf ( "0x%08x : 0x%08llx : %d, ", (uint) mchunk, mchunk->S_AType, mchunk->S_ChunkSize ) ;
        }
    }
    return memAllocated ;
}

void
Calculate_CurrentMemoryAllocationInfo ( )
{
    DLNode * node, * nextNode ;
    NamedByteArray * nba ;
    _Q_->MemAllocated = 0 ;
    _Q_->MemRemaining = 0 ;
    _Q_->NumberOfByteArrays = 0 ;
    if ( _Q_ && _Q_->MemorySpace0 )
    {
        for ( node = DLList_First ( _Q_->MemorySpace0->NBAs ) ; node ; node = nextNode )
        {
            nextNode = DLNode_Next ( node ) ;
            nba = (NamedByteArray*) node ;
            NBA_Show ( nba ) ;
            _Q_->MemAllocated += nba->MemAllocated ;
            _Q_->MemRemaining += nba->MemRemaining ;
            _Q_->NumberOfByteArrays += nba->NumberOfByteArrays ;
        }
    }
    _Q_->MemAllocated += ( _Q_->NumberOfByteArrays * sizeof (ByteArray) ) ;
    //int32 plistTotal = _MemList_GetCurrentMemAllocated ( _Q_->PermanentMemList, 0 ) ;
    _Q_->MemAllocated += _MemList_GetCurrentMemAllocated ( _Q_->PermanentMemList, 0 ) ;
}

void
CfrTil_MemoryAllocated ( )
{
    Calculate_CurrentMemoryAllocationInfo ( ) ;
    int32 memDiff = _Q_->MmapMemoryAllocated - _Q_->MemAllocated ;
    int32 dsu = DataStack_Depth ( ) * sizeof (int32 ) ;
    int32 dsa = ( STACK_SIZE * sizeof (int32 ) ) - dsu ;
    Printf ( (byte*) "\n%-28s" "Used = %9d : Available = %9d", "Data Stack", dsu, dsa ) ;
    Printf ( (byte*) "\n%-28s" "Used = %9d : Available = %9d", "Total Categorized Mem", _Q_->MemAllocated - _Q_->MemRemaining, _Q_->MemRemaining ) ;
    Printf ( (byte*) "\nMem Alloc Continuous Total       =  %d : %s", _Q_->MmapMemoryAllocated, "<=: _Q_->ChunkMemoryAllocated" ) ;
    Printf ( (byte*) "\nMem Alloc Current Info           =  %9d : %s", _Q_->MemAllocated, "<=: _Q_->MemAllocated <=: Used + Available" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( (byte*) "\nCurrent Unaccounted Diff (leak?) =  %9d : %s", memDiff, "<=: _Q_->ChunkMemoryAllocated - _Q_->MemAllocated" ) ; //+ _Q_->UnaccountedMem ) ) ;
    Printf ( (byte*) "\nCalculator :: %d - ( %d + %d ) = %d", _Q_->MmapMemoryAllocated, _Q_->MemAllocated - _Q_->MemRemaining, _Q_->MemRemaining, memDiff ) ; //memReportedAllocated ) ; ;//+ _Q_->UnaccountedMem ) ) ;
    fflush ( stdout ) ;
}

NamedByteArray *
_OVT_Find_NBA ( byte * name )
{
    // needs a Word_Find that can be called before everything is initialized
    NamedByteArray *nba = (NamedByteArray*) _Word_Find_Minimal ( _Q_->MemorySpace0->NBAs, - 1, (byte *) name ) ;
    return nba ;
}

NamedByteArray *
MemorySpace_NBA_New ( MemorySpace * memSpace, byte * name, int32 size, int32 atype )
{
    NamedByteArray *nba = NamedByteArray_New ( name, size, atype ) ;
    DLList_AddNodeToHead ( memSpace->NBAs, (DLNode*) nba ) ;
    return nba ;
}

void
MemorySpace_Init ( MemorySpace * ms )
{
    OpenVmTil * ovt = _Q_ ;
    _Q_->MemorySpace0 = ms ;
    ms->NBAs = _DLList_New ( OPENVMTIL ) ;

    ms->OpenVmTilSpace = MemorySpace_NBA_New ( ms, (byte*) "OpenVmTilSpace", 2 * M, OPENVMTIL ) ; //4.1 * M, CFRTIL_INTERNAL ) ;
    ms->CfrTilInternalSpace = MemorySpace_NBA_New ( ms, (byte*) "CfrTilInternalSpace", 4.1 * M, CFRTIL ) ; //4.1 * M, CFRTIL_INTERNAL ) ;
    ms->ObjectSpace = MemorySpace_NBA_New ( ms, (byte*) "ObjectSpace", ovt->ObjectsSize, OBJECT_MEMORY ) ;
    ms->TempObjectSpace = MemorySpace_NBA_New ( ms, (byte*) "TempObjectSpace", ovt->TempObjectsSize, TEMPORARY ) ;
    ms->CompilerTempObjectSpace = MemorySpace_NBA_New ( ms, (byte*) "CompilerTempObjectSpace", ovt->CompilerTempObjectsSize, COMPILER_TEMP_OBJECT_MEMORY ) ;
    ms->CodeSpace = MemorySpace_NBA_New ( ms, (byte*) "CodeSpace", ovt->MachineCodeSize, CODE ) ;
    ms->SessionObjectsSpace = MemorySpace_NBA_New ( ms, (byte*) "SessionObjectsSpace", ovt->SessionObjectsSize, SESSION ) ;
    ms->DictionarySpace = MemorySpace_NBA_New ( ms, (byte*) "DictionarySpace", ovt->DictionarySize, DICTIONARY ) ;
    ms->LispTempSpace = MemorySpace_NBA_New ( ms, (byte*) "LispTempSpace", ovt->LispTempSize, LISP_TEMP ) ;
    ms->BufferSpace = MemorySpace_NBA_New ( ms, (byte*) "BufferSpace", 2 * M, BUFFER ) ;
    ms->ContextSpace = MemorySpace_NBA_New ( ms, (byte*) "ContextSpace", ovt->ContextSize, CONTEXT ) ;
    ms->HistorySpace = MemorySpace_NBA_New ( ms, (byte*) "HistorySpace", HISTORY_SIZE, HISTORY ) ;

    ms->BufferList = _DLList_New ( OPENVMTIL ) ; // put it here to minimize allocating chunks for each node and the list

    CompilerMemByteArray = ms->CodeSpace->ba_ByteArray ; //init CompilerSpace ptr

    if ( _Q_->Verbosity > 2 ) Printf ( (byte*) "\nSystem Memory has been initialized.  " ) ;
}

MemorySpace *
MemorySpace_New ( OpenVmTil * ovt )
{
    MemorySpace *memSpace = (MemorySpace*) MemList_AllocateChunk ( & _MemList_, sizeof ( MemorySpace ), OPENVMTIL ) ;
    ovt->MemorySpace0 = memSpace ;
    MemorySpace_Init ( memSpace ) ; // can't be initialized until after it is hooked into it's System
    return memSpace ;
}

