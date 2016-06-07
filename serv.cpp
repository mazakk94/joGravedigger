#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>


#define readI(fd, x) read(fd, x, sizeof(int))
#define readF(fd, x) read(fd, x, sizeof(float))
#define writeI(fd, x) write(fd, x, sizeof(int))
#define writeF(fd, x) write(fd, x, sizeof(float))

using namespace std;

struct timeval tp;

void childend(int signum)
{
	wait(NULL);
	printf("Connection end\n");
}

int main(int argc, char** argv) {
    unsigned int fdc, size, on=1; //client
    struct sockaddr_in lsa; //server
    struct sockaddr_in laddr; //client
	if(signal(SIGCHLD, childend) == SIG_IGN)
		signal(SIGCHLD, SIG_IGN);
	
	fdc = socket(PF_INET, SOCK_STREAM, 0);
	lsa.sin_family = PF_INET;
	lsa.sin_port = htons(8080);
	lsa.sin_addr.s_addr = INADDR_ANY;
	bind(fdc, (struct sockaddr*)&lsa, sizeof(lsa));
	listen(fdc, 10);
	
	size=sizeof(laddr);
	int corpses = 2;
    
	gettimeofday(&tp, NULL);
    long int t = tp.tv_sec * 1000 + tp.tv_usec / 1000;    
    
	int fd2;
	while(1){
		fd2 = accept(fdc, (struct sockaddr*)&laddr, &size);
		if(!fork()){
			close(fdc);
			int rank;
			readI(fd2, &rank);
			printf("Connected: %i\n", rank);
			writeI(fd2, &corpses);
			//write(fd2, &t, sizeof(long));
			close(fd2);
			return 0;
		}else{
            gettimeofday(&tp, NULL);
            long int tempTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;   
			//long tempTime = t;
			if((int)(tempTime - t) > 10000){
				srand (time(NULL));
				corpses += rand() % 4 + 1;
				t = tempTime;
				cout<<"Corpses: "<<corpses<<" "<<t<<endl;
			}
		}
	}
    return 0;
}