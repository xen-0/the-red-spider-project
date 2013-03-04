
// Copyright 2013 Charles Mita
// licensed under the Red Spider Project license
// See License.txt that shipped with your copy of this software for details

/*
TODO:

IMPORTANT:
Need to port functionality to other platforms - currently Windows specific

Some more command line arguments need to be passed (array size, starting arrangement, etc).

If I can think of a sensible way of including a spherical option for the universe, I'll add it in.
Maybe even a Universe based on the projective plane.

I'd like to experiment with the rule set for Red vs Blue interactions.

Perhaps create some seeding patterns for creating the initial state
*/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include <process.h>
#include <ctime>

using namespace std;

int lengthx=40; //vertical
int lengthy=80; //horizontal
int generations = 400;
int redseed = 7;
int blueseed = 7;
int margin = 2;
bool stop = false;
int delay = 20;
char spacetype = 'T';

//int redkilledbyblue(int countR,int countB)
//{
//	return (countB>0 ? 0 : -1);
//}

int spawning(int countR, int countB)
{
	if((countR + countB) ==3){
		return(countB>countR ? 1 : -1);
	}
	if(((countR + countB) == 6) && countR>countB){return -1;}
	return 0;
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
	//this is the same as the torus case, but we need to invert the axis at a pair of opposite edges
	//if((i!=0) && (i!=lengthx-1)){
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
	//}
	//else{

	//}
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
			//if ((countR + countB) == 3) {
			//	//take on the colour of the most popular neighbour
			//	return ( countR > countB ? -1:1);
			//	//if (countR > countB) { return -1; } else { return 1; }
			//} 
			//else { return 0; }
		case 1:
			if ((countR + countB) > 3 || (countR + countB) < 2) { return 0; } else return 1;
			//return ( countR > countB ? 0 : 1);
		case -1:
			if((countR + countB) > 3 || (countR + countB) <2 ){return 0;} else return -1;//redkilledbyblue(countR,countB);
			//return ( countB > countR ? 0: -1);
		default:
			return 2;
	}
}

void nextgen(int** nextuniverse, int** universe)
{
	//int ** nextuniverse = new int*[lengthx];
	int ** neighbourhood = new int*[3];
	/*for(int i=0;i<lengthx;i++){
		nextuniverse[i] = new int[lengthy];
	}*/
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
			}
			nextuniverse[i][j] = nextcolour(neighbourhood);
		}
	}
	for(int i=0;i<3;i++){
		delete [] neighbourhood[i];
	}
	delete [] neighbourhood;
	//return nextuniverse;
}

void clearscreen()
{
	//Windows specific...
	std::system("cls");
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

void filluniverse(int ** universe)
{
	ifstream inputstate;
	inputstate.open("start.txt", ifstream::in);
	string row;
	int a = 0;
	int b=0;
	if(inputstate.is_open() && a<lengthx){
		while(getline(inputstate,row)){
			istringstream ss(row);
			string field;
			b=0;
			while(getline(ss,field,',')){
				if (b>=lengthy) break;
				if(field.empty()){
					universe[a][b]=0;
					b++;
					continue;
				}
				switch(field.at(0))	{
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
	}
	else{
		seeduniverse(universe);
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

int main(int argc, _TCHAR* argv[])
{
	int n=1;
	while(n<argc){
		if(argv[n][0] =='-'){
			switch(argv[n][1]){
				case 'h':
				case 'H':
					cout<<"Some help info here later"<<endl;
					return 0;
				case 'T' :
				case 't':
					spacetype='T';
					break;
				case 'K':
				case 'k':
					spacetype='K';
					break;
				case 'x':
				case 'X':
					//code to retrive int from next argument
					break;
				case 'y':
				case 'Y':
					//code to retrieve int from next argument
					break;
				default:
					goto inv;
			}
			n++;
		}
		else{
inv:	cout<<"Invalid command argument"<<endl;
		return 0;
		}
	}
	if (spacetype=='S' && lengthx!=lengthy){
		cout<<"Grid must be square for spherical space"<<endl;
		return 0;
	}
	HANDLE hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
	CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
	GetConsoleScreenBufferInfo( hstdout, &bufferinfo );
	clearscreen();
	drawborder();
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
	filluniverse(universe);
	int prev = 0;
	_beginthread(rupt,0,NULL);
	bool firstrun = true;
	while(!stop){
		n++;
		coords(0,0); //to stop the console window scrolling down
		cout<<" ";
		//clearscreen();
		for(int i=0;i<lengthx;i++){
			for(int j=0;j<lengthy;j++){
				if(olduniverse[i][j] != 0){
					coords(j+margin,i+margin);
					cout<<" ";
				}
				switch(universe[i][j]){
					case 0:
						//SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 0x1 );
						//cout<<"- ";
						/*if(0==prev) output+="- "; else {
							consolecolour(prev);
							cout<<output;
							output="- ";
							prev=0;
						}*/
						//coords(j+1,i+1);
						//cout<<" ";
						break;
					case 1:
						SetConsoleTextAttribute( hstdout, 0xB );
						//cout<<"O ";
						//if(1==prev) output+="O "; else {
						//	consolecolour(prev);
						//	cout<<output;
						//	output="O ";
						//	prev=1;
						//}
						coords(j+margin,i+margin);
						cout<<"O";
						break;
					case -1:
						SetConsoleTextAttribute( hstdout, 0xC);
						//cout<<"X ";
						//if(-1 == prev) output+="X "; else {
						//	consolecolour(prev);
						//	cout<<output;
						//	output="X ";
						//	prev=-1;
						//}
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
			//output+='\n';
		}
		//consolecolour(prev);
		//getchar();
		//cout<<output;
		Sleep(delay);
	/*	if(!firstrun){
			for(int i=0;i<lengthx;i++){
				delete [] olduniverse[i];
			}
			delete [] olduniverse;
		}*/
		int** temp;
		temp = olduniverse;
		olduniverse = universe;
		universe=temp;
		nextgen(universe,olduniverse);
		firstrun=false;
	}
	//coords(lengthx,lengthy);
	for(int i=0;i<lengthx;i++){
		delete [] olduniverse[i];
		delete [] universe[i];
	}
	delete [] olduniverse;
	delete [] universe;
	cout<<"Press Enter to exit"<<endl;
	cout<<n<<endl;
	getchar();
	return 0;
}


