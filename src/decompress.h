#ifndef __DECOMPRESS_H__
#define __DECOMPRESS_H__

/* open and read the file, decompress and put the decompressed content
 * into the return buffer
 *
 * the returned buffer is allocated using new[]
 */
unsigned char * decompress(const char * file, unsigned int *size);

#endif

