
#include "../includes/cfrtil.h"

void
_CfrTil_Run ( CfrTil * cfrTil )
{
    while ( 1 )
    {
        OVT_MemListFree_Session ( ) ;
        cfrTil = _CfrTil_New ( cfrTil ) ;
        if ( cfrTil )
        {
            if ( ! setjmp ( cfrTil->JmpBuf0 ) )
            {
                System_RunInit ( _Context_->System0 ) ;
                switch ( _Q_->RestartCondition )
                {
                    case 0:
                    case INITIAL_START:
                    case FULL_RESTART:
                    case RESTART:
                    case RESET_ALL: CfrTil_ResetAll_Init ( cfrTil ) ;
                    case ABORT: CfrTil_SyncStackPointerFromDsp ( cfrTil ) ;
                    default:
                    case QUIT:
                    case STOP: ;
                }
                // check if reset is ok ...
                if ( cfrTil && _Context_ && _Context_->System0 )
                {
                    Ovt_RunInit ( _Q_ ) ;
                    System_Time ( cfrTil->Context0->System0, 0, ( char* ) "Startup", 1 ) ; //_Q_->StartedTimes == 1 ) ;
                    _CfrTil_Version ( 0 ) ;
                    CfrTil_InterpreterRun ( ) ;
                }
            }
        }
    }
}

void
CfrTil_CpuState_Show ( )
{
    _CfrTil_->SaveCpuState ( ) ;
    _CpuState_Show ( _CfrTil_->cs_CpuState ) ;
}

void
_CfrTil_NamespacesInit ( CfrTil * cfrTil )
{
    //Namespace * ns = _Namespace_New ( ( byte* ) "Namespaces", 0 ) ;
    Namespace * ns = _DataObject_New ( NAMESPACE, ( byte* ) "Namespaces", 0, 0, 0, 0 ) ;
    ns->State = USING ; // nb. _Namespace_SetState ( ns, USING ) ; // !! can't be used with "Namespaces"
    cfrTil->Namespaces = ns ;
    CfrTil_AddCPrimitives ( ) ;
}

void
_CfrTil_SetStackPointerFromDsp ( CfrTil * cfrTil )
{
    cfrTil->DataStack->StackPointer = Dsp ;
}

void
CfrTil_SyncStackPointerFromDsp ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetStackPointerFromDsp ( cfrTil ) ;
}

void
_CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    Dsp = cfrTil->DataStack->StackPointer ;
}

void
CfrTil_SetDspFromStackPointer ( CfrTil * cfrTil )
{
    if ( cfrTil && cfrTil->DataStack ) _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
}

void
CfrTil_DataStack_InitEssential ( CfrTil * cfrTil )
{
    cfrTil->DataStack->StackPointer = cfrTil->DataStack->InitialTosPointer ;
    _CfrTil_SetDspFromStackPointer ( cfrTil ) ;
    cfrTil->SaveDsp = Dsp ;
}

#if 0
int32
_CfrTil_DataStack_Depth ( )
{
    //_CfrTil_SetStackPointerFromDsp ( _CfrTil_ ) ;
    //return Stack_Depth ( _DataStack_ ) ;
    DataStack_Depth ( ) ;
}
#endif

void
_CfrTil_DataStack_Init ( CfrTil * cfrTil )
{
    CfrTil_DataStack_InitEssential ( cfrTil ) ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nData Stack reset." ) ;
}

void
CfrTil_DataStack_Init ( )
{
    _CfrTil_DataStack_Init ( _CfrTil_ ) ;
}

