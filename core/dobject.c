
#include "../includes/cfrtil.h"

#if 0 // i like this idea -- use this!? -- Do not delete! -- from sl5.c

Object *
obMake ( enum otype type32, int count, ... )
{
    Object *ob, *arg ;
    va_list args ;
    int i ;
    va_start ( args, count ) ;
    ob = ( Object* ) _Allocate ( sizeof (Object ) + ( count - 1 ) * sizeof (Object * ), Pnba_SL5 ) ;

    ob->type = type32 ;
    for ( i = 0 ; i < count ; i ++ ) ob->p[i] = va_arg ( args, Object * ) ;
    va_end ( args ) ;
    return ob ;
}
#endif

// remember : Word = DynamicObject = DObject = Namespace

DObject *
_DObject_FindSlot_BottomUp ( DObject * dobject, byte * name )
{
    Word * word ;
    do
    {
        if ( ( word = Word_FindInOneNamespace ( dobject, name ) ) ) break ;
        dobject = dobject->ContainingNamespace ;
    }
    while ( dobject ) ;
    return ( DObject * ) word ;
}

DObject *
_DObject_SetSlot ( DObject * dobject, byte * name, int32 value )
{
    DObject * ndobject = _DObject_FindSlot_BottomUp ( dobject, name ) ;
    if ( ! ndobject ) return _DObject_NewSlot ( dobject, name, value ) ;
    else return ndobject ;
}

void
DObject_SubObjectInit ( DObject * dobject, Word * proto )
{
    dobject->CType |= proto->CType ;
    dobject->Slots = proto->Slots ;
    proto->State |= USING ;
}

