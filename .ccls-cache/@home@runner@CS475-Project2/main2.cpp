#include <stdio.h>
#define _USE_MATH_DEFINES
#include <float.h>
#include <math.h>
#include <omp.h>
#include <stdlib.h>
#include <string>

// setting the number of threads:
#ifndef NUMT
#define NUMT 2
#endif

// setting the number of capitals we want to try:
#ifndef NUMCAPITALS
#define NUMCAPITALS 5
#endif

// maximum iterations to allow looking for convergence:
#define MAXITERATIONS 100

struct city {
  std::string name;
  float longitude;
  float latitude;
  int capitalnumber;
  float mindistance;
};

#include "UsCities.data"

// setting the number of cities we want to try:
#define NUMCITIES (sizeof(Cities) / sizeof(struct city))

struct capital {
  std::string name;
  float longitude;
  float latitude;
  float longsum;
  float latsum;
  int numsum;
};

struct capital Capitals[NUMCAPITALS];

float Distance(int city, int capital) {
  float dx = Cities[city].longitude - Capitals[capital].longitude;
  float dy = Cities[city].latitude - Capitals[capital].latitude;
  return sqrtf(dx * dx + dy * dy);
}

int main(int argc, char *argv[]) {
#ifdef _OPENMP
  omp_set_num_threads(NUMT);
#else
  fprintf(stderr, "No OpenMP support!\n");
  return 1;
#endif

  // Seed initial capitals
  for (int k = 0; k < NUMCAPITALS; k++) {
    int cityIndex = k * (NUMCITIES - 1) / (NUMCAPITALS - 1);
    Capitals[k].longitude = Cities[cityIndex].longitude;
    Capitals[k].latitude = Cities[cityIndex].latitude;
  }

  double time0, time1;
  time0 = omp_get_wtime();

  for (int n = 0; n < MAXITERATIONS; n++) {
    // Initialize sums for new capital calculation
    float longsum[NUMCAPITALS] = {0.0};
    float latsum[NUMCAPITALS] = {0.0};
    int numsum[NUMCAPITALS] = {0};

#pragma omp parallel for
    for (int i = 0; i < NUMCITIES; i++) {
      int capitalnumber = -1;
      float mindistance = FLT_MAX;

      // Find closest capital
      for (int k = 0; k < NUMCAPITALS; k++) {
        float dist = Distance(i, k);
        if (dist < mindistance) {
          mindistance = dist;
          capitalnumber = k;
        }
      }

      Cities[i].capitalnumber = capitalnumber;

// Update the sums for each capital
#pragma omp atomic
      longsum[capitalnumber] += Cities[i].longitude;
#pragma omp atomic
      latsum[capitalnumber] += Cities[i].latitude;
#pragma omp atomic
      numsum[capitalnumber]++;
    }

    // Update capital positions
    for (int k = 0; k < NUMCAPITALS; k++) {
      if (numsum[k] > 0) {
        Capitals[k].longitude = longsum[k] / numsum[k];
        Capitals[k].latitude = latsum[k] / numsum[k];
      }
    }
  }

  time1 = omp_get_wtime();

  double megaCityCapitalsPerSecond =
      (double)NUMCITIES * (double)NUMCAPITALS / (time1 - time0) / 1000000.;

  // Extra credit: find the actual city nearest to each capital
  for (int k = 0; k < NUMCAPITALS; k++) {
    float minDistance = FLT_MAX;
    std::string closestCity = "Unknown";
    for (int i = 0; i < NUMCITIES; i++) {
      float dx = Cities[i].longitude - Capitals[k].longitude;
      float dy = Cities[i].latitude - Capitals[k].latitude;
      float dist = sqrtf(dx * dx + dy * dy);
      if (dist < minDistance) {
        minDistance = dist;
        closestCity = Cities[i].name;
      }
    }
    Capitals[k].name = closestCity;
  }

  // Print the capital positions with city names
  if (NUMT == 1) {
    for (int k = 0; k < NUMCAPITALS; k++) {
      fprintf(stderr, "\t%3d:  %8.2f , %8.2f , %s\n", k, Capitals[k].longitude,
              Capitals[k].latitude, Capitals[k].name.c_str());
    }
  }

  fprintf(stderr, "%2d , %4ld , %4d , %8.3lf\n", NUMT, NUMCITIES, NUMCAPITALS,
          megaCityCapitalsPerSecond);
}
