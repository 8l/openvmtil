
#include "../includes/cfrtil.h"

#if LINUX

void
_DisplaySignal ( int signal )
{
    if ( signal )
    {
        byte * location = _Context_->Location ;
        //byte * location = (byte*) Context_Location () ;
        switch ( signal )
        {
            case SIGSEGV:
            {
                Printf ( ( byte* ) "\nSIGSEGV : memory access violation - %s", location ) ;
                break ;
            }
            case SIGFPE:
            {
                Printf ( ( byte* ) "\nSIGFPE : arithmetic exception - %s", location ) ;
                break ;
            }
            case SIGILL:
            {
                Printf ( ( byte* ) "\nSIGILL : illegal instruction - %s", location ) ;
                break ;
            }
            case SIGTRAP:
            {
                Printf ( ( byte* ) "\nSIGTRAP : int3/trap - %s", location ) ;
                break ;
            }
            default: break ;
        }
    }
}

void
Linux_SetupSignals ( int startTimes )
{
    struct sigaction signalAction ;
    // from http://www.linuxjournal.com/article/6483
    int32 i, result ;
    Mem_Clear ( ( byte* ) & signalAction, sizeof ( struct sigaction ) ) ;
    signalAction.sa_sigaction = OpenVmTil_SignalAction ;
    signalAction.sa_flags = SA_SIGINFO | SA_RESTART ; // restarts the set handler after being used instead of the default handler
    for ( i = SIGHUP ; i <= _NSIG ; i ++ )
    {
        result = sigaction ( i, &signalAction, NULL ) ;
        if ( result && ( startTimes == 1 ) )
        {
            if ( _Q_ && ( _Q_->Verbosity > 2 ) ) printf ( "\nLinux_SetupSignals : signal number = " INT_FRMT_02 " : result = " INT_FRMT " : This signal can not have a handler.", i, result ) ;
            continue ;
        }
    }
    signal ( SIGWINCH, SIG_IGN ) ; // a fix for a netbeans problem
}

void
Linux_RestoreTerminalAttributes ( )
{
    tcsetattr ( STDIN_FILENO, TCSANOW, &_SavedTerminalAttributes_ ) ;
}

void
Linux_SetInputMode ( struct termios * savedTerminalAttributes )
{
    struct termios terminalAttributes ;
    // Make sure stdin is a terminal. /
    if ( ! isatty ( STDIN_FILENO ) )
    {
        Printf ( ( byte* ) "Not a terminal.\n" ) ;
        exit ( EXIT_FAILURE ) ;
    }

    // Save the terminal attributes so we can restore them later. /
    tcgetattr ( STDIN_FILENO, savedTerminalAttributes ) ;
    atexit ( Linux_RestoreTerminalAttributes ) ;

    tcgetattr ( STDIN_FILENO, &terminalAttributes ) ;

    //terminalAttributes.c_iflag &= ~ ( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
    //        | IGNCR | ICRNL | IXON ) ;
    //terminalAttributes.c_lflag &= ~ ( ICANON | ECHO | ECHONL | ISIG ) ; // | IEXTEN ) ;
    terminalAttributes.c_lflag &= ~ ( ICANON | ECHO | ECHONL ) ; // | ISIG ) ; // | IEXTEN ) ;
    //terminalAttributes.c_cflag &= ~ ( CSIZE | PARENB ) ;
    //terminalAttributes.c_cflag |= CS8 ;
    //terminalAttributes.c_cc [ VMIN ] = 1 ;
    //terminalAttributes.c_cc [ VTIME ] = 0 ;

    tcsetattr ( STDIN_FILENO, TCSANOW, &terminalAttributes ) ;
    //setvbuf ( stdout, NULL, _IONBF, 1 ) ; // ??
}

#if 0

void
Linux_SetInputMode_Reset ( )
{
    struct termios terminalAttributes ;
    tcgetattr ( STDIN_FILENO, &terminalAttributes ) ;
    terminalAttributes.c_iflag &= ~ ( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
        | IGNCR | ICRNL | IXON ) ;
    terminalAttributes.c_lflag &= ~ ( ICANON | ECHO | ECHONL | ISIG | IEXTEN ) ;
    terminalAttributes.c_cflag &= ~ ( CSIZE | PARENB ) ;
    terminalAttributes.c_cflag |= CS8 ;
    terminalAttributes.c_cc [ VMIN ] = 1 ;
    terminalAttributes.c_cc [ VTIME ] = 0 ;

    tcsetattr ( STDIN_FILENO, TCSANOW, &terminalAttributes ) ;
    setvbuf ( stdout, NULL, _IONBF, 1 ) ; // ??
}

void
Linux_SetInputMode_EchoOn ( )
{
    struct termios terminalAttributes ;
    tcgetattr ( STDIN_FILENO, &terminalAttributes ) ;

    terminalAttributes.c_iflag &= ~ ( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
        | IGNCR | ICRNL | IXON ) ;
    terminalAttributes.c_lflag &= ~ ( ICANON | ISIG | IEXTEN ) ;
    terminalAttributes.c_cflag &= ~ ( CSIZE | PARENB ) ;
    terminalAttributes.c_cflag |= CS8 ;
    terminalAttributes.c_cc [ VMIN ] = 1 ;
    terminalAttributes.c_cc [ VTIME ] = 0 ;

    tcsetattr ( STDIN_FILENO, TCSANOW, &terminalAttributes ) ;
    setvbuf ( stdout, NULL, _IONBF, 1 ) ; // ??
}
#endif

void
LinuxInit ( struct termios * savedTerminalAttributes )
{
    Linux_SetInputMode ( savedTerminalAttributes ) ; // nb. save first !! then copy to _Q_ so atexit reset from global _Q_->SavedTerminalAttributes
    Linux_SetupSignals ( _Q_ ? ! _Q_->StartedTimes : 1 ) ;
}

#endif


