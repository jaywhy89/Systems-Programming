
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Team Herbert!!!", /* Team name */

    "Joo Young Kang", /* First member full name */
    "995903330", /* First member student number */
    "justin.kang@mail.utoronto.ca", /* First member email address */

    "He Zhang", /* Second member full name */
    "1000347546", /* Second member student number */
    "heherbert.zhang@mail.utoronto.ca" /* Second member email address */
};


unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
    unsigned my_key;
public:
    sample *next;
    unsigned count;

    sample(unsigned the_key) {
        my_key = the_key;
        count = 0;
    };

    unsigned key() {
        return my_key;
    }

    void print(FILE *f) {
        printf("%d %d\n", my_key, count);
    }
};

struct parame {
    int forloop_start;
    int forloop_end;
};
typedef struct parame param;
void * thread_process(void* p);


// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample, unsigned> h;

int
main(int argc, char* argv[]) {
    int i, j, k;
    int rnum;
    unsigned key;
    sample *s;

    // Print out team information
    printf("Team Name: %s\n", team.team);
    printf("\n");
    printf("Student 1 Name: %s\n", team.name1);
    printf("Student 1 Student Number: %s\n", team.number1);
    printf("Student 1 Email: %s\n", team.email1);
    printf("\n");
    printf("Student 2 Name: %s\n", team.name2);
    printf("Student 2 Student Number: %s\n", team.number2);
    printf("Student 2 Email: %s\n", team.email2);
    printf("\n");

    // Parse program arguments
    if (argc != 3) {
        printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
        exit(1);
    }
    sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
    sscanf(argv[2], " %d", &samples_to_skip);

    // initialize a 16K-entry (2**14) hash of empty lists
    h.setup(14);

    // process streams starting with different initial numbers
    if (num_threads == 1) {
        param p;
        p.forloop_start = 0;
        p.forloop_end = 4;
        thread_process(&p);
    } else if (num_threads == 2) {
        pthread_t thread[2];
        param p;
        p.forloop_start = 0;
        p.forloop_end = 2;
        param p2;
        p2.forloop_start = 2;
        p2.forloop_end = 4;
        pthread_create(&thread[0], NULL, thread_process, &p);
        pthread_create(&thread[1], NULL, thread_process, &p2);

        //join
        pthread_join(thread[0], NULL);
        pthread_join(thread[1], NULL);

    } else if (num_threads == 4) {
        pthread_t thread[4];
        param p[4];
        int i;
        for (i = 0; i < 4; i++) {
            p[i].forloop_start = i;
            p[i].forloop_end = i + 1;
            pthread_create(&thread[i], NULL, thread_process, &p[i]);
        }

        for (i = 0; i < 4; i++) {
            pthread_join(thread[i], NULL);
        }

    }


    // print a list of the frequency of all samples
    h.print();
}

void* thread_process(void *para) {
    param* p = (param *) para;
    int i, j, k;
    int rnum;
    unsigned key;
    sample *s;

    for (i = p->forloop_start; i < p->forloop_end; i++) {
        rnum = i;

        // collect a number of samples
        for (j = 0; j < SAMPLES_TO_COLLECT; j++) {

            // skip a number of samples
            for (k = 0; k < samples_to_skip; k++) {
                rnum = rand_r((unsigned int*) &rnum);
            }

            // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
            key = rnum % RAND_NUM_UPPER_BOUND;
            //critical section
            __transaction_atomic{
                // if this sample has not been counted before
                if (!(s = h.lookup(key))) {

                    // insert a new element for it into the hash table
                    s = new sample(key);

                    h.insert(s);

                }

                // increment the count for the sample

                s->count++;
            }

        }
    }

}
