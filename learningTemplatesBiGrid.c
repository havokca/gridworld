#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <signal.h>

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
#define SLEEPTIME 50000
#define ALPHA 0.1
#define GAMMA 0.95 
#define EPSILON 0.1

int templateInUse[3];
int templateCreated[3];
int rewardLocation[MAXX][MAXY][3];
int grid[MAXX][MAXY];
double val[MAXX][MAXY];
double template[MAXX][MAXY][3];
float policy[MAXX][MAXY][4];
int canmove[4];
int xloc, yloc;
int rewardx, rewardy, rewardAx, rewardAy, rewardBx, rewardBy;
int episodeNumber;
int step;
WINDOW *myScreen;

void showMap(void);
void doGridLearning(void);
int choseMove(void);
double getReward(void);
int doMove(int);
void sigfun(int);
void fuckedIndex(void);

void sigfun(int sig)
{
    endwin();
    printf("Exiting program due to CTRL-C\n");
    exit(0);
}

void fuckedIndex() {
    endwin();
    printf("An index is fucked\n");
    exit(0);
}

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
    
    (void) signal(SIGINT, sigfun);
    
    //initialize gridworld
    for( i = 0; i < MAXX; i++ ) {
        for( j = 0; j < MAXY; j++) {
            grid[i][j] = FLOOR;
            val[i][j] = 0;
            for( k = 0; k < 3; k++ ) {
                template[i][j][k] = 0;
                templateCreated[k] = 0;
                templateInUse[k] = 0;
                rewardLocation[i][j][k] = 0;
            }
        }
    }
    
    
    //randomly place wall with single "slit" opening
    /*if( random() % 2 == 0 ) { //vertical wall
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
     */
    //randomly place rewards A and B and C
    do{
        i = random() % MAXX;
        j = random() % MAXY;
    }while( grid[i][j] == WALL );
    rewardAx = i;
    rewardAy = j;
    do{
        i = random() % MAXX;
        j = random() % MAXY;
    }while( grid[i][j] == WALL || ( rewardAx == i && rewardAy == j ) );
    rewardBx = i;
    rewardBy = j;
    
    showMap();
    
    doGridLearning();
    
    //leave window open until keypress
    temp = getchar();
    endwin();
    return 0;
}

void doGridLearning() {
    int i, j, k, moves, oldx, oldy, reward, ret, spawnNewTemplate, templateChosen, numActiveTemplates;
    int count = 0;
    int best = 0;
    int move = 0;
    int numTemplatesUsed = 0;
    double temp;
    int up, down, left, right;
    for( count; count < EPISODES; count++ ) {
        episodeNumber++;
        //50-50 chance of the reward being in either location
        if(( (double)random() / (double)RAND_MAX ) < 0.5 ) { 
            rewardx = rewardAx;
            rewardy = rewardAy;
        } else {
            rewardx = rewardBx;
            rewardy = rewardBy;
        }
        val[rewardAx][rewardAy] = 0;
        val[rewardBx][rewardBy] = 0;
        grid[rewardAx][rewardAy] = FLOOR;
        grid[rewardBx][rewardBy] = FLOOR;
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
        //re-enable any created templates
        for( k = 0; k < 3; k++ ) {
            if( templateCreated[k] == 1 ) {
                templateInUse[k] = 1;
            }
        }
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
            
            /* chose the move based on the best value from the templates being maintained by the system
             if the reward isn't where it's predicted to be, disable the associated template for this episode
             if the reward shows up and there's no prediction for where it shows up, create a new template
             */
            best = 0;
            numActiveTemplates = 0;
            //printf("Episode %d, Step %d -- Determining # of templates used:", episodeNumber, step);
            //fflush(NULL);
            for( k = 0; k < 3; k++ ) {
                if( templateInUse[k] == 1 ) {
                    numActiveTemplates++;
                }
            }
            //printf(" %d\n", numActiveTemplates);
            //fflush(NULL);
            if( numActiveTemplates == 0 ) { //no templates in use, so move in a random direction
                //printf("No templates active: Moving in random direction. Direction chosen =");
                //fflush(NULL);
                do{
                    do{
                        move = random() % 4;
                    }while( !canmove[move] );    
                }while( doMove(move) == -1 );
                //printf(" %d\n", move);
                //fflush(NULL);
            } else { //otherwise, use the best active template
                //printf("%d templates active. Moving in direction = ", numActiveTemplates);
                //fflush(NULL);
                for( k = 0; k < 3; k++ ) {
                    if( canmove[UP] ) {
                        if( (template[xloc][yloc-1][k] > best) && templateInUse[k] == 1 ){
                            move = UP;
                            best = template[xloc][yloc-1][k];
                            templateChosen = k;
                        }
                    }
                    if( canmove[DOWN] ) {
                        if( (template[xloc][yloc+1][k] > best) && templateInUse[k] == 1 ){
                            move = DOWN;
                            best = template[xloc][yloc+1][k];
                            templateChosen = k;
                        }
                    }
                    if( canmove[RIGHT] ) {
                        if( (template[xloc+1][yloc][k] > best) && templateInUse[k] == 1 ){
                            move = RIGHT;
                            best = template[xloc+1][yloc][k];
                            templateChosen = k;
                        }                
                    }
                    if( canmove[LEFT] ) {
                        if( (template[xloc-1][yloc][k] > best) && templateInUse[k] == 1 ){
                            move = LEFT;
                            best = template[xloc-1][yloc][k];
                            templateChosen = k;
                        }
                    }
                }
                if( best == 0 ) {//active templates don't predict a good move, so move randomly
                    do{
                        do{
                            move = random() % 4;
                        }while( !canmove[move] );    
                    }while( doMove(move) == -1 );
                    templateChosen = -1;
                } else {
                    doMove(move);
                }
                //printf("%d by following template %d\n", move, templateChosen);
                //fflush(NULL);
                //   doMove(move);
            }
            
            reward = getReward();
            //printf("Received reward of %d", reward);
            //fflush(NULL);
            // is reward > zero?
                //>zero reward: did a template predict it to be there?
                    // if so, do TD learning on that template
                    // if not, spawn a new template and associate the reward-state with that template
                //<=zero reward: did an active template predict a reward for the current state?
                    // if so, disable that template for this episode
                    // if not, do TD learning on all active templates
            
            //		val[oldx][oldy] += ALPHA*(reward+(GAMMA*val[xloc][yloc])-val[oldx][oldy]);
            if( /*reward > 0*/ xloc == rewardx && yloc == rewardy ) {
                //printf(" which is > 0");
                //fflush(NULL);
                spawnNewTemplate = 1;
                for( k = 0; k < 3; k++ ) {
                    if(rewardLocation[xloc][yloc][k] == 1){
                        //printf(" -- PREDICTIVE TEMPLATE FOUND!");
                        //getchar();
                        //fflush(NULL);
                        spawnNewTemplate = 0;
                        //do TD learning on template k
                        template[xloc][yloc][k] = REWARD;
                        template[oldx][oldy][k] += ALPHA*(reward+(GAMMA*template[xloc][yloc][k])-template[oldx][oldy][k]);
                    }
                }
                if( spawnNewTemplate == 1 ) {
                    //printf(" -- No predictive template found\nSpawning new template #%d\n", numTemplatesUsed);
                    //fflush(NULL);
                    if( numTemplatesUsed > 2 ) { fuckedIndex(); }
                    templateCreated[numTemplatesUsed] = 1;
                    templateInUse[numTemplatesUsed] = 1;
                    rewardLocation[xloc][yloc][numTemplatesUsed] = 1;
                    numTemplatesUsed++;
                    //create new template
                    //associate reward at xloc,yloc with that template
                }
            }else {
                //printf(" which is <= 0\n");
                //fflush(NULL);
                for( k = 0; k < 3; k++ ) {
                    if(templateInUse[k] == 1){
                        if(rewardLocation[xloc][yloc][k] == 1){
                            //disable template k for remainder of episode
                            //printf("REWARD PREDICTED BUT NOT FOUND! DISABLING TEMPLATE!\n");
                            //getchar();
                            //fflush(NULL);
                            templateInUse[k] = 0;
                        }
                    }
                }
                //if it's a step that doesn't provide a reward, just do TD learning on the template we're following
                if( numActiveTemplates >= 0 ) {
                    //printf("\nTemplates are active, but reward wasn't reached - doing TD learning on all active templates");
                    //fflush(NULL);
                    for( k = 0; k < 3; k++ ) {
                        if( templateInUse[k] == 1) {
                            //printf("\nDoing TD learning on template %d\n", k);
                            //fflush(NULL);
                            template[oldx][oldy][k] += ALPHA*(reward+(GAMMA*template[xloc][yloc][k])-template[oldx][oldy][k]);
                        }
                    }
                }
            }
            
            
            
            if( episodeNumber % 1000 == 0 ) { //|| episodeNumber == 1 ) {//&& step == 1 ) {
                showMap();
                usleep((useconds_t)SLEEPTIME);
            }
        }while( step < MAXSTEPS && ( xloc != rewardx || yloc != rewardy ));
    }
    return;
}

