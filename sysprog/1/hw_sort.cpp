#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include <stdbool.h>
#include <cstdlib>
#include <fcntl.h>

#define check_resched_without_time {    \
	int old_i = curT;				    \
	curT = (curT + 1) % task_count;	    \
	if (setjmp(tasks[old_i].env) == 0)  \
		longjmp(tasks[curT].env, 1);};

#define check_resched {						                      \
	/*							                                  \
	 * For an example: just resched after each line, without      \
	 * timeslices.						                          \
	 */                                                           \
	tasks[curT].elapsed_time += clock() - tasks[curT].start_time; \
	int old_i = curT;				                              \
	curT = (curT + 1) % task_count;	                              \
	if (setjmp(tasks[old_i].env) == 0)			                  \
		longjmp(tasks[curT].env, 1);                              \
    tasks[curT].start_time = clock();}


struct task {
        /**
         * Important thing - execution position where that
         * coroutine stopped last time.
         */
        jmp_buf env;
        /**
         * This flag is set when the coroutine has finished its
         * task. It is used to wait until all the coroutines are
         * finished.
         */
        bool is_finished;
	    /**
	     * Just local variables for arbitrary usage. You can put
	     * here whatever you want.
	     */
	    long elapsed_time;
	    long start_time;
	    int *arr;
	    int *stack;
	    int top;
	    int l;
	    int h;
	    int x;
	    int i;
	    int j;
	    int p;
	    int t;
	    int N;
};

static int task_count = 0;
static int curT = 0;
static struct task *tasks;

static void
q_sort_coro()
{
    tasks[curT].start_time = clock();
    check_resched
    // Keep popping from stack while is not empty 
    while ( tasks[curT].top >= 0 )
    {
        check_resched
        // Pop h and l
        tasks[curT].h = tasks[curT].stack[ tasks[curT].top-- ];
        check_resched
        tasks[curT].l = tasks[curT].stack[ tasks[curT].top-- ];
        check_resched
        // Set pivot element at its correct position 
        // in sorted array
        tasks[curT].x = tasks[curT].arr[tasks[curT].h];
        check_resched
        tasks[curT].i = (tasks[curT].l - 1);
        check_resched

        tasks[curT].j = tasks[curT].l;
        check_resched
        while (tasks[curT].j <= tasks[curT].h- 1)
        {
            check_resched
            if (tasks[curT].arr[tasks[curT].j] <= tasks[curT].x)
            {
                check_resched
                tasks[curT].i++;
                check_resched
                tasks[curT].t = tasks[curT].arr[ tasks[curT].i ];
                check_resched
                tasks[curT].arr[tasks[curT].i] = tasks[curT].arr[tasks[curT].j];
                check_resched
                tasks[curT].arr[tasks[curT].j] = tasks[curT].t;
                check_resched
            }
            tasks[curT].j++;
            check_resched
        }
        tasks[curT].t = tasks[curT].arr[tasks[curT].i + 1];
        check_resched
        tasks[curT].arr[tasks[curT].i + 1] = tasks[curT].arr[tasks[curT].h];
        check_resched
        tasks[curT].arr[tasks[curT].h] = tasks[curT].t;
        check_resched
        tasks[curT].p = tasks[curT].i + 1;
        check_resched
  
        // If there are elements on left side of pivot, 
        // then push left side to stack 
        if ( tasks[curT].p-1 > tasks[curT].l )
        {
            check_resched
            tasks[curT].top++;
            check_resched
            tasks[curT].stack[ tasks[curT].top ] = tasks[curT].l;
            check_resched
            tasks[curT].top++;
            check_resched
            tasks[curT].stack[ tasks[curT].top ] = tasks[curT].p - 1;
            check_resched
        } 
  
        // If there are elements on right side of pivot, 
        // then push right side to stack 
        if ( tasks[curT].p+1 < tasks[curT].h )
        {
            check_resched
            tasks[curT].top++;
            check_resched
            tasks[curT].stack[ tasks[curT].top ] = tasks[curT].p + 1;
            check_resched
            tasks[curT].top++;
            check_resched
            tasks[curT].stack[ tasks[curT].top ] = tasks[curT].h;
            check_resched
        } 
    }
    tasks[curT].is_finished = true;

    while (true) {
        bool is_all_finished = true;
        for (int i = 0; i < task_count; ++i) {
            if (! tasks[i].is_finished) {
//                printf("Task_id=%d still active, "\
//					"re-scheduling\n", i);
                is_all_finished = false;
                break;
            }
        }
        if (is_all_finished) {
            printf("No more active tasks to schedule.\n");
            return;
        }
        check_resched_without_time;
    }
}

