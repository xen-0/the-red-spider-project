#include <iostream>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
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
            univCurrent[i] = randNumber < probBlue ? STATE_BLUE :
                randNumber < probBlue + probRed ? STATE_RED : STATE_DEAD;
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
                univCurrent[ x * boundY + y ] = CalcState( point );
            }
        }
    }

    void DrawUniverse( rsTerminal * terminal ) const
    {
        rsCOORD_t cursor;
        for ( int x = 0 ; x < boundX ; x++ ) {
            cursor.x = x + offsetX;
            for (int y = 0 ; y < boundY ; y++ ) {
                cursor.y = y + offsetY;
                //always draw the top left cell, to prevent screen scrolling on Windows
                if ( ( x == 0 && y == 0 ) || univCurrent[ x * boundY + y ] != univPrev[ x * boundY + y ] ) {
                    switch ( univCurrent[ x * boundY + y ] ) {
                        case STATE_DEAD:
                            terminal->SetCursorPosition( cursor );
                            std::cout<<" ";
                            break;
                        case STATE_BLUE:
                            terminal->SetTextColour( COLOUR_BLUE | COLOUR_WHITE | COLOUR_GREEN );
                            terminal->SetCursorPosition( cursor );
                            std::cout<<"O";
                            break;
                        case STATE_RED:
                            terminal->SetTextColour( COLOUR_RED | COLOUR_WHITE );
                            terminal->SetCursorPosition( cursor );
                            std::cout<<"X";
                            break;
                        default:
                            terminal->SetTextColour( COLOUR_GREEN | COLOUR_WHITE );
                            terminal->SetCursorPosition( cursor );
                            std::cout<<"Z";
                    }
                }
            }
            std::cout << std::flush;
        }
    }

    void DrawBorder ( rsTerminal * terminal ) const
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

    virtual void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) const = 0;

    cellState_t CalcState( const rsCOORD_t point )
    {
        int countB = 0;
        int countR = 0;
        cellState_t neighbourhood[3][3];
        GetNeighbourhood( point, neighbourhood );
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
    void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) const
    {
        for ( int i = 0 ; i < 3 ; i++ ) {
            for ( int j = 0; j < 3 ; j++ ) {
                int x = ( point.x + i - 1 ) % boundX;
                int y = ( point.y + j - 1 ) % boundY;
                if ( x < 0 ) { x += boundX; }
                if ( y < 0 ) { y += boundY; }
                neighbourhood[i][j] = univPrev[ x * boundY + y ];
            }
        }
    }
};

class rsUniverseKlein: public rsUniverse
{
public:
    rsUniverseKlein( const int x, const int y ) : rsUniverse( x, y ) {};
private:
    void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) const
    {
        for ( int i = 0; i < 3; i++ ) {
            for ( int j = 0; j < 3; j++ ) {
                int x = ( point.x + i - 1 );
                int y;
                if ( -1 == x || boundX == x ) {
                    y = ( boundY - point.y - j ) % boundY;
                } else {
                    y = ( point.y + j + - 1 ) % boundY;
                }
                x %= boundX;
                if ( x < 0 ) { x += boundX; }
                if ( y < 0 ) { y += boundY; }
                neighbourhood[i][j] = univPrev[ x * boundY + y];
            }
        }
    }
};

class rsUniverseSphere: public rsUniverse
{
public:
        rsUniverseSphere( const int x ) : rsUniverse( x, x ) {};
private:
    void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) const
    {
        enum univPolePositions_t { NORMAL, NORTH_0, NORTH_X, NORTH_Y, SOUTH_0, SOUTH_X, SOUTH_Y };
        univPolePositions_t positions = NORMAL;
        if ( point.x == 0 && point.y == 0 ) positions = NORTH_0;
        if ( point.x == 0 && point.y == 1 ) positions = NORTH_Y;
        if ( point.x == 1 && point.y == 0 ) positions = NORTH_X;
        if ( point.x == boundX - 1 && point.y == boundY - 1 ) positions = SOUTH_0;
        if ( point.x == boundX - 1 && point.y == boundY - 2 ) positions = SOUTH_Y;
        if ( point.x == boundX - 2 && point.y == boundY - 1 ) positions = SOUTH_X;

        switch ( positions ) {
        case NORMAL:
            for ( int i = 0; i < 3; i++ ){
                for ( int j = 0; j < 3; j++ ) {
                    int x = point.x + i - 1;
                    int y = point.y + j - 1;
                    if ( x < 0 ) {
                        x = y;
                        y = 0;
                    }
                    if ( y < 0 ) {
                        y = x;
                        x = 0;
                    }
                    if ( x == boundX ) {
                        x = y;
                        y = boundY - 1;
                    }
                    if ( y == boundY ) {
                        y = x;
                        x = boundX - 1;
                    }
                    neighbourhood[i][j] = univPrev[ x * boundY + y ];
                }
            }
            break;
        case NORTH_0:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[ i + 1 ][ j + 1 ] = univPrev[ i * boundY + j ];
                }
            }
            break;
        case NORTH_Y:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 3; j++ ) {
                    neighbourhood[ i + 1 ][j] = univPrev[ i * boundY + j ];
                }
            }
            neighbourhood[0][2] = univPrev[ 2 * boundX + 0 ];
            break;
        case NORTH_X:
            for ( int i = 0; i < 3; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[i][ j + 1 ] = univPrev[ i * boundY+ j ];
                }
            }
            neighbourhood[2][0] = univPrev[2]; //this is wrong?
            break;
        case SOUTH_0:
            for ( int i = 0; i < 2; i++ ) {
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[i][j] = univPrev[ (boundX - 2 + i) * boundY + (boundY - 2 + j) ];
                }
            }
            break;
        case SOUTH_Y:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 3; j++ ) {
                    neighbourhood[i][j] = univPrev[ (boundX - 2 + i) * boundY + (boundY - 3 + j) ];
                }
            }
            neighbourhood[2][0] = univPrev[ (boundX - 3) * boundY + (boundY - 1) ];
            break;
        case SOUTH_X:
            for ( int i = 0; i < 3; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[i][j] = univPrev[ (boundX - 3 + i) * boundY + (boundY - 2 + j) ];
                }
            }
            neighbourhood[0][2] = univPrev[ (boundX - 1) * boundY + (boundY - 3) ];
            break;
        }
        return;
    }
};

