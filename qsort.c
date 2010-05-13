/**********************************************************************************
**	Filename 		: qsort.c
**	Authors 		: Manu Kaul and Ahmad Bijairimi 
**  Last Modified	: Tuesday, 4 March 2010
**  
**  Description		: Parallel Quick Sort using Pthreads	
**
**********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXLEN 		100000000       // Max length of array to sort
#define MAXDEPTH	6       		// Recursion Tree Depth

/*
 * Wirth Macro 
 */
#define ELEM_SWAP(a,b) { register elem_type t=(a);(a)=(b);(b)=t; }

//double buf[MAXLEN];             // Array to be sorted
double *buf;

int thread_count = 1;           // Thread Counts to get the log2N
                                        // depth correct
int real_thread_count = 1;      // Actual threads spawned

/*
 * Type defintion for Wirth Median 
 */
typedef double elem_type;
pthread_mutex_t mutexthread;

/*
 * Function Declarations 
 */
int partition(int, int);
int partition_new(int, int, int);
void serial_quickSort(int, int);
int floor_log2(int);
void print();

/*
 * Thread arguments sent via this struct 
 */
struct thread_data {
    int st_idx;
    int en_idx;
};

/*****************************************************
	Wirth Algorithm Implementation to compute Median
*/
elem_type kth_smallest(elem_type a[], int n, int k)
{
    register i, j, l, m;
    register elem_type x;
    l = 0;
    m = n - 1;
    while (l < m) {
        x = a[k];
        i = l;
        j = m;
        do {
            while (a[i] < x)
                i++;
            while (x < a[j])
                j--;
            if (i <= j) {
                ELEM_SWAP(a[i], a[j]);
                i++;
                j--;
            }
        } while (i <= j);
        if (j < k)
            l = i;
        if (k < i)
            m = j;
    }
    return a[k];
}

#define median_low(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2)-1)) )
#define median_high(a,n) kth_smallest(a,n,(((n)&1)?((n)/2):(((n)/2))))


/*****************************************************
/* Calculate floor(Log_2)  */
int floor_log2(int n)
{
    unsigned int temp = (unsigned) n - 1;
    int log2n = 0;
    /*
     * Right Shift by n times to get how many times raised to 2 = log to
     * base 2 
     */
    for (log2n = 0; temp > 0; log2n++, temp >>= 1);
    return log2n - 1;
}

/*****************************************************
/* Select the Pivot Element .
	Strategy 1: Select a random sample of size O(logN) from the array and compute median 
				log to base 2 N is a good sampling choice as it scales well for smaller 
				and larger data sets alike
*/
int select_pivot(int low_idx, int high_idx)
{
    int i = 0;
    int N = floor_log2(high_idx - low_idx + 1); 		 
    int *random_indexes = calloc(N, sizeof(int));        
    // array
    double *random_elements = calloc(N, sizeof(double));        
    // array
    double final_median = 0;

    srand(time(NULL));          // Seed the RNG
    for (i = 0; i < N; i++) {
        random_indexes[i] = rand() % (high_idx - low_idx + 1) + low_idx;
        random_elements[i] = buf[random_indexes[i]];
    }
    final_median =
        (median_low(random_elements, N) +
         median_high(random_elements, N)) / 2;

    // Free allocated memory 
    free(random_indexes);
    free(random_elements);

    return final_median;
}



/*****************************************************
* Thread function that will attempt to sort locally
*/
void *parallel_qsort(void *arg_struct)
{
    int st_idx, en_idx, rc;
    double median = 0;
    struct thread_data index_left, index_right;
    void *status;

    pthread_t first_thread;
    pthread_t second_thread;

    /*
     * Extract the thread args from the structure passed in 
     */
    struct thread_data *args;
    args = (struct thread_data *) arg_struct;
    st_idx = args->st_idx;
    en_idx = args->en_idx;

    if (en_idx > st_idx) {

        int N = en_idx - st_idx + 1;    // Get number of
        // elements in
        // this array

        if (floor_log2(thread_count) < MAXDEPTH && N > floor_log2(MAXLEN)) {    // If 

            /*
             * 1. Pivot Selection Strategy 
             */
            median = select_pivot(st_idx, en_idx);
            // printf("Median chosen ---> %ld\n", median);

            /*
             * 2. Partition the data into subarrays lesser and greater
             * than pivot 
             */
            int pivot_index = partition_new(st_idx, en_idx, median);    // partition_new( 

            /*
             * Update the thread counter 
             */
            pthread_mutex_lock(&mutexthread);   /* Lock Mutex */
            thread_count += 2;
            ++real_thread_count;
            pthread_mutex_unlock(&mutexthread); /* Unlock Mutex */

            /*
             * Fire the thread to sort the lesser than sub-array 
             */
            index_left.st_idx = st_idx;
            index_left.en_idx = pivot_index - 1;
            /*
             * Re-use current thread to sort the greater array ... so no
             * new thread creation 
             */
            index_right.st_idx = pivot_index + 1;
            index_right.en_idx = en_idx;
            // printf("****** Parallel Sorting now (%d,%d) and (%d,%d) N = 
            // %d depth = %d\n", st_idx, pivot_index - 1, pivot_index + 1, 
            // en_idx, N, floor_log2( thread_count ) );

            /*
             * Now we have called a new thread to sort the lesser array 
             */
            rc = pthread_create(&first_thread, NULL, parallel_qsort,
                                (void *) &index_left);

            rc = pthread_create(&second_thread, NULL, parallel_qsort,
                                (void *) &index_right);
            pthread_join(first_thread, NULL);
            pthread_join(second_thread, NULL);
        } else {
            // printf("****** Serial Sorting now (%d,%d) N = %d depth =
            // %d\n", st_idx, en_idx, N, floor_log2( thread_count ) );
            serial_quickSort(st_idx, en_idx);   // Sort Serially the list 
        }

        /*
         * void *pquick(void *arg){ ... pthread_t leftthr,rightthr; ...
         * 
         * if (right > left){ if (recursionlev<MAXLEVELS) { ...
         * pthread_create(&leftthr, NULL, pquick, (void *) &leftarg);
         * pthread_create(&rightthr, NULL, pquick, (void *) &rightarg);
         * pthread_join(leftthr, NULL); pthread_join(rightthr, NULL); }
         * else { quicksort(left); // Serial quicksort quicksort(right); }
         * 
         * } }
         */
    }                           // End of main if 

    /*
     * And... EXIT 
     */
    pthread_exit(NULL);
}



