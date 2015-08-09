
#include "../includes/cfrtil.h"

// we have to remember that namespaces are being moved to the beginning of the Namespaces list by _Namespace_AddToUsingList

Word *
_TC_NextWord ( Word * runWord )
{
    Word * nextrw ;
    if ( ! runWord ) return _CfrTil_->Namespaces ;
    if ( runWord == _CfrTil_->Namespaces ) return ( Word* ) DLList_First ( runWord->Lo_List ) ;
    if ( Is_NamespaceType ( runWord ) && ! (runWord->CType & ALIAS ) ) if ( nextrw = ( Word* ) DLList_First ( runWord->Lo_List ) ) return nextrw ;
    if ( nextrw = ( Word* ) DLNode_Next ( ( Node* ) runWord ) ) return nextrw ;
    return ( Word* ) DLNode_Next ( ( Node* ) runWord->ContainingNamespace ) ; // this doesn't finish a namespace started in the middle
}

// map starting from any word
// used now only with tab completion

Word *
_TC_Map ( Word * first, MapFunction mf )
{
    Word * word = first, *nextWord ;
    do
    {
        nextWord = _TC_NextWord ( word ) ;
        if ( mf ( ( Symbol* ) word ) ) return nextWord ;
        word = nextWord ;
        if ( kbhit ( ) ) return nextWord ;
    }
    while ( word != first ) ;
    return 0 ;
}

void
TabCompletion_Run ( )
{
    TabCompletionInfo * tci = _Context_->ReadLiner0->TabCompletionInfo0 ;
    tci->RunWord = _TC_Map ( tci->RunWord, ( MapFunction ) _TabCompletionFunction ) ;
}

TabCompletionInfo *
TabCompletionInfo_New ( int32 type )
{
    TabCompletionInfo *tci = ( TabCompletionInfo * ) _Mem_Allocate ( sizeof (TabCompletionInfo ), type ) ;
    return tci ;
}

byte *
Word_GenerateFullNamespaceQualifiedName ( Word * w )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    Buffer_Clear ( _CfrTil_->TabCompletionB ) ;
    byte * b0 = ( char* ) Buffer_Data ( _CfrTil_->TabCompletionB ), * wn = w->Name ;
    Stack_Init ( rl->TciNamespaceStack ) ;
    Stack * nsStk = rl->TciNamespaceStack ;
    Namespace *ns ;
    int32 i ;

    String_Init ( b0 ) ;
    for ( ns = Is_NamespaceType ( w ) ? w : w->ContainingNamespace ; ns ; ns = ns->ContainingNamespace ) // && ( tw->ContainingNamespace != _CfrTil_->Namespaces ) )
    {
        _Stack_Push ( nsStk, ( int32 ) ns->Name ) ;
    }
    for ( i = Stack_Depth ( nsStk ) ; i ; i -- )
    {
        byte * b1 = ( byte* ) _Stack_Pop ( nsStk ) ;
        if ( b1 )
        {
            strcat ( ( CString ) b0, ( CString ) b1 ) ;
            if ( ( i > 1 ) || ( ( w == tci->OriginalWord ) && tci->EndDottedFlag ) ) strcat ( ( CString ) b0, "." ) ;
        }
    }
    if ( ! Is_NamespaceType ( w ) )
    {
        strcat ( ( CString ) b0, "." ) ;
        strcat ( ( CString ) b0, ( CString ) wn ) ; // namespaces are all added above
    }
    return b0 ;
}