void
_CfrTil_Init ( CfrTil * cfrTil, Namespace * nss )
{
    int32 type = OPENVMTIL ;
    _CfrTil_ = cfrTil ;
    _Q_->CfrTil0 = cfrTil ;
    // TODO : organize these buffers and their use 
    cfrTil->OriginalInputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->InputLineB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->SourceCodeSPB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->LambdaCalculusPB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TokenB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->PrintfB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->HistoryExceptionB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->DebugB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB2 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringInsertB3 = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->TabCompletionB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->StringMacroB = _Buffer_NewPermanent ( BUFFER_SIZE ) ;
    cfrTil->OriginalInputLine = Buffer_Data ( cfrTil->OriginalInputLineB ) ;
    cfrTil->SourceCodeScratchPad = Buffer_Data ( cfrTil->SourceCodeSPB ) ;
    cfrTil->LispPrintBuffer = Buffer_Data ( cfrTil->LambdaCalculusPB ) ;
    cfrTil->TokenBuffer = Buffer_Data ( cfrTil->TokenB ) ;
    CfrTil_SetState ( cfrTil, CFRTIL_RUN | OPTIMIZE_ON | INLINE_ON, true ) ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nSystem Memory is being reallocated.  " ) ;
    cfrTil->ContextStack = Stack_New ( 256, type ) ;
    cfrTil->ObjectStack = Stack_New ( 1 * K, type ) ;
    cfrTil->TokenList = _DLList_New ( type ) ;
    cfrTil->PeekTokenList = _DLList_New ( type ) ;
    _Context_ = cfrTil->Context0 = _Context_New ( cfrTil, type ) ;
    if ( nss ) // && ( _Q_->Signal <= ABORT ) )
    {
        cfrTil->Namespaces = nss ;
    }
    else _CfrTil_NamespacesInit ( cfrTil ) ;
    cfrTil->Debugger0 = _Debugger_New ( type ) ; // nb : must be after System_NamespacesInit
    if ( cfrTil->SaveDsp && cfrTil->DataStack )// with _Q_->RestartCondition = STOP from Debugger_Stop
    {
        Dsp = cfrTil->SaveDsp ;
    }
    else
    {
        cfrTil->DataStack = Stack_New ( _Q_->DataStackSize, type ) ;
        _CfrTil_DataStack_Init ( cfrTil ) ;
    }
    CfrTil_ReadTables_Setup ( cfrTil ) ;
    CfrTil_LexerTables_Setup ( cfrTil ) ;
    cfrTil->LC = 0 ; //LC_New ( ) ;
    cfrTil->cs_CpuState = CpuState_New ( type ) ;
    CfrTil_MachineCodePrimitive_AddWords ( ) ;
    cfrTil->StoreWord = _Word_FindAny ( "store" ) ;
    cfrTil->PokeWord = _Word_FindAny ( "poke" ) ;
    cfrTil->LispNamespace = Namespace_Find ( "Lisp" ) ;
}

CfrTil *
_CfrTil_New ( CfrTil * cfrTil )
{
    // nb. not all of this logic has really been needed or used or tested; it should be reworked according to need
    Namespace * nss ;
    if ( cfrTil && ( _Q_->RestartCondition < RESTART ) )
    {
        nss = cfrTil->Namespaces ; // in this case (see also below) only preserve Namespaces, all else is recycled and reinitialized
        if ( cfrTil->LogFILE ) fclose ( cfrTil->LogFILE ) ;
        OVT_MemListFree_CfrTilInternal ( ) ;
    }
    else
    {
        nss = 0 ;
    }
    cfrTil = ( CfrTil* ) _Mem_Allocate ( sizeof ( CfrTil ), CFRTIL ) ;
    _CfrTil_Init ( cfrTil, nss ) ;
    return cfrTil ;
}

void
CfrTil_Delete ( CfrTil * cfrTil, int stackFlag )
{
    NBAsMemList_FreeVariousTypes ( - 1 & ~ ( OPENVMTIL | HISTORY | ( stackFlag ? ( DATA_STACK | OBJECT_MEMORY | CODE | DICTIONARY ) : 0 ) ) ) ; //CFRTIL|CONTEXT|SESSION|OBJECT_MEMORY|CODE|DICTIONARY ) ;
    if ( _Q_->Verbosity > 2 ) Printf ( ( byte* ) "\nAll CFRTIL memory has been freed. Command line history was preserved." ) ;
}

void
CfrTil_Lexer_SourceCodeOn ( )
{
    Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
}

void
_CfrTil_AddStringToSourceCode ( byte * str )
{
    strcat ( ( char* ) _CfrTil_->SourceCodeScratchPad, ( char* ) str ) ;
    strcat ( ( CString ) _CfrTil_->SourceCodeScratchPad, ( CString ) " " ) ;
}

void
SC_ScratchPadIndex_Init ( )
{
    _CfrTil_->SC_ScratchPadIndex = strlen ( ( char* ) _CfrTil_->SourceCodeScratchPad ) ;
}

void
__CfrTil_InitSourceCode ( )
{
    _CfrTil_->SourceCodeScratchPad [ 0 ] = 0 ;
    _CfrTil_->SC_ScratchPadIndex = 0 ;
    SetState ( _CfrTil_, SOURCE_CODE_INITIALIZED, true ) ;
}