double getReward() {
    double reward = 0;
   // if( xloc == rewardx && yloc == rewardy ) { reward = REWARD; }
    return reward;
}


void showMap() {
    int i, j, k, ind;
    char myBuf[300];
    snprintf((char *)myBuf, (size_t)300, "Trial Number - %d || Step - %d || [%d,%d] || template 1 = %d || template 2 = %d || template 3 = %d\n", episodeNumber, step, xloc, yloc, templateInUse[0], templateInUse[1], templateInUse[2]); 
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
            else if( (i == rewardAx && j == rewardAy) || (i == rewardBx && j == rewardBy) ) {
                if( i == rewardx && j == rewardy ) { 
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
                        snprintf((char *)myBuf, (size_t)300, "### ");
                    }else {
                        snprintf((char *)myBuf, (size_t)300, "%3.0f ", val[k][j]);
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
    //Display templateInUse tables
    for( j = 0; j < MAXY; j++ ) {
        addch((const chtype)'|');
        for( i = 0; i < MAXX; i++ ) {
            if( grid[i][j] == WALL ) {
                snprintf((char *)myBuf, (size_t)300, "### ");
            } else { 
                snprintf((char *)myBuf, (size_t)300, "%3.0f ", template[i][j][0]);
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
                snprintf((char *)myBuf, (size_t)300, "### ");
            } else { 
                snprintf((char *)myBuf, (size_t)300, "%3.0f ", template[i][j][1]);
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
                snprintf((char *)myBuf, (size_t)300, "### ");
            } else {
                snprintf((char *)myBuf, (size_t)300, "%3.0f ", template[i][j][2]);
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
    //fprintf(stderr, "Attempting to move %d", dir);
    if( dir == LEFT && canmove[LEFT] ) { xloc--; }
    if( dir == RIGHT && canmove[RIGHT] ) { xloc++; }
    if( dir == UP && canmove[UP] ) { yloc--; }
    if( dir == DOWN && canmove[DOWN] ) { yloc++; }
    if( xloc >= 0 || xloc < MAXX || yloc >= 0 || yloc < MAXY ) { return 0; }
    if( xloc > MAXX || xloc < 0 || yloc < 0 || yloc > MAXY ) { fuckedIndex(); }
    else { return -1; }
}
