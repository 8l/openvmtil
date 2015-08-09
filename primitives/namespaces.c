
#include "../includes/cfrtil.h"

/*
 * cfrTil namespaces basic understandings :
 * 0. to allow for ordered search thru lists in use ...
 * 1. Namespaces are also linked on the _Context->System0->Namespaces list with status of USING or NOT_USING
 *      and are moved the front or back of that list if status is set to USING with the 'Symbol node'
 *      This list is ordered so we can set an order to our search thru the namespaces in use. 
 *      The first word found within the ordered USING list will be used.
 */

// "root" -- ?? is this all the namespaces we want/need in this list ??
// no ! -- this needs updating

#if 0

void
CfrTil_Namespaces_Root ( )
{
    //DLList_Map1 ( _Context->System0->Words->Lo_List, ( MapFunction1 ) _Namespace_DoSetState, USING ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _Namespace_DoSetState, USING, 0 ) ;
    Namespace * ns1 = _Namespace_Find ( ( byte* ) "Root", 0, 1 ) ;
    Namespace * ns2 = _Namespace_Find ( ( byte* ) "Namespace", 0, 1 ) ;
    Namespace * ns3 = _Namespace_Find ( ( byte* ) "Io", 0, 1 ) ;
    Namespace * ns4 = _Namespace_Find ( ( byte* ) "System", 0, 1 ) ;
    Namespace * ns5 = _Namespace_Find ( ( byte* ) "OpenVmTil", 0, 1 ) ;
    Namespace * ns6 = _Namespace_Find ( ( byte* ) "Logic", 0, 1 ) ;
    Namespace * ns7 = _Namespace_Find ( ( byte* ) "Combinators", 0, 1 ) ;
    Namespace * ns8 = _Namespace_Find ( ( byte* ) "Compiler", 0, 1 ) ;
    Namespace * ns9 = _Namespace_Find ( ( byte* ) "Interpreter", 0, 1 ) ;
    Namespace * ns10 = _Namespace_Find ( ( byte* ) "Int", 0, 1 ) ;
    Namespace * ns11 = _Namespace_Find ( ( byte* ) "Stack", 0, 1 ) ;
    Namespace * ns12 = _Namespace_Find ( ( byte* ) "Word", 0, 1 ) ;

    //DLList_Map1 ( _Context->System0->Words->Lo_List, ( MapFunction1 ) _Namespace_DoSetState, NOT_USING ) ;
    //_Namespace_Map_2 ( DLList * list, cell state, MapSymbolFunction2 mf, cell one, cell two )
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) _Namespace_DoSetState, NOT_USING, 0 ) ;

    _Namespace_SetState ( ns1, USING ) ;
    _Namespace_SetState ( ns2, USING ) ;
    _Namespace_SetState ( ns3, USING ) ;
    _Namespace_SetState ( ns4, USING ) ;
    _Namespace_SetState ( ns5, USING ) ;
    _Namespace_SetState ( ns6, USING ) ;
    _Namespace_SetState ( ns7, USING ) ;
    _Namespace_SetState ( ns8, USING ) ;
    _Namespace_SetState ( ns9, USING ) ;
    _Namespace_SetState ( ns10, USING ) ;
    _Namespace_SetState ( ns11, USING ) ;
    _Namespace_SetState ( ns12, USING ) ;
    //
}
#endif

void
Do_Namespace_WithStatus_2 ( DLNode * node, MapFunction2 nsf, int32 nsStateFlag, int32 one, int32 two )
{
    Namespace * ns = ( Namespace * ) node ;
    if ( ns->State == nsStateFlag )
    {
        nsf ( node, one, two ) ;
    }
}

void
_CfrTil_TreeMap ( MapSymbolFunction2 msf2, int32 state, int32 two, int32 three )
{
    _Tree_Map_State_2 ( _CfrTil_->Namespaces->Lo_List, state, msf2, two, three ) ;
}

void
_CfrTil_NamespacesMap ( MapSymbolFunction2 msf2, int32 state, int32 two, int32 three )
{
    _Tree_Map_State_2 ( _CfrTil_->Namespaces->Lo_List, state, msf2, two, three ) ;
}

// list/print namespaces

