
#include "../includes/cfrtil.h"

void
CfrTil_IncludeFile ( )
{
    byte * filename = ( byte* ) _DataStack_Pop ( ) ;
    _CfrTil_ContextNew_IncludeFile ( filename ) ;
}

int32
_File_Size ( FILE * file )
{
    fseek ( file, 0, SEEK_END ) ;
    int32 size = ftell ( file ) ;
    rewind ( file ) ;
    return size ;
}

int32
_File_Exists ( byte * fname )
{
    struct stat sbuf ;
    if ( ! stat (fname, &sbuf )) return true ; // nb. this logic is the reverse of the system call
    else return false ;
}

byte *
_File_ReadToString_ ( FILE * file )
{
    int32 size, result ;

    size = _File_Size ( file ) ;
    byte * fstr = _Mem_Allocate ( size + 1, SESSION ) ;
    result = fread ( fstr, 1, size, file ) ;
    //fclose ( file ) ;
    if ( result != size ) return 0 ;
    return fstr ;
}

byte *
_File_ReadToString ( byte * name )
{
    FILE * file ;
    int32 size, result ;

    file = fopen ( name, "rb" ) ;
#if 0    
    if ( file == NULL ) return 0 ;
    size = _File_Size ( file ) ;
    byte * fstr = _Mem_Allocate ( size + 1, SESSION ) ;
    result = fread ( fstr, 1, size, file ) ;
    if ( result != size ) return 0 ;
    return fstr ;
#else
    return _File_ReadToString_ ( file ) ;
#endif    
}

void
File_Open ( )
{
    byte * filename = ( byte* ) _DataStack_Pop ( ) ;
    FILE * file = fopen ( ( char* ) filename, "r" ) ;
    if ( file == NULL )
    {
        perror ( "\nFile_Open error : " ) ;
        CfrTil_Quit ( ) ;
    }
    else _DataStack_Push ( ( int32 ) file ) ;
}

void
File_Close ( )
{
    FILE * file = ( FILE* ) _DataStack_Pop ( ) ;
    fclose ( file ) ;
}

void
File_Read ( )
{
    int32 size = _DataStack_Pop ( ) ;
    byte * ptr = ( byte * ) _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    int32 result = fread ( ptr, size, 1, file ) ;
    _DataStack_Push ( result ) ;
    if ( result != size )
    {
        if ( ! feof ( file ) )
        {
            if ( ferror ( file ) )
            {
                perror ( "\nFile_Read error : " ) ;
                CfrTil_Quit ( ) ;
            }
        }
    }
}

void
File_Write ( )
{
    int32 size = _DataStack_Pop ( ) ;
    byte * ptr = ( byte * ) _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    fwrite ( ptr, size, 1, file ) ;
}

void
File_Seek ( )
{
    int32 offset = _DataStack_Pop ( ) ;
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    fseek ( file, offset, SEEK_SET ) ;
}

void
File_Tell ( )
{
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    int32 result = ftell ( file ) ;
    _DataStack_Push ( result ) ;
}

void
File_Size ( )
{
    FILE * file = ( FILE * ) _DataStack_Pop ( ) ;
    _DataStack_Push ( _File_Size ( file ) ) ;
}

void
File_Exists ()
{
    byte * name = ( byte* ) _DataStack_Pop ( ) ;
    _DataStack_Push ( _File_Exists ( name ) ) ;
}