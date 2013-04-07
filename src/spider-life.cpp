// Copyright 2013 Charles Mita
// licensed under the Red Spider Project license
// See License.txt that shipped with your copy of this software for details

/*
TODO:

IMPORTANT:
Need to port functionality to other platforms - currently Windows specific

I'd like to experiment with the rule set for Red vs Blue interactions.

Perhaps create some seeding patterns for creating the initial state
*/

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <process.h>
#include <ctime>

using namespace std;

int lengthx=66; //vertical
int lengthy=66; //horizontal
int redseed = 7;
int blueseed = 7;
int margin = 2;
bool stop = false;
int delay = 0;
char spacetype = 'T';
char* filename;

int spawning(int countR, int countB)
{
    if((countR + countB) ==3){
        return(countB>countR ? 1 : -1);
    }
    return ((((countR + countB) == 6) && countR>countB) ? -1 : 0);
}

void calcneighbourhood_torus(int i, int j, int** neighbourhood, int** universe)
{
    for(int k=0;k<3;k++){
        for(int l=0;l<3;l++){
            int x = (i + k - 1)%lengthx;
            int y = (j + l - 1)%lengthy;
            if (x < 0) { x +=lengthx; }
            if (y < 0) { y +=lengthy; }
            neighbourhood[k][l] = universe[x][y];
        }
    }
}

void calcneighbourhood_klein(int i,int j,int** neighbourhood, int** universe)
{
    for(int k=0;k<3;k++){
        for(int l=0;l<3;l++){
            int x = (i + k-1);
            int y;
            if(-1==x || lengthx==x){
                y=(lengthy-j-l)%lengthy;
            }
            else{
                y = (j + l-1)%lengthy;
            }
            x%=lengthx;
            if (x < 0) { x +=lengthx; }
            if (y < 0) { y +=lengthy; }
            neighbourhood[k][l] = universe[x][y];
        }
    }
}

void calcneighbourhood_proj(int i,int j, int** neighbourhood, int** universe)
{
    int flag = 0;
    flag |= (i==0 && j==0);
    flag |= (i==0 && j==lengthy-1) <<1;
    flag |= (i==lengthx-1 && j==0) <<2;
    flag |= (i==lengthx-1 && j==lengthy-1) <<3;
    switch(flag){
      case 0:
        for(int k=0;k<3;k++){
            for(int l=0;l<3;l++){
                int x = (i+k-1);
                int y = (j+l-1);
                if(-1==x || lengthx==x){
                    x = (-1==x ? lengthx-1 : 0);
                    y=lengthy-1-y;
                }
                if(-1==y || lengthy==y){
                    y = (-1==y ? lengthy-1 : 0);
                    x=lengthx-1-x;
                }
                neighbourhood[k][l] = universe[x][y];
            }
        }
        return;
      case 1:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k+1][l+1] = universe[k][l];
            }
        }
        neighbourhood[0][1] = universe[lengthx-1][lengthy-1];
        neighbourhood[0][2] = universe[lengthx-1][lengthy-2];
        neighbourhood[2][0] = universe[lengthx-2][lengthy-1];
        return;
      case 2:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k+1][l]=universe[k][lengthy-2+l];
            }
        }
        neighbourhood[0][0] = universe[lengthx-1][1];
        neighbourhood[0][1] = universe[lengthx-1][0];
        neighbourhood[2][2] = universe[lengthx-2][0];
        return;
      case 4:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k][l+1] = universe[lengthx-2+k][l];
            }
        }
        neighbourhood[0][0] = universe[1][lengthy-1];
        neighbourhood[0][1] = universe[0][lengthy-1];
        neighbourhood[2][2] = universe[0][lengthy-2];
        return;
      case 8:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k][l]=universe[lengthx-2+k][lengthy-2+l];
            }
        }
        neighbourhood[2][0] = universe[0][1];
        neighbourhood[2][1] = universe[0][0];
        neighbourhood[0][2] = universe[1][0];
        return;
      default:
        return;
    }
}

