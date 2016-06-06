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


#define readI(fd, x) read(fd, x, sizeof(int))
#define readF(fd, x) read(fd, x, sizeof(float))
#define writeI(fd, x) write(fd, x, sizeof(int))
#define writeF(fd, x) write(fd, x, sizeof(float))


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
    //return processList.size()-1;
    return -1;
}

void printProcessList(int rank, vector < pair <int, long int> > processList){
    int i = 0;
    //if(processList.size() > 3)
    for(pair<int, long int>& process : processList) {
        printf("%d: process[%d]: %d, %ld\n", rank, i, process.first, process.second);        
        i++;
    }
}

void sortProcessList(int rank){
    /*     BUBBLE SORT XD    */
    //cout << rank << " : SORT PRZED" << endl;
    //printProcessList(rank, processList);
    pair <int, long int> tmp;
    for(unsigned int j = 0; j < processList.size() - 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){            
            if(processList[i].second > processList[i+1].second 
                && processList[i+1].second > 0) {  
                //if(rank == 1) printf("PRZED: i %d j %d\np[i].1: %d, p[i+1].1 %d\np[i].2  %ld p[i+1].2 %ld\n", i,j, processList[i].first, processList[i+1].first, processList[i].second, processList[i+1].second);
                tmp = processList[i+1]; 
                //if(rank == 1 ) printf("TMP first: %d, second: %ld\n",  tmp.first, tmp.second);
                         
                processList[i+1] = processList[i];
                
                processList[i] = tmp; 
                //if(rank == 1) printf("i first: %d, second: %ld\n", processList[i].first, processList[i].second); 
                //if(rank == 1) printf("PO\np[i].1: %d, p[i+1].1 %d\np[i].2  %ld p[i+1].2 %ld\n",  processList[i].first, processList[i+1].first, processList[i].second, processList[i+1].second);
            }
        }          
    }    
    
    for(unsigned int j = 0; j < processList.size()- 1; j++){
        for(unsigned int i = 0; i < processList.size() - 1; i++){
            if(processList[i].second == processList[i+1].second && 
                processList[i].first > processList[i+1].first ) {
                //if(rank == 1) printf("%d %d\n", i,j);
                //if(rank == 1) printf("PRZED: i %d j %d\np[i].1: %d, p[i+1].1 %d\np[i].2  %ld p[i+1].2 %ld\n", i,j, processList[i].first, processList[i+1].first, processList[i].second, processList[i+1].second);
                
                tmp = processList[i+1]; 
                //if(rank == 1 ) printf("TMP first: %d, second: %ld\n",  tmp.first, tmp.second);
                         
                processList[i+1] = processList[i];
                //if(rank == 1) printf("i+1 first: %d, second: %ld\n", processList[i+1].first, processList[i+1].second);
                processList[i] = tmp; 
                //if(rank == 1) printf("i first: %d, second: %ld\n", processList[i].first, processList[i].second);  
                //if(rank == 1) printf("PO\np[i].1: %d, p[i+1].1 %d\np[i].2  %ld p[i+1].2 %ld\n",  processList[i].first, processList[i+1].first, processList[i].second, processList[i+1].second);
            }
        }          
    }        
   
   cout << rank << " : SORT PO" << endl;
   printProcessList(rank, processList);
   
}

