#include <stdio.h>
#include <stdlib.h>

static unsigned short calcCRC(const unsigned char * buffer, unsigned int cnt) {

  static unsigned short CRCTable[256] = { 0, 0 };

  if (CRCTable[1] == 0) {
    for (unsigned int i = 0; i < 256; i++) {
      unsigned short val = i;
      for (unsigned int j = 0; j < 8; j++) {

        if (val & 1 == 1)
          val = (val >> 1) ^ 0xA001;
        else
          val >>= 1;
      }
      CRCTable[i] = val;
    }
  }

  unsigned short val = 0;

  while (cnt > 0) {
    val = CRCTable[(val^(*buffer++)) & 0xFF] ^ (val >> 8);
    cnt--;
  }

  return val;
}

typedef struct {

  unsigned short bitBuffer;
  unsigned short bitBufferHigh;
  unsigned short bitsInBitBuffer;
  unsigned char * file;
  unsigned int sourcePos;
  unsigned short getBitTable[256];

} decompState;


static unsigned short GetBitsFromBitBuffer(unsigned char num, decompState * s) {

  if (num > 16) {
    printf("error decoding spream: too many bits\n");
    exit(1);
  }

  unsigned short result = s->bitBuffer & ((1 << num) -1);

  if (s->bitsInBitBuffer < num) {

    /* first remove all the bits that are left in high */

    s->bitBuffer >>= s->bitsInBitBuffer;
    s->bitBuffer |= s->bitBufferHigh << (16-s->bitsInBitBuffer);
    num -= s->bitsInBitBuffer;

    /* refill the high part again */
    s->sourcePos += 2;
    s->bitBufferHigh = ((unsigned short)s->file[s->sourcePos]) | ((unsigned short)s->file[s->sourcePos+1] << 8);
    s->bitsInBitBuffer = 16;

  }

  s->bitBuffer >>= num;
  s->bitBuffer |= s->bitBufferHigh << (16-num);
  s->bitBufferHigh >>= num;
  s->bitsInBitBuffer -= num;

  return result;
}


static void doSomething(unsigned short crcIdx, decompState * s) {

  int count = GetBitsFromBitBuffer(5, s);

  if (count == 0) return;

  unsigned char stack[16];

  for (int i = 0; i < count; i++)
    stack[i] = GetBitsFromBitBuffer(4, s);

  unsigned short mask = 0x8000;
  unsigned short bx = 0;

  unsigned short idx = crcIdx;

  for (int i = 1; i < 17; i++) {

    for (int j = 0; j < count; j++) {

      if (stack[j] == i) {

        unsigned short val = (1 << stack[j]) -1;
        s->getBitTable[idx++] = val;

        val = (bx >> (16-stack[j]));

        unsigned short val2=0;
        for (int k = 0; k < stack[j]; k++) {
          val2 <<= 1;
          if (val & 1) val2 |= 1;
          val >>= 1;
        }
        s->getBitTable[idx++] = val2;

        val = (((unsigned short)stack[j]) << 8) + j;
        s->getBitTable[idx+30] = val;

        bx += mask;

      }
    }

    mask >>= 1;
  }
}

static unsigned short crazyGetBit(unsigned short crcIdx, decompState * s) {
  while ((s->getBitTable[crcIdx] & s->bitBuffer) != s->getBitTable[crcIdx+1])
    crcIdx+=2;
  crcIdx+=2;

  unsigned short cx = s->getBitTable[crcIdx+30];

  GetBitsFromBitBuffer(cx >> 8, s);
  cx &= 0xFF;

  if (cx < 2) return cx;

  cx--;

  return GetBitsFromBitBuffer(cx, s) | (1 << cx);
}

