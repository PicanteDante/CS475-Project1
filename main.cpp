#include <stdio.h>
//#define _USE_MATH_DEFINES
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

// settings:
int NowYear = 2024; // 2024- 2029
int NowMonth = 0;   // 0 - 11
int NowMonthReal = 0;

float NowPrecip;        // inches of rain per month
float NowTemp = 52.0;   // temperature this month
float NowHeight = 50.0; // grain height in inches
int NowNumDeer = 4;     // number of deer in the current population
int NowAbductions = 0;

const float GRAIN_GROWS_PER_MONTH = 50.0;
const float ONE_DEER_EATS_PER_MONTH = 1;

const float AVG_PRECIP_PER_MONTH = 10.0; // average
const float AMP_PRECIP_PER_MONTH = 9.0;  // plus or minus
const float RANDOM_PRECIP = 4.0;         // plus or minus noise

const float AVG_TEMP = 60.0;    // average
const float AMP_TEMP = 20.0;    // plus or minus
const float RANDOM_TEMP = 15.0; // plus or minus noise

const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

unsigned int seed = 0;

omp_lock_t Lock;
volatile int NumInThreadTeam;
volatile int NumAtBarrier;
volatile int NumGone;

float Ranf(float low, float high) {
  float r = (float)rand();       // 0 - RAND_MAX
  float t = r / (float)RAND_MAX; // 0. - 1.

  return low + t * (high - low);
}

float SQR(float x) { return x * x; }

void InitBarrier(int n) {
  NumInThreadTeam = n;
  NumAtBarrier = 0;
  omp_init_lock(&Lock);
}

// have the calling thread wait here until all the other threads catch up:

void WaitBarrier() {
  omp_set_lock(&Lock);
  {
    NumAtBarrier++;
    if (NumAtBarrier == NumInThreadTeam) {
      NumGone = 0;
      NumAtBarrier = 0;
      // let all other threads get back to what they were doing
      // before this one unlocks, knowing that they might immediately
      // call WaitBarrier( ) again:
      while (NumGone != NumInThreadTeam - 1)
        ;
      omp_unset_lock(&Lock);
      return;
    }
  }
  omp_unset_lock(&Lock);

  while (NumAtBarrier != 0)
    ; // this waits for the nth thread to arrive

#pragma omp atomic
  NumGone++; // this flags how many threads have returned
}

void Deer() {
  while (NowYear < 2030) {
    // Compute temporary next value for deer population
    int nextNumDeer = NowNumDeer;
    int carryingCapacity = (int)(NowHeight);

    if (nextNumDeer < carryingCapacity) {
      nextNumDeer *= Ranf(1.1, 2.0);
    } else if (nextNumDeer > carryingCapacity) {
      nextNumDeer *= 0.75;
    }

    if (nextNumDeer < 0) {
      nextNumDeer = 0;
    }

    // Done computing
    WaitBarrier();

    // Assign the computed next value to the actual variable
    NowNumDeer = nextNumDeer;

    // Done assigning
    WaitBarrier();

    // Wait for the Watcher to print and update month/year
    WaitBarrier();
  }
}

void Grain() {
  while (NowYear < 2030) {
    // Compute temporary next value for grain height
    float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.0));
    float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.0));

    float nextHeight = NowHeight;
    nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
    nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

    if (nextHeight < 0.0)
      nextHeight = 0.0;

    // Done computing
    WaitBarrier();

    // Assign the computed next value to the actual variable
    NowHeight = nextHeight;

    // Done assigning
    WaitBarrier();

    // Wait for the Watcher to print and update month/year
    WaitBarrier();
  }
}

void Watcher() {
  while (NowYear < 2030) {
    // Wait for Deer, Grain, and MyAgent to finish computing
    WaitBarrier();

    // Wait for Deer, Grain, and MyAgent to finish assigning
    WaitBarrier();

    // Update Abductions
    NowNumDeer -= NowAbductions;

    // Print current state
    printf("%d,%.2f,%.2f,%.2f,%d,%d\n", NowMonthReal,
           (NowTemp - 32) * (5. / 9.), NowPrecip * 2.54, NowHeight * 2.54,
           NowNumDeer, NowAbductions);

    // Update month and year
    NowMonth++;
    NowMonthReal++;
    if (NowMonth > 11) {
      NowMonth = 0;
      NowYear++;
    }

    // Calculate new weather conditions
    float ang = (30. * (float)NowMonth + 15.) *
                (M_PI / 180.); // angle of earth around the sun

    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemp = temp + Ranf(-RANDOM_TEMP, RANDOM_TEMP);
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
    if (NowPrecip < 0.)
      NowPrecip = 0.;

    // Done updating
    WaitBarrier();
  }
}

void MyAgent() { // Alien lifeform
  while (NowYear < 2030) {
    // Base abduction rate is 20% of the deer population with a maximum of 30%
    float baseAbductions = (0.2 * NowNumDeer) * Ranf(0.75, 1.5);

    float heightEffect =
        1.0 -
        Ranf(0.0, (NowHeight /
                   100.0)); // 50% abduction rate modulated by grain height
    // printf("base abudctions: %.2f\n", baseAbductions);
    //  Reducing abduction rate based on precipitation
    float rainEffect =
        1.0 - (NowPrecip /
               75.0); // Reduce effectiveness by up to ~10% based on
                      // precipitation (assuming max typical rain is 10 inches)
    // if (rainEffect < 0.5)
    //   rainEffect = 0.5; // Ensuring at least 50% effectiveness

    // printf("rain effect: %.2f\n", rainEffect);

    // printf("abductions: %.2f\n", rainEffect * baseAbductions);
    int abductions = (int)(baseAbductions * rainEffect);

    if (abductions > NowNumDeer - 2)
      abductions = NowNumDeer - 2; // Ensure at least one deer remains

    // if (abductions < 0)
    //   abductions = 0; // Don't abduct a negative number of deer

    WaitBarrier(); // Done computing

    // NowNumDeer -= abductions; // Update global deer count after abductions
    NowAbductions = abductions;
    WaitBarrier(); // Done assigning
    WaitBarrier(); // Done printing
  }
}

int main(int argc, char *argv[]) {
  // seed = time(NULL);
  omp_set_num_threads(4); // Total number of threads including MyAgent
  InitBarrier(4);         // Initialize the barrier for 4 threads

  printf("Month,Temp,Precip,Height,Deer,Abductions\n");
#pragma omp parallel sections
  {
#pragma omp section
    { Deer(); }

#pragma omp section
    { Grain(); }

#pragma omp section
    { Watcher(); }

#pragma omp section
    {
      MyAgent(); // your own
    }
  } // implied barrier -- all functions must return in order
    // to allow any of them to get past here
}