// A utility function to print contents of arr
void
printArr( int *arr, int n )
{
    int i;
    for ( i = 0; i < n; ++i )
        printf( "%d ", arr[i] );
    printf("\n");
}

void
merge(int k, task *arrs)
{
    if (k == 1){
        return;
    }

    int a = 0, b = 0;
    int *tmp = (int*)calloc(arrs[1].N + arrs[0].N, sizeof(int));
    int i = 0;
    while(a <= arrs[0].N - 1 && b <= arrs[1].N - 1)
    {
        if(arrs[0].arr[a] <= arrs[1].arr[b])
        {
            tmp[i] = arrs[0].arr[a];
            a++;
        } else {
            tmp[i] = arrs[1].arr[b];
            b++;
        }
        i++;
    }
    if(a == arrs[0].N)
    {
        while(b <= arrs[1].N - 1)
        {
            tmp[i] = arrs[1].arr[b];
            b++;
            i++;
        }
    }
    else {
        if (b == arrs[1].N) {
            while (a <= arrs[0].N - 1) {
                tmp[i] = arrs[0].arr[a];
                a++;
                i++;
            }
        }
    }

    free(arrs[1].arr);
    free(arrs[0].arr);
    int tmpN = arrs[1].N + arrs[0].N;


    arrs[1].arr = arrs[k-1].arr;
    arrs[1].N = arrs[k-1].N;

    arrs[k-1].arr = tmp;
    arrs[k-1].N = tmpN;



    merge(k-1, arrs+1);
}


int
main(int argc, char *argv[])
{
    long start_time = clock();
    if (argc < 2) 
    {
        fprintf(stderr, "Использование: %s <имя_файла> <имя_файла>...\n",
                argv[0]);
        _exit(1);
    }

    task_count = argc - 1;
    tasks = (task*)calloc(task_count, sizeof(struct task));

    long now = clock();
    long delta = 0;

    FILE *fp;
    int N_all = 0;
    for(int i = 0; i < task_count; i++)
    {
        long start_delta = clock();
        fp=fopen(argv[i+1], "rt");
        if(fp == nullptr)
        {
            printf("Cannot open file %s.", argv[i+1]);
            fflush(stdout);
            return 1;
        }

        int  N;
        fscanf(fp, "%d", &N);
        N_all += N;
        int *arr = (int*)calloc(N, sizeof(int));
        int *stack = (int*)calloc(N, sizeof(int));
        int num;
        for( int j = 0; j < N; j++)
        {
            fscanf(fp, "%d", &num);
            arr[j]=num;
            stack[j]=0;
        }
        // qsort initialization
        int top = -1;
        // push initial values of l and h to stack
        stack[ ++top ] = 0; // l
        stack[ ++top ] = N-1; // h

        task tmp;


        tmp.is_finished = false;
        tmp.arr = arr;
        tmp.stack = stack;
        tmp.elapsed_time = 0;
        tmp.top = top;    // int top;
        tmp.l = 0;        // int l;
        tmp.h = N-1;      // int h;
        tmp.x = 0;        // int x;
        tmp.i = 0;        // int i;
        tmp.j = 0;        // int j;
        tmp.p = 0;        // int p;
        tmp.t = 0;        // int t;
        tmp.N = N;        // int N;

        tasks[i] = tmp;
        //printf("%d : ", i);
        //printArr(tasks[i].arr,15);
        delta += clock() - start_delta;
        setjmp(tasks[i].env);
    }


    q_sort_coro();
    long coro_elaps = clock() - now;


//    for(int i = 0; i < task_count; i++)
//    {
//        printf("sorted %d : ", i);
//        printArr(tasks[i].arr, tasks[i].N);
//    }

    merge(task_count, tasks);

//    printf("out : ");
//    printArr(tasks[task_count - 1].arr, N_all);
//    printf("\nTimes:\n\n");



    for(int i = 0; i < task_count; i++)
    {
        printf("coro %d work time = %ldms\n", i, tasks[i].elapsed_time);
    }
    printf("\nmeasured coros time    = %ldms\n", coro_elaps-delta);

    fflush(stdout);
    long whole_time = clock() - start_time;
    printf("\n\nwhole time = %ldms\n", whole_time);
}