/******************************/
int main(int argc, char *argv[])
{

    pthread_t first_thread;
    pthread_attr_t attr;

    struct thread_data index_main;
    void *status;
    int i, ttime, rc;

	/* Allocate buffer space */
	buf = (double *)malloc(MAXLEN * sizeof(double));

    /*
     * Explicitly set the pthreads to be joinable 
     */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&mutexthread, NULL);

    /*
     * Populate the array with random double numbers 
     */
    for (i = 0; i < MAXLEN; i++)
        buf[i] = genrand_int31();

    /*
     * Initialize and set thread detached attribute 
     */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // ttime=timer();
    clock_t startclock, stopclock;

    startclock = clock();

    /*
     * Fire the first thread 
     */
    index_main.st_idx = 0;
    index_main.en_idx = MAXLEN - 1;
    rc = pthread_create(&first_thread, &attr, parallel_qsort,
                        (void *) &index_main);
    pthread_join(first_thread, &status);

    // ttime=timer() - ttime;

    stopclock = clock();

    // printf("Elapsed time: %f \n",ttime/1000000.0);
    fprintf(stderr, "%s%.2f%s\n", "Time: ",
            (stopclock - startclock) / (float) CLOCKS_PER_SEC, " sec.");

    printf
        ("Total Actual Thread Count = <%d>, Recursion Tree Branches = <%d>\n",
         real_thread_count, thread_count);

    printf("After\n");
    (MAXLEN <= 100) && (print(), 0);    // Print out only to test 
    pthread_mutex_destroy(&mutexthread);
    pthread_attr_destroy(&attr);
	free(buf);
	
    pthread_exit(NULL);
}

/******************************/
/*
 * Partition the incoming array in < and > subarrays for the pivot 
 */
int partition(int start_idx, int end_idx)
{
    // chose the Pivot as the last element in the array.. change selection 
    // strategy 
    int pivot = buf[end_idx];
    int j = start_idx - 1;
    int i = 0;


    // printf("partition(): Pivot = %d\n", pivot); 
    for (i = start_idx; i < end_idx; i++) {

        if (pivot >= buf[i]) {
            j = j + 1;
            //int temp = buf[j];
            //buf[j] = buf[i];
            //buf[i] = temp;
			ELEM_SWAP(buf[i], buf[j])
        }
    }
    buf[end_idx] = buf[j + 1];
    buf[j + 1] = pivot;

    return (j + 1);
}


/******************************/
/*
 * Partition the incoming array in < and > subarrays for the pivot 
 */
int partition_new(int start_idx, int end_idx, int median)
{
    // chose the Pivot as the last element in the array.. change selection 
    // strategy 
    int pivot = median;
    int j = start_idx - 1;
    int i = 0;

    // printf("partition(): Pivot = %d\n", pivot); 
    for (i = start_idx; i < end_idx; i++) {

        if (pivot >= buf[i]) {
            j = j + 1;
            int temp = buf[j];
            buf[j] = buf[i];
            buf[i] = temp;
			//ELEM_SWAP(buf[i], buf[j])
        }
    }
    buf[end_idx] = buf[j + 1];	
    buf[j + 1] = pivot;
    return (j + 1);
}


/******************************/
void serial_quickSort(int start_idx, int end_idx)
{
    if (start_idx < end_idx) {  // When they crossover its time to stop
        int q = partition(start_idx, end_idx);
        int i = 0;
        serial_quickSort(start_idx, q - 1);     /* Call the new subarray
                                                 * that is less than pivot 
                                                 */
        serial_quickSort(q + 1, end_idx);       /* Call the new subarray
                                                 * that is greater than
                                                 * pivot */
    }
}

void print()
{
    int i;
    for (i = 0; i < MAXLEN; i++) {
        printf("%f \n", buf[i]);
    }
    printf("\n");
}
