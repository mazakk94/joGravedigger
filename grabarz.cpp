#include <mpi.h>
#include <netdb.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

#define SIZE 30
#define MAX_SIZE 256
#define MSG_TAG 100

#define readI(fd, x) read(fd, x, sizeof(int))
#define writeI(fd, x) write(fd, x, sizeof(int))


using namespace std;

struct timeval tp;
struct sockaddr_in sa;
struct hostent* addrent;
vector <pair <int, long int>> processList;

int corpse = 0;
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
    return -1;
}

void printProcessList(int rank, vector < pair <int, long int> > processList){
    int i = 0;
    for(pair<int, long int>& process : processList) {
        printf("%d: process[%d]: %d, %ld\n", rank, i, process.first, process.second);        
        i++;
    }
}

void sortProcessList(int rank){
    /*     BUBBLE SORT XD    */
    pair <int, long int> tmp;
    for(unsigned int j = 0; j < processList.size() - 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){            
            if(processList[i].second > processList[i+1].second 
                && processList[i+1].second > 0) { 
                tmp = processList[i+1]; 
                processList[i+1] = processList[i];
                processList[i] = tmp; 
            }
        }          
    }    
    
    for(unsigned int j = 0; j < processList.size()- 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){
            if(processList[i].second == processList[i+1].second && 
                processList[i].first > processList[i+1].first ) {
                tmp = processList[i+1]; 
                processList[i+1] = processList[i];                
                processList[i] = tmp;
            }
        }          
    }        
}

bool canITakeCorpse(int size, int rank, int corpse){
    sortProcessList(rank);
    if(getPosition(rank) < corpse) 
        return true;
    else 
        return false;    
}

long int getNewPriority(){    
    gettimeofday(&tp, NULL);
    long int clock = tp.tv_sec * 1000 + tp.tv_usec / 1000;    
    return clock;
}

int sendRelease(int size, long int rank){
        
    int corpse_taken = getPosition(rank)+1; 
    cout << rank << ": rozsyłam release " << endl;  
    processList.erase (processList.begin(), processList.begin() + corpse_taken);
    corpse -= corpse_taken;    
    return size - corpse_taken; // = msg_count 
}

void receiveMessages(int msg_count, int size, int rank, long int *msg, bool flag){    
    MPI_Status status;    
    while(msg_count != size - 1){                
        MPI_Recv(msg, 3, MPI_LONG_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);         
        if (msg[2] == 1){ //request        
            msg_count++;
            if(getPosition(msg[0]) < 0)
                addToProcessList(msg[0], msg[1]);        
        }
    }   
}

int pogrzeb(int size, int rank){       
       
    int new_msg_count = sendRelease(size, rank);  
    cout << rank << ": WYKONUJE POGRZEB " << endl;
    sleep(5); 
    return new_msg_count;
}

pair<int, int> askForCorpseNum(int rank, int current_corpses){

    pair<int, int> corpses_pair;
    int fd, con = 0;
    struct sockaddr_in sa;
    struct hostent* addrent;
    fd = socket(PF_INET, SOCK_STREAM, 0);
    addrent=gethostbyname("localhost");
    sa.sin_family = PF_INET;
    sa.sin_port = htons(8080);
    memcpy(&sa.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
    con = connect(fd, (struct sockaddr*)&sa, sizeof(sa));
    int corpses = 0;
    if(con == 0){
        int count;
        writeI(fd, &rank);
        readI(fd, &count);
        if(count > current_corpses){
            corpses = count - current_corpses;
            current_corpses = count;
        }
        close(fd);
    }
    close(fd);
    corpses_pair.first = corpses;
    corpses_pair.second = current_corpses;
    return corpses_pair;    
}


int main(int argc, char **argv) {
    
    /*    INITIALIZATIONS    */
    int size, rank, len;
	char processor[100];
    long int priority = 0;
	MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // ktory watek
    MPI_Comm_size(MPI_COMM_WORLD, &size); // ile watkow
	MPI_Get_processor_name(processor, &len);
    
    int current_corpses = 0;
    int msg_count = 0;        
    
    while(1){
    /*  START   */ 
        long int msg[3];        
        int type = 1; 
        
        if(getPosition(rank) < 0){ //nie ma mnie na liście
            priority = getNewPriority();
            addToProcessList(rank, priority);   
            cout << rank << ": pobieram wartość zegara " << priority << endl;

            msg[0] = (long)rank;
            msg[1] = priority; 
            msg[2] = type;  
            //rozgłaszanie
            
            cout << rank << ": rozsyłam priorytety" << endl;
            for(int i = 0; i < size; i++){
                if(i != rank){
                    MPI_Send( msg, 3, MPI_LONG_INT, i, MSG_TAG, MPI_COMM_WORLD );                    
                }
            }            
        }        
        
        cout << rank << ": czekam na trupy " << endl;
        while(corpse < 1){
            pair<int, int> corpses_pair;
            
            
            
            corpses_pair = askForCorpseNum(rank, current_corpses);
            corpse = corpses_pair.first;
            current_corpses = corpses_pair.second;         
        }
        cout << rank << ": doczekałem się " << corpse << " trupów " << endl;
        
        //odbieranie
        int new_rank = rank;
        cout << rank << ": odbieram priorytety" << endl;
        receiveMessages(msg_count, size, rank, msg, 0); 
        rank = new_rank;
        
        cout << rank << ": sprawdzam czy moge brac trupa" << endl;
        if(canITakeCorpse(size, rank, corpse)){
            cout << rank << ": wiem, że moge brać trupa" << endl;
            msg_count = pogrzeb(size, rank);
        } else {
            msg_count = size - 1 - corpse;
            processList.erase (processList.begin(), processList.begin() + corpse);
            corpse = 0;            
        }
    }
	cout << ": koniec pracy procesu " << rank << endl;
	MPI_Finalize();
}


/*
mpirun -default-hostfile none -np 10 ./g.exe
mpiCC grabarz.cpp -Wall  -std=c++11 -o g.exe
g++ serv.cpp -o serv
*/