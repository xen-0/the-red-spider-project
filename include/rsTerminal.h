#ifndef RS_TERMINAL_H
#define RS_TERMINAL_H
#include "rsTypes.h"
#ifdef _WIN32
#include <Windows.h>
#endif


enum terminalColours_t {
    COLOUR_DEFAULT = 0,
    COLOUR_BLUE = 1,
    COLOUR_GREEN = 2,
    COLOUR_RED = 4,
    COLOUR_WHITE = 8
};

class rsTerminal
{
    public:
        rsTerminal();
        ~rsTerminal();
        void ClearTerminal();
        void SetCursorPosition( rsCOORD_t pos );
        void SetTextColour( int terminalColour );
        //void PrintText( const char * text );
        void SetToDefault();
        void HideCursor();
        void ShowCursor();
    private:
        #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO defaultBufferInfo;
        HANDLE hStdOut;
        #endif
         
};

#endif
