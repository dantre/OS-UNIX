#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include  <signal.h>

#include <netdb.h>
#include <netinet/in.h>

#include <unistd.h>

#define PORT 5001

#define HEIGHT 20
#define WIDTH 40

char board[HEIGHT][WIDTH] = { 	    
"##*********##***************************",
"##********#**#***##*****#*******###*****",
"***********#*#***#*#****#******###******",
"************#*****#*****#***************",
"****************************************",
"****************************************",
"****************************************",
"*************************#**************",
"***********************#*#**************",
"*************##******##**************##*",
"************#***#****##**************##*",
"*##********#*****#***##*****************",
"*##********#***#*##****#*#**************",
"***********#*****#*******#**************",
"************#***#***********************",
"*************##*************************",
"****************************************",
"****************************************",
"****************************************",
"****************************************"};
                             
void doprocessing (int sock);
void *start_server();

void *make_step();
void print_board();

int count(int i, int j);
int get_value(int x, int y);

void ctrlc_handler(int param);

pthread_t thread;

int main()
{
	printf("Creates server on port 5001\n");
	printf("Returns a new condition Game of life every second\n");	
	
	signal(SIGINT, ctrlc_handler);
	
    if (pthread_create(&thread, NULL, start_server, NULL) != 0) {
        return EXIT_FAILURE;
    }
	
	pthread_t update;
	while (1) {
		if (pthread_create(&update, NULL, make_step, NULL) != 0) {
			exit(1);			
		}
		sleep(1);
		if(pthread_kill(update, 0) == 0)
		{
			printf("ERROR: Too long update Game of Life\n");
			pthread_cancel(thread);
			pthread_cancel(update);
			exit(1);
		}				
	}
}

void ctrlc_handler(int param) {	
	printf("Exit progran\n");
	pthread_cancel(thread);
	exit(1); 
}

void *start_server()
{	
	int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;   
	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = PORT;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding\n");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept\n");
            exit(1);
        }
        pid = fork();
        if (pid < 0) {
            perror("ERROR on fork\n");
            exit(1);
        }
        if (pid == 0) {
            close(sockfd);
            doprocessing(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    }	
}

void doprocessing (int sock)
{
    int n;
    n = write(sock, board, WIDTH * HEIGHT);    
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}

int count(int i, int j)
{
    int x, y, count = 0;

    for (x = -1; x <= 1; x++)
        for (y = -1; y <= 1; y++)
            if ((x || y) && get_value(x+i,y+j))
                count++;
    return count;
}

int get_value(int x, int y)
{
    if (x>=0 && x<HEIGHT && y>=0 && y<WIDTH && board[x][y]=='#')
        return 1;
    return 0;
}

void *make_step()
{
	//sleep(2);
    int	i, j, t;
    char tmpBoard[HEIGHT][WIDTH];

    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++)
        {
            t = count(i, j);
            if (t == 2) tmpBoard[i][j] = board[i][j];
            if (t == 3) tmpBoard[i][j] = '#';
            if (t < 2) tmpBoard[i][j] = '*';
            if (t > 3) tmpBoard[i][j] = '*';
        }
    for (i = 0; i < HEIGHT; i++)
        for (j = 0; j < WIDTH; j++)
            board[i][j] = tmpBoard[i][j];
	return (void*)0;
}

void print_board()
{
	system("clear");
    int i,j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++)
            printf("%c", board[i][j]);
        printf("\n");
    }
}
