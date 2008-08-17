#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short CRCTable[256];

void calcCRCTable(void) {

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

unsigned short calcCRC(const unsigned char * buffer, unsigned int cnt) {

    calcCRCTable();

    unsigned short val = 0;

    while (cnt > 0) {
        val = CRCTable[(val^(*buffer++)) & 0xFF] ^ (val >> 8);
        cnt--;
    }

    return val;
}

unsigned char file[100000];
unsigned char decoded[100000];
short bitsInBitBuffer = 0;
unsigned short bitBuffer;
unsigned short bitBufferHigh;
unsigned int targetPos = 0;
unsigned int sourcePos = 18;

unsigned short GetBitsFromBitBuffer(unsigned char num) {

  if (num > 16) {
    printf("too many bits\n");
    exit(1);
  }

  unsigned short result = bitBuffer & ((1 << num) -1);


  if (bitsInBitBuffer < num) {

    /* first remove all the bits that are left in high */

    bitBuffer >>= bitsInBitBuffer;
    bitBuffer |= bitBufferHigh << (16-bitsInBitBuffer);
    num -= bitsInBitBuffer;

    /* refill the high part again */
    sourcePos += 2;
    bitBufferHigh = ((unsigned short)file[sourcePos]) | ((unsigned short)file[sourcePos+1] << 8);
    bitsInBitBuffer = 16;

  }

  bitBuffer >>= num;
  bitBuffer |= bitBufferHigh << (16-num);
  bitBufferHigh >>= num;
  bitsInBitBuffer -= num;

  return result;
}

void doSomething(unsigned short crcIdx) {

  int count = GetBitsFromBitBuffer(5);

  if (count == 0) return;

  unsigned char stack[16];

  for (int i = 0; i < count; i++)
    stack[i] = GetBitsFromBitBuffer(4);

  unsigned short mask = 0x8000;
  unsigned short bx = 0;

  unsigned short idx = crcIdx;

  for (int i = 1; i < 17; i++) {

    for (int j = 0; j < count; j++) {

      if (stack[j] == i) {

        unsigned short val = (1 << stack[j]) -1;
        CRCTable[idx++] = val;

        val = (bx >> (16-stack[j]));

        unsigned short val2=0;
        for (int k = 0; k < stack[j]; k++) {
          val2 <<= 1;
          if (val & 1) val2 |= 1;
          val >>= 1;
        }
        CRCTable[idx++] = val2;

        val = (((unsigned short)stack[j]) << 8) + j;
        CRCTable[idx+30] = val;

        bx += mask;

      }
    }

    mask >>= 1;
  }
}

unsigned short crazyGetBit(unsigned short crcIdx) {
  while ((CRCTable[crcIdx] & bitBuffer) != CRCTable[crcIdx+1])
    crcIdx+=2;
  crcIdx+=2;

  unsigned short cx = CRCTable[crcIdx+30];

  unsigned char al = GetBitsFromBitBuffer(cx >> 8);
  cx &= 0xFF;

  if (cx < 2) return cx;

  cx--;

  unsigned short a = GetBitsFromBitBuffer(cx);

  return a | (1 << cx);
}

void decompress2(unsigned short key) {


  unsigned int numPasses = file[17];

  bitBuffer = ((unsigned short)file[sourcePos]) | ((unsigned short)file[sourcePos+1] << 8);

  GetBitsFromBitBuffer(2);

  do {

    doSomething(0x40);
    doSomething(0x80);
    doSomething(0xC0);

    unsigned short internalPasses = GetBitsFromBitBuffer(16);

    while (true) {  // internal Passes loop

      unsigned short num = crazyGetBit(0x40);

      if (num > 0) {

        while (num) {

          decoded[targetPos] = file[sourcePos] ^ key;
          num--;
          targetPos++;
          sourcePos++;
        }

        key = key >> 1 | ((key & 1) ? 0x8000 : 0);  // ror key, 1

        // refill bitbuffer

        unsigned short b = (unsigned short)file[sourcePos] |
          ((unsigned short)file[sourcePos+1] << 8);
        unsigned short a = b;

        for (int i = 0; i < bitsInBitBuffer; i++)
          a = a << 1 | ((a & 0x8000) ? 1 : 0);  // ror a, 1

        unsigned short d = (1 << bitsInBitBuffer) -1;

        bitBuffer &= d;
        d &= a;

        a = (unsigned short)file[sourcePos+2] |
          ((unsigned short)file[sourcePos+3] << 8);

        bitBuffer |= b << bitsInBitBuffer;
        bitBufferHigh = (a << bitsInBitBuffer) | d;
      }

      internalPasses--;
      if (internalPasses == 0) break;

      unsigned ofs = crazyGetBit(0x80)+1;
      num = crazyGetBit(0xC0) + 2;

      for (int i = 0; i < num; i++) {
        decoded[targetPos] = decoded[targetPos-ofs];
        targetPos++;
      }
    }

    numPasses--;
  } while (numPasses > 0);
}



unsigned char * decompress(const char * fname, unsigned int *size) {

    {
        FILE * f = fopen(fname, "rb");
        if (!f) {
//            printf("file not found\n");
            return 0;
        }
        fread(file, 1, 100000, f);
        fclose(f);
    }

    unsigned int header = ((int)file[0] << 24) + ((int)file[1] << 16) + ((int)file[2] << 8) + file[3];

    if (header != 0x524E4301) {   // RNC1
//        printf("no compressed file %x\n", header);
        return 0;
    }

    unsigned int decodedSize = ((unsigned int)file[ 4] << 24) + ((unsigned int)file[ 5] << 16) + ((unsigned int)file[ 6] << 8) + (unsigned int)file[ 7];
    unsigned int encodedSize = ((unsigned int)file[ 8] << 24) + ((unsigned int)file[ 9] << 16) + ((unsigned int)file[10] << 8) + (unsigned int)file[11];
    unsigned short decodedCRC = ((unsigned short)file[12] << 8) + (unsigned short)file[13];
    unsigned short encodedCRC = ((unsigned short)file[14] << 8) + (unsigned short)file[15];

    printf("%i %i %x %x\n", encodedSize, decodedSize, encodedCRC, decodedCRC);

    unsigned short CRC = calcCRC(file+18, encodedSize);

    if (CRC != encodedCRC) {
//      printf("encoded Checksums don't match: %x  %x\n", encodedCRC, CRC);
      return 0;
    }

    bitsInBitBuffer = 0;
    targetPos = 0;
    sourcePos = 18;

    decompress2(0x1984);

    CRC = calcCRC(decoded, decodedSize);

    if (CRC != decodedCRC) {
//      printf("decoded Checksums don't match savin anyway: %x  %x\n", decodedCRC, CRC);
      return 0;
    }

    unsigned char * res = new unsigned char[decodedSize];
    memcpy(res, decoded, decodedSize);

    if (size) {
      *size = decodedSize;
    }

    return res;
}

