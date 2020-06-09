/*
	HGU 20-1 OS PA4
	21500463 Leekyungmin
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FIFO ".ddtrace"
#define MAXBUF 200
#define LISTSIZE 10
#define LOOP_INFINITE 1
#define true 1
#define false 0
#define bool int

//program functions
void print_intro();
int open_fifo();
int createIndexM(char val[MAXBUF], char list[LISTSIZE][MAXBUF]);
int createIndexT(int val, int* list);
void add_lock(int _index_m, int _index_t);
void rm_lock(int _index_m, int _index_t);
bool hasCycle();
bool isCycle(int num_curr, int _isvisit[LISTSIZE]);
void print_deadlock(int _index_m, char m[MAXBUF], int _index_t, int _t);
//universal functions
int isIn_intArray(int num, int* arr, int size);
bool isEmpty_intArray(int* arr, int size);
void print_intArray(int* arr, int size);
//debug functions
void print_graph(int _graph[LISTSIZE][LISTSIZE] , int size);
void print_intArray(int* arr, int size);
void test();

int num_m = 0; 
char list_m[LISTSIZE][MAXBUF];
int num_t = 0; 
int list_t[LISTSIZE];
int graph[LISTSIZE][LISTSIZE] = {0, };	// cols: node A, rows: node B
int locked[LISTSIZE][LISTSIZE] = {0, };	// cols: thread N, rows: node 

int main ()
{
	print_intro();
	int fd = open_fifo();
	while (LOOP_INFINITE) 
	{
		char buf[MAXBUF];
		char tmp[MAXBUF];
		char m[MAXBUF];
		int len, t, index_m, index_t;
		if ((len = read(fd, buf, MAXBUF)) == -1)
			break ;
		if (len > 0){
			sscanf(buf, "%s %s %*s %d", tmp, m, &t);	
			index_m = createIndexM(m, list_m);
			index_t = createIndexT(t, list_t);
			if(!strcmp("Lock", tmp))
			{
				add_lock(index_m, index_t);
				if(hasCycle())
				{
					print_deadlock(index_m, m, index_t, t);
					exit(0);
				}
			}
			else
			{
				rm_lock(index_m, index_t);
			}	
			// print_graph(graph, 5);
			// print_graph(locked, 5);
		}
	}
	close(fd) ;
	return 0 ;
}

void print_intro()
{
	printf("\n   ------------------------------------------\n");
	printf("\n    Runtime Deadlock Detector \n");
	printf("\n      ** 출력되는 번호들(thread or mutex)은 자체 구분 번호입니다 (타겟프로그램 변수명 등과 관계없음) \n");
	printf("\n      ** 데드락 발생 시, mutex's data address와 thread's tid가 함께 표시됩니다. \n");
	printf("\n   ------------------------------------------\n");
	printf("\n\n    Waiting for deadlock occur... \n\n");
	printf("\n   ------------------------------------------\n");
}

int open_fifo()
{
    if (mkfifo(FIFO, 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}
	int tmp = open(FIFO, O_RDONLY | O_SYNC) ;
	return tmp;
}

int createIndexM(char val[MAXBUF], char list[LISTSIZE][MAXBUF])
{
	int index =0;
	for(int i =0; i<LISTSIZE; i++)
	{
		if(!strcmp(val, list[i]))
		{
			return i+1;
		}
	}
	strcpy(list[num_m], val);
	num_m++;
	index = num_m;
	return index;
}

int createIndexT(int val, int* list)
{
	int index =0;
	if((index=isIn_intArray(val, list, num_t))==-1)
	{
		list[num_t] = val;
		(num_t)++;
		index = num_t;
	}
	return index;
}

void add_lock(int _index_m, int _index_t)
{
	//create node
	if(graph[_index_m-1][_index_m-1]==false)
	{
		graph[_index_m-1][_index_m-1]=_index_t;
		// printf("     Mutex(node) %d create (thread %d)\n", _index_m, _index_t);
	}
	//create edge
	for(int i =0; i<LISTSIZE; i++)
	{
		if(locked[_index_t-1][i]==true)
		{
			graph[i][_index_m-1]=_index_t;
			// printf("     Edge create (node %d -> node %d)\n", i+1, _index_m);
		}
	}
	// printf("\n");
	locked[_index_t-1][_index_m-1]=1;
}

void rm_lock(int _index_m, int _index_t)
{
	for(int i =0; i<LISTSIZE; i++)
	{
		if(graph[i][_index_m-1]==_index_t)
			graph[i][_index_m-1]=0;
		if(graph[_index_m-1][i]==_index_t)
			graph[_index_m-1][i]=0;
		locked[_index_t-1][_index_m-1]=0;
	}
}

bool hasCycle()
{
	for(int i =0; i<LISTSIZE; i++)
	{
		int isvisit[LISTSIZE] = {false, };
		if(isCycle(i, isvisit)==true)
		{
			return true;
		}	
	}
	return false;
}

bool isCycle(int num_curr, int _isvisit[LISTSIZE])
{
	bool result= false;
	for(int i =0; i<LISTSIZE; i++)
	{
		if(i==num_curr)
		{
			continue;
		}
		else if(graph[num_curr][i]>0)
		{
			if(_isvisit[i]==true)
			{
				return true;
			}
			else
			{
				_isvisit[num_curr] = true;
				if((result = isCycle(i, _isvisit))==true)
				{
					break;
				}
			}
			
		}
	}
	return result;
}

void print_deadlock(int _index_m, char _m[MAXBUF], int _index_t, int _t)
{
	printf("\n\n   !!--------------------------------------!!\n\n");
	printf("    Deadlock occur! \n\n");
	printf("      - Cycle is made after Thread %d (tid: %d) lock Mutex %d (address: %s)\n\n", _index_t, _t, _index_m, _m);
	printf("      - All involved threads and mutexs are below (currently locked) \n\n");
	for(int i =0; i<LISTSIZE; i++)
	{
		if(!isEmpty_intArray(locked[i], LISTSIZE))
			printf("        * Thread %d (tid: %d)\n", i+1, list_t[i]);
		for(int j=0; j<LISTSIZE; j++)
		{
			if(locked[i][j]==true)
			{
				printf("          - mutex %d (address: %s)\n", j+1, list_m[j]);
			}
		}
	}
	printf("\n");
	printf("      - Checker program will be terminated \n");
	printf("\n   !!--------------------------------------!!\n\n\n");
}

void print_graph(int _graph[LISTSIZE][LISTSIZE] , int size)
{
	for(int i =0; i<size; i++)
	{
		printf("   ");
		for(int j =0; j<size; j++)
		{
			printf("%d - ", _graph[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

int isIn_intArray(int num, int* arr, int size)
{
	for(int i =0; i<size; i++)
	{
		if(num==arr[i])
		{
			return i+1;
		}
		else{}
	}
	return -1;
}

bool isEmpty_intArray(int* arr, int size)
{
	for(int i=0; i<size; i++)
	{
		if(arr[i]!=0)
			return false;
	}
	return true;
}

void print_intArray(int* arr, int size)
{
  printf("\n   ");
  for(int i =0; i< size; i++)
  {
    printf("%d-", arr[i]);
  }
  printf("\n\n");
}

void test()
{
  printf("\n\n----TEST----\n\n");
}