void calcneighbourhood_sphere(int i, int j, int** neighbourhood, int** universe)
{
    //this can only possibly make sense if lengthx==lengthy
    int flag = 0;
    flag |= (i==0 && j==0);
    flag |= (i==0 && j==1) << 1;
    flag |= (i==1 && j==0) << 2;
    flag |= (i==lengthx-1 && j==lengthy-1) << 3;
    flag |= (i==lengthx-1 && j==lengthy-2) << 4;
    flag |= (i==lengthx-2 && j==lengthy-1) << 5;
    
    switch(flag){
      case 0:
        for(int k=0;k<3;k++){
            for(int l=0;l<3;l++){
                int x = (i + k - 1);
                int y = (j + l - 1);
                if(x<0){
                    x=y;
                    y=0;
                }
                if(y<0){
                    y=x;
                    x=0;
                }
                if(x==lengthx){
                    x=y;
                    y=lengthy-1;
                }
                if(y==lengthy){
                    y=x;
                    x=lengthx-1;
                }
                neighbourhood[k][l] = universe[x][y];
            }
        }
        return;
      case 1:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k+1][l+1] = universe[k][l];
            }
        }
        return;
      case 2:
        for(int k=0;k<2;k++){
            for(int l=0;l<3;l++){
                neighbourhood[k+1][l]=universe[k][l];
             }
         }
        neighbourhood[0][2]=universe[2][0];
        return;;
      case 4:
        for(int k=0;k<3;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k][l+1]=universe[k][l];
            }
        }
        return;
      case 8:
        for(int k=0;k<2;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k][l] = universe[lengthx-2+k][lengthy-2+l];
            }
        }
        return;
      case 16:
         for(int k=0;k<2;k++){
             for(int l=0;l<3;l++){
                neighbourhood[k][l] = universe[lengthx-2+k][lengthy-3+l];
             }
             neighbourhood[2][0] = universe[lengthx-3][lengthy-1];
         }
         return;
      case 32:
        for(int k=0;k<3;k++){
            for(int l=0;l<2;l++){
                neighbourhood[k][l]=universe[lengthx-3+k][lengthy-2+l];
            }
            neighbourhood[0][2] = universe[lengthx-1][lengthy-3];
        }
        return;
      default:
        for(int k=0;k<3;k++){
            for(int l=0;l<3;l++){
                neighbourhood[k][l] = 2;
            }
        }
    }
}

int nextcolour(int** neighbourhood)
{
    int countB = 0;
    int countR = 0;
    for (int i = 0; i < 3; i += 2){
        for (int j = 0; j < 3; j++){
            if (neighbourhood[i][j]==1) { countB++; }
            else { if (neighbourhood[i][j] == -1) { countR++; } }
        }
    }
    for (int i = 0; i < 3; i += 2) 	{
        if (neighbourhood[1][i] == 1) { countB++; }
        else {
            if (neighbourhood[1][i] == -1) { countR++; }
        }
    }
    switch (neighbourhood[1][1]) {
      case 0:
        return spawning(countR,countB);
      case 1:
        return (( (countR + countB > 3) || (countR + countB < 2) ) ? 0 : 1);
      case -1:
        return (( (countR + countB > 3) || (countR + countB < 2) ) ? 0 : -1);
      default:
        return 2;
	}
}

void nextgen(int** nextuniverse, int** universe)
{
    int ** neighbourhood = new int*[3];
    for(int i=0;i<3;i++){
        neighbourhood[i]=new int[3];
    }
    for(int i=0; i<lengthx; i++){
        for(int j=0; j<lengthy; j++){
            switch(spacetype){
              case 'K':
                calcneighbourhood_klein(i,j,neighbourhood,universe);
                break;
              case 'T':
                calcneighbourhood_torus(i,j,neighbourhood,universe);
                break;
              case 'S':
                calcneighbourhood_sphere(i,j,neighbourhood,universe);
                break;
              case 'P':
                calcneighbourhood_proj(i,j,neighbourhood,universe);
            }
            nextuniverse[i][j] = nextcolour(neighbourhood);
        }
    }
    for(int i=0;i<3;i++){
        delete [] neighbourhood[i];
    }
    delete [] neighbourhood;
}

void clearscreen()
{
    //Windows specific...
    system("cls");
}

