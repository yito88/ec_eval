/* ENCODER */

#include <isa-l.h>
#include <jerasure.h>
#include <reed_sol.h>

#include "encoder.h"
#include "measure.h"

/* ==== Prototype ==== */
static void readData(ENC_PARAM* pParam, FILE* fp, uint32_t chunkIdx);
static void encodeChunk(ENC_PARAM* pParam);
static void storeEncodedData(ENC_PARAM* pParam, uint32_t chunkIdx);

uint32_t Encode(ENC_PARAM* pParam)
{
  FILE* fp;
  uint32_t chunkIdx;

  StartTime(TOTAL_ENCODE);

  /* open a file */
  fp = fopen(pParam->fileName, "rb");
  if (fp == NULL) {
    fprintf(stderr, "File Open Failed.\n");
    return -1;
  }

  /* encode per chunk */
  for (chunkIdx = 0; chunkIdx < pParam->chunkNum; chunkIdx++) {
    /* read source data */
    readData(pParam, fp, chunkIdx);

    /* encode */
    encodeChunk(pParam);

    /* store to files */
    storeEncodedData(pParam, chunkIdx);
  }

  fclose(fp);

  EndTime(TOTAL_ENCODE);

  return 0;
}

/* ==== Static Function ==== */
static void readData(ENC_PARAM* pParam, FILE* fp, uint32_t chunkIdx)
{
  uint32_t i;
  uint32_t readSize;
  uint32_t blockSize;

  if (chunkIdx == (pParam->chunkNum - 1)) {
    /* when last read, readSize is adjusted */
    readSize = pParam->orgDataSize % pParam->chunkSize;
  } else {
    readSize = pParam->chunkSize;
  }

  /* read data */
  fread(pParam->pChunk, sizeof(uint8_t), readSize, fp);

  /* padding */
  if (readSize != pParam->chunkSize) {
    uint32_t rest = pParam->chunkSize - readSize;
    uint8_t* restHead = pParam->pChunk + readSize;
    memset(restHead, 0, sizeof(uint8_t) * rest);
  }

  /* set data pointer */
  blockSize = pParam->chunkSize / pParam->k;
  for (i = 0; i < pParam->k; i++) {
    pParam->src[i] = pParam->pChunk + (blockSize * i);
  }
}

static void encodeChunk(ENC_PARAM* pParam)
{
  uint32_t blockSize;
  blockSize = pParam->chunkSize / pParam->k;

  StartTime(TOTAL_ENCODE_CHUNK);

  switch (pParam->encType) {
    case ENC_TYPE_NON:
      /* nothing to do */
      break;

    case ENC_TYPE_JERASURE:
      jerasure_matrix_encode(pParam->k, pParam->m, pParam->w, (int*)pParam->array, (char**)pParam->src, (char**)pParam->coding, blockSize);
      break;

    case ENC_TYPE_ISAL:
      //ec_encode_data(blockSize, pParam->k, pParam->m, pParam->gfTbls, pParam->src, pParam->coding);
      break;

    default:
      break;
  }

  EndTime(TOTAL_ENCODE_CHUNK);
}

static void storeEncodedData(ENC_PARAM* pParam, uint32_t chunkIdx)
{
  FILE* fp;
  char* fileName;
  uint32_t i;
  uint32_t totalNum;
  uint32_t blockSize;
  uint32_t blockNo;

  uint8_t type;
  uint8_t** src;

  totalNum = pParam->k + pParam->m;
  blockSize = pParam->chunkSize / pParam->k;

  fileName = (char*)malloc(sizeof(uint8_t)*32);

  for (i = 0; i < totalNum; i++) {
    if (i < pParam->k) {
      /* part of source data */
      type = 'k';
      blockNo = i;
      src = pParam->src;
    } else {
      /* part of parity */
      type = 'm';
      blockNo = i - pParam->k;
      src = pParam->coding;
    }

    if (CheckFailPos(pParam, i) == true) {
      sprintf(fileName, "%s/encoded_%c%d_fail", ENCODED_DIR, type, blockNo);
    } else {
      sprintf(fileName, "%s/encoded_%c%d", ENCODED_DIR, type, blockNo);
    }

    if (chunkIdx == 0) {
      /* the first chunk */
      fp = fopen(fileName, "wb");
    } else {
      /* NOT the first chunk */
      fp = fopen(fileName, "ab");
    }

    fwrite(src[blockNo], sizeof(uint8_t), blockSize, fp);
    fclose(fp);
  }

  free(fileName);
}

