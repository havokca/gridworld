#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>

#define MAXX 20 
#define MAXY 16 
#define REWARD 100 
#define WALL -1
#define FLOOR 0
#define UP 0 
#define RIGHT 1
#define DOWN 2
#define LEFT 3
#define EPISODES 1000000
#define MAXSTEPS 1000000
#define SLEEPTIME 5000
#define ALPHA 0.1
#define GAMMA 0.8 
#define EPSILON 0.1

int phi[3];
int grid[MAXX][MAXY];
double val[MAXX][MAXY];
double phiVal[MAXX][MAXY][3];
float policy[MAXX][MAXY][4];
int canmove[4];
int xloc, yloc;
int rewardx, rewardy, rewardAx, rewardAy, rewardBx, rewardBy, rewardCx, rewardCy;
int episodeNumber;
int step;
WINDOW *myScreen;

void showMap(void);
void doGridLearning(void);
int choseMove(void);
double getReward(void);
int doMove(int);

int main(int argc, char* argv[]) {
int i, j, k, n;
int wallposition, wallopening;
int counter;
char temp;

srandom(time(NULL));
xloc = 0;
yloc = 0;
episodeNumber = 0;
myScreen = initscr();
refresh();

//create gridworld
for( i = 0; i < MAXX; i++ ) {
	for( j = 0; j < MAXY; j++) {
		grid[i][j] = FLOOR;
		val[i][j] = 0;
		for( k = 0; k < 3; k++ ) {
			phiVal[i][j][k] = 0;
		}
	}
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

//randomly place rewards A and B and C
do{
	i = random() % MAXX;
	j = random() % MAXY;
}while( grid[i][j] == WALL );
//grid[i][j] = REWARD;
rewardAx = i;
rewardAy = j;
do{
	i = random() % MAXX;
	j = random() % MAXY;
}while( grid[i][j] == WALL || ( rewardAx == i && rewardAy == j ) );
rewardBx = i;
rewardBy = j;
//val[i][j] = REWARD;
do{
	i = random() % MAXX;
	j = random() % MAXY;
}while( grid[i][j] == WALL || ( rewardAx == i && rewardAy == j ) || ( rewardBx == i && rewardBy == j ) );
rewardCx = i;
rewardCy = j;

showMap();

doGridLearning();

//leave window open until keypress
temp = getchar();
endwin();
return 0;
}

void doGridLearning() {
int i, j, k, moves, oldx, oldy, reward, ret;
int count = 0;
int up, down, left, right;
for( count; count < EPISODES; count++ ) {
	episodeNumber++;
	//randomly choose features phi0 to phi2
/*	for( i = 0; i < 3; i++ ) {
		if( ( (float)random() / (float)RAND_MAX ) < 0.5 ) {
			phi[i] = 1;
		} else {
			phi[i] = 0;
		}
	}
*/	if( ((float)random() / (float)RAND_MAX) < 0.7 ) {
		phi[0] = 1;
	} else {
		phi[0] = 0;
	}
	if( ((float)random() / (float)RAND_MAX) < 0.3 ) {
		phi[1] = 1;
	} else { 
		phi[1] = 0;
	}
	if( ((float)random() / (float)RAND_MAX) < 0.5 ) {
		phi[2] = 1;
	} else { 
		phi[2] = 0;
	}
	if( phi[0] == 0 && phi[1] == 0 && phi[2] == 1 ) {
		rewardx = rewardCx;
		rewardy = rewardCy;
	} else if( ( phi[0] == 1 && phi[1] == 0 ) || ( phi[0] == 0 && phi[1] == 1 )) {
		rewardx = rewardAx;
		rewardy = rewardAy;
	} else {
		rewardx = rewardBx;
		rewardy = rewardBy;
	}
	for( k = 0; k < 3; k++ ) {
		phiVal[rewardCx][rewardCy][k] = 0;
		phiVal[rewardAx][rewardAy][k] = 0;
		phiVal[rewardBx][rewardBy][k] = 0;
		phiVal[rewardx][rewardy][k] = REWARD;
	}
	val[rewardAx][rewardAy] = 0;
	val[rewardBx][rewardBy] = 0;
	val[rewardCx][rewardCy] = 0;
	grid[rewardAx][rewardAy] = FLOOR;
	grid[rewardBx][rewardBy] = FLOOR;
	grid[rewardCx][rewardCy] = FLOOR;
	grid[rewardx][rewardy] = REWARD;
	val[rewardx][rewardy] = REWARD;
	//randomly chose starting position
        do{
                i = random() % MAXX;
                j = random() % MAXY;
	}while( grid[i][j] == WALL || (i == rewardx && j == rewardy) );
        xloc = i;
        yloc = j;
	step = 0;
	do{
		step++;
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
		oldx = xloc;
		oldy = yloc;
		do{
			ret = doMove(choseMove());
		}while( ret == -1 );
		reward = getReward();
		val[oldx][oldy] += ALPHA*(reward+(GAMMA*val[xloc][yloc])-val[oldx][oldy]);
		for( k = 0; k < 3; k ++ ) {
			if( phi[k] == 1 ) {
				phiVal[oldx][oldy][k] += ALPHA*(reward+(GAMMA*phiVal[xloc][yloc][k])-phiVal[oldx][oldy][k]);
			}
		}
		if( episodeNumber % 1000 == 0 && step == 1 ) {
			showMap();
			usleep((useconds_t)SLEEPTIME);
		}
	}while( step < MAXSTEPS && ( xloc != rewardx || yloc != rewardy ));
}
return;
}

double getReward() {
double reward = 0;
if( xloc == rewardx && yloc == rewardy ) { reward = REWARD; }
return reward;
}

int choseMove() {
int dir;
int myMove = -1;
int best = 0;
int rphi = 0;
//randomly chose an rphi
if( phi[0] == 0 && phi[1] == 0 && phi[2] == 0 ) {
	rphi = -1;
} else {
	do{
		rphi = random() % 3;
	}while( phi[rphi] == 0 );
}
if( rphi == -1 ) {
	if( (val[xloc][yloc-1] > best) && canmove[UP] ) {
		dir = UP;
		best = val[xloc][yloc-1];
	}
	if( (val[xloc+1][yloc] > best) && canmove[RIGHT] ) {
		dir = RIGHT;
		best = val[xloc+1][yloc];
	}
	if( (val[xloc][yloc+1] > best) && canmove[DOWN] ) {
		dir = DOWN;	
		best = val[xloc][yloc+1];
	}
	if( (val[xloc-1][yloc] > best) && canmove[LEFT] ) {
		dir = LEFT;
		best = val[xloc-1][yloc];
	}
	if( best == 0 || (random() % RAND_MAX) < EPSILON ) {
		do{
			myMove = random() % 4;
		}while( !canmove[myMove] );
	} else {
		myMove = dir;
	}
} else { 
        if( (phiVal[xloc][yloc-1][rphi] > best) && canmove[UP] ) {
                dir = UP;
                best = phiVal[xloc][yloc-1][rphi];
        }
        if( (phiVal[xloc+1][yloc][rphi] > best) && canmove[RIGHT] ) {
                dir = RIGHT;
                best = phiVal[xloc+1][yloc][rphi];
        }
        if( (phiVal[xloc][yloc+1][rphi] > best) && canmove[DOWN] ) {
                dir = DOWN;
                best = phiVal[xloc][yloc+1][rphi];
        }
        if( (phiVal[xloc-1][yloc][rphi] > best) && canmove[LEFT] ) {
                dir = LEFT;
                best = phiVal[xloc-1][yloc][rphi];
        }
        if( best == 0 || (random() % RAND_MAX) < EPSILON ) {
                do{
                        myMove = random() % 4;
                }while( !canmove[myMove] );
        } else {
                myMove = dir;
        }
}

return myMove;
}

void showMap() {
int i, j, k, ind;
char myBuf[200];
snprintf((char *)myBuf, (size_t)200, "Trial Number - %d || Step - %d || [%d,%d] || phi[0] = %d || phi[1] = %d || phi[2] = %d\n", episodeNumber, step, xloc, yloc, phi[0], phi[1], phi[2]); 
clear();
wnoutrefresh(myScreen);
ind = 0;
while( myBuf[ind] != '\0' ) {
	addch((const chtype)myBuf[ind++]);
}
for( i = 0; i < (MAXX+2); i++ ) { 
	addch((const chtype)'-');
	}
addch((const chtype)' ');
addch((const chtype)' ');
addch((const chtype)' ');
for( i = 0; i < (4*MAXX)+2; i++ ) {
        addch((const chtype)'-');
        }
for( j = 0; j < MAXY; j++ ) {
	addch((const chtype)'\n');
	for( i = 0; i < MAXX; i++ ) {
		if( i == 0 ) { 
 			addch((const chtype)'|'); 
		}
		if( i == xloc && j == yloc ) {
                        addch((const chtype)'A');
                }
		else if( (i == rewardAx && j == rewardAy) || (i == rewardBx && j == rewardBy) || (i == rewardCx && j == rewardCy )) {
                        if ( i == rewardx && j == rewardy && i == rewardCx && j == rewardCy ) {
				addch((const chtype)'T');
			}else if( i == rewardx && j == rewardy ) { 
				addch((const chtype)'R');
			} else {
				addch((const chtype)'r');
			}
                }
		else if( grid[i][j] == WALL ) {
			addch((const chtype)'#');
		} else {
			addch((const chtype)' ');
		}
		if( i == MAXX -1 ) { 
			addch((const chtype)'|');
			//add spacing
			addch((const chtype)' ');
			addch((const chtype)' ');
			addch((const chtype)' ');
			addch((const chtype)'|');
			//begin line for value landscape
			for( k = 0; k < MAXX; k++ ) {
				if( grid[k][j] == WALL ) {
					snprintf((char *)myBuf, (size_t)200, "### ");
				}else {
					snprintf((char *)myBuf, (size_t)200, "%3.0f ", val[k][j]);
				}
				ind = 0;
				while( myBuf[ind] != '\0' ) {
					addch((const chtype)myBuf[ind++]);
				}
			}
			addch((const chtype)'|'); 
		}
	}
}
addch((const chtype)'\n');
for( i = 0; i < (MAXX+2); i++ ) { 
	addch((const chtype)'-');
	}
addch((const chtype)' ');
addch((const chtype)' ');
addch((const chtype)' ');
for( i = 0; i < (4*MAXX)+2; i++ ) {
        addch((const chtype)'-');
        }

addch((const chtype)'\n');
addch((const chtype)'\n');
//Display phi tables
for( j = 0; j < MAXY; j++ ) {
	addch((const chtype)'|');
	for( i = 0; i < MAXX; i++ ) {
		if( grid[i][j] == WALL ) {
			snprintf((char *)myBuf, (size_t)200, "### ");
		} else { 
			snprintf((char *)myBuf, (size_t)200, "%3.0f ", phiVal[i][j][0]);
		}
		ind = 0;
		while( myBuf[ind] != '\0' ) {
			addch((const chtype)myBuf[ind++]);
		}
	}
	addch((const chtype)' ');
	addch((const chtype)' ');
	addch((const chtype)'|');
        for( i = 0; i < MAXX; i++ ) {
                if( grid[i][j] == WALL ) {
                        snprintf((char *)myBuf, (size_t)200, "### ");
                } else { 
                        snprintf((char *)myBuf, (size_t)200, "%3.0f ", phiVal[i][j][1]);
                }
                ind = 0;
                while( myBuf[ind] != '\0' ) {
                        addch((const chtype)myBuf[ind++]);
                }
        }
	addch((const chtype)'\n');
}
addch((const chtype)'\n');
addch((const chtype)'\n');
for( j = 0; j < MAXY; j++ ) {
        addch((const chtype)'|');
        for( i = 0; i < MAXX; i++ ) {
                if( grid[i][j] == WALL ) {
                        snprintf((char *)myBuf, (size_t)200, "### ");
                } else {
                        snprintf((char *)myBuf, (size_t)200, "%3.0f ", phiVal[i][j][2]);
                }
                ind = 0;
                while( myBuf[ind] != '\0' ) {
                        addch((const chtype)myBuf[ind++]);
                }
        }
	addch((const chtype)'\n');
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
