#ifdef BASIC_AP_UINT
	#include "basic_ap_uint.h"
#else
	#include "my_ap_uint.h"
#endif

#define WORKLOAD_TASK_SIZE  3
#define INDEX_SIZE          2
#define BASE_SIZE           2


#ifndef WORKLOAD_CHUNK
#define WORKLOAD_CHUNK 1024*16
#endif

#ifdef ENTRY_TYPE_0
unsigned int computeTaskEntryType0(__global unsigned char* restrict pairs, unsigned int pi);
unsigned int computeDistance(ap_uint_512 pattern, int plen, ap_uint_512 text,  int tlen);
void printSequence(ap_uint_512 w, int len);
#endif

#ifdef ENTRY_TYPE_1
unsigned int computeTaskEntryType1(__global unsigned char* restrict pairs, unsigned int pi);
unsigned int computeDistance(ap_uint_512 pattern, int plen, ap_uint_512 text,  int tlen);
void printSequence(ap_uint_512 w, int len);
#endif

#ifdef ENTRY_TYPE_2
unsigned int computeTaskEntryType2(__global unsigned char* restrict pairs, unsigned int pi);
unsigned int computeDistance(ap_uint_1024 pattern, int plen, ap_uint_1024 text,  int tlen);
void printSequence1024(ap_uint_1024 w, int len);
#endif




/**
 * from version 11 we assume that pattern index, and text index is the same worload index  
 */
void doWorkloadTask(__global unsigned char* restrict pairs ,
                    /*__global*/ unsigned int* workload, unsigned int wi, unsigned int li)
{
    unsigned int pi = wi; // workload[wi*WORKLOAD_TASK_SIZE+0];
    unsigned int ti = wi; // workload[wi*WORKLOAD_TASK_SIZE+1];

#ifdef ENTRY_TYPE_0
    unsigned int d = computeTaskEntryType0(pairs,  pi);
#endif
#ifdef ENTRY_TYPE_1
    unsigned int d = computeTaskEntryType1(pairs,  pi);
#endif
#ifdef ENTRY_TYPE_2
    unsigned int d = computeTaskEntryType2(pairs,  pi);
#endif

#ifdef FPGA_DEBUG
    printf("[FPGA] pi=%d  ", pi);
    printf(" task %d = %d\n", wi, d);
#endif
    
    //workload[wi*WORKLOAD_TASK_SIZE+2] = d;
    workload[li] = d;
}


__kernel void kmer(__global unsigned char* restrict pairs ,
		   __global unsigned int* workload, 
		   unsigned int workloadLength)
{
#ifdef FPGA_DEBUG
	// test_my_ap_uint();
#endif

	
	unsigned int workload_result[WORKLOAD_CHUNK];
	
    for (int i=0; i < workloadLength; /*i++*/)
    {
    	int base_i = i;
        int li;
	
    	// compute to local memory
    	for (li=0; (li < WORKLOAD_CHUNK) && (i < workloadLength); li++, i++)
	{
	   doWorkloadTask(pairs, workload_result, i, li);
	}
		
	// transfer the results back to the main table
	for (int ti=0; ti < li; ti++)
		workload[(base_i + ti)*WORKLOAD_TASK_SIZE+2] = workload_result[ti];
     }
}

void readBigEndian512bits(__global unsigned char* restrict p, ap_uint_512p ret)
{
    ap_uint_512_zero(ret);
    
    #pragma unroll
    for (int i=0; i < 512/8; i++)
    {
        // ret |= p[i] << (i * 8);
        // ap_uint_512_shift_left_self(8, ret);
        ap_uint_512_orHighByteConcurrent(ret, i, p[i]);

	// printf("[%d] = 0x%02X\n", i, p[i]);
    }   

#ifdef FPGA_DEBUG
    printf("Long Word: ");
    ap_uint_512_print(AP_UINT_FROM_PTR(ret));
    printf("\n");
#endif
}


#ifdef ENTRY_TYPE_2

