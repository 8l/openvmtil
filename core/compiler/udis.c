
#include "../../includes/cfrtil.h"

void
_Udis_PrintInstruction ( ud_t * ud, byte * address, byte * prefix, byte * postfix, byte * debugAddress )
{
    byte buffer [ 128 ] ;
    postfix = GetPostfix ( address, ( char* ) ud_insn_asm ( ud ), postfix, buffer ) ; // buffer is returned as postfix by GetPostfix
    if ( address == debugAddress )
    Printf ( ( byte* ) c_ud ( "%s" UINT_FRMT_0x08 " %-16s %-28s%s\n"), prefix, ( int32 ) ud_insn_off ( ud ), ud_insn_hex ( ud ),
        ud_insn_asm ( ud ), postfix ) ;
    else
    Printf ( ( byte* ) "%s" UINT_FRMT_0x08 " %-16s %-28s%s\n", prefix, ( int32 ) ud_insn_off ( ud ), ud_insn_hex ( ud ),
        ud_insn_asm ( ud ), postfix ) ;
}

int32
_Udis_GetInstructionSize ( ud_t * ud, byte * address )
{
    ud_set_input_buffer ( ud, address, 16 ) ;
    ud_set_pc ( ud, ( int32 ) address ) ;
    int32 isize = ud_disassemble ( ud ) ;
    return isize ;
}

//void (*pud_init) ( ud_t * ) ;

ud_t *
_Udis_Init ( ud_t * ud )
{
#if 1 
    ud_init ( ud ) ;
    ud_set_mode ( ud, 32 ) ;
    ud_set_syntax ( ud, UD_SYN_INTEL ) ;
#else  

    //pud_init = ( void(* ) ( ud_t * ) ) _dlsym ( "libudis86", "ud_init" ) ;
    //pud_init ( ud ) ;
    ( ( void(* ) ( ud_t * ) ) _dlsym ( "ud_init", "/usr/local/lib/libudis86.so" ) )( ud ) ;
    ( ( void (* )( struct ud*, uint8_t ) ) _dlsym ( "ud_set_mode", "/usr/local/lib/libudis86.so" ) ) ( ud, 32 ) ;
    ( ( void (* )( struct ud*, void (* )( struct ud* ) ) ) _dlsym ( "ud_set_syntax", "/usr/local/lib/libudis86.so" ) ) ( ud, UD_SYN_INTEL ) ;
#endif   
    return ud ;
}

int32
_Udis_OneInstruction ( ud_t * ud, byte * address, byte * prefix, byte * postfix )
{
    if ( address )
    {
        int32 isize ;
        ud_set_input_buffer ( ud, address, 16 ) ;
        ud_set_pc ( ud, ( int32 ) address ) ;
        isize = ud_disassemble ( ud ) ;
        SetState ( _Q_->psi_PrintStateInfo, PSI_NEWLINE, true ) ;
        _Udis_PrintInstruction ( ud, address, prefix, postfix, _CfrTil_->Debugger0->DebugAddress ) ;
        return isize ;
    }
    return 0 ;
}

void
_Udis_Disassemble ( ud_t *ud, byte* address, int32 number, int32 cflag, byte * debugAddress )
{
    if ( address )
    {
        char * iasm ;
        int32 isize = 0 ;
        if ( number > 1024 ) number = 1024 ;
        ud_set_input_buffer ( ud, ( byte* ) address, number ) ;
        ud_set_pc ( ud, ( int32 ) address ) ;
        while ( ( number -= isize ) > 0 )
        {
            isize = ud_disassemble ( ud ) ;
            iasm = ( char* ) ud_insn_asm ( ud ) ;
            address = ( byte* ) ( int32 ) ud_insn_off ( ud ) ;
            if ( ! ( stricmp ( ( byte* ) "invalid", ( byte* ) iasm ) ) )
            {
                if ( address == debugAddress )
                    Printf ( ( byte* ) c_ud ("%s" UINT_FRMT_0x08 " %-16s %-28s%s"), "", ( int32 ) ud_insn_off ( ud ), ud_insn_hex ( ud ), ud_insn_asm ( ud ), "" ) ;
                else Printf ( ( byte* ) "%s" UINT_FRMT_0x08 " %-16s %-28s%s", "", ( int32 ) ud_insn_off ( ud ), ud_insn_hex ( ud ), ud_insn_asm ( ud ), "" ) ;
                break ;
            }
            _Udis_PrintInstruction ( ud, address, ( byte* ) "", ( byte* ) "", debugAddress ) ;
            if ( ! ( stricmp ( ( byte* ) "ret", ( byte* ) iasm ) ) && cflag ) break ; //isize = 1024 ; // cause return after next print insn
        }
    }
    Printf ( ( byte* ) "\n" ) ;
}