void
_CfrTil_ForAllNamespaces ( MapSymbolFunction2 msf2 )
{
    Printf ( ( byte* ) "\nusing :" ) ;
    _CfrTil_NamespacesMap ( msf2, USING, 1, 1 ) ;
    Printf ( ( byte* ) "\nnotUsing :" ) ;
    _CfrTil_NamespacesMap ( msf2, NOT_USING, 1, 1 ) ;
}

void
Namespace_PrettyPrint ( Namespace* ns, int32 indentFlag, int32 indentLevel )
{
    if ( indentFlag )
    {
        Printf ( ( byte* ) "\n" ) ;
        while ( indentLevel -- ) Printf ( ( byte* ) "\t" ) ;
    }
    Printf ( ( byte* ) " - %s", c_dd ( ns->Name ) ) ;
}

void
CfrTil_Namespace_New ( )
{
    Namespace * ns = Namespace_FindOrNew_SetUsing ( ( byte* ) _DataStack_Pop ( ), _CfrTil_Namespace_InNamespaceGet ( ), 1 ) ;
    _Namespace_DoNamespace ( ns ) ;

}

void
CfrTil_Namespace_NotUsing ( )
{
    Namespace * ns = Namespace_Find ( ( byte* ) _DataStack_Pop ( ) ) ;
    if ( ns )
    {
        _Namespace_RemoveFromUsingList ( ns ) ;
        _CfrTil_->InNamespace = ( Namespace* ) _Tree_Map_FromANode ( DLNode_Next ( ( DLNode* ) ns ), ( cMapFunction_1 ) _Namespace_IsUsing ) ;
    }
}

void
CfrTil_Namespace_UsingFirst ( )
{
    Namespace * ns = Namespace_Find ( ( byte* ) _DataStack_Pop ( ) ) ;
    if ( ns )
    {
        _Namespace_AddToUsingList ( ns ) ;
    }
}

void
CfrTil_Namespace_UsingLast ( )
{
    _Namespace_UsingLast ( ( byte* ) _DataStack_Pop ( ) ) ;
}

// "in"

void
CfrTil_PrintInNamespace ( )
{
    Printf ( ( byte* ) "\nCurrent Namespace Being Compiled : %s\n",
        _CfrTil_Namespace_InNamespaceGet ( )->Name ) ;
}

// list/print namespaces

void
CfrTil_Namespaces ( )
{

    Printf ( ( byte* ) "\nAll Namespaces : \n<list> ':' '-' <namespace>" ) ;
    _CfrTil_ForAllNamespaces ( ( MapSymbolFunction2 ) Symbol_NamespacePrettyPrint ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
Symbol_SetNonTREED ( Symbol * symbol, int32 one, int32 two )
{
    Namespace * ns = ( Namespace * ) symbol ;
    ns->State &= ~ TREED ;
}

void
Symbol_Namespaces_PrintTraverse ( Symbol * symbol, int32 containingNamespace, int32 indentLevel )
{
    Namespace * ns = ( Namespace * ) symbol ;
    if ( ns->ContainingNamespace == ( Namespace* ) containingNamespace )
    {
        if ( ! ( ns->State & TREED ) )
        {
            ns->State |= TREED ;
            Namespace_PrettyPrint ( ns, 1, indentLevel ) ;
            //byte * name = ns->s_Name ; // debug only
            //byte * cnmame = ns->ContainingNamespace->s_Name ; //debug only
            _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverse, ( int32 ) ns, indentLevel + 1 ) ;
        }
    }
}

void
Symbol_Namespaces_PrintTraverseWithWords ( Symbol * symbol, int32 containingNamespace, int32 indentLevel )
{
    Namespace * ns = ( Namespace * ) symbol ;
    if ( ns->ContainingNamespace == ( Namespace* ) containingNamespace )
    {
        if ( ! ( ns->State & TREED ) )
        {
            ns->State |= TREED ;
            Namespace_PrettyPrint ( ns, 1, indentLevel ) ;
            //byte * name = ns->s_Name ; // debug only
            //byte * cnmame = ns->ContainingNamespace->s_Name ; //debug only
            DLList_Map1 ( ns->Lo_List, ( MapFunction1 ) _DLNode_AsWord_Print, 0 ) ;
            _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverseWithWords, ( int32 ) ns, indentLevel + 1 ) ;
        }
    }
}

