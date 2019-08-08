//=====================================================================
//
// isecure.h - secure hash encrypt
//
// NOTE:
// for more information, please see the readme file.
//
// Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
// Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
//
//=====================================================================
#ifndef __ISECURE_H__
#define __ISECURE_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


//=====================================================================
// 32BIT INTEGER DEFINITION 
//=====================================================================
#ifndef __INTEGER_32_BITS__
#define __INTEGER_32_BITS__
#if defined(_WIN64) || defined(WIN64) || defined(__amd64__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_IA64) || \
	defined(_M_AMD64)
	typedef unsigned int ISTDUINT32;
	typedef int ISTDINT32;
#elif defined(_WIN32) || defined(WIN32) || defined(__i386__) || \
	defined(__i386) || defined(_M_X86)
	typedef unsigned long ISTDUINT32;
	typedef long ISTDINT32;
#elif defined(__MACOS__)
	typedef UInt32 ISTDUINT32;
	typedef SInt32 ISTDINT32;
#elif defined(__APPLE__) && defined(__MACH__)
	#include <sys/types.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif defined(__BEOS__)
	#include <sys/inttypes.h>
	typedef u_int32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#elif (defined(_MSC_VER) || defined(__BORLANDC__)) && (!defined(__MSDOS__))
	typedef unsigned __int32 ISTDUINT32;
	typedef __int32 ISTDINT32;
#elif defined(__GNUC__)
	#include <stdint.h>
	typedef uint32_t ISTDUINT32;
	typedef int32_t ISTDINT32;
#else 
	typedef unsigned long ISTDUINT32; 
	typedef long ISTDINT32;
#endif
#endif


//=====================================================================
// Global Macros
//=====================================================================
#ifndef __IINT8_DEFINED
#define __IINT8_DEFINED
typedef char IINT8;
#endif

#ifndef __IUINT8_DEFINED
#define __IUINT8_DEFINED
typedef unsigned char IUINT8;
#endif

#ifndef __IUINT16_DEFINED
#define __IUINT16_DEFINED
typedef unsigned short IUINT16;
#endif

#ifndef __IINT16_DEFINED
#define __IINT16_DEFINED
typedef short IINT16;
#endif

#ifndef __IINT32_DEFINED
#define __IINT32_DEFINED
typedef ISTDINT32 IINT32;
#endif

#ifndef __IUINT32_DEFINED
#define __IUINT32_DEFINED
typedef ISTDUINT32 IUINT32;
#endif

#ifndef __IINT64_DEFINED
#define __IINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef __int64 IINT64;
#else
typedef long long IINT64;
#endif
#endif

#ifndef __IUINT64_DEFINED
#define __IUINT64_DEFINED
#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 IUINT64;
#else
typedef unsigned long long IUINT64;
#endif
#endif

#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__BORLANDC__) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif


//=====================================================================
// DETECTION WORD ORDER
//=====================================================================
#ifndef IWORDS_BIG_ENDIAN
    #ifdef _BIG_ENDIAN_
        #if _BIG_ENDIAN_
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MISPEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
            #define IWORDS_BIG_ENDIAN 1
        #endif
    #endif
    #ifndef IWORDS_BIG_ENDIAN
        #define IWORDS_BIG_ENDIAN  0
    #endif
#endif

#ifndef IASSERT
#define IASSERT(x) assert(x)
#endif