class rsUniverseProj: public rsUniverse
{
public:
    rsUniverseProj( const int x, const int y ) : rsUniverse( x, y ) {};
private:
    void GetNeighbourhood( const rsCOORD_t point, cellState_t neighbourhood[3][3] ) const
    {
        enum univCorner_t { NONE, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };
        univCorner_t corner = NONE;
        if ( point.x == 0 && point.y == 0 ) corner = TOP_LEFT;
        if ( point.x == 0 && point.y == boundY - 1 ) corner = TOP_RIGHT;
        if ( point.x == boundX - 1 && point.y == 0 ) corner = BOTTOM_LEFT;
        if ( point.x == boundX - 1 && point.y == boundY - 1 ) corner = BOTTOM_RIGHT;
        switch ( corner ) {
        case NONE:
            for ( int i = 0; i < 3; i++ ){
                for ( int j = 0; j < 3; j++ ) {
                    int x = point.x + i - 1;
                    int y = point.y + j - 1;
                    if ( -1 == x || boundX == x ) {
                        x = ( -1 == x ? boundX - 1 : 0 );
                        y = boundY - 1 - y;
                    }
                    if ( -1 == y || boundY == y ) {
                        y = ( -1 == y ? boundY - 1 : 0 );
                        x = boundX - 1 - x;
                    }
                    neighbourhood[i][j] = univPrev[ x * boundY + y ];
                }
            }
            break;
        case TOP_LEFT:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[ i + 1 ][ j + 1 ] = univPrev[ i * boundY + j ];
                }
            }
            neighbourhood[0][1] = univPrev[ (boundX - 1) * boundY + (boundY - 1) ];
            neighbourhood[0][2] = univPrev[ (boundX - 1) * boundY + (boundY - 2) ];
            neighbourhood[2][0] = univPrev[ (boundX - 2) * boundY + (boundY - 1) ];
            break;
        case TOP_RIGHT:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[ i + 1 ][j] = univPrev[ i * boundY + boundY - 2 + j ];
                }
            }
            neighbourhood[0][0] = univPrev[ (boundX - 1) * boundY + 1 ];
            neighbourhood[0][1] = univPrev[ (boundX - 1) * boundY + 0 ];
            neighbourhood[2][2] = univPrev[ (boundX - 2) * boundY + 0 ];
            break;
        case BOTTOM_LEFT:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[i][j] = univPrev[ (boundX - 2 + i) * boundY + j ];
                }
            }
            neighbourhood[0][0] = univPrev[ 1 * boundY + boundY - 1 ];
            neighbourhood[0][1] = univPrev[ 0 * boundY + boundY - 1 ];
            neighbourhood[2][2] = univPrev[ 0 * boundY + boundY - 2 ];
            break;
        case BOTTOM_RIGHT:
            for ( int i = 0; i < 2; i++ ){
                for ( int j = 0; j < 2; j++ ) {
                    neighbourhood[i][j] = univPrev[ (boundX - 2 + i) * boundY + boundY - 2 + j ];
                }
            }
            neighbourhood[2][0] = univPrev[1];
            neighbourhood[2][1] = univPrev[0];
            neighbourhood[0][2] = univPrev[1 * boundY];
            break;
        }
        return;
    }
};

void Interrupt( bool* stop )
{
    getchar();
    *stop = true;
    return;
}

int main( int argc, char* argv[] )
{
    rsUniverse* pUniverse;
    int sizeX = 60;
    int sizeY = 30;
    int period = 60;
    rsUniverseTorus uni( sizeX, sizeY );
    pUniverse = &uni;
    pUniverse->SeedUniverse( 7823641 );
    pUniverse->offsetX = 5;
    pUniverse->offsetY = 2;
    rsTerminal terminal;
    terminal.ClearTerminal();
    terminal.HideCursor();
    bool stop = false;
    std::thread interruptThread( Interrupt, &stop );
    pUniverse->DrawBorder( &terminal );
    //for( int i = 0; i < 300; i++ ) {
    while ( !stop ) {
        auto startTime = std::chrono::steady_clock::now();
        pUniverse->DrawUniverse( &terminal );
        pUniverse->NextState();
        auto endTime = std::chrono::steady_clock::now();
        int timeDiff = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
        int delay = period > timeDiff ? period - timeDiff : period;
        std::this_thread::sleep_for( std::chrono::milliseconds( delay ) );
    }
    interruptThread.join();
    terminal.ShowCursor();
    int posX = 0;
    int posY = uni.offsetY + sizeY + 2;
    rsCOORD_t cursor = { posX, posY };
    terminal.SetCursorPosition( cursor );
    terminal.SetToDefault();
    return 0;
}