void
_InitSourceCode ( int32 force )
{
    if ( force || ( ! GetState ( _CfrTil_, SOURCE_CODE_INITIALIZED ) ))
    {
        Lexer_SourceCodeOn ( _Context_->Lexer0 ) ;
        __CfrTil_InitSourceCode ( ) ;
    }
}

void
_CfrTil_InitSourceCode ( )
{
    _InitSourceCode ( 1 ) ;
    SC_ScratchPadIndex_Init ( ) ;
    //SetState ( _CfrTil_, SOURCE_CODE_INITIALIZED, true ) ;
}

void
_CfrTil_InitSourceCode_WithName ( byte * name )
{
    _InitSourceCode ( 1 ) ;
    _CfrTil_AddStringToSourceCode ( name ) ;
    SC_ScratchPadIndex_Init ( ) ;
    //SetState ( _CfrTil_, SOURCE_CODE_INITIALIZED, true ) ;
}

void
CfrTil_InitSourceCode_WithCurrentInputChar ( )
{
    Lexer * lexer = _Context_->Lexer0 ;
    _InitSourceCode ( 1 ) ;
    _Lexer_AppendCharToSourceCode ( lexer, lexer->TokenInputCharacter ) ;
}

void
_CfrTil_FinishSourceCode ( Word * word )
{
    // keep a LambdaCalculus LO_Define0 created SourceCode value
    if ( ! word->SourceCode ) word->SourceCode = String_New ( _CfrTil_->SourceCodeScratchPad, DICTIONARY ) ;
    Lexer_SourceCodeOff ( _Context_->Lexer0 ) ;
    SetState ( _CfrTil_, SOURCE_CODE_INITIALIZED, false ) ;
}

void
_CfrTil_UnAppendFromSourceCode ( int nchars )
{
    int plen = strlen ( ( CString ) _CfrTil_->SourceCodeScratchPad ) ;
    if ( plen >= nchars )
    {
        _CfrTil_->SourceCodeScratchPad [ strlen ( ( CString ) _CfrTil_->SourceCodeScratchPad ) - nchars ] = 0 ;
    }
    SC_ScratchPadIndex_Init ( ) ;
}

void
_CfrTil_UnAppendTokenFromSourceCode ( byte * tkn )
{
    _CfrTil_UnAppendFromSourceCode ( strlen ( ( CString ) tkn ) + 1 ) ;
}

void
_CfrTil_AppendCharToSourceCode ( byte c )
{
    if ( _CfrTil_->SC_ScratchPadIndex < ( BUFFER_SIZE - 1 ) )
    {
        _CfrTil_->SourceCodeScratchPad [ _CfrTil_->SC_ScratchPadIndex ++ ] = c ;
        _CfrTil_->SourceCodeScratchPad [ _CfrTil_->SC_ScratchPadIndex ] = 0 ;
    }
}

//----------------------------------------------------------------------------------------|
//              get from/ add to head  |              | get from head      add to tail    |      
// TokenList Tail <--> TokenList Head  |<interpreter> | PeekList Head <--> PeekList Tail  |
// token token token token token token | currentToken | token token token token token ... |
//----------------------------------------------------------------------------------------|

byte *
_CfrTil_AddSymbolToHeadOfTokenList ( Symbol * tknSym )
{
    DLList_AddNodeToHead ( _CfrTil_->TokenList, ( DLNode* ) tknSym ) ;
}

byte *
_CfrTil_AddNewTokenSymbolToHeadOfTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    _CfrTil_AddSymbolToHeadOfTokenList ( tknSym ) ;
}

byte *
_CfrTil_GetTokenFromPeekedTokenList ( )
{
    Symbol * peekTokenSym ;
    if ( peekTokenSym = ( Symbol* ) _DLList_First ( _CfrTil_->PeekTokenList ) )
    {
        DLNode_Remove ( ( DLNode* ) peekTokenSym ) ;
        _CfrTil_AddSymbolToHeadOfTokenList ( peekTokenSym ) ;
        return _Context_->Lexer0->OriginalToken = peekTokenSym->S_Name ;
    }
    return 0 ;
}

void
_CfrTil_AddTokenToTailOfPeekTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    DLList_AddNodeToTail ( _CfrTil_->PeekTokenList, ( DLNode* ) tknSym ) ;
}

void
_CfrTil_AddTokenToHeadOfPeekTokenList ( byte * token )
{
    Symbol * tknSym = _Symbol_New ( token, TEMPORARY ) ;
    DLList_AddNodeToHead ( _CfrTil_->PeekTokenList, ( DLNode* ) tknSym ) ;
}


