#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>

#define MAXX 30 
#define MAXY 30
#define REWARD 100
#define WALL -1
#define FLOOR 0
#define UP 0 
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define TRIALS 10000
#define SLEEPTIME 5000

int grid[MAXX][MAXY];
int val[MAXX][MAXY];
float policy[MAXX][MAXY][4];
int canmove[4];
int xloc, yloc;
int trialNumber;
WINDOW *myScreen;

void showMap(void);
void doGridLearning(void);
int doMove(int);

int main(int argc, char* argv[]) {
int i, j, k, n, tsum;
int wallposition, wallopening;
int counter;
char temp;

srandom(time(NULL));
xloc = 0;
yloc = 0;
trialNumber = 0;
myScreen = initscr();
refresh();

//create gridworld
for( i = 0; i < MAXX; i++ ) {
	for( j = 0; j < MAXY; j++) {
		grid[i][j] = FLOOR;
		tsum = 0;
/*		for( k = 0; k < 4; k++ ) {
			policy[i][j][k] = (float)random()/(float)RAND_MAX;
			tsum += policy[i][j][k];
		} 
		for( k = 0; k < 4; k++ ) {
			policy[i][j][k] = policy[i][j][k] / tsum;
		}
*/	}
}


//randomly place wall with single "slit" opening
if( random() % 2 == 0 ) { //vertical wall
	wallposition = random() % (MAXX-2);
	wallopening = random() % MAXY;
	for( i = 0; i < MAXY; i++ ) {
		grid[wallposition+1][i] = WALL;
	}
	grid[wallposition+1][wallopening] = FLOOR;
}else { //horizontal wall
	wallposition = random() % (MAXY-2);
	wallopening = random() % MAXX;
	for( i = 0; i < MAXX; i++ ) {
		grid[i][wallposition+1] = WALL;
	}
	grid[wallopening][wallposition+1] = FLOOR;
}

//randomly place reward
do{
	i = random() % MAXX;
	j = random() % MAXY;
}while( grid[i][j] == WALL );
grid[i][j] = REWARD;

showMap();

doGridLearning();
temp = getchar();
endwin();
return 0;
}

void doGridLearning() {
int i, j, moves;
int count = 0;
int up, down, left, right;
//randomly chose starting position
        do{
                i = random() % MAXX;
                j = random() % MAXY;
        }while( grid[i][j] == WALL || grid[i][j] == REWARD );
        xloc = i;
        yloc = j;

for( count; count < TRIALS; count++ ) {
	trialNumber++;
	//reset freedom of movement
	for( moves = 0; moves < 4; moves++ ) {
		canmove[moves] = 0;
	}
	//determine freedom of movement
	if( xloc > 0 ) { 
		if( grid[xloc-1][yloc] != WALL ) {
			canmove[LEFT] = 1;
		}
	}
	if( xloc < (MAXX-1) ) {
		if( grid[xloc+1][yloc] != WALL ) {
			canmove[RIGHT] = 1;
		}
	} 
	if( yloc > 0 ) {
		if( grid[xloc][yloc-1] != WALL ) {
			canmove[UP] = 1;
		}
	}
	if( yloc < (MAXY-1) ) {
		if( grid[xloc][yloc+1] != WALL ) {
			canmove[DOWN] = 1;
		}
	}
	if( grid[xloc][yloc] == REWARD ) { showMap(); break; }
	do{
		showMap();
		usleep((useconds_t)SLEEPTIME);
	}while( doMove( random() % 4 ) == -1 ); 
}

return;
}

void showMap() {
int i, j;
char myBuf[100];
snprintf((char *)myBuf, (size_t)100, "Trial Number - %d\n", trialNumber); 
clear();
wnoutrefresh(myScreen);
i = 0;
while( myBuf[i] != '\0' ) {
	addch((const chtype)myBuf[i++]);
}
for( i = 0; i < (MAXX+2); i++ ) { 
	addch((const chtype)'-');
	}
for( j = 0; j < MAXY; j++ ) {
	addch((const chtype)'\n');
	for( i = 0; i < MAXX; i++ ) {
		if( i == 0 ) { 
 			addch((const chtype)'|'); 
		}
		if( grid[i][j] == FLOOR ) {
			addch((const chtype)' ');
		}
		if( grid[i][j] == WALL ) {
			addch((const chtype)'#');
		}
		if( grid[i][j] == REWARD ) {
			addch((const chtype)'R');
		}
		if( i == xloc && j == yloc ) {
			addch((const chtype)'\b');
			addch((const chtype)'A');
		}
		if( i == MAXX -1 ) { 
			addch((const chtype)'|'); 
		}
	}
}
addch((const chtype)'\n');
for( i = 0; i < (MAXX+2); i++ ) { 
	addch((const chtype)'-');
	}

wrefresh(myScreen);
return;
}

int doMove(int dir) {
if( dir == LEFT && canmove[LEFT] ) { xloc--; }
if( dir == RIGHT && canmove[RIGHT] ) { xloc++; }
if( dir == UP && canmove[UP] ) { yloc--; }
if( dir == DOWN && canmove[DOWN] ) { yloc++; }
if( xloc >= 0 || xloc < MAXX || yloc >= 0 || xloc < MAXY ) { return 0; }
else { return -1; }
}