void readBigEndian1024bits(__global unsigned char* restrict p, ap_uint_1024p ret)
{
    ap_uint_1024_zero(ret);
    
    #pragma unroll
    for (int i=0; i < 1024/8; i++)
    {
        // ret |= p[i] << (i * 8);
        // ap_uint_512_shift_left_self(8, ret);
        ap_uint_1024_orHighByteConcurrent(ret, i, p[i]);

	// printf("[%d] = 0x%02X\n", i, p[i]);
    }   

#ifdef FPGA_DEBUG
    printf("Long Word: ");
    ap_uint_1024_print(AP_UINT_FROM_PTR(ret));
    printf("\n");
#endif
}

#endif




#ifdef ENTRY_TYPE_0

void readPairs(__global unsigned char* restrict pattern , unsigned int pi, ap_uint_512p ret)
{
    unsigned int offset = pi * 512 /  8;
    
    readBigEndian512bits(&pattern[offset], ret);  
}


unsigned int computeTaskEntryType0(__global unsigned char* restrict pairs,
                 unsigned int pi)
{
	int d = 0;

	ap_uint_512 pairs_word;
	
	ap_uint_512 pattern_word;
	ap_uint_512 text_word;

	readPairs(pairs, pi, AP_UINT_PTR(pairs_word));

#ifdef PATTERN_LEN
	unsigned char pl = PATTERN_LEN;
	unsigned char tl = TEXT_LEN;
#else
	unsigned char pl = ap_uint_512_getHighByte(pairs_word, 0);
	unsigned char tl = ap_uint_512_getHighByte(pairs_word, 1);
#endif

#ifdef FPGA_DEBUG
	printf("pattern len: %d\ttext len: %d\n", pl, tl);
#endif
	int alignedTextStart = 1 + 1 + (((pl * BASE_SIZE) + 7) / 8) ;	// in bytes

	ap_uint_512_shift_left_bytes(pairs_word, 2, AP_UINT_PTR(pattern_word));
	ap_uint_512_shift_left_bytes(pairs_word, alignedTextStart, AP_UINT_PTR(text_word));


#ifdef FPGA_DEBUG
	printf("T:   ");
	printSequence(text_word, tl);
	printf("\n");
	printf("P:   ");        
	printSequence(pattern_word, pl);
	printf("\n");
#endif

	/*printf("T:   ");
	ap_uint_512_print(text_word);
	printf("\n");
	printf("P:   ");        
	ap_uint_512_print(pattern_word);
	printf("\n");*/


	d = computeDistance(pattern_word,  pl, text_word,  tl);	// we just compare pattern
	
	return d;
}
#endif



#ifdef ENTRY_TYPE_1

void readPairs(__global unsigned char* restrict pattern , unsigned int pi, ap_uint_512p pword,  ap_uint_512p tword )
{
    unsigned int offsetp = (pi * 2 * 512) /  8;
    unsigned int offsett = ((pi * 2 * 512) + 512) /  8;
    
    readBigEndian512bits(&pattern[offsetp], pword);  
    readBigEndian512bits(&pattern[offsett], tword);  
}

unsigned int computeTaskEntryType1(__global unsigned char* restrict pairs,
                 unsigned int pi)
{
	int d = 0;

	ap_uint_512 pairs_word_p;
	ap_uint_512 pairs_word_t;
	
	ap_uint_512 pattern_word;
	ap_uint_512 text_word;

	readPairs(pairs, pi, AP_UINT_PTR(pairs_word_p), AP_UINT_PTR(pairs_word_t));

#ifdef PATTERN_LEN
	unsigned char pl = PATTERN_LEN;
	unsigned char tl = TEXT_LEN;
#else
	unsigned char pl = ap_uint_512_getHighByte(pairs_word_p, 0);
	unsigned char tl = ap_uint_512_getHighByte(pairs_word_t, 0);
#endif

#ifdef FPGA_DEBUG
	printf("pattern len: %d\ttext len: %d\n", pl, tl);
#endif
	//int alignedTextStart = 1 + 1 + (((pl * BASE_SIZE) + 7) / 8) ;	// in bytes

	ap_uint_512_shift_left_bytes(pairs_word_p, 1, AP_UINT_PTR(pattern_word));
	ap_uint_512_shift_left_bytes(pairs_word_t, 1, AP_UINT_PTR(text_word));


#ifdef FPGA_DEBUG
	printf("T:   ");
	printSequence(text_word, tl);
	printf("\n");
	printf("P:   ");        
	printSequence(pattern_word, pl);
	printf("\n");
#endif

	/*printf("T:   ");
	ap_uint_512_print(text_word);
	printf("\n");
	printf("P:   ");        
	ap_uint_512_print(pattern_word);
	printf("\n");*/


	d = computeDistance(pattern_word,  pl, text_word,  tl);	// we just compare pattern
	
	return d;
}
#endif