void seeduniverse(int** universe)
{
    //simple seeding
    srand(time(0));
    for(int i=0;i<lengthx;i++){
        for(int j=0;j<lengthy;j++){
            int seed = rand()%100;
            if(seed<blueseed){
                universe[i][j]=1;
            }
            else{
                if(seed<redseed+blueseed){
                    universe[i][j]=-1;
                }
            }
        }
    }
}

bool filluniverse(int ** universe)
{
    ifstream inputstate;
    if (filename==0){
        seeduniverse(universe);
        return true;
    }
    inputstate.open(filename, ifstream::in);
    string row;
    int a = 0;
    int b=0;
    if(inputstate.is_open() && a<lengthx){
        while(getline(inputstate,row)){
            istringstream ss(row);
            char cell;
            b=0;
            while(ss.get(cell)){
                if (b>=lengthy) break;
                switch(cell)	{
                  case '1':
                  case 'B':
                  case 'b':
                  case 'O':
                  case 'o':
                      universe[a][b] = 1;
                      break;
                  case '2':
                  case 'X':
                  case 'x':
                  case 'R':
                  case 'r':
                      universe[a][b] = -1;
                      break;
                  default:
                      universe[a][b]=0;
                      break;            
                }
                b++;
            }
            a++;
        }
        inputstate.close();
        return true;
    }
    else{
        //seeduniverse(universe);
        printf("Could not open file: %s \n",filename);
        return false;
    }
}

void consolecolour(int state)
{
    switch(state)
    {
      case 0:
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0x1 );
        break;
      case 1:
          SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0xB );
          break;
      case -1:
          SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0xC);
          break;
      default:
          SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0xA);
          break;
    }
}

void coords(int x, int y)
{
    COORD position = {x,y};
    SetConsoleCursorPosition( GetStdHandle( STD_OUTPUT_HANDLE ), position ) ;
}

void rupt(void*)
{
    getchar();
    stop=true;
}

void drawborder()
{
    SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0x1 );
    //top and bottom
    for(int i=0;i<=lengthy;i++)
    {
        coords(margin-1+i,margin-1);
        cout<<"-";
        coords(margin-1+i,lengthx+margin);
        cout<<"-";
    }
    //sides
    for(int i=0;i<=lengthx;i++)
    {
        coords(margin-1,margin+i-1);
        cout<<"|";
        coords(lengthy+margin,margin+i-1);
        cout<<"|";
    }
    coords(margin-1,margin-1);
    cout<<"/";
    coords(lengthy+margin,lengthx+margin);
    cout<<"/";
    coords(margin-1,lengthx+margin);
    cout<< "\\" ;
    coords(lengthy+margin,margin-1);
    cout<<"\\";
}

void helpinfo(bool argsonly)
{
    char** arguments = new char*[5];
    arguments[0] = (char*)"-X [n] -Y [n]: Specify array dimensions\n";
    arguments[1] = (char*)"-T, -S, -K, -P: Specifiy topology (toroidal, spherical, Klein bottle, projective plane)\n";
    arguments[2] = (char*)"-F [filename]: File name specifying the initial state\n";
    arguments[3] = (char*)"-A: Show this list again\n";
    arguments[4] = (char*)"-H: Show help\n";

    char copyright[] = "Copyright Charles Mita 2013.\n"
                      "Part of the Red Spider Project, licensed under the Red Spider Project License.\n"
                      "See License.txt that shipped with your copy of the software for details.\n";

    char preface[] = "Game of Spiders is an implementation of Conway's Game of Life and a variant, Highlife, using two distinct species.\n"
                    "The starting arrangement can be specified using a text file that stores the initial state of the array.\n"
                    "'X' denotes that a Highlife species lives in that cell.\n'O' denotes that a Conway species lives in that cell\n"
                    "A period, '.' denotes that nothing lives in that cell.\n"
                    "Cells not specified are left as empty/dead.\n"
                    "The array is truncated or extended with empty cells to match the specified size.\n";
    if (!argsonly) printf("\n\n%s\n",preface);
    for(int i=0;i<5;i++){
        printf("%s",arguments[i]);
        //delete [] arguments[i];
    }
    delete [] arguments;
    if(!argsonly) printf("\n%s\n",copyright);
    return;
}

