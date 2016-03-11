#include <jerasure.h>
#include <reed_sol.h>

#include "decoder.h"
#include "measure.h"

/* ==== Prototype ==== */
static void readEncodedData(ENC_PARAM* pParam, uint32_t chunkIdx, uint32_t* erasures);
static uint32_t decodeChunk(ENC_PARAM* pParam, uint32_t* erasures);
static void storeDecodedChunk(ENC_PARAM* pParam, uint32_t chunkIdx);

/* ==== Main Function ==== */
uint32_t Decode(ENC_PARAM* pParam)
{
  uint32_t chunkIdx;
  uint32_t* erasures;
  uint32_t ret = 0;

  StartTime(TOTAL_DECODE);

  /* erased position */
  erasures = (uint32_t*)malloc(sizeof(uint32_t) * (pParam->k + pParam->m));

  for (chunkIdx = 0; chunkIdx < pParam->chunkNum; chunkIdx++) {
    /* open encoded file */
    readEncodedData(pParam, chunkIdx, erasures);

    /* decoding */
    ret = decodeChunk(pParam, erasures);
    if (ret == -1) {
      fprintf(stderr, "!!!! FAIL to decode !!!!\n");
    }

    /* store decoded file */
    storeDecodedChunk(pParam, chunkIdx);
  }

  free(erasures);

  EndTime(TOTAL_DECODE);

  return ret;
}

/* ==== Static Function ==== */
static void readEncodedData(ENC_PARAM* pParam, uint32_t chunkIdx, uint32_t* erasures)
{
  FILE* fp;
  char* fileName;
  uint32_t totalNum;
  uint32_t blockNo;
  uint32_t blockSize;
  uint32_t numErased;
  uint32_t i;
  uint8_t** data;
  uint8_t type;

  fileName = (char*)malloc(sizeof(uint8_t)*32);

  totalNum = pParam->k + pParam->m;
  blockSize = pParam->chunkSize / pParam->k;

  numErased = 0;

  for (i = 0; i < totalNum; i++) {
    /* read a block from file */
    /* set file name */
    if (i < pParam->k) {
      /* source data */
      type = 'k';
      blockNo = i;
      data = pParam->src;
    } else {
      /* parity */
      type = 'm';
      blockNo = i - pParam->k;
      data = pParam->coding;
    }
    sprintf(fileName, "%s/encoded_%c%d", ENCODED_DIR, type, blockNo);

    /* open file */
    fp = fopen(fileName, "rb");

    if (fp == NULL) {
      /* read fail */
#ifdef DEBUG
      printf("Fail to read %s.\n", fileName);
#endif /* DEBUG */
      erasures[numErased] = i;
      numErased++;
    } else {
      /* read success */
      /* TODO: alignment */
      uint32_t offset = blockSize * chunkIdx;
      fseek(fp, offset, SEEK_SET);
      fread(data[blockNo], sizeof(uint8_t), blockSize, fp);
    }

    fclose(fp);
  }

  /* set enc position */
  erasures[numErased] = -1;

#ifdef DEBUG
  if (numErased != 0) {
    printf("%d File Read Fail\n", numErased);
  }
#endif /* DEBUG */

  free(fileName);
}

static uint32_t decodeChunk(ENC_PARAM* pParam, uint32_t* erasures)
{
  uint32_t blockSize;
  uint32_t ret;

  StartTime(TOTAL_DECODE_CHUNK);

  ret = 0;
  blockSize = pParam->chunkSize / pParam->k;

  switch (pParam->encType) {
    case ENC_TYPE_NON:
      /* nothing to do */
      break;

    case ENC_TYPE_JERASURE:
      ret = jerasure_matrix_decode(pParam->k, pParam->m, pParam->w, (int*)pParam->array, 1, (int*)erasures, (char**)pParam->src, (char**)pParam->coding, blockSize);
      break;

    default:
      break;
  }

  EndTime(TOTAL_DECODE_CHUNK);

  return ret;
}

static void storeDecodedChunk(ENC_PARAM* pParam, uint32_t chunkIdx)
{
  FILE* fp;
  char* fileName;
  uint32_t blockSize;
  uint32_t storedSize;
  uint32_t writeSize;
  uint32_t i;

  blockSize = pParam->chunkSize / pParam->k;

  storedSize = chunkIdx * pParam->chunkSize;

  fileName = (char*)malloc(sizeof(uint8_t)*32);
  sprintf(fileName, "%s/decoded", DECODED_DIR);

  if (chunkIdx == 0) {
    fp = fopen(fileName, "wb");
  } else {
    fp = fopen(fileName, "ab");
  }

  for (i = 0; i < pParam->k; i++) {
    /* adjust write size */
    if (pParam->orgDataSize < (storedSize + blockSize)) {
      /* next write will be over */
      writeSize = pParam->orgDataSize - storedSize;
    } else {
      writeSize = blockSize;
    }

    fwrite(pParam->src[i], sizeof(uint8_t), writeSize, fp);

    storedSize += writeSize;
  }

  free(fileName);
  fclose(fp);
}
