#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
unsigned  long long ans[100]={0};

typedef struct _thread_data_t {
    int tid;
    unsigned long long stuff;
} thread_data_t;

void *child(void *arg) {
    thread_data_t data = *(thread_data_t *)arg;
    double distance_squared, x, y;
    long long in_circle=0;
    int xx;
    pthread_t thread_id = pthread_self();
    unsigned int seed = (unsigned long int)thread_id;
    for (xx = 0; xx < data.stuff; xx++) {
        x = (double) rand_r(&seed)/(RAND_MAX)*2.0-1.0;
        y = (double) rand_r(&seed)/(RAND_MAX)*2.0-1.0;
        distance_squared = x*x + y*y;
        if (distance_squared <= 1.0) in_circle++;
    }
    ans[data.tid] = in_circle;
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    double pi_estimate;
    unsigned long long  number_of_cpu, number_of_tosses, number_in_circle;
    if ( argc < 2) {
        exit(-1);
    }
    number_of_cpu = atoi(argv[1]);
    number_of_tosses = atoi(argv[2]);
    if (( number_of_cpu < 1) || ( number_of_tosses < 0)) {
        exit(-1);
    }
    if(number_of_cpu > 1){
        number_of_tosses=(number_of_tosses/number_of_cpu)*number_of_cpu;
    }
    pthread_t all_thread[number_of_cpu];
    thread_data_t t_data[number_of_cpu];
    for(int i=0;i<number_of_cpu;i++) {
        t_data[i].tid=i;
        t_data[i].stuff=number_of_tosses/number_of_cpu;
	pthread_create(&all_thread[i], NULL, child, (void *)&t_data[i]);
    }
    for(int i=0;i<number_of_cpu;i++) {
        pthread_join(all_thread[i], NULL);
    }
    number_in_circle=0;
    for(int i=0;i<number_of_cpu;i++) {
        number_in_circle+=ans[i];
    }
    pi_estimate = 4*number_in_circle/((double) number_of_tosses);
    printf("%f\n",pi_estimate);
    return 0;
}