Boolean
_TabCompletionFunction ( Word * word )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    byte * searchToken ;
    if ( word )
    {
        searchToken = tci->SearchToken ;
        Word * tw = tci->TrialWord = word ;
        byte * twn = tw->Name, *fqn ;
        if ( twn ) //&& tw->ContainingNamespace )
        {
            int32 strOpRes1, strOpRes2, strOpRes3 ;
            if ( ! strlen ( searchToken ) ) // we match anything when user ends with a dot ( '.' ) ...
            {
                // except .. We don't want to jump down into a lower namespace here.
                if ( ( tw->ContainingNamespace == tci->OriginalContainingNamespace ) ) // || ( tw->ContainingNamespace == _CfrTil_->Namespaces ) )
                {
                    strOpRes1 = 1 ;
                }
                else return false ;
            }
            else
            {
                byte bufw [128], bufo[128] ;
                strOpRes1 = ! strnicmp ( ( CString ) twn, ( CString ) searchToken, strlen ( ( CString ) searchToken ) ) ; // strstr == token : the start of the dictionary entry
                if ( ! strOpRes1 ) strOpRes2 = ( int32 ) strstr ( ( CString ) twn, ( CString ) searchToken ) ; // == ( String ) twn ) ;// strstr == token : the start of the dictionary entry
                if ( ! ( strOpRes1 | strOpRes2 ) )
                {
                    strToLower ( bufw, twn ) ;
                    strToLower ( bufo, searchToken ) ;
                    strOpRes3 = ( int32 ) strstr ( ( CString ) bufw, ( CString ) bufo ) ; // == ( String ) twn ) ;// strstr == token : the start of the dictionary entry
                }
            }
            if ( strOpRes1 | strOpRes2 | strOpRes3 ) //|| ( word == tci->OriginalWord ? tci->OriginalWord->CType &  NAMESPACE_TYPES : 0 ))
            {
                if ( ! tci->MarkWord ) tci->MarkWord = word ;
                fqn = Word_GenerateFullNamespaceQualifiedName ( tw ) ;
                _TC_StringInsert_AtCursor ( tci, ( CString ) fqn ) ;
                return true ;
            }
        }
    }
    tci->TrialWord = 0 ;
    return false ;
}

// added 0.756.541
// problem here is that with the Class word '.' (dot) it loops and doesn't return

int32
_TC_FindPrevious_NamespaceQualifiedIdentifierStart ( TabCompletionInfo * tci, CString s, int32 pos )
{
    int f, l, last = 0, dot ; // these refer to positions in the string s
    do
    {
        l = String_LastCharOfLastToken_FromPos ( s, pos ) ;
        if ( ! last ) tci->TokenLastChar = last = l ;
        if ( ( last == pos ) && ( s [last] <= ' ' ) && ( last != _ReadLine_CursorPosition ( _Context_->ReadLiner0 ) ) ) return last ;
        f = String_FirstCharOfToken_FromPosOfLastChar ( s, l ) ;
        if ( f > 0 )
        {
            dot = String_IsThereADotSeparatorBackFromPosToLastNonDelmiter ( s, f - 1 ) ;
        }
        else break ;
    }
    while ( pos = dot ) ;
    return f ;
}

void
_TC_StringInsert_AtCursor ( TabCompletionInfo * tci, CString strToInsert )
{
    ReadLiner * rl = _Context_->ReadLiner0 ;
    int32 stiLen, newCursorPos, startCursorPos = _ReadLine_CursorPosition ( rl ) ;
    int32 slotStart = _TC_FindPrevious_NamespaceQualifiedIdentifierStart ( tci, ( CString ) rl->InputLine, startCursorPos ) ; //cursorPos0 ) ; //rl->i32_CursorPosition ) ;
    stiLen = strlen ( ( CString ) strToInsert ) ;
    newCursorPos = slotStart + stiLen ; //+ ( Is_NamespaceType (tci->TrialWord) && tci->EndDottedFlag ? 1 : 0 ) ;
    if ( newCursorPos < stiLen )
    {
        ReadLine_InputLine_Clear ( rl ) ;
        strcpy ( ( CString ) rl->InputLine, ( CString ) _CfrTil_->OriginalInputLine ) ;
    }
    ReadLine_SetCursorPosition ( rl, newCursorPos ) ;
    _ReadLine_InsertStringIntoInputLineSlotAndShow ( rl, slotStart, startCursorPos, ( byte* ) strToInsert ) ; // 1 : TokenLastChar is the last char of the identifier
}

