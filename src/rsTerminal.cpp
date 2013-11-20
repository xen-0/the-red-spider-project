#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdio>
#endif
#include "rsTerminal.h"

rsTerminal::rsTerminal()
{
   #ifdef _WIN32
   hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
   GetConsoleScreenBufferInfo( hStdOut, &defaultBufferInfo );
   #else
   #endif 
}

rsTerminal::~rsTerminal()
{
    #ifdef _WIN32
    CloseHandle( hStdOut );
    #endif
}

void rsTerminal::ClearTerminal()
{
    #ifdef _WIN32
    system( "cls" ); //i'm lazy
    #else
    printf( "\033[2J" );
    #endif
}

void rsTerminal::SetCursorPosition( rsCOORD_t pos )
{
    #ifdef _WIN32
    COORD position = { pos.x, pos.y };
    SetConsoleCursorPosition( hStdOut, position );
    #else
    //POSIX terminal is indexed from 1
    //and oriented other way
    printf( "\033[%d;%dH", pos.y + 1, pos.x + 1);
    #endif
}

void rsTerminal::SetTextColour( int terminalColour )
{
    #ifdef _WIN32
    if ( COLOUR_DEFAULT == terminalColour ) {
        SetConsoleTextAttribute( hStdOut, defaultBufferInfo.wAttributes & 7 );
        return;
    }
    int winColour = 0;
    if ( COLOUR_BLUE & terminalColour ) { winColour |= FOREGROUND_BLUE; }
    if ( COLOUR_GREEN & terminalColour ) { winColour |= FOREGROUND_GREEN; }
    if ( COLOUR_RED & terminalColour ) { winColour |= FOREGROUND_RED; }
    if ( COLOUR_WHITE & terminalColour ) { winColour |= FOREGROUND_INTENSITY; } 
    SetConsoleTextAttribute( hStdOut, winColour );
    #else
    if ( COLOUR_DEFAULT == terminalColour ) {
        printf( "\033[39m" );
        return;
    }
    int colourOffset = 0;
    if ( COLOUR_RED & terminalColour ) { colourOffset |= 1; }
    if ( COLOUR_GREEN & terminalColour ) { colourOffset |= 2; }
    if ( COLOUR_BLUE & terminalColour ) { colourOffset |= 4; }
    printf( "\033[3%dm", colourOffset);
    #endif
}

void rsTerminal::SetToDefault()
{
    #ifdef _WIN32
    SetConsoleTextAttribute( hStdOut, defaultBufferInfo.wAttributes );
    #else
    printf( "\033[39m\033[49m" );
    #endif
}

void rsTerminal::HideCursor()
{
    #ifdef _WIN32
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = false;
    SetConsoleCursorInfo( hStdOut, &cursor );
    #else
    printf( "\033[?25l" );
    #endif
}

void rsTerminal::ShowCursor()
{
    #ifdef _WIN32
    CONSOLE_CURSOR_INFO cursor;
    cursor.bVisible = true;
    SetConsoleCursorInfo( hStdOut, &cursor );
    #else
    printf( "\033[?25h" );
    #endif
}

