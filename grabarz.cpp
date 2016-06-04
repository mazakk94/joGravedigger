/*#include <stdio.h>
//#include <iostream>
//#include <vector>
//#include <arpa/inet.h>
//#include <stdlib.h>
//#include <sys/wait.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <string.h>
//#include <sstream>
//#include <fstream>
//#include <cstdlib>
//#include <cstdio>
//#include <ctime>
//#include <sys/ipc.h>
//#include <time.h>
*/

#include <mpi.h>
#include <netdb.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

/*
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>
*/

#define SIZE 30
#define MAX_SIZE 256
#define MSG_TAG 100

/*
#define readI(fd, x) read(fd, x, sizeof(int))
#define readF(fd, x) read(fd, x, sizeof(float))
#define writeI(fd, x) write(fd, x, sizeof(int))
#define writeF(fd, x) write(fd, x, sizeof(float))
*/

using namespace std;

struct timeval tp;
struct sockaddr_in sa;
struct hostent* addrent;
vector <pair <int, long int>> processList;
int corpse = 2;

/*
struct msgbuf {
    long type;
    int cmd;
    char nick[10];
    char text[256];
    char date[30];
    int pid;
    int status;
};
*/

int my_fd = 0;

void addToProcessList(int rank, long int priority){    
    pair<long int, long int> process;
    process.first = rank;
    process.second = priority;    
    processList.push_back(process);    
}

int getPosition(int rank){
    for(unsigned int i = 0; i < processList.size(); i++){
        if(rank == processList[i].first )
            return i;
    }
    return processList.size()-1;
}

void sortProcessList(){
    /*     BUBBLE SORT XD    */
    pair <int, long int> tmp;
    for(unsigned int j = 0; j < processList.size() - 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){            
            if(processList[i].second > processList[i+1].second 
                && processList[i+1].second > 0) {
                tmp.first = processList[i+1].first;
                tmp.second = processList[i+1].second;                
                processList[i+1].first = processList[i].first;
                processList[i+1].second = processList[i].second;
                processList[i].first = tmp.first;            
                processList[i].second = tmp.second;            
            }
        }          
    }    
    /*
    for(unsigned int j = 0; j < processList.size() - 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){
            if(processList[i].second == processList[i+1].second && 
                processList[i].first > processList[i+1].first ) {
                tmp = processList[i+1];
                processList[i+1] = processList[i];
                processList[i] = tmp;            
            }
        }          
    }    
    */
    
}

bool canITakeCorpse(int size, int rank, int corpse){
    sortProcessList();
    int diff = size - processList.size(); // jak nie mamy odpowiedzi od wszystkich to zakladamy, że
    if(getPosition(rank) + diff < corpse) // ten, którego zegara nie znamy jest przed nami
        return true;
    else 
        return false;    
}

long int getNewPriority(){    
    gettimeofday(&tp, NULL);
    long int clock = tp.tv_sec * 1000 + tp.tv_usec / 1000;    
    return clock;
}

void printProcessList(vector < pair <int, long int> > processList){
    int i = 0;
    if(processList.size() > 3)
        for(pair<int, long int>& process : processList) {
            printf("process[%d]: %d, %ld\n", i, process.first, process.second);        
            i++;
        }
}

void sendRelease(int size, long int rank){
    long int msg[3];
    msg[0] = rank;
    msg[1] = -1;
    msg[2] = 2;
    for(int i = 0; i < size; i++){
        if(i != rank){
            //wyślij priorytet
            MPI_Send( msg, 3, MPI_LONG_INT, i, MSG_TAG, MPI_COMM_WORLD );
            printf("  Wyslalem release: %ld, priority: %ld, type: %ld do %d\n", msg[0], msg[1], msg[2], i );
        }
    }    
}

void pogrzeb(int size, int rank){
    
    cout << "PRZEBIEGA POGRZEB JEDNEGO Z TRUPÓW...\n";
    corpse--;
    sendRelease(size, rank); //-> DO ZAKODZENIA
    sleep(5);    
    
    //TUTAJ !
}

