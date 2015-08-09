
#include "../includes/cfrtil.h"
#if 0
AtomStringNode *
_DLList_Atom_Intern ( DLList * dllist, byte * name )
{
    AtomStringNode * asn = ( AtomStringNode * ) Mem_Allocate ( sizeof ( AtomStringNode ), ATOM_MEMORY, 0 ) ;
    name = String_New ( name, ATOM_MEMORY ) ;
    _Symbol_Init ( ( Symbol* ) asn, name ) ;
    asn->S_Type = STRING ;
    DLList_AddNodeToTail ( dllist, ( DLNode* ) asn ) ;
    return asn ;
}

byte *
_DLList_Atom_Find ( DLList * dllist, register byte * name )
{
    register byte * lname ;
    AtomStringNode * asn ;
    register DLNode * tnode ;
    if ( name && name [0] )
    {
        for ( tnode = DLList_First ( dllist ) ; tnode ; tnode = DLNode_Next ( tnode ) ) // index = DLNode_NextNode ( &_Q->AtomList, (DLNode *) index ) )
        {
            asn = ( AtomStringNode* ) tnode ;
            lname = asn->S_Name ;
            if ( lname && ( string_Equal ( lname, name ) ) )
            {
                return lname ;
            }
        }
    }
    return 0 ;
}

AtomStringNode *
_Atom_Intern ( byte * name )
{
    return _DLList_Atom_Intern ( _Q_->AtomList, name ) ;
}

byte *
_Atom_Find ( byte * name )
{
    return _DLList_Atom_Find ( _Q_->AtomList, name ) ;
}

byte *
_DLList_Atomize ( DLList * dllist, byte * istring )
{
    AtomStringNode * asn ;
    byte * atom ;
    if ( istring ) //&& strcmp ( ( char* ) istring, "" ) ) // don't add blank lines to history
    {
        atom = _DLList_Atom_Find ( dllist, istring ) ;
        if ( ! atom )
        {
            asn = _DLList_Atom_Intern ( dllist, istring ) ;
            return asn->S_Name ;
        }
        else return atom ;
    }
    return 0 ;
}

byte *
_OpenVmTil_Atomize ( byte * istring )
{
    return _DLList_Atomize ( _Q_->AtomList, istring ) ;
}

/*
void
OpenVmTil_Atom ( )
{
    byte * string = ( byte* ) _DataStack_Pop ( ) ;
    _OpenVmTil_Atom ( string ) ;
}
 */
#endif