#ifdef __cplusplus
extern "C" {
#endif

//=====================================================================
// Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
//=====================================================================
typedef struct
{
	IUINT32 i[2];                   /* Number of _bits_ handled mod 2^64 */
	IUINT32 buf[4];                                    /* Scratch buffer */
	unsigned char in[64];                              /* Input buffer */
}	HASH_MD5_CTX;

void HASH_MD5_Init(HASH_MD5_CTX *ctx, unsigned long RandomNumber);
void HASH_MD5_Update(HASH_MD5_CTX *ctx, const void *input, unsigned int len);
void HASH_MD5_Final(HASH_MD5_CTX *ctx, unsigned char digest[16]);



//=====================================================================
// From http://www.mirrors.wiretapped.net/security/cryptography
//=====================================================================
typedef struct {
	IUINT32 state[5];
    IUINT32 count[2];
    unsigned char buffer[64];
}	HASH_SHA1_CTX;

void HASH_SHA1_Init(HASH_SHA1_CTX *ctx);
void HASH_SHA1_Update(HASH_SHA1_CTX *ctx, const void *input, unsigned int len);
void HASH_SHA1_Final(HASH_SHA1_CTX *ctx, unsigned char digest[20]);


//=====================================================================
// UTILITIES
//=====================================================================

// convert digests to string
char* hash_digest_to_string(const unsigned char *in, int size, char *out);

// calculate md5sum and convert digests to string
char* hash_md5sum(const void *in, unsigned int len, char *out);

// calculate sha1sum and convert digests to string
char* hash_sha1sum(const void *in, unsigned int len, char *out);

// calculate crc32 and return result
IUINT32 hash_crc32(const void *in, unsigned int len);

// sum all bytes together
IUINT32 hash_checksum(const void *in, unsigned int len);


//=====================================================================
// Diffie-Hellman key exchange
// http://zh.wikipedia.org/wiki/Diffie%E2%80%93Hellman_key_exchange
// usage: 1. get an local asymmetric-key a from DH_Random
//        2. calculate A=(5 ^ a) % p by DH_Exchange 
//        3. send A to remote
//        4. obtain symmetrical-key by DH_Key(local_key, RemoteA)
//=====================================================================

// returns random local key
IUINT64 DH_Random();

// calculate A/B which will be sent to remote
IUINT64 DH_Exchange(IUINT64 local);

// get final symmetrical-key from local key and remote A/B
IUINT64 DH_Final(IUINT64 local, IUINT64 remote);

// get qword from hex string 
void DH_STR_TO_U64(const char *str, IUINT64 *x);

// hex string from qword, capacity of str must above 17
void DH_U64_TO_STR(IUINT64 x, char *str);


//=====================================================================
// CRYPTO RC4
//=====================================================================
typedef struct {
	int x;
	int y;
	unsigned char box[256];
}	CRYPTO_RC4_CTX;


void CRYPTO_RC4_Init(CRYPTO_RC4_CTX *ctx, const void *key, int keylen);

void CRYPTO_RC4_Apply(CRYPTO_RC4_CTX *ctx, const void *in, void *out, 
	size_t size);

void CRYPTO_RC4_Crypto(const void *key, int keylen, const void *in,
	void *out, size_t size, int ntimes);



//=====================================================================
// CRYPTO XTEA: https://en.wikipedia.org/wiki/XTEA
//=====================================================================

void CRYPTO_XTEA_Encipher(int nrounds, const IUINT32 key[4], IUINT32 v[2]);

void CRYPTO_XTEA_Decipher(int nrounds, const IUINT32 key[4], IUINT32 v[2]);


//=====================================================================
// LCG: https://en.wikipedia.org/wiki/Linear_congruential_generator
//=====================================================================

// rand() in stdlib.h (c99), output range: 0 <= x <= 32767
IUINT32 random_std_c99(IUINT32 *seed);

// rand() in stdlib.h (msvc), output range: 0 <= x <= 32767
IUINT32 random_std_msvc(IUINT32 *seed);

// minstd_rand in C++, output range: 0 <= x < 0x7fffffff
IUINT32 random_std_cpp(IUINT32 *seed);


//=====================================================================
// Statistically perfect random generator
//=====================================================================
typedef struct
{
	IUINT32 seed;      // random seed
	IUINT32 size;      // array size
	IUINT32 avail;     // available numbers
	IUINT32 *state;    // states array
}	RANDOM_BOX;


// initialize random box
void RANDOM_BOX_Init(RANDOM_BOX *box, IUINT32 *state, IUINT32 size);

// change seed
void RANDOM_BOX_Seed(RANDOM_BOX *box, IUINT32 seed);

// next random number within 0 <= x < size
IUINT32 RANDOM_BOX_Next(RANDOM_BOX *box);


//=====================================================================
// PCG: PCG is a family of simple fast statistically good algorithms
//=====================================================================
typedef struct
{
	IUINT64 state;    // RNG state.  All values are possible.
	IUINT64 inc;      // Must *always* be odd.
}	RANDOM_PCG;

// initialize pcg 
void RANDOM_PCG_Init(RANDOM_PCG *pcg, IUINT64 initstate, IUINT64 initseq);

// next random number 
IUINT32 RANDOM_PCG_Next(RANDOM_PCG *pcg);

// next random number within 0 <= x < bound
IUINT32 RANDOM_PCG_RANGE(RANDOM_PCG *pcg, IUINT32 bound);


#ifdef __cplusplus
}
#endif

#endif


