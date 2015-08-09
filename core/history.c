
#include "../includes/cfrtil.h"

HistoryStringNode *
HistoryStringNode_New ( byte * hstring )
{
    HistoryStringNode * hsn = ( HistoryStringNode * ) _Mem_Allocate ( sizeof ( HistoryStringNode ), HISTORY ) ;
    _Symbol_Init_AllocName ( ( Symbol* ) hsn, hstring, HISTORY ) ; // use Name for history string
    hsn->S_CType = HISTORY_NODE ;
    return hsn ;
}

HistoryStringNode *
HistorySymbolList_Find ( byte * hstring )
{
    HistoryStringNode * hsn = 0 ;
    DLNode * node, * nextNode ;
#if 1   
    for ( node = DLList_First ( _HistorySpace_.StringList ) ; node ; node = nextNode ) // index = DLNode_NextNode ( &_Q->HistoryList, (DLNode *) index ) )
    {
        nextNode = DLNode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) node ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#else // some work towards eliminating the StringList and just using the MemList
    for ( node = DLList_First ( _HistorySpace_.MemList ) ; node ; node = nextNode ) // index = DLNode_NextNode ( &_Q->HistoryList, (DLNode *) index ) )
    {
        nextNode = DLNode_Next ( node ) ;
        hsn = ( HistoryStringNode* ) ( ( MemChunk * ) node + 1 ) ;
        if ( ( hsn->S_Name ) && ( String_Equal ( ( char* ) hsn->S_Name, ( char* ) hstring ) ) )
        {
            return hsn ;
        }
    }
#endif    
    return 0 ;
}

void
ReadLine_ShowHistoryNode ( ReadLiner * rl )
{
    rl->EscapeModeFlag = 0 ;
    if ( rl->HistoryNode && rl->HistoryNode->S_Name )
    {
        //Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
        //byte * dst = Buffer_Data ( buffer ) ;
        char * dst = ( char* ) Buffer_Data ( _CfrTil_->HistoryExceptionB ) ;
        _String_ConvertStringToBackSlash ( dst, rl->HistoryNode->S_Name ) ;
        _ReadLine_PrintfClearTerminalLine ( ) ;
        __ReadLine_DoStringInput ( rl, String_FilterForHistory ( dst ), rl->AltPrompt ) ;
        ReadLine_SetCursorPosition ( rl, rl->EndPosition ) ;
        //Buffer_SetAsUnused ( buffer ) ;
    }
    else
    {
        ReadLine_ClearCurrentTerminalLine ( rl, rl->EndPosition ) ; // nb : this is also part of ShowString
        ReadLine_ShowNormalPrompt ( rl ) ;
    }
    _ReadLine_CursorToEnd ( rl ) ;
}

void
_OpenVmTil_AddStringToHistoryList ( byte * istring )
{
    HistoryStringNode * hsn ;
    if ( istring && strcmp ( ( char* ) istring, "" ) ) // don't add blank lines to history
    {
        //byte * nstring = String_FilterForHistory ( istring ) ;
        Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;
        byte * nstring = Buffer_Data ( buffer ) ;
        _String_ConvertStringToBackSlash ( nstring, istring ) ;

        hsn = HistorySymbolList_Find ( nstring ) ;
        if ( ! hsn )
        {
            hsn = HistoryStringNode_New ( nstring ) ;
        }
        else DLNode_Remove ( ( DLNode* ) hsn ) ;
        DLList_AddNodeToTail ( _HistorySpace_.StringList, ( DLNode* ) hsn ) ;
        DLList_After ( _HistorySpace_.StringList ) ; // ! properly set Object.dln_Node
        Buffer_SetAsUnused ( buffer ) ;
    }
}

void
OpenVmTil_AddStringToHistory ( )
{
    byte * string = ( byte* ) _DataStack_Pop ( ) ;
    _OpenVmTil_AddStringToHistoryList ( string ) ;
}

void
OpenVmTil_AddStringToHistoryOn ( )
{
    ReadLiner_SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, true ) ;
}

void
OpenVmTil_AddStringToHistoryOff ( )
{
    ReadLiner_SetState ( _Context_->ReadLiner0, ADD_TO_HISTORY, false ) ;
}

void
HistorySpace_Delete ( )
{
    //MemList_FreeExactType ( HISTORY ) ;
    OVT_MemListFree_HistorySpace ( ) ;
}

#if 1
HistorySpace *
_HistorySpace_Init ( OpenVmTil * ovt, int32 reset )
{
    _HistorySpace_.StringList = & _HistorySpace_._StringList ;
    DLList_Init ( _HistorySpace_.StringList, &_HistorySpace_._StringList_HeadNode, &_HistorySpace_._StringList_TailNode ) ;
    if ( ovt ) _HistorySpace_.HistorySpaceNBA = ovt->MemorySpace0->HistorySpace ;
    if ( reset ) _NamedByteArray_Init ( _HistorySpace_.HistorySpaceNBA, ( byte* ) "HistorySpace", HISTORY_SIZE, HISTORY ) ;
}
#endif

void
_HistorySpace_New ( OpenVmTil * ovt, int32 resetFlag )
{
    if ( resetFlag )
    {
        HistorySpace_Delete ( ) ;
    }
    _HistorySpace_Init ( ovt, resetFlag ) ;
}

void
HistorySpace_Reset ( void )
{
    _HistorySpace_New ( _Q_, 1 ) ;
}

#if 0
void
HistorySpace_Check ( )
{
    _HistorySpace_New ( _Q_, _Q_ && ( _Q_->SignalExceptionsHandled > 1 ) ) ;
}
#endif