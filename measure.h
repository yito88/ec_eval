#ifndef _MEASURE_H
#define _MEASURE_H

#include "eval.h"

typedef enum timeTarget {
  TOTAL = 0,           /* total time */
  TOTAL_ENCODE,        /* including read/write files */
  TOTAL_DECODE,        /* including read/write files */
  TOTAL_ENCODE_CHUNK,  /* total time of encoding all chunks */
  TOTAL_DECODE_CHUNK,  /* total time of decoding all chunks */
  MAX_ENCODE_CHUNK,    /* Max. time of encoding a chunk */
  MAX_DECODE_CHUNK,    /* Max. time of decoding a chunk */
  NUM_OF_TIME_TARGET,
} TIME_TARGET;

void InitTime(void);
void StartTime(TIME_TARGET target);
void EndTime(TIME_TARGET target);
void PrintTimeResult(void);

#endif /* _MEASURE_H */

