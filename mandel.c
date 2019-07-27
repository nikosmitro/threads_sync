/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <signal.h> /* -- Added -- */
#include <errno.h> /* -- Added -- */
#include <semaphore.h> /* -- Added -- */
#include <pthread.h> /* -- Added -- */

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/
/* -- Added -- */
#define perror_pthread(ret, msg) \
        do { errno = ret; perror(msg); } while (0)

/* --Added-- */
void ResetAndExit(int sign)  /* Use Ctrl + C to stop the process & reset the color of the termina*/
{
        signal(sign, SIG_IGN); /* Ignore Signal */
        reset_xterm_color(1); /* Reset Color*/
        exit(-1); /* Exit*/
}
/* -- Added -- */
int n;    /* Threads variable -- Global variable*/

/* -- Added -- */
typedef struct
{
        pthread_t tid;          /* Created Thread ID. Used in order to join later*/
        int l;          /* Corresponding Line*/
        sem_t mutex;    /* Corresponding semaphore between line and the next one*/
}data;
data *struct_ptr;        /* Malloc for N malloc inside main*/
/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
        /*
         * x and y traverse the complex plane.
         */
        double x, y;

        int n;
        int val;

        /* Find out the y value corresponding to this line */
        y = ymax - ystep * line;

        /* and iterate for all points on this line */
        for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

                /* Compute the point's color value */
                val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
                if (val > 255)
                        val = 255;

                /* And store it in the color_val[] array */
                val = xterm_color(val);
                color_val[n] = val;
        }
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
        int i;

        char point ='@';
        char newline='\n';

        for (i = 0; i < x_chars; i++) {
                /* Set the current color, then output the point */
                set_xterm_color(fd, color_val[i]);
                if (write(fd, &point, 1) != 1) {
                        perror("compute_and_output_mandel_line: write point");
                        exit(1);
                }
        }

        /* Now that the line is done, output a newline character */
        if (write(fd, &newline, 1) != 1) {
                perror("compute_and_output_mandel_line: write newline");
                exit(1);
        }
}

void *compute_and_output_mandel_line(void *arg)
{
        int k;          /*Current line*/
        int line=*(int*)arg;    /* Firt line of the Thread (0 ... n-1)*/

        for(k = line; k < y_chars; k+=n) {      /* Apply the same for the following lines, using a step of size n*/
                /* A temporary array, used to hold color values for the line being drawn*/
                int color_val[x_chars];
                compute_mandel_line(k, color_val); /* Calculate the k-th line *in parallel*/

                /*Synchronization*/
                sem_wait(&struct_ptr[(k % n)].mutex); /*Waiting for the semaphore of their Thread*/
                output_mandel_line(1, color_val); /*Print *synchronized*/
                sem_post(&struct_ptr[((k % n)+1)%n].mutex); /*Incresement to the next Thread semaphore*/
        }
        return 0;
}

int main(void)
{
        /* -- Added -- */
        signal(SIGINT, ResetAndExit); /* In case of Ctrl + C Signal get to ResetAndExit*/

        int line;
        int ret;

        xstep = (xmax - xmin) / x_chars;
        ystep = (ymax - ymin) / y_chars;

        /* -- Added -- */
        printf("Enter Number of Threads: "); /* Get number of threads*/

        /* Silence scanf input checking message*/
        if (scanf("%d", &n) == 1) {
                printf("%d", n);
        } else {
                printf("failed to read integer.\n");
        }

        if ((n < 1) || (n > y_chars-1)) {
                printf("invalid input \n");
                return -1;
        }
        printf("\n");
        struct_ptr = (data*)malloc(n*sizeof(data)); /*Implement malloc*/
        sem_init(&struct_ptr[0].mutex, 0, 1); /* 0-th Semaphore initialized with a value of 1*/

        if (n > 1) { /* Check for more Threads*/
                for (line = 1; line < n; line++) { /* Then we have a same number of semaphores*/
                        sem_init(&struct_ptr[line].mutex, 0, 0); /* initialized with a value 0 which then are just waiting*/
                }
        }

        for (line = 0; line < n; line++) { /*Create n threads while each of of them goes through*/
                struct_ptr[line].l=line;  /* corresponding line*/
                ret = pthread_create(&struct_ptr[line].tid, NULL, compute_and_output_mandel_line, &struct_ptr[line].l);
                if (ret) {
                        perror_pthread(ret, "pthread_create"); /*Check for errors*/
                        exit(1);
                }
        }

        for (line = 0; line < n; line++) { /* Join the threads at the end*/
                ret = pthread_join(struct_ptr[line].tid, NULL);
                if (ret){
                        perror_pthread(ret, "pthread_join"); /* Check for errors*/
                }
        }

        for (line = 0; line < n; line++) { /*Then destroy the semaphores*/
                sem_destroy(&struct_ptr[line].mutex);
        }

        reset_xterm_color(1);
        return 0;
}
 