void
CfrTil_Namespaces_PrettyPrintTree ( )
{
    PrintStateInfo_SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
    Printf ( ( byte* ) "\nNamespaceTree - All Namespaces : " ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_SetNonTREED, 0, 0 ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverse, ( int32 ) _CfrTil_->Namespaces, 1 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
CfrTil_Namespaces_PrettyPrintTreeWithWords ( )
{
    PrintStateInfo_SetState ( _Q_->psi_PrintStateInfo, PSI_PROMPT, false ) ;
    Printf ( ( byte* ) "%s%s%s%s", c_dd ( "\nNamespaceTree - All Namespaces -" ), " with ", c_ud ( "words" ), " : " ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_SetNonTREED, 0, 0 ) ;
    _Namespace_MapAny_2Args ( ( MapSymbolFunction2 ) Symbol_Namespaces_PrintTraverseWithWords, ( int32 ) _CfrTil_->Namespaces, 1 ) ;
    Printf ( ( byte* ) "\n" ) ;
}

void
_Namespace_Symbol_Print ( Symbol * symbol, int32 printFlag, int32 str )
{
    char buffer [128] ;
    Namespace * ns = ( Namespace * ) symbol ;
    sprintf ( buffer, "%s ", ns->Name ) ;
    if ( printFlag )
    {
        Printf ( ( byte* ) "%s", buffer ) ;
    }
    else strcat ( ( char* ) str, buffer ) ;
}

// prints all the namespaces on the SearchList
// 'using'

byte *
_CfrTil_UsingToString ( )
{
    Buffer * buffer = Buffer_New ( BUFFER_SIZE ) ;  
    byte * b = Buffer_Data ( buffer ) ;
    strcpy ( ( char* ) b, "" ) ;
    _Tree_Map_State_2 ( _CfrTil_->Namespaces->Lo_List, USING, ( MapSymbolFunction2 ) _Namespace_Symbol_Print, 0, ( int32 ) b ) ;
    b = String_New ( ( byte* ) b, TEMPORARY ) ;
    Buffer_SetAsUnused ( buffer ) ; ;
    return b ;
}

void
CfrTil_Using ( )
{
    Printf ( ( byte* ) "\nUsing Namespaces :> " ) ;
    _Tree_Map_State_2 ( _CfrTil_->Namespaces->Lo_List, USING, ( MapSymbolFunction2 ) _Namespace_Symbol_Print, 1, 0 ) ;
    //CfrTil_EmitString ( ( byte* ) "\n" ) ;
    Printf ( ( byte* ) "\n" ) ;
}

/*
void
_CfrTil_NamespaceQualifier ( )
{
    if ( _Context->Finder0->QualifyingNamespace ) _Context->Finder0->FinderFlags |= NAMESPACE_QUALIFER ;
}
 */

// this namespace is will be taken out of the system

void
_Namespace_RemoveFromUsingListAndClear ( Namespace * ns )
{
    if ( ns )
    {
        //_Namespace_SetState ( ns, NOT_USING ) ;
        _Namespace_Clear ( ns ) ;
        if ( ns == _CfrTil_->InNamespace ) _CfrTil_->InNamespace = ( Namespace* ) DLNode_Next ( ( DLNode* ) ns ) ; //DLList_First ( _CfrTil_->Namespaces->Lo_List ) ;
        if ( ns == _Context_->Finder0->QualifyingNamespace ) Finder_SetQualifyingNamespace ( _Context_->Finder0, 0 ) ;
        DLNode_Remove ( ( DLNode* ) ns ) ;
    }
}

void
_CfrTil_RemoveNamespaceFromUsingListAndClear ( byte * name )
{
    _Namespace_RemoveFromUsingListAndClear ( Namespace_Find ( name ) ) ;
}

// RPN : Reversed Polish (logic) Notation : with operator first
//( "aString" - w )

/*
void
CfrTil_NamespaceSealed ( )
{
    _CfrTil_Namespace_InNamespaceGet ( )->Symbol->Category |= SEALED ;
}

void
CfrTil_SealANamespace ( DLNode * node )
{
    Namespace * ns = Namespace_From_DLNode ( node ) ;
    ns->Symbol->Category |= SEALED ;
}

void
CfrTil_SealNamespaces ( )
{
    //_Context->NamespacesFlag = 0 ;

    _CfrTil_ForAllNamespaces ( ( MapFunction2 ) CfrTil_SealANamespace, 0 ) ;
}
 */
