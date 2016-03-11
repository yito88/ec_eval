#/bin/zsh

RESULT_FILE="result.txt"

# test file
TEST_FILE="testfile"
DD_BS=4096
DD_COUNT=$((8 * 1024))
#echo "test size = $((DD_BS * DD_COUNT / 1024))[KB]"
#dd if=/dev/urandom of=${TEST_FILE} bs=${DD_BS} count=${DD_COUNT} &> /dev/null

# default encoding config
ENC_TYPE=1
k=3
m=2
w=8
CHUNK_SIZE=64  # [KB]
FAIL_NUM=0

if [ -e ${RESULT_FILE} ]
then
  rm -f ${RESULT_FILE}
else
  touch ${RESULT_FILE}
fi

if [ -e encoded ]
then
  rm -f encoded/*
else
  mkdir encoded
fi

if [ -e decoded ]
then
  rm -f decoded/*
else
  mkdir decoded
fi

DD_COUNT=1
while [ $DD_COUNT -ne $((64 * 1024)) ]
#while [ $CHUNK_SIZE -ne 256 ]
do
  echo "test size: $((DD_BS * DD_COUNT / 1024))[KB]" | tee -a ${RESULT_FILE}
  #echo "chunk size: ${CHUNK_SIZE}[KB]" | tee -a ${RESULT_FILE}
  # == create test file ==
  dd if=/dev/urandom of=${TEST_FILE} bs=${DD_BS} count=${DD_COUNT} &> /dev/null
  
  # == main ==
  i=0
  while [ $i -ne 10 ]
  do
    echo "k = ${k}, m = ${m}, w = ${w}, chunkSize = ${CHUNK_SIZE}[KB], numOfFail = ${FAIL_NUM}" | tee -a ${RESULT_FILE}

    ./eval ${TEST_FILE} ${ENC_TYPE} ${k} ${m} ${w} ${CHUNK_SIZE} ${FAIL_NUM} 2>&1 | tee -a ${RESULT_FILE}

    # == remove encoded/decoded files ==
    rm -f encoded/* decoded/*

    i=`expr $i + 1`
  done
  
  # == remove test file ==
  rm -f ${TEST_FILE}
 
  DD_COUNT=`expr $DD_COUNT \* 2`
  #CHUNK_SIZE=`expr $CHUNK_SIZE \* 2`

  echo "" | tee -a ${RESULT_FILE}
done

# == remove test file ==
#rm -f ${TEST_FILE}

