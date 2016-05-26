#include <stdio.h>
#include <mpi.h>

#include <iostream>
#include <sys/time.h>

using namespace std;


int main(int argc, char **argv)
{
    
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    printf("%ld \n",ms);
    
    
	int size,rank, len;
	char processor[100];
	//sleep(2);
	MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor, &len);

	printf("Hello world: %d of %d na (%s)\n", rank, size, processor);
	//sleep(2);
	MPI_Finalize();
}