int main(int argc, char* argv[])
{
    int n=1;
    while(n<argc){
        if(argv[n][0] =='-'){
            switch(argv[n][1]){
              case 'h':
              case 'H':
                  helpinfo(false);
                  return 0;
              case 'A':
              case 'a':
                  helpinfo(true);
                  return 0;
              case 'T' :
              case 't':
                  spacetype='T';
                  break;
              case 'K':
              case 'k':
                  spacetype='K';
                  break;
              case 'S':
              case 's':
                  spacetype='S';
                  break;
              case 'p':
              case 'P':
                  spacetype='P';
                  break;
              case 'x':
              case 'X':
                  int x;
                  if (n==argc-1){
                      cout<<"-X must be followed by an integer"<<endl;
                      goto inv;
                  }
                  x = strtol(argv[n+1],new char*,10);
                  if(x>3){lengthx=x;}
                  else{
                      cout<<"Array must be greater than 3x3"<<endl;
                      goto inv;
                  }
                  n++;
                  break;
              case 'y':
              case 'Y':
                  int y;
                  if (n==argc-1){
                      cout<<"-Y must be followed by an integer"<<endl;
                      goto inv;
                  }
                  y = strtol(argv[n+1],new char*,10);
                  if(y>3){lengthy=y;}
                  else{
                      cout<<"Array must be greater than 3x3"<<endl;
                      goto inv;
                  }
                  n++;
                  break;
              case 'f':
              case 'F':
                  if (n==argc-1 || argv[n+1][0] == '-'){
                      cout<<"-F must be followed by a file specification"<<endl;
                      goto inv;
                  }
                  filename = argv[n+1];
                  n++;
                  break;
              default:
                  goto inv;
			}
            n++;
        }
        else{
inv:
            cout<<"Invalid command argument"<<endl;
            return 0;
        }
    }
    if(spacetype=='S' && (lengthy!=lengthx)){
        cout<<"Height and Width must be equal if -S argument is passed"<<endl;
        return 0;
    }
    int** universe = new int*[lengthx];
    int** olduniverse = new int*[lengthx];
    for(int i=0;i<lengthx;i++)	{
        universe[i] = new int[lengthy];
        olduniverse[i] = new int[lengthy];
        for(int j=0;j<lengthy;j++){
            universe[i][j] = 0;
            olduniverse[i][j] = 0;
        }
    }
    n=0;
    if(filluniverse(universe)){
        HANDLE hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
        CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
        GetConsoleScreenBufferInfo( hstdout, &bufferinfo );
        clearscreen();
        drawborder();
        int prev = 0;
        _beginthread(rupt,0,NULL);
        while(!stop){
            n++;
            coords(0,0); //to stop the console window scrolling down
            cout<<" ";
            for(int i=0;i<lengthx;i++){
                for(int j=0;j<lengthy;j++){
                    if(olduniverse[i][j] != 0){
                        coords(j+margin,i+margin);
                        cout<<" ";
                    }
                    switch(universe[i][j]){
                      case 0:
                          break;
                      case 1:
                          SetConsoleTextAttribute( hstdout, 0xB );
                          coords(j+margin,i+margin);
                          cout<<"O";
                          break;
                      case -1:
                          SetConsoleTextAttribute( hstdout, 0xC);
                          coords(j+margin,i+margin);
                          cout<<"X";
                          break;
                      default:
                          SetConsoleTextAttribute( hstdout, 0xA);
                          coords(j+margin,i+margin);
                          cout<<"Z";
                    }
                    SetConsoleTextAttribute( hstdout, bufferinfo.wAttributes );
                }
                cout<<endl;
            }
            Sleep(delay);
            int** temp;
            temp = olduniverse;
            olduniverse = universe;
            universe=temp;
            nextgen(universe,olduniverse);
        }
        cout<<"Press Enter to exit"<<endl;
        cout<<n<<endl;
        getchar();
    }
    for(int i=0;i<lengthx;i++){
        delete [] olduniverse[i];
        delete [] universe[i];
    }
    delete [] olduniverse;
    delete [] universe;
    return 0;
}

