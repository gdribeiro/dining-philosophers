#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define true 1

// Number of philosophers sitting on the table
int NumPhilosophers;
// Stores the philosophers state H-hungry, T-thinking, E-eating
char* philosopherState;
// Condtion variables for the chopsticks, will wake up the threads waiting for
// it
pthread_cond_t* eatTurn;
// Mutex to control access to the crucial section
pthread_mutex_t chopsticksDispute;
// Threads
pthread_t* philosophersThreads;
// Keeps the seat number of each philosopher
int* seats;

// Funtion passed to threads
// which will dispute for the resourses
void philosophize(void* ptr);
// Initialize the state in the string
void philosophersStateInit();
// Print the philosophers' states
void printPhilosophersState(char c, int seat);
// Initialize the Conditional Variables to Access the Chopsticks
void initCondTurn();
// Initialize philosophers threads to philosophize
void initPhilosophers();
// Waits for all philosophers to finish
void joinPhilosophers();
// parse the input
void parse(int argc, char** argv);
// monitors the chopsticks' picking up action
void pickUp(int seat);
// Monitors the chopsticks' putting down action
void putDown(int seat);

int main(int argc, char** argv) {
  // parse the input
  parse(argc, argv);
  // Initialize philosophers' states
  philosophersStateInit();
  // Initialize the semaphors chopsticks
  initCondTurn();
  // Creat Threads philosophers and let them run
  initPhilosophers();
  // wait for all philosophers to finish...what means, Never
  joinPhilosophers();

  exit(0);
}

void philosophize(void* ptr) {
  // Stores the set of each philosopher
  int seat = *(int*)ptr;
  printf("Seat teken: %d \n", seat);
  // printf("Left: %d, Right: %d \n",((seat + (NumPhilosophers - 1)) %
  // NumPhilosophers),((seat+1) % NumPhilosophers));
  while (true) {
    // picks up the chopsticks to eat
    pickUp(seat);
    // Eats for a ramdom time up to 10s
    sleep(rand() % 10);
    // puts down the chopsticks
    putDown(seat);
    // sleep for a ramdom time up to 10s
    sleep(rand() % 10);
  }
}

// monitors the chopsticks' picking up action
void pickUp(int seat) {
  // begin of crucial section
  pthread_mutex_lock(&chopsticksDispute);
  // Changes the state of the philosopher calling the funtion to H-hungry
  printPhilosophersState('H', seat);
  // While the philosophers on the left and on the right are eating wait untill
  // they finish
  while (
      (philosopherState[((seat + (NumPhilosophers - 1)) % NumPhilosophers)] ==
       'E') ||
      (philosopherState[(seat + 1) % NumPhilosophers] == 'E')) {
    // Waits the condition and sets the mutex free
    pthread_cond_wait(&eatTurn[seat], &chopsticksDispute);
  }
  // Changes the state to E-eating
  printPhilosophersState('E', seat);
  // End of crucial section
  pthread_mutex_unlock(&chopsticksDispute);
}

// Monitors the chopsticks' putting down action
void putDown(int seat) {
  // begin of crucial section
  pthread_mutex_lock(&chopsticksDispute);
  // Changes the state of the philosopher calling the funtion to H-hungry
  printPhilosophersState('T', seat);
  // Signals the philosopher on the right and on the left because the chopsticks
  // are free
  // This signal wakes up only one thread
  pthread_cond_signal(&eatTurn[(seat + 1) % NumPhilosophers]);
  pthread_cond_signal(
      &eatTurn[(seat + (NumPhilosophers - 1)) % NumPhilosophers]);
  // End of crucial section
  pthread_mutex_unlock(&chopsticksDispute);
}

// Initialize philosophers threads to philosophize
void initPhilosophers() {
  int i;
  // Allocate memmory for the seat numbers
  seats = (int*)malloc(NumPhilosophers * sizeof(int));
  if (seats == NULL) printf("Error while allocating memory to seat numbers!\n");
  // Allocate memmory for the thread's descriptors
  philosophersThreads = (pthread_t*)malloc(NumPhilosophers * sizeof(pthread_t));
  if (philosophersThreads == NULL)
    printf("Error while allocating memory to the Threads!\n");
  for (i = 0; i < NumPhilosophers; i++) {
    seats[i] = i;
    int err = pthread_create(&philosophersThreads[i], NULL,
                             (void*)&philosophize, (void*)&seats[i]);
    if (err != 0) {
      printf("Error initializing the threads!\n");
      exit(EXIT_FAILURE);
    }
  }
}

void printPhilosophersState(char c, int seat) {
  // Changes it's state to Thinking
  philosopherState[seat] = c;
  // Print philosophers' state
  printf("%s\n", philosopherState);
}

// Initialize the semaphores chopsticks
void initCondTurn() {
  int i;
  // chopsticks are the diputed resourse
  eatTurn = (pthread_cond_t*)malloc(NumPhilosophers * sizeof(pthread_cond_t));
  if (eatTurn == NULL)
    printf("Error while allocating memory to the chopsticks!\n");
  for (i = 0; i < NumPhilosophers; i++) {
    pthread_cond_init(&eatTurn[i], NULL);
  }
  pthread_mutex_init(&chopsticksDispute, NULL);
}

// Waits for all philosophers to finish
void joinPhilosophers() {
  int i;
  for (i = 0; i < NumPhilosophers; i++) {
    pthread_join(philosophersThreads[i], NULL);
  }
}

void philosophersStateInit() {
  int i;
  // Allocate memmory for the philosophers' states
  philosopherState = (char*)malloc(NumPhilosophers * sizeof(char));
  if (philosopherState == NULL)
    printf("Error while allocating memory to the philosophersState!\n");
  // Initial state is Thinking
  for (i = 0; i < NumPhilosophers; i++) {
    philosopherState[i] = 'T';
  }
}

// parse the input
void parse(int argc, char** argv) {
  if (argc != 2) {
    printf(
        "The imput format should be ./semaphores_philosophers "
        "NUMBER_OF_PHILOSOPHER\n");
    exit(EXIT_FAILURE);
  }
  NumPhilosophers = atoi((const char*)argv[1]);
  if (NumPhilosophers == 0) {
    printf(
        "If tere is no philosopher to take a seat the food will get cold! "
        "Enter a number higher or equal to 2.\n");
    exit(EXIT_FAILURE);
  } else if (NumPhilosophers == 1) {
    printf(
        "Philosophers are not very realistic people and they need their "
        "friends chopsticks to eat, only one philosopher has only it's own "
        "chopstick, so he would starve forever... Enter a number higher or "
        "equal to 2.\n");
    exit(EXIT_FAILURE);
  }
}