#ifdef ENTRY_TYPE_2

void readPairs(__global unsigned char* restrict pattern , unsigned int pi, ap_uint_1024p pword,  ap_uint_1024p tword )
{
    // every pair is 2 * 1024
    unsigned int offsetp = (pi * 2 * 1024) /  8;
    unsigned int offsett = ((pi * 2 * 1024) + 512) /  8;
    
    readBigEndian1024bits(&pattern[offsetp], pword);  
    readBigEndian1024bits(&pattern[offsett], tword);  
}

unsigned int computeTaskEntryType2(__global unsigned char* restrict pairs, unsigned int pi)
{
	int d = 0;

	ap_uint_1024 pairs_word_p;
	ap_uint_1024 pairs_word_t;
	
	ap_uint_1024 pattern_word;
	ap_uint_1024 text_word;

	readPairs(pairs, pi, AP_UINT_PTR(pairs_word_p), AP_UINT_PTR(pairs_word_t));

#ifdef PATTERN_LEN
	unsigned int pl = PATTERN_LEN;
	unsigned int tl = TEXT_LEN;
#else
	unsigned int pl = ap_uint_1024_getHighByte(pairs_word_p, 0);
	unsigned int tl = ap_uint_1024_getHighByte(pairs_word_t, 0);
#endif

#ifdef FPGA_DEBUG
	printf("pattern len: %d\ttext len: %d\n", pl, tl);
#endif
	//int alignedTextStart = 1 + 1 + (((pl * BASE_SIZE) + 7) / 8) ;	// in bytes

	ap_uint_1024_shift_left_bytes(pairs_word_p, 1, AP_UINT_PTR(pattern_word));
	ap_uint_1024_shift_left_bytes(pairs_word_t, 1, AP_UINT_PTR(text_word));


#ifdef FPGA_DEBUG
	printf("T:   ");
	printSequence1024(text_word, tl);
	printf("\n");
	printf("P:   ");        
	printSequence1024(pattern_word, pl);
	printf("\n");
#endif

	/*printf("T:   ");
	ap_uint_512_print(text_word);
	printf("\n");
	printf("P:   ");        
	ap_uint_512_print(pattern_word);
	printf("\n");*/


	d = computeDistance(pattern_word,  pl, text_word,  tl);	// we just compare pattern
	
	return d;
}
#endif

/**
* we assume the sequence is left aligned to the 512 bits word, and stored in Most Significant Bit First order
* @param w
* @param offset
* @param len	length in bases
*/
void printSequence(ap_uint_512 w, int len)
{
	char sym[]={'A','C','G', 'T'};
	
	
	for (int i=0; i < len; i++)
	{
	   int last = 512 - 1 - (i * BASE_SIZE);
	   unsigned char isym = ap_uint_512_get_bit(w, last - 0 ) << 1 | ap_uint_512_get_bit(w, last -1);  
           printf("%c", sym[isym]);	   
	}

}

/**
* we assume the sequence is left aligned to the 512 bits word, and stored in Most Significant Bit First order
* @param w
* @param offset
* @param len	length in bases
*/
void printSequence1024(ap_uint_1024 w, int len)
{
	char sym[]={'A','C','G', 'T'};
	
	
	for (int i=0; i < len; i++)
	{
	   int last = 1024 - 1 - (i * BASE_SIZE);
	   unsigned char isym = ap_uint_1024_get_bit(w, last - 0 ) << 1 | ap_uint_1024_get_bit(w, last -1);  
           printf("%c", sym[isym]);	   
	}

}
