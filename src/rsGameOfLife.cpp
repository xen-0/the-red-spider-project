#include <iostream>
#include <ctime>
#include <cstdlib>
#include "rsTerminal.h"
#include "rsTypes.h"

enum cellState_t {
    STATE_RED,
    STATE_BLUE,
    STATE_DEAD,
    STATE_OTHER //debug value
};

class rsUniverse
{
public:
    unsigned int offsetX;
    unsigned int offsetY;
    unsigned int probBlue;
    unsigned int probRed;
    rsUniverse( const int x, const int y )
    {
        univCurrent = new cellState_t[ x * y ];
        univPrev = new cellState_t[ x * y ];
        boundX = x;
        boundY = y;
        probBlue = 7;
        probRed = 7;
        offsetX = 0;
        offsetY = 0;
    }

    ~rsUniverse()
    {
        delete [] univCurrent;
        delete [] univPrev; 
    }

    void SeedUniverse( int seed )
    {
        srand( seed );
        for( int i = 0; i < boundX * boundY; i++ ) {
            int randNumber = rand() % 100;
            univCurrent[i] = randNumber < probBlue ? STATE_BLUE : randNumber < probBlue + probRed ? STATE_RED : STATE_DEAD;
        }
        
    }

    void NextState()
    {
        cellState_t* temp = univPrev;
        univPrev = univCurrent;
        univCurrent = temp;

        for ( int x = 0 ; x < boundX ; x++ ) {
            for ( int y = 0 ; y < boundY ; y++ ) {
                rsCOORD_t point = { x , y };
                univCurrent[ x * (boundX) + y ] = CalcState( point );
            }
        }
    }

    const void DrawUniverse( rsTerminal * terminal )
    {
        rsCOORD_t cursor;
        for ( int x = 0 ; x < boundX ; x++ ) {
            cursor.y = x + offsetY;
            for (int y = 0 ; y < boundY ; y++ ) {
                cursor.x = y + offsetX;
                terminal->SetCursorPosition( cursor );
                switch ( univCurrent[ x * (boundX) + y] ) {
                    case STATE_DEAD:
                        if ( univPrev[ x * (boundX) + y ] != STATE_DEAD  ) {
                            std::cout<<" ";
                        }
                        break;
                    case STATE_BLUE:
                        terminal->SetTextColour( COLOUR_BLUE | COLOUR_WHITE | COLOUR_GREEN );
                        std::cout<<"O";
                        break;
                    case STATE_RED:
                        terminal->SetTextColour( COLOUR_RED | COLOUR_WHITE );
                        std::cout<<"X";
                        break;
                    default:
                        terminal->SetTextColour( COLOUR_GREEN | COLOUR_WHITE );
                        std::cout<<"Z";
                }
            }
            std::cout<<std::endl;
        }
        //std::cout<<std::endl;
        //terminal->SetToDefault();
    }

    const void DrawBorder ( rsTerminal * terminal )
    {
        //only draw border if a margin has been set
        if ( offsetX == 0 || offsetY == 0 ) return;
        rsCOORD_t point;
        terminal->SetTextColour( COLOUR_BLUE );
        //top and bottom
        for ( int x = 0; x <= boundX; x++ ) {
            point.x = offsetX + x - 1;
            point.y = offsetY - 1;
            terminal->SetCursorPosition(point);
            std::cout<<"-";
            point.y = offsetY + boundY;
            terminal->SetCursorPosition(point);
            std::cout<<"-";
        }
        //sides
        for ( int y = 0; y <= boundY; y++ ) {
            point.y = offsetY + y - 1;
            point.x = offsetX - 1;
            terminal->SetCursorPosition(point);
            std::cout<<"|";
            point.x = offsetX + boundX;
            terminal->SetCursorPosition(point);
            std::cout<<"|";
        }
        //corners
        point.x = offsetX - 1;
        point.y = offsetY - 1;
        terminal->SetCursorPosition(point);
        std::cout<<"/";
        point.x += boundX + 1;
        terminal->SetCursorPosition(point);
        std::cout<<"\\";
        point.y += boundY + 1;
        terminal->SetCursorPosition(point);
        std::cout<<"/";
        point.x -= (boundX + 1);
        terminal->SetCursorPosition(point);
        std::cout<<"\\";
    }
    
protected:
    cellState_t* univCurrent;
    cellState_t* univPrev;
    int boundX;
    int boundY;

    virtual void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) = 0;

    cellState_t CalcState( const rsCOORD_t point )
    {
        int countB = 0;
        int countR = 0;
        cellState_t neighbourhood[3][3];
        GetNeighbourhood(point, neighbourhood);
        for ( int i = 0 ; i < 3 ; i += 2 ) {
            for ( int j = 0; j < 3 ; j++ ) {
                switch ( neighbourhood[i][j] ) {
                    case STATE_BLUE:
                        countB++;
                        break;
                    case STATE_RED:
                        countR++;
                        break;
                }

            }
        }
        if ( neighbourhood[1][0] == STATE_BLUE ) countB++;
        if ( neighbourhood[1][0] == STATE_RED ) countR++;
        if ( neighbourhood[1][2] == STATE_BLUE ) countB++;
        if ( neighbourhood[1][2] == STATE_RED ) countR++;

        switch ( neighbourhood[1][1] ) {
            case STATE_DEAD:
                if ( ( countR + countB ) == 3 ) {
                    return  countB > countR ? STATE_BLUE : STATE_RED;
                }
                return ( ( countR + countB == 6 ) && countR > countB ) ? STATE_RED : STATE_DEAD;
            case STATE_RED:
                return ( (countR + countB > 3) || (countR + countB < 2) ) ? STATE_DEAD : STATE_RED ;
            case STATE_BLUE:
                return ( (countR + countB > 3) || (countR + countB < 2) ) ? STATE_DEAD : STATE_BLUE ;
            default:
                return STATE_OTHER;
        }
    }
};

class rsUniverseTorus: public rsUniverse
{
public:
    rsUniverseTorus( const int x, const int y ) : rsUniverse( x, y ) {};
private:
    void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] )
    {
        for ( int i = 0 ; i < 3 ; i++ ) {
            for ( int j = 0; j < 3 ; j++ ) {
                int x = ( point.x + i - 1 ) % boundX;
                int y = ( point.y + j - 1 ) % boundY;
                if ( x < 0 ) { x += boundX; }
                if ( y < 0 ) { y += boundY; }
                neighbourhood[i][j] = univPrev[ x * (boundX) + y ];
            }
        }
    }
};

int main( int argc, char* argv[] )
{
    rsUniverse * pUniverse;
    rsUniverseTorus uni(66, 66);
    pUniverse = &uni;
    //universe.SeedUniverse( time(0) );
    pUniverse->SeedUniverse( 7823641 );
    pUniverse->offsetX = 1;
    pUniverse->offsetY = 1;
    rsTerminal terminal;
    terminal.ClearTerminal();
    terminal.HideCursor();
    pUniverse->DrawBorder( &terminal );
    for( int i = 0; i < 100; i++ ) {
        pUniverse->DrawUniverse( &terminal );
        pUniverse->NextState();
    }
    std::cout<<"DONE"<<std::endl;
    terminal.ShowCursor();
    terminal.SetToDefault();
    //getchar();
    return 0;
}