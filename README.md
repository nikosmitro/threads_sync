# threads_sync

Program - Exercise 1.

  The program simplesync.c initialize the variable val=0 and then creates two threads that run the same time. The first thread does val++   N times and the second does val-- N times. The threads are synchronized using two methods:
  i) atomic operations
  ii)mutexes.
   Two executables are created by the Makefile : i) simplesync-atomic ii) simplesync-mutex.


Program - Exercise 2.

  The program mandel.c computes and prints the Mandelbort set. In order to make the program faster and reduce the execution time, N         threads are created that run at the same time and coputes the lines. Each thread i computes the lines i, i+n, i+2*n... for i=1,2,..n-1.
  and then prints it. Semaphores are used for the synchronization of the threads.

  
Program - Exercise 3.

  The program simulates a kindergarten. The entrance/exit of teachers and children must be checked and synchronized so that the ratio of     children and teacher is the desired one. N threads are used, C for children and N-C for theachers. If the ratio is at the limit, no more   children are allowed to enter until a new teacher enter. The number of teachers that can enter are limitless but if there are not enough   children for a teacher, the teacher exits. Conditional variables and mutexes are used for the synchronization of the threads.