byte *
_TabCompletionInfo_GetAPreviousIdentifier ( ReadLiner *rl, int32 start )
{
    byte b [128] ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    tci->TokenLastChar = ReadLine_LastCharOfLastToken_FromPos ( rl, start ) ;
    tci->TokenFirstChar = ReadLine_FirstCharOfToken_FromLastChar ( rl, tci->TokenLastChar ) ;
    tci->TokenLength = tci->TokenLastChar - tci->TokenFirstChar + 1 ; // zero array start
    strncpy ( ( CString ) b, ( CString ) & rl->InputLine [ tci->TokenFirstChar ], tci->TokenLength ) ;
    b [ tci->TokenLength ] = 0 ;
    return TemporaryString_New ( b ) ;
}

// nb. the notation (function names) around parsing in tab completion is based in 'reverse parsing' - going back in the input line from the cursor position

void
TabCompletionInfo_Init ( )
{
    Namespace * piw ;
    ReadLiner * rl = _Context_->ReadLiner0 ;
    TabCompletionInfo * tci = rl->TabCompletionInfo0 ;
    memset ( tci, 0, sizeof ( TabCompletionInfo ) ) ;
    ReadLiner_SetState ( rl, TAB_WORD_COMPLETION, true ) ;
    strcpy ( ( CString ) _CfrTil_->OriginalInputLine, ( CString ) rl->InputLine ) ; // we use this extra buffer at ReadLine_TC_StringInsert_AtCursor
    tci->Identifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, _ReadLine_CursorPosition ( rl ) ) ;
    tci->DotSeparator = ReadLine_IsThereADotSeparator ( rl, tci->TokenFirstChar - 1 ) ;
    if ( tci->TokenFirstChar ) tci->PreviousIdentifier = _TabCompletionInfo_GetAPreviousIdentifier ( rl, tci->TokenFirstChar - 1 ) ; // TokenStart refers to start of 'Identifier'
    if ( ReadLine_IsLastCharADot ( rl, rl->i32_CursorPosition ) ) //ReadLine_IsDottedToken ( rl ) )
    {
        tci->EndDottedFlag = 1 ; // using a dot in a tabCompletion causes the search to remain in the same namespace and down tree otherwise the search is of the whole word tree
        tci->SearchToken = ( byte * ) "" ; // everything matches
        rl->InputLine [-- rl->i32_CursorPosition] = ' ' ; // discard the final '.' 
    }
    else tci->SearchToken = tci->Identifier ? tci->Identifier : ( byte* ) "" ;
    if ( ( tci->OriginalWord = _CfrTil_FindInAnyNamespace ( tci->Identifier ) ) )
    {
        if ( tci->EndDottedFlag && Is_NamespaceType ( tci->OriginalWord ) )
        {
            tci->RunWord = ( Word* ) DLList_First ( tci->OriginalWord->Lo_List ) ;
            tci->OriginalContainingNamespace = tci->OriginalWord ;
        }
        else
        {
            tci->EndDottedFlag = 0 ;
            tci->OriginalContainingNamespace = tci->OriginalWord->ContainingNamespace ? tci->OriginalWord->ContainingNamespace : _CfrTil_->Namespaces ;
            tci->RunWord = tci->OriginalWord ;
        }
    }
    else
    {
        if ( ( tci->DotSeparator ) && ( piw = _CfrTil_FindInAnyNamespace ( tci->PreviousIdentifier ) ) )
        {
            if ( Is_NamespaceType ( piw ) )
            {
                tci->RunWord = ( Word* ) DLList_First ( piw->Lo_List ) ;
                tci->OriginalContainingNamespace = piw ;
            }
        }
    }
    if ( ! tci->RunWord ) tci->RunWord = _CfrTil_->Namespaces ;
    if ( ! tci->OriginalContainingNamespace ) tci->OriginalContainingNamespace = _CfrTil_->Namespaces ;
    tci->OriginalRunWord = tci->RunWord ;
    tci->MarkWord = 0 ;
}