unsigned char * decompress(const char * fname, unsigned int *size) {

  decompState st;

  /* read the file, allocate the right amount of memory */
  {
    FILE * f = fopen(fname, "rb");
    if (!f) {
      printf("file %s not found\n", fname);
      exit(1);
    }

    fseek(f, 0, SEEK_END);
    unsigned long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize > 10000000) {
      printf("file %s too big\n", fname);
      exit(1);
    }

    if (fsize < 20) {
      printf("file %s too small, doesn't even contain header\n", fname);
      exit(1);
    }

    // we allocate 4 more byes, they are used to refill the bit buffer
    st.file = new unsigned char[fsize+4];

    fread(st.file, 1, fsize, f);
    fclose(f);
  }

  /* check the header of the file */
  {
    unsigned int header = ((int)st.file[0] << 24) + ((int)st.file[1] << 16) + ((int)st.file[2] << 8) + st.file[3];

    if (header != 0x524E4301) {   // RNC1
      printf("file %s is no compressed file\n", fname);
      exit(1);
    }
  }

  /* get CRC and sizes for compressed and decompressed data */
  unsigned int decodedSize = ((unsigned int)st.file[ 4] << 24) + ((unsigned int)st.file[ 5] << 16)
    + ((unsigned int)st.file[ 6] << 8) + (unsigned int)st.file[ 7];
  unsigned int encodedSize = ((unsigned int)st.file[ 8] << 24) + ((unsigned int)st.file[ 9] << 16)
    + ((unsigned int)st.file[10] << 8) + (unsigned int)st.file[11];
  unsigned short decodedCRC = ((unsigned short)st.file[12] << 8) + (unsigned short)st.file[13];
  unsigned short encodedCRC = ((unsigned short)st.file[14] << 8) + (unsigned short)st.file[15];

  /* check the CRC sum of the compressed data */
  {
    unsigned short CRC = calcCRC(st.file+18, encodedSize);

    if (CRC != encodedCRC) {
      printf("encoded Checksums don't match in file %s\n", fname);
      exit(1);
    }
  }

  unsigned char * decoded = new unsigned char[decodedSize];

  st.bitsInBitBuffer = 0;
  st.bitBufferHigh = 0;
  unsigned int targetPos = 0;
  st.sourcePos = 18;
  unsigned short key = 0x1984;
  unsigned int numPasses = st.file[17];

  st.bitBuffer = ((unsigned short)st.file[st.sourcePos]) | ((unsigned short)st.file[st.sourcePos+1] << 8);

  GetBitsFromBitBuffer(2, &st);

  do {

    doSomething(0x40, &st);
    doSomething(0x80, &st);
    doSomething(0xC0, &st);

    unsigned short internalPasses = GetBitsFromBitBuffer(16, &st);

    while (true) {  // internal Passes loop

      unsigned short num = crazyGetBit(0x40, &st);

      if (num > 0) {

        if (targetPos+num > decodedSize) {
          printf("error decoding file: too long\n");
          exit(1);
        }

        while (num) {

          decoded[targetPos] = st.file[st.sourcePos] ^ key;
          num--;
          targetPos++;
          st.sourcePos++;
        }

        key = key >> 1 | ((key & 1) ? 0x8000 : 0);  // ror key, 1

        // refill bitbuffer

        unsigned short b = (unsigned short)st.file[st.sourcePos] |
          ((unsigned short)st.file[st.sourcePos+1] << 8);
        unsigned short a = b;

        for (int i = 0; i < st.bitsInBitBuffer; i++)
          a = a << 1 | ((a & 0x8000) ? 1 : 0);  // ror a, 1

        unsigned short d = (1 << st.bitsInBitBuffer) -1;

        st.bitBuffer &= d;
        d &= a;

        a = (unsigned short)st.file[st.sourcePos+2] |
          ((unsigned short)st.file[st.sourcePos+3] << 8);

        st.bitBuffer |= b << st.bitsInBitBuffer;
        st.bitBufferHigh = (a << st.bitsInBitBuffer) | d;
      }

      internalPasses--;
      if (internalPasses == 0) break;

      unsigned ofs = crazyGetBit(0x80, &st)+1;
      num = crazyGetBit(0xC0, &st) + 2;

      if (targetPos+num > decodedSize) {
        printf("error decoding file: too long\n");
        exit(1);
      }

      for (int i = 0; i < num; i++) {
        decoded[targetPos] = decoded[targetPos-ofs];
        targetPos++;
      }
    }

    numPasses--;

  } while (numPasses > 0);

  delete [] st.file;

  /* check CRC of the decoded data */
  {
    unsigned short CRC = calcCRC(decoded, decodedSize);

    if (CRC != decodedCRC) {
      printf("decoded Checksums don't match in file %s\n", fname);
      exit(1);
    }
  }

  /* set decoded size and return */
  if (size) {
    *size = decodedSize;
  }

  return decoded;
}

