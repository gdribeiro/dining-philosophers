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
// Semaphores/Resourses that will be disputed
sem_t* chopsticks;
// Semaphore for writing the state's change on the screen just to provide
// sinchrony
sem_t stateMutex;
// Threads
pthread_t* philosophersThreads;
// Keeps the seat number of each philosopher
int* seats;
// To implement fairness and limited wait
int* hunger;

// Funtion passed to threads
// which will dispute for the resourses
void philosophize(void* ptr);
// Initialize the state in the string
void philosophersStateInit();
// Print the philosophers' states
void printPhilosophersState(char c, int seat);
// Initialize the semaphores chopsticks
void initSemChopsticks();
// Initialize philosophers threads to philosophize
void initPhilosophers();
// Waits for all philosophers to finish
void joinPhilosophers();
// After all set-up it set the semaphors and let the philosophers compete for it
void letTheFightBegin();
// parse the input
void parse(int argc, char** argv);

int main(int argc, char** argv) {
  // parse the input
  parse(argc, argv);
  // Initialize philosophers' states
  philosophersStateInit();
  // Initialize the semaphors chopsticks
  initSemChopsticks();
  // Creat Threads philosophers and let them run
  initPhilosophers();
  // After all set up, let them compete for the chopsticks
  letTheFightBegin();
  // wait for all philosophers to finish...what means, Never
  joinPhilosophers();

  exit(0);
}

void philosophize(void* ptr) {
  int evenodd;
  int left, right;
  // Stores the set of each philosopher
  int seat = *(int*)ptr;
  printf("Seat teken: %d \n", seat);
  if (seat == 0) {
    evenodd = 1;
    left = NumPhilosophers;
    right = 1;
  } else if (seat % 2) {
    evenodd = 0;
    left = (seat - 1) % NumPhilosophers;
    right = (seat + 1) % NumPhilosophers;
  } else {
    evenodd = 1;
    left = (seat - 1) % NumPhilosophers;
    right = (seat + 1) % NumPhilosophers;
  }

  while (true) {
    printf("seat:%d hunger:%d\n", seat, hunger[seat]);

    // Waits for the chopsticks
    // Contains the states Hungry and Eating
    switch (evenodd) {
      // The odd philosophers start taking the chopstick on the left first
      case 0:
        // Before wait, changes it's state to H-hungry
        // Prints philosophers' state
        printPhilosophersState('H', seat);
        // Waits for the chopsticks on the left first
        sem_wait(&chopsticks[seat]);
        // If it's neighbors are hungrier it releases the chopsticks and
        // increase it's own hunger level
        if ((hunger[left] > hunger[seat]) || (hunger[right] > hunger[seat])) {
          hunger[seat] = hunger[seat]++;
          sem_post(&chopsticks[seat]);
          break;
        }
        // Waits for the chopstick on the right
        sem_wait(&chopsticks[(seat + 1) % NumPhilosophers]);
        // After get both chopsticks reset hunger
        hunger[seat] = 0;
        // Changes the state to E-eating
        // Print philosophers' state
        printPhilosophersState('E', seat);
        // Eats for a ramdom time up to 10s
        sleep(rand() % 2);
        // Free the chopsticks after eat
        sem_post(&chopsticks[seat]);
        sem_post(&chopsticks[(seat + 1) % NumPhilosophers]);
        break;
      // The even phisolophers start taking the chopstick on the right first
      default:
        // Before wait, changes it's state to H-hungry
        // Print the philosophers' state
        printPhilosophersState('H', seat);
        // Waits for the chopstick on the right
        sem_wait(&chopsticks[(seat + 1) % NumPhilosophers]);
        // If it's neighbors are hungrier it releases the chopsticks and
        // increase it's own hunger level
        if ((hunger[left] > hunger[seat]) || (hunger[right] > hunger[seat])) {
          hunger[seat] = hunger[seat]++;
          sem_post(&chopsticks[seat]);
          break;
        }
        // Waits for the chopsticks on the left first
        sem_wait(&chopsticks[seat]);
        // After get both chopsticks reset hunger
        hunger[seat] = 0;
        // Changes the state to E-eating
        // Print the philosophers' state
        printPhilosophersState('E', seat);
        // Eats for a ramdom time up to 10s
        sleep(rand() % 2);
        // Free the chopsticks after eat
        sem_post(&chopsticks[(seat + 1) % NumPhilosophers]);
        sem_post(&chopsticks[seat]);
        break;
    }
    // Changes the state to Thinking
    printPhilosophersState('T', seat);
    // sleep for a ramdom time up to 10s
    sleep(rand() % 2);
  }
}

// After all set-up it set the semaphors and let the philosophers compete for it
void letTheFightBegin() {
  int i;
  for (i = 0; i < NumPhilosophers; i++) {
    sem_post(&chopsticks[i]);
  }
}

// Initialize philosophers threads to philosophize
void initPhilosophers() {
  int i;
  // Allocate memmory for the seat numbers
  seats = (int*)malloc(NumPhilosophers * sizeof(int));
  hunger = (int*)malloc(NumPhilosophers * sizeof(int));
  if (seats == NULL) printf("Error while allocating memory to seat numbers!\n");
  // Allocate memmory for the thread's descriptors
  philosophersThreads = (pthread_t*)malloc(NumPhilosophers * sizeof(pthread_t));
  if (philosophersThreads == NULL)
    printf("Error while allocating memory to the Threads!\n");
  for (i = 0; i < NumPhilosophers; i++) {
    seats[i] = i;
    hunger[i] = 0;
    int err = pthread_create(&philosophersThreads[i], NULL,
                             (void*)&philosophize, (void*)&seats[i]);
    if (err != 0) {
      printf("Error initializing the threads!\n");
      exit(EXIT_FAILURE);
    }
  }
}

void printPhilosophersState(char c, int seat) {
  sem_wait(&stateMutex);
  // Changes it's state to Thinking
  philosopherState[seat] = c;
  // Print philosophers' state
  printf("%s\n", philosopherState);
  sem_post(&stateMutex);
}

// Initialize the semaphores chopsticks
void initSemChopsticks() {
  // chopsticks are the diputed resourse
  chopsticks = (sem_t*)malloc(NumPhilosophers * sizeof(sem_t));
  if (chopsticks == NULL)
    printf("Error while allocating memory to the chopsticks!\n");
  int i, semInit;
  for (i = 0; i < NumPhilosophers; i++) {
    semInit = sem_init(&chopsticks[i], 0, 0);
    // In case of error sem_init() returns non zero value
    if (semInit != 0) {
      printf("Error while allocating the semaphores to the chopsticks!\n");
      exit(EXIT_FAILURE);
    }
  }
}

// Waits for all philosophers to finish
void joinPhilosophers() {
  int i;
  for (i = 0; i < NumPhilosophers; i++) {
    pthread_join(philosophersThreads[i], NULL);
  }
}

void philosophersStateInit() {
  int i, semInit;
  // Allocate memmory for the philosophers' states
  philosopherState = (char*)malloc(NumPhilosophers * sizeof(char));
  if (philosopherState == NULL)
    printf("Error while allocating memory to the philosophersState!\n");
  // Initial state is Thinking
  for (i = 0; i < NumPhilosophers; i++) {
    philosopherState[i] = 'T';
  }
  semInit = sem_init(&stateMutex, 0, 1);
  // In case of error sem_init() returns non zero value
  if (semInit != 0) {
    printf("Error while allocating the semaphores to the chopsticks!\n");
    exit(EXIT_FAILURE);
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
