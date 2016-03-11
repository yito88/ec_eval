#ifndef _EVAL_H
#define _EVAL_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define ENCODED_DIR "./encoded"
#define DECODED_DIR "./decoded"

/* ==== STRUCT ==== */
typedef enum encodeType {
  ENC_TYPE_NON = 0,
  ENC_TYPE_JERASURE,
  ENC_TYPE_ISAL,
} ENC_TYPE;

typedef struct encodeParam {
  ENC_TYPE encType;
  char*  fileName;
  uint8_t*  array;
  uint8_t*  gfTbls;
  uint32_t  k;
  uint32_t  m;
  uint32_t  w;
  uint32_t  chunkSize;
  uint32_t  orgDataSize;
  uint32_t  chunkNum;
  uint8_t*  pChunk;
  uint8_t** src;
  uint8_t** coding;
  uint32_t* failPos;
} ENC_PARAM;

/* ==== Utility Function ==== */
bool CheckFailPos(ENC_PARAM* pParam, uint32_t pos);

#endif /* _EVAL_H */
