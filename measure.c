#include <sys/time.h>
#include "measure.h"

/* ==== Static variables ==== */
double s_Time[NUM_OF_TIME_TARGET];
double s_tmpStartTimeSec[NUM_OF_TIME_TARGET];
double s_tmpStartTimeUsec[NUM_OF_TIME_TARGET];

/* ==== Prototype ==== */

/* ==== Global Function ==== */
void InitTime(void)
{
  uint32_t i;

  for (i = 0; i < NUM_OF_TIME_TARGET; i++) {
    s_Time[i] = 0;
  }
}

void StartTime(TIME_TARGET target)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  s_tmpStartTimeSec[target] = tv.tv_sec;
  s_tmpStartTimeUsec[target] = tv.tv_usec;
}

void EndTime(TIME_TARGET target)
{
  struct timeval tv;
  double time;

  gettimeofday(&tv, NULL);
  time = (tv.tv_sec - s_tmpStartTimeSec[target]) * 1.0E6 + (tv.tv_usec - s_tmpStartTimeUsec[target]);

  if (target == TOTAL_ENCODE_CHUNK) {
    if (s_Time[MAX_ENCODE_CHUNK] < time) s_Time[MAX_ENCODE_CHUNK] = time;
  } else if (target == TOTAL_DECODE_CHUNK) {
    if (s_Time[MAX_DECODE_CHUNK] < time) s_Time[MAX_DECODE_CHUNK] = time;
  }

  s_Time[target] += time;
}

void PrintTimeResult(void)
{
  printf("==== Time Result ====\n");
  printf("TOTAL_TIME:         %f\n", s_Time[TOTAL]);
  printf("TOTAL_ENCODE:       %f\n", s_Time[TOTAL_ENCODE]);
  printf("TOTAL_DECODE:       %f\n", s_Time[TOTAL_DECODE]);
  printf("TOTAL_ENCODE_CHUNK: %f\n", s_Time[TOTAL_ENCODE_CHUNK]);
  printf("TOTAL_DECODE_CHUNK: %f\n", s_Time[TOTAL_DECODE_CHUNK]);
  printf("MAX_ENCODE_CHUNK:   %f\n", s_Time[MAX_ENCODE_CHUNK]);
  printf("MAX_DECODE_CHUNK:   %f\n", s_Time[MAX_DECODE_CHUNK]);
}

/* ==== Static Function ==== */

