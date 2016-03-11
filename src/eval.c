#include <jerasure.h>
#include <reed_sol.h>

#include "eval.h"
#include "encoder.h"
#include "decoder.h"
#include "measure.h"

/* ==== Prototype ==== */
static void initEncParam(ENC_PARAM* param, char* argv[]);
static void getOriginalDataSize(ENC_PARAM* pParam);
static void calcChunkNum(ENC_PARAM* pParam);
static void getMatrix(ENC_PARAM* pParam);
static void allocateBuffer(ENC_PARAM* pParam);
static void finalizeEnc(ENC_PARAM* pParam);
static void setFailPos(ENC_PARAM* pParam, uint32_t numOfFailNum);

/* ==== Main Function ==== */
int main(int argc, char* argv[])
{
  ENC_PARAM param;
  uint32_t ret;

  if (argc != 8) {
    fprintf(stderr, "arg: file, ENC_TYPE, k, m, w, chunkSize[KB], numOfFailNum\n");
    exit(0);
  }

  StartTime(TOTAL);

  /* initialize param */
  initEncParam(&param, argv);

  /* == Encode == */
  ret = Encode(&param);
  if (ret != 0) {
    fprintf(stderr, "Encoding Fails.\n");
    goto END;
  }

  /* == Decode == */
  ret = Decode(&param);
  if (ret != 0) {
    fprintf(stderr, "Decoding Fails.\n");
    goto END;
  }

END:
  finalizeEnc(&param);

  EndTime(TOTAL);
  PrintTimeResult();

  return 0;
}

/* ==== Utility Function ==== */
bool CheckFailPos(ENC_PARAM* pParam, uint32_t pos)
{
  uint32_t idx, ofst;

  idx = pos / 32;
  ofst = pos & 31;

  if ((pParam->failPos[idx] & (1 << ofst)) == 0) {
    return false;
  } else {
    return true;
  }
}

/* ==== Static Function ==== */
static void initEncParam(ENC_PARAM* pParam, char* argv[])
{
  uint32_t mod;
  uint32_t numOfFailNum;

  /* set encode configuration */
  pParam->fileName = argv[1];
  pParam->encType = atoi(argv[2]);
  pParam->k = atoi(argv[3]);
  pParam->m = atoi(argv[4]);
  pParam->w = atoi(argv[5]);
  pParam->chunkSize = atoi(argv[6]) * 1024;
  numOfFailNum = atoi(argv[7]);

  /* adjusted chunk size */
  mod = pParam->chunkSize % (pParam->k * pParam->w * sizeof(long));
  if (mod != 0) {
    pParam->chunkSize -= mod;
  }

#ifdef DEBUG
  printf(" Target File: %s\n ENC_TYPE: %d\n k = %d\n m = %d\n w = %d\n chunkSize = %d[B]\n # of fail = %d\n", pParam->fileName, pParam->encType, pParam->k, pParam->m, pParam->w, pParam->chunkSize, numOfFailNum);
#endif /* DEBUG */

  /* TODO: adjust parameters */

  /* source size */
  getOriginalDataSize(pParam);
  /* calc number of chunk */
  calcChunkNum(pParam);

  /* get matrix or tables */
  getMatrix(pParam);

  /* allocate buffer */
  allocateBuffer(pParam);

  /* set fail position */
  setFailPos(pParam, numOfFailNum);
}

static void getOriginalDataSize(ENC_PARAM* pParam)
{
  struct stat status;

  stat(pParam->fileName, &status);	
  pParam->orgDataSize = status.st_size;
}

static void calcChunkNum(ENC_PARAM* pParam)
{
  pParam->chunkNum = pParam->orgDataSize / pParam->chunkSize;
  if (pParam->orgDataSize > (pParam->chunkNum * pParam->chunkSize)) {
    pParam->chunkNum++;
  }
}

static void getMatrix(ENC_PARAM* pParam)
{
  //uint32_t size;
  //size = pParam->k * (pParam->k + pParam->m);

  /* initialize */
  pParam->array = NULL;
  pParam->gfTbls = NULL;

  switch (pParam->encType) {
    case ENC_TYPE_NON:
      /* nothing to do */
      break;

    case ENC_TYPE_JERASURE:
      pParam->array = (uint8_t*)reed_sol_vandermonde_coding_matrix(pParam->k, pParam->m, pParam->w);
      break;

    case ENC_TYPE_ISAL:
      //pParam->array = (uint8_t*)calloc(1, sizeof(uint8_t)*size);
      //gf_gen_rs_matrix(pParam->array, (pParam->k + pParam->m), pParam->k);

      //pParam->gfTbls = (uint8_t*)calloc(1, sizeof(uint8_t)*(size * 32));
      //ec_init_tables(pParam->k, pParam->m, &pParam->array[pParam->k * pParam->k], pParam->gfTbls);
      /* TODO: cauchy1 matrix */
      break;

    default:
      break;
  }

}

static void allocateBuffer(ENC_PARAM* pParam)
{
  uint32_t i;
  uint32_t blockSize;

  pParam->pChunk = (uint8_t*)malloc(sizeof(uint8_t)*(pParam->chunkSize));

  pParam->src = (uint8_t**)malloc(sizeof(uint8_t*)*(pParam->k));
  pParam->coding = (uint8_t**)malloc(sizeof(uint8_t*)*(pParam->m));

  blockSize = pParam->chunkSize / pParam->k;
  for (i = 0; i< pParam->m; i++) {
    pParam->coding[i] = (uint8_t*)malloc(sizeof(uint8_t)*blockSize);
  }
}

static void finalizeEnc(ENC_PARAM* pParam)
{
  uint32_t i;
  
  if (pParam->array != NULL) {
    free(pParam->array);
  }
  if (pParam->gfTbls != NULL) {
    free(pParam->gfTbls);
  }

  for (i = 0; i< pParam->m; i++) {
    free(pParam->coding[i]);
  }
  free(pParam->coding);
  free(pParam->src);
  free(pParam->pChunk);
  free(pParam->failPos);
}

static void setFailPos(ENC_PARAM* pParam, uint32_t numOfFailNum)
{
  uint32_t numOfIdx, mod;
  uint32_t pos, idx, ofst;
  uint32_t totalPosNum;
  uint32_t i;

  totalPosNum = pParam->k + pParam->m;
  numOfIdx = totalPosNum / 32;  /* 32 means bits of int */
  mod = totalPosNum & 31;

  /* bitmap for fail position */
  if (mod != 0) numOfIdx++;
  pParam->failPos = (uint32_t*)malloc(sizeof(uint32_t)*numOfIdx);

  if (numOfFailNum >= pParam->k) {
    /* all read fail */
    memset(pParam->failPos, 0xFF, sizeof(uint32_t)*numOfIdx);
    return;
  }

  /* set fail position at pseudo random */
  srand((unsigned)time(NULL));
  for (i = 0; i < numOfFailNum; i++) {
    bool setFlag = false;
    do {
      pos = rand() % totalPosNum;
      if (CheckFailPos(pParam, pos) == false) {
        /* set only non-set bit */
        idx = pos / 32;
        ofst = pos & 31;
        pParam->failPos[idx] |= 1 << ofst;
        setFlag = true;
      }
    } while (setFlag == false);
  }
#ifdef DEBUG
  printf("debug: failPos = 0x%x\n", pParam->failPos[0]);
#endif /* DEBUG */
}