bool canITakeCorpse(int size, int rank, int corpse){
    sortProcessList(rank);
    int diff = 0;//size - processList.size(); // jak nie mamy odpowiedzi od wszystkich to zakladamy, że
    cout << rank << ": position: " << getPosition(rank) << " corpse: " << corpse << endl;
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

int sendRelease(int size, long int rank){
    /*
    //long int msg[3];
    //msg[0] = rank;
    //msg[1] = -1;
    //msg[2] = 0;
    for(int i = 0; i < size; i++){
        if(i != rank){
            //wyślij priorytet
            //MPI_Send( msg, 3, MPI_LONG_INT, i, MSG_TAG, MPI_COMM_WORLD );
            //printf(" %ld: Wyslalem release: %ld, priority: %ld, type: %ld do %d\n", rank, msg[0], msg[1], msg[2], i );
        }
    }*/
    
    int corpse_taken = getPosition(rank)+1; //CHYBA +1
    cout << "RELEASE PRZED:" << endl;
    printProcessList(rank, processList);
    processList.erase (processList.begin(), processList.begin() + corpse_taken);
    corpse -= corpse_taken;
    cout << rank << " : corpse -= corpse_taken = " << corpse << endl;
    cout << "RELEASE PO:" << endl;
    printProcessList(rank, processList);
    
    return size - corpse_taken; // = msg_count 
}

void receiveMessages(int msg_count, int size, int rank, long int *msg, bool flag){
    MPI_Status status;
    
    //int additionalCounter = 0;
    while(msg_count != size - 1){
        cout << rank << " : recv msg 1" << msg_count << endl;
        //if(additionalCounter == size)
        //    break;
        
        cout << rank << " : recv msg 2" << msg_count << endl;
        if( MPI_Recv(msg, 3, MPI_LONG_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status) != 0){
            cout << "\t\t\t\tBREAK!!****" << endl;
            break;
        }
        //int recv = MPI_Recv(msg, 3, MPI_LONG_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        //cout << rank << " MPI_RECV count: "/* << status.count*/ << " recv: " << recv << endl;
        cout << rank << " : recv msg 3" << msg_count << endl;
        printf(" %d: Otrzymalem rank: %ld, priority: %ld, type: %ld od %d\n", rank, msg[0], msg[1], msg[2], status.MPI_SOURCE);
        
        sleep(2);   
        /*
        if(msg[2] == 0){ //release
            corpse--;   
            //msg_count--;
            printf("\t%d: usuwam proces nr ", rank);
            cout << getPosition(msg[0]) << endl;
            
            
            //cout << rank <<  " : LISTA PRZED RCV RLS" << endl;
            //printProcessList(rank, processList);
            if(getPosition(msg[0]) == -1){
                cout << rank << " : nie znalazlem!" << endl;
            } else {
                processList.erase(processList.begin()+getPosition(msg[0])); 
                cout << rank << " : usunalem" << getPosition(msg[0]) << endl;
                
            }
            
            cout << rank <<  " : LISTA PO RCV RLS" << endl;
            printProcessList(rank, processList);
            
            //usun z listy proces który wykonał pogrzeb
        } else */
        if (msg[2] == 1){ //request        
            msg_count++;
            if(getPosition(msg[0]) < 0)
                addToProcessList(msg[0], msg[1]);  
            cout << rank << " : dodaje:" << msg[0] << " " << msg[1] << endl;
        /*
        //printf("%d : LISTA W RCV 1\n", rank);
        //printProcessList(rank, processList);
        
            if (canITakeCorpse(size, rank, corpse)){
                printf("%d: BIORE GO SZYBCIEJ!\n", rank);
                //goto potrzeb
                //printf("BIORE GO !\n");
                
                
                return true;  //jak 1 to jump do startu
                
            }*/            
        } else { // msg[2] == 2, czyli batch
            //corpse += (int)msg[1];
            cout << "ELSE " << rank << ": " << corpse << endl;
        }
        //if(flag)
        //    additionalCounter++;
    }   
    cout << rank << " : WYCHODZE Z RECV" << endl;
}

int pogrzeb(int size, int rank){
    
    cout << " " << rank << " :PRZEBIEGA POGRZEB JEDNEGO Z TRUPÓW...\n";
    //corpse--;      
    sleep(5);    
    //cout << rank << "KONIEC POGRZEBU...\n";
    int new_msg_count = sendRelease(size, rank); 
    
    cout << rank << "KONIEC POGRZEBU...\n";
    
    printf("%d corpse: %d\n", rank, corpse);
    //receiveMessages(6,  size,  rank, msg, 1);
    return new_msg_count;
}


int askForCorpseNum(int rank, char *adres){
    int fd = 0;
    int con = 0;
    struct sockaddr_in sa;
    struct hostent* addrent;
    int count = -1;
    
    while(1){
		
		fd = socket(PF_INET, SOCK_STREAM, 0);
		addrent=gethostbyname(adres);
		sa.sin_family = PF_INET;
		sa.sin_port = htons(8080);
		memcpy(&sa.sin_addr.s_addr, addrent->h_addr, addrent->h_length);
		con = connect(fd, (struct sockaddr*)&sa, sizeof(sa));
		if(con == 0){
            writeI(fd, &rank);			
			readI(fd, &count);
			printf("Corpses:%d\n", count);
			close(fd);
			break;
		}
		close(fd);
	}
    if(count == -1){
        cout << rank << " : brak wartosci z serwera" << endl;
        count = 0;
    }
    
    return count;
}


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
     
   // char adres[] = "localhost";
    
    bool start = true;    
    int msg_count = 0;
    
    while(1){
    /*  START   */ 

        long int msg[3];        
        int type = 1; 
        
    
        if(!start){
            cout << rank << " : MSG COUNT = " << msg_count << endl; 
        }
        
        if(getPosition(rank) < 0){ //nie ma mnie na liście
            priority = getNewPriority();
            printf("%d: my priority: %ld \n", rank, priority);
            addToProcessList(rank, priority);   

            msg[0] = (long)rank;
            msg[1] = priority; 
            msg[2] = type;  
            //rozgłaszanie
            for(int i = 0; i < size; i++){
                if(i != rank){
                    //wyślij priorytet
                    MPI_Send( msg, 3, MPI_LONG_INT, i, MSG_TAG, MPI_COMM_WORLD );
                    printf("%d: Wyslalem rank: %ld, priority: %ld, type: %ld do %d\n", rank, msg[0], msg[1], msg[2], i );
                }
            }
            
        }
        
        
        printProcessList(rank, processList);
        
        
        
        
              
        
        while(corpse < 1){
            printf("%d : czekam na pojawienie sie nowych trupów...\n", rank);
            sleep(5);
            //corpse = askForCorpseNum(rank, adres);
            //receiveMessages(msg_count, size, rank, msg, 0);
            
        }
        
        
        /*
        - wysylam wszystkim swoj priorytet
        - odbieram od wszystkich priorytety
        - dodaje ich do listy
        */
        
        
        
        /*
            0 - release
            1 - request
            2 - batch
        */
        
        
        
        
        
        
        
        //odbieranie
        int new_rank = rank;        
        cout << rank << " : GOTCORPSE1: " << endl;
        receiveMessages(msg_count, size, rank, msg, 0); 
        printf("%d %d GOTCORPSE2\n", rank, new_rank);
        rank = new_rank;
        
        //zeruje count
        /*
        if(gotCorpse){            
            //cout << rank << " : LISTA PRZED" << endl;
            //printProcessList(rank, processList);      
            printf("GOTCORPSE: %d\n", rank);
            pogrzeb(size, rank);
            receiveMessages(msg_count, size, rank, msg, 0);  
            cout << rank << " : LISTA PO" << endl;
            printProcessList(rank, processList);
            
            continue;
        }*/
        
            //odbierz msg
            
        
        
        //if(rank == 1)
        //printProcessList(rank, processList);
        
        printf("%d WYSZEDLEM Z RECEIVE, count: %d\n", rank, msg_count);
        if(canITakeCorpse(size, rank, corpse)){
            printf("%d : BIORE GO !\n", rank);
            //cout << rank << " : LISTA PRZED" << endl;
            //printProcessList(rank, processList);
            //printf("CANITAKECORPSE: %d\n", rank);
            msg_count = pogrzeb(size, rank);
            cout << rank << " : new_msg_count" << msg_count << endl;
            //receiveMessages(msg_count, size, rank, msg, 0); 
            //cout << rank << " : LISTA PO" << endl;
            //printProcessList(rank, processList);
            //
            
            //cout << "LISTA PO" << endl;
            //printProcessList(rank, processList);
        } else {
            printf("%d MUSZE CZEKAC :(\n", rank);
            //czekaj na nowe wiadomosci
            msg_count = size - 1 - corpse;
            processList.erase (processList.begin(), processList.begin() + corpse);
            cout << rank << " : musze_czekac msg_count" << msg_count << endl;
            printProcessList(rank, processList);
            corpse = 0;            
            //receiveMessages(msg_count, size, rank, msg, 0);  
        }
        
        //printf("My ID: %d, priority: %ld, corpses: %d\n", rank, priority, corpse); 
        //printProcessList(rank, processList);
        
        
        sleep(5);
        
        //processList.erase(processList.begin()+1);
        start = false;
    }
	cout << "koniec pracy procesu " << rank << endl;
	MPI_Finalize();
}


/*

mpiCC grabarz.cpp  -std=c++11 -o g.exe
mpirun -default-hostfile none -np 5 ./g.exe localhost
mpirun -default-hostfile none -np 5 ./a.out 

*/