bool receiveMessages(int msg_count, int size, int rank, long int *msg){
    MPI_Status status;
    while(msg_count != size - 1){
        //odbieraj wiadomosci
        //cout << rank << ": odbieram..." << endl;
        MPI_Recv(msg, 3, MPI_LONG_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf(" %d: Otrzymalem rank: %ld, priority: %ld, type: %ld od %d\n", rank, msg[0], msg[1], msg[2], status.MPI_SOURCE);
        if(msg[2] == 0){ //release
            corpse--;   
            msg_count--;
            processList.erase(processList.begin()+getPosition(msg[0])); 
            //usun z listy proces który wykonał pogrzeb
        } else if (msg[2] == 1){ //request
            msg_count++;
            addToProcessList(msg[0], msg[1]);
            if (canITakeCorpse(size, rank, corpse)){
                printf("%d: BIORE GO (szybciej) !\n", rank);
                //goto potrzeb
                pogrzeb(size, rank);
                return true;  //jak true to jump do startu
                
            }
            
        } else { // msg[2] == 2, czyli batch
            corpse += (int)msg[1];
        }
        
    }  
    return false;
}




//tu sie moze zapetlic jak serwer padł
/*
int askForCorpseNum(int fd){
    //int fd = 0, 
    int con = -1, count = 0;
    con = connect(fd, (struct sockaddr*)&sa, sizeof(sa));
    
    if(con == 0){        
        readI(fd, &count);
        readI(fd, &my_fd);
        printf("Corpses:%d\n", count);
        //close(fd);
    } else {
        do {
            printf("Host connection failed!\n");
            printf("Trying to connect in 5s ...\n");
            sleep(5);
            
            con = connect(fd, (struct sockaddr*)&sa, sizeof(sa));            
            if(con == 0){
                readI(fd, &count);
                readI(fd, &my_fd);
                //printf("Corpses:%d\n", count);
                //close(fd);
                break;
            }            
        } while(con != 0);     
    }  
    
    return count;
}
*/

/*
int ask2(int fd){
    //int fd = 0, 
    //int con = -1, 
    int count = 0;
    int x = 2;
    writeI(fd, &x);
    //readI(fd, &count);
    //printf("ASK2 Corpses:%d\n", count);
        //close(fd);
    
    
    return count;
}

*/



int main(int argc, char **argv) {
    
    /*    INITIALIZATIONS    */
    int size, rank, len;
	char processor[100];
    long int priority = 0;
    //int corpse = 2;
	MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // ktory watek
    MPI_Comm_size(MPI_COMM_WORLD, &size); // ile watkow
	MPI_Get_processor_name(processor, &len);
    //MPI_Status status;
    
    //DO SERWERA MICHAŁA
    /* 
            
    int count = 0;     
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    //cout << "fd: " << fd << endl;
    addrent=gethostbyname(argv[1]);
    sa.sin_family = PF_INET;
    sa.sin_port = htons(1234);
    memcpy(&sa.sin_addr.s_addr, addrent->h_addr, addrent->h_length);    
    */
    
    //DO MOJEGO SERWERA
    /*
        
    int status;
    int id;
    struct msgbuf message;
    //id = msgget (15071410, 0644 | IPC_CREAT);
    
    
   
    message.type = 1;
    message.cmd = command;
    message.pid = getpid();
    msgsnd(id, &message, sizeof(message) - sizeof(long), 0); 
    
    
    //int msgstatus;
    //msgstatus = msgrcv(id, &message, sizeof(message) - sizeof(long), getpid(), MSG_NOERROR);
    
    */
    
    /*
    //askForCorpseNum(argv[1]);
    //count = askForCorpseNum(fd);
    //printf("OUTSIDE Corpses:%d\n", count);
    //cout << "my fd: " << my_fd << endl;    
    
    //count = ask2(fd);
    */
    
    
    while(1){
    /*  START   */    
        
        priority = getNewPriority();
        printf("%d: my priority: %ld \n", rank, priority);
        
        //printf("My pid : %d z %d na (%s)\n", rank, size, processor);            
        
        //  <pid, priorytet>
        
        addToProcessList(rank, priority);    
        printProcessList(processList);
        
        
        
        while(corpse < 1){
            printf("czekam na pojawienie sie nowych trupów...\n");
            sleep(5);
            /*
                TUTAJ TRZEBA SIE ZAPYTAC SERWER O TO ILE JEST TRUPÓW    
            */
        }
        
        
        /*
        - wysylam wszystkim swoj priorytet
        - odbieram od wszystkich priorytety
        - dodaje ich do listy
        */
        
        int type = 1; 
        long int msg[3];
        msg[0] = (long)rank;
        msg[1] = priority; 
        msg[2] = type;
        
        /*
            0 - release
            1 - request
            2 - batch
        */
        
        
        int msg_count = 0;    
        
        //rozgłaszanie
        for(int i = 0; i < size; i++){
            if(i != rank){
                //wyślij priorytet
                MPI_Send( msg, 3, MPI_LONG_INT, i, MSG_TAG, MPI_COMM_WORLD );
                //printf(" Wyslalem rank: %ld, priority: %ld, type: %ld do %d\n", msg[0], msg[1], msg[2], i );
            }
        }    
        
        
        bool gotCorpse = receiveMessages(msg_count, size, rank, msg);  
        if(gotCorpse)
            continue;
        
        
        if(rank == 0)
            printProcessList(processList);
        
        if(canITakeCorpse(size, rank, corpse)){
            printf("BIORE GO !\n");
            pogrzeb(size, rank);
        } else {
            printf("MUSZE CZEKAC :(\n");
            //czekaj na nowe wiadomosci
            receiveMessages(msg_count, size, rank, msg);  
        }
        
        //printf("My ID: %d, priority: %ld, corpses: %d\n", rank, priority, corpse); 
        //printProcessList(processList);
        
        
        
        
        //processList.erase(processList.begin()+1);
    }
	cout << "koniec pracy procesu " << rank << endl;
	MPI_Finalize();
}


/*

mpiCC grabarz.cpp  -std=c++11 -o g.exe
mpirun -default-hostfile none -np 5 ./g.exe localhost
mpirun -default-hostfile none -np 5 ./a.out 

*/