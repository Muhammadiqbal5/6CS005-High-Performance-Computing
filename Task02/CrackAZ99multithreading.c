// Muhammad Iqbal
// 1718170

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#include <crypt.h>

 

/******************************************************************************
  Demonstrates how to crack an encrypted password using a simple
  "brute force" algorithm. Works on passwords that consist only of 2 uppercase
  letters and a 2 digit integer. 
 
  Compile with:
    cc -o CrackAZ99mltithreading CrackAZ99multithreading.c -lcrypt -pthread
 
  If you want to analyse the results then use the redirection operator to send
  output to a file that you can view using an editor or the less utility:
    ./CrackAZ99-With-Data-With-Threads > results.txt
 
  Dr Kevan Buckley, University of Wolverhampton, 2018 Modified by Dr. Ali Safaa 2019
******************************************************************************/

int n_passwords = 1;	// A counter used to track the number of combinations explored so far

 

char *encrypted_passwords[] = {

  "$6$AS$17zWYQGXZOLVBhLwBdPgq5y9SvbW1Bxy7E0IxDBEyYqv5A3BrPC8bs3SviLK5yZ7iWO2KC3yXO6PXJQTrciyJ1"

};

/**
Required by lack of standard function in C.  
*/

 
typedef struct arguments_t {
  int start;
  int finish;
  char *salt_and_encrypted;
} arguments_t;


void substr(char *dest, char *src, int start, int length){

  memcpy(dest, src + start, length);

  *(dest + length) = '\0';

}

 

/**
This function can crack the kind of password explained above. All combinations
that are tried are displayed and when the password is found, #, is put at the
 start of the line. Note that one of the most time consuming operations that
 it performs is the output of intermediate results, so performance experiments
 for this kind of program should not include this. i.e. comment out the printfs.
*/

 

void *crack(void *arguments){
  arguments_t *args = (arguments_t*) arguments;
  int x, y, z;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, args->salt_and_encrypted, 0, 6);

  for(x=args->start; x<=args->finish; x++){
    for(y='A'; y<='Z'; y++){
      for(z=0; z<=99; z++){
        sprintf(plain, "%c%c%02d", x, y, z);
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(args->salt_and_encrypted, enc) == 0){
	  printf ("Password Found: %-8d%s %s\n", count, plain, enc);
        //} else {
          //printf(" %-8d%s %s\n", count, plain, enc);
        }
      }
    }
  }
  printf("%d solutions explored\n", count);
}

int time_difference(struct timespec *start, 
                    struct timespec *finish, 
                    long long int *difference) {
  long long int ds =  finish->tv_sec - start->tv_sec; 
  long long int dn =  finish->tv_nsec - start->tv_nsec;
  
  if(dn < 0 ) {
    ds--;
    dn += 1000000000; 
  } 
  *difference = ds * 1000000000 + dn;
  return !(*difference > 0);
 
}

int main(int argc, char *argv[]){

  int i;

  printf("Enter the number for threads you want to create between 1 to 100 \n");
  scanf("%d",&i);

  struct timespec start, finish;   
  long long int time_elapsed;

  pthread_t t1, t2;
  arguments_t t1_arguments;
  t1_arguments.start = 65;
  t1_arguments.finish = 77;

  arguments_t t2_arguments;
  t2_arguments.start = 78;
  t2_arguments.finish = 90;

  clock_gettime(CLOCK_MONOTONIC, &start);

 for(i=0;i<n_passwords;i<i++) {

    t1_arguments.salt_and_encrypted = encrypted_passwords[i];
    t2_arguments.salt_and_encrypted = encrypted_passwords[i];
    pthread_create(&t1, NULL, crack, &t1_arguments);
    pthread_create(&t2, NULL, crack, &t2_arguments);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

  }
  

  clock_gettime(CLOCK_MONOTONIC, &finish);

  time_difference(&start, &finish, &time_elapsed);
  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
                                         (time_elapsed/1.0e9));


return 0;

}
