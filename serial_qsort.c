#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAXLEN 100000000
// Global Array
long a[MAXLEN];
int counter =0;

/* Function Prototypes */
double genrand_res53(void);
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], int key_length);
static void next_state(void);
unsigned long genrand_int32(void);
long genrand_int31(void);	
	
	
/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UMASK 0x80000000UL /* most significant w-r bits */
#define LMASK 0x7fffffffUL /* least significant r bits */
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v) ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

static unsigned long state[N]; /* the array for the state vector  */
static int left = 1;
static int initf = 0;
static unsigned long *next;

/* initializes state[N] with a seed */
void init_genrand(unsigned long s)
{
    int j;
    state[0]= s & 0xffffffffUL;
    for (j=1; j<N; j++) {
        state[j] = (1812433253UL * (state[j-1] ^ (state[j-1] >> 30)) + j); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array state[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        state[j] &= 0xffffffffUL;  /* for >32 bit machines */
    }
    left = 1; initf = 1;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        state[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { state[0] = state[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        state[i] = (state[i] ^ ((state[i-1] ^ (state[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        state[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { state[0] = state[N-1]; i=1; }
    }

    state[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
    left = 1; initf = 1;
}

static void next_state(void)
{
    unsigned long *p=state;
    int j;

    /* if init_genrand() has not been called, */
    /* a default initial seed is used         */
    if (initf==0) init_genrand(5489UL);

    left = N;
    next = state;
    
    for (j=N-M+1; --j; p++) 
        *p = p[M] ^ TWIST(p[0], p[1]);

    for (j=M; --j; p++) 
        *p = p[M-N] ^ TWIST(p[0], p[1]);

    *p = p[M-N] ^ TWIST(p[0], state[0]);
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;

    if (--left == 0) next_state();
    y = *next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    unsigned long y;

    if (--left == 0) next_state();
    y = *next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return (long)(y>>1);
}



void print() {
	int i;
  for ( i = 0; i < MAXLEN; i++) {
   	printf("%ld \n", a[i]);
  }
  printf("\n");
}

/* Partition the array in < ad > subarrays for the pivot */
int partition( int start_idx, int end_idx) {
// chose the Pivot as the last element in the array.. change selection strategy	
  int pivot = a[ end_idx ];		
  int j = start_idx - 1;
  int i = 0;

//printf("partition(): Pivot = %d\n", pivot);	
  for ( i = start_idx; i < end_idx; i++) {

    if (pivot >= a[i]) {
      j = j + 1;
      int temp = a[j];
      a[j] = a[i];
      a[i] = temp;
    }
  }
  a[ end_idx ] = a[j + 1];
  a[j + 1] = pivot;

  return (j + 1);
}

void serial_quickSort(int start_idx, int end_idx ) {
  if (start_idx < end_idx) {		// When they crossover its time to stop
	
	
    int q = partition( start_idx, end_idx);
	// print out the partitional subarrays at each step 
	int i =0;
//	for( i=0; i < 10; i++ )
//		printf(" %d ", a[i]);
//	printf(" ***** quicksort(): Pivot Position in array ==> %d\n", q);	
//	printf("\n");
	counter += 2;
	//printf("The counter is --> %d\n", counter);
	serial_quickSort( start_idx, q - 1);		/* Call the new subarray that is less than pivot */
    serial_quickSort( q + 1, end_idx );		/* Call the new subarray that is greater than pivot */
  }
}

int main(int argc, char *argv[]) {
   clock_t startclock, stopclock;
int i=0;
//assign random values to array
  	for (i=0; i< MAXLEN; i++)
		a[i] = genrand_int31();

	startclock = clock();
   	serial_quickSort(0, MAXLEN-1);
 	stopclock = clock();

   fprintf(stderr, "%s%.2f%s\n", "Time: ",
            (stopclock-startclock) / (float) CLOCKS_PER_SEC,
            " sec.");  
// print();
  
}
