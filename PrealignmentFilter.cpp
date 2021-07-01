/*
 * Copyright (C) 2020 Universitat Autonoma de Barcelona - David Castells-Rufas <david.castells@uab.cat>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/* 
 * File:   FPGAKmerFilter.cpp
 * Author: dcr
 * 
 * Created on February 20, 2020, 6:04 PM
 */

#include "PrealignmentFilter.h"
#include "PerformanceLap.h"
#include "TextUtils.h"
// #include "benchmark/benchmark_edit_alg.h"
#include "edlib.h"

#define WORKLOAD_TASK_SIZE  3
#define INDEX_SIZE          2
#define BASE_SIZE           2
#define LOAD_BASES_ALIGNMENT_BITS   512

#define KMER_K          5
#define KMER_K_BITS     (KMER_K*BASE_SIZE)
#define KMER_BINS       (1 << KMER_K_BITS)

/**
 * Compute the number of bytes required to store the number of bases, considering
 * that we require memory alignment
 * @param bases
 * @return 
 */
unsigned int alignedSequenceSize(int bases)
{
    unsigned int lenBits = bases * BASE_SIZE;         // length in bases
    unsigned int lenAlignedUnits = ((lenBits  + (LOAD_BASES_ALIGNMENT_BITS-1)) / LOAD_BASES_ALIGNMENT_BITS) * LOAD_BASES_ALIGNMENT_BITS; // round up

    unsigned int lenAlignedUnitsBytes = lenAlignedUnits / 8;
    return lenAlignedUnitsBytes;
}

PrealignmentFilter::PrealignmentFilter() 
{
	
}



PrealignmentFilter::~PrealignmentFilter() 
{
}

void PrealignmentFilter::addInput(string pattern, string text) 
{
    int pl = pattern.size();
    int tl = text.size();
    
    
    m_basesPattern.push_back(pattern);
    m_basesText.push_back(text);
    m_basesPatternLength.push_back(pl);
    m_basesTextLength.push_back(tl);

}

#define dna_encode_valid(c)   (((c == 'A') || (c == 'a'))? 0: ((c == 'C') || (c == 'c'))? 1: ((c == 'G') || (c == 'g'))? 2: 3)



string PrealignmentFilter::decodeSequence(unsigned char* pattern, unsigned int offset, unsigned len)
{
	char* sym[]={"A", "C", "G", "T"};

	string bases = "";
	int j = 0;
	for (int i=0; i < len; j++ )
	{
		unsigned char v = pattern[j];

		for (int k=0; (k < 4) && (i < len); k++, i++)
		{
			int isym = (v >> (6-k*2)) & 0x3;
			bases.append(sym[isym]);			
		}
	}

	return bases;
}


void PrealignmentFilter::encodeEntry0( unsigned char* pPattern, unsigned int offset, string pattern, string text )
{
	int pl = pattern.size();
	int tl = text.size();

	assert(pl < 256);
	assert(tl < 256);

	if (m_verbose)
	{
		printf("PL: %d TL: %d\n", pl, tl);
	}

	pPattern[offset + 0] = pl;
	pPattern[offset + 1] = tl;

	int baseByteIdx;
	int baseBitIdx;

	int alignedTextStart = 2 + (((pl * 2) + 7) / 8) ;

	for (int i=0; i < pl; i++)
	{
		int bc = dna_encode_valid(pattern[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + 2 + baseByteIdx] = (pPattern[offset + 2 + baseByteIdx] & nmask) | vset; 
	}

	for (int i=0; i < tl; i++)
	{
		int bc = dna_encode_valid(text[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + alignedTextStart  + baseByteIdx] = (pPattern[offset + alignedTextStart + baseByteIdx] & nmask) | vset; 
	}
}

void PrealignmentFilter::encodeEntry1( unsigned char* pPattern, unsigned int offset, string pattern, string text )
{
	int pl = pattern.size();
	int tl = text.size();

	assert(pl < 256);
	assert(tl < 256);

	if (m_verbose)
	{
		printf("PL: %d TL: %d\n", pl, tl);
	}

	pPattern[offset + 0] = pl;


	int baseByteIdx;
	int baseBitIdx;

	for (int i=0; i < pl; i++)
	{
		int bc = dna_encode_valid(pattern[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + 1 + baseByteIdx] = (pPattern[offset + 1 + baseByteIdx] & nmask) | vset; 
	}

	offset += (512/8);

	pPattern[offset + 0] = tl;

	for (int i=0; i < tl; i++)
	{
		int bc = dna_encode_valid(text[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + 1 + baseByteIdx] = (pPattern[offset + 1 + baseByteIdx] & nmask) | vset; 
	}
}


void PrealignmentFilter::encodeEntry2( unsigned char* pPattern, unsigned int offset, string pattern, string text )
{
	int pl = pattern.size();
	int tl = text.size();

	int pBytes = (pl+3)/4;
	int tBytes = (tl+3)/4;
	
	int bytesPerWord = (512/8);
	int pWords = ((pBytes + 1) + (bytesPerWord-1)) / bytesPerWord;
	int tWords = ((tBytes + 1) + (bytesPerWord-1)) / bytesPerWord;


	if (m_verbose)
	{
		printf("PL: %d TL: %d\n", pl, tl);
	}

	pPattern[offset + 0] = pl;


	int baseByteIdx;
	int baseBitIdx;

	for (int i=0; i < pl; i++)
	{
		int bc = dna_encode_valid(pattern[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + 1 + baseByteIdx] = (pPattern[offset + 1 + baseByteIdx] & nmask) | vset; 
	}

	offset += pWords * bytesPerWord;

	pPattern[offset + 0] = tl;

	for (int i=0; i < tl; i++)
	{
		int bc = dna_encode_valid(text[i]);
		baseByteIdx = i / 4;
		baseBitIdx = (3 - i%4) * 2;
		
		unsigned char vset = bc << baseBitIdx;
		unsigned char mask = 3 << baseBitIdx;
		unsigned char nmask = ~mask;
	
		pPattern[offset + 1 + baseByteIdx] = (pPattern[offset + 1 + baseByteIdx] & nmask) | vset; 
	}
}




void PrealignmentFilter::encodeEntry3( unsigned char* pPattern, unsigned int offset, string pattern, string text )
{
	printf("encodeEntry3 not implemented");
	exit(0);
}

void PrealignmentFilter::setVerbose(bool verbose)
{
	m_verbose = verbose;
}

int PrealignmentFilter::determineEncodingType(size_t* requiredMemory)
{
	// @todo we do not support multiple different encoding types in the same set
	int pl = m_patternLen;	// m_basesPattern[0].size();
	int tl = m_textLen; 	// m_basesText[0].size();

	int pBytes = (pl+3)/4;
	int tBytes = (tl+3)/4;
	
	int bytesPerWord = (512/8);

	printf("pBytes=%d tBytes=%d\n", pBytes, tBytes);

	if ((pBytes + tBytes + 2) <= bytesPerWord) 
	{
		// both pattern and text in the same 512 bits word
		printf("Entry Type 0\n");
		if (requiredMemory != NULL)
			*requiredMemory =  (512/8);
		return 0;
	}
	if (((pBytes + 1) < bytesPerWord) && ((tBytes + 1) < bytesPerWord))
	{
		// pattern in a 512 bits word, and text in the following 512 bits word
		printf("Entry Type 1\n");
		if (requiredMemory != NULL)
			*requiredMemory =  2 * bytesPerWord;
		return 1;
	}
	int pWords = ((pBytes + 1) + (bytesPerWord-1)) / bytesPerWord;
	int tWords = ((tBytes + 1) + (bytesPerWord-1)) / bytesPerWord;

	if (requiredMemory != NULL)
		*requiredMemory =  (pWords + tWords) * bytesPerWord;

	printf("Entry Type 2\n");
	return 2;
	
	// Unsupported encoding
	printf("Unsupported Encoding ! Lengths %d+%d = BYTES %d\n", pl, tl, (pBytes+tBytes+2));
	exit(-1);
	
}

void* PrealignmentFilter::allocMem(size_t size)
{
#ifdef USE_OPENCL_SVM
	return clSVMAllocAltera(m_context, 0 , size, 1024);
#else
	return alignedMalloc(size);
#endif
}

void PrealignmentFilter::freeMem(void* p)
{	
#ifdef USE_OPENCL_SVM
	clSVMFreeAltera(m_context, p);
#else
	alignedFree(p);
#endif
}

void PrealignmentFilter::computeAll(int realErrors)
{
    // allocate memory buffers
    //size_t requiredPatternMemory = countRequiredMemory(m_basesPatternLength);    
    //size_t requiredTextMemory = countRequiredMemory(m_basesTextLength);

    size_t requiredEntryMemory;
    int encodingType = determineEncodingType(&requiredEntryMemory);
    
    size_t requiredMemory = requiredEntryMemory * m_basesPatternLength.size();

    unsigned char* pattern = (unsigned char*) allocMem(requiredMemory);
    unsigned int* workload = (unsigned int*) allocMem(m_basesTextLength.size() * sizeof(unsigned int) * WORKLOAD_TASK_SIZE);
    
    // now fill
    unsigned int poff = 0;  // pattern offset
    unsigned int toff = 0;  // text offset
    
    PerformanceLap lap;
    lap.start();
  

    for (int i=0; i < m_basesPatternLength.size(); i++)
    {
        // fill the workload
        workload[i*WORKLOAD_TASK_SIZE+0] = i;   // pattern
        workload[i*WORKLOAD_TASK_SIZE+1] = i;   // text
        

        // fill the pattern
	int pl = m_basesPattern[i].size();
	int tl = m_basesText[i].size();

	int pBytes = (pl+3)/4;
	int tBytes = (tl+3)/4;

	if (encodingType == 0)
	{
		// Type 0 format
		encodeEntry0(pattern, i*requiredEntryMemory, m_basesPattern[i],  m_basesText[i]);
	}
	else if (encodingType == 1)
	{
		// Type 1 format
		encodeEntry1(pattern, i*requiredEntryMemory, m_basesPattern[i], m_basesText[i]);
	}
	else if (encodingType == 2)
	{
		// Type 2 format
		encodeEntry2(pattern, i*requiredEntryMemory, m_basesPattern[i], m_basesText[i]);
	}
 
    }
    lap.stop();
	printf("Enconding of entries took %f seconds\n", lap.lap());

//    printf("Invoke kernel\n");
	if (m_memBanks == 1)
	    invokeKernelSingleBuffer(pattern, requiredMemory, workload, m_basesPatternLength.size());
	else
	    invokeKernelMultipleBuffers(pattern, requiredMemory, (unsigned char*) workload, m_basesPatternLength.size());
    
	int FP = 0;
	int FN = 0;
	int total = m_basesPatternLength.size();
	int totalCorrect = total;
    
    for (int i=0; i < total; i++)
    {
        bool accepted;
        unsigned int detectedErrors = workload[i*WORKLOAD_TASK_SIZE+2];


	if (m_verbose)
	        printf("[%d] distance=%d max error=%d\n", i, detectedErrors, m_threshold);
//        my_benchmark_check(m_original[i], m_basesPattern[i], m_basesText[i], accepted);


	if ((detectedErrors <= m_threshold) && (realErrors > m_threshold)) 	
	{
		int recheckedErrors = recheckErrors(m_basesPattern[i], m_basesText[i]);

		if (recheckedErrors != realErrors)
		{
			if (m_verbose)
			{
				printf("EDLIB:%d PLANNED:%d\n", recheckedErrors, realErrors);
				printf("PATTERN: %s\n", m_basesPattern[i].c_str());
				printf("TEXT:    %s\n", m_basesText[i].c_str());
			}
			totalCorrect--;
		}
		else
			FP++;
	}
	
	if ((detectedErrors > m_threshold) && (realErrors <= m_threshold))
	{
		int recheckedErrors = recheckErrors(m_basesPattern[i], m_basesText[i]);

		if (recheckedErrors != realErrors)
		{
			if (m_verbose)
			{
				printf("EDLIB:%d PLANNED:%d\n", recheckedErrors, realErrors);
				printf("PATTERN: %s\n", m_basesPattern[i].c_str());
				printf("TEXT:    %s\n", m_basesText[i].c_str());
			}
			totalCorrect--;
		}
		else
			FN++;
	}	
    }

    printf("FP: %d (%0.2f %%)  FN: %d (%0.2f %%) Total: %d\n", FP, (FP*100.0/totalCorrect), FN, (FN*100.0/totalCorrect), totalCorrect);

    // free all
    freeMem(pattern);
    
}

extern int recheckErrors(string& pattern, string& text);

int PrealignmentFilter::recheckErrors(string& pattern, string& text)
{
	return ::recheckErrors(pattern, text);	
}

void PrealignmentFilter::setReportTime(bool reportTime)
{
   m_reportTime = reportTime;
}

/**
 * Invoke a single buffer kernel, all the input data is continously located in memory 
 */
void PrealignmentFilter::invokeKernelSingleBuffer(unsigned char* pattern, unsigned int totalPairsSize, unsigned int* workload, unsigned int tasks)
{
    cl_int ret;
    
    PerformanceLap lap;
    
#ifndef USE_OPENCL_SVM
    m_memPattern = clCreateBuffer(m_context, CL_MEM_READ_WRITE, totalPairsSize, NULL, &ret);
    SAMPLE_CHECK_ERRORS(ret);

    m_memWorkload = clCreateBuffer(m_context, CL_MEM_READ_WRITE, tasks*WORKLOAD_TASK_SIZE*sizeof(unsigned int), NULL, &ret);
    SAMPLE_CHECK_ERRORS(ret);
#endif

    lap.start();

#ifdef USE_OPENCL_SVM
    ret = clSetKernelArgSVMPointerAltera(m_kmerKernel, 0, pattern);
    SAMPLE_CHECK_ERRORS(ret);

    ret = clSetKernelArgSVMPointerAltera(m_kmerKernel, 1, workload);
    SAMPLE_CHECK_ERRORS(ret);

    ret = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, (void*) pattern, totalPairsSize, NULL, NULL );

    ret = clEnqueueSVMMap(m_queue, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, (void*) workload, tasks*WORKLOAD_TASK_SIZE*sizeof(unsigned int), NULL, NULL );

#else
    ret = clEnqueueWriteBuffer(m_queue, m_memPattern, CL_TRUE, 0, totalPairsSize, pattern, 0, NULL, NULL);
    SAMPLE_CHECK_ERRORS(ret);
    
    ret = clEnqueueWriteBuffer(m_queue, m_memWorkload, CL_TRUE, 0, tasks*WORKLOAD_TASK_SIZE*sizeof(unsigned int), workload, 0, NULL, NULL);
    SAMPLE_CHECK_ERRORS(ret);

    ret = clSetKernelArg(m_kmerKernel, 0, sizeof(cl_mem), (void *)&m_memPattern);
    SAMPLE_CHECK_ERRORS(ret);
    
    ret = clSetKernelArg(m_kmerKernel, 1, sizeof(cl_mem), (void *)&m_memWorkload);
    SAMPLE_CHECK_ERRORS(ret);
#endif

    ret = clSetKernelArg(m_kmerKernel, 2, sizeof(cl_int), (void *)&tasks);
    SAMPLE_CHECK_ERRORS(ret);

    lap.stop();
    double transferTime = lap.lap();
    if (m_reportTime)
        printf("Argument Setting= %f seconds\n", transferTime);
    
    lap.start();
    
    // send the events to the FPGA
    size_t wgSize[3] = {1, 1, 1};
    size_t gSize[3] = {1, 1, 1};

    ret = clEnqueueNDRangeKernel(m_queue, m_kmerKernel, 1, NULL, gSize, wgSize, 0, NULL, NULL);
    SAMPLE_CHECK_ERRORS(ret);
    
#ifndef USE_OPENCL_SVM
    ret = clFinish(m_queue);
    SAMPLE_CHECK_ERRORS(ret);
#endif    
    lap.stop();
    
    double kernelTime = lap.lap();
    if (m_reportTime)
        printf("Kernel time= %f seconds\n", kernelTime);
    
    lap.start();
    
#ifdef USE_OPENCL_SVM
    ret = clEnqueueSVMUnmap(m_queue, (void*) pattern, 0, NULL, NULL );
    SAMPLE_CHECK_ERRORS(ret);

    ret = clEnqueueSVMUnmap(m_queue, (void*) workload, 0, NULL, NULL );
    SAMPLE_CHECK_ERRORS(ret);

    lap.stop();
#else
    ret = clEnqueueReadBuffer(m_queue, m_memWorkload, CL_TRUE, 0, tasks*WORKLOAD_TASK_SIZE*sizeof(unsigned int), workload, 0, NULL, NULL);
    SAMPLE_CHECK_ERRORS(ret);
    
    lap.stop();
    
    ret = clReleaseMemObject(m_memPattern);
    SAMPLE_CHECK_ERRORS(ret);
    
    ret = clReleaseMemObject(m_memWorkload);
    SAMPLE_CHECK_ERRORS(ret);
#endif
    
   
    transferTime += lap.lap();
    if (m_reportTime)
        printf("Result fetch= %f seconds\n", lap.lap());

    double Mpairs = tasks / 1E6;
    printf("Throughput. Kernel: %0.2f +Trans: %0.2f\n", (Mpairs/kernelTime), (Mpairs/(kernelTime+transferTime)));
}


/**
 * Invoke a single buffer kernel, all the input data is continously located in memory 
 */
void PrealignmentFilter::invokeKernelMultipleBuffers(unsigned char* pattern, unsigned int totalPairsSize, unsigned char* workload, unsigned int tasks)
{
	printf("Not supported!\n");
	exit(0);
}


size_t PrealignmentFilter::countRequiredMemory(vector<int>& len)
{
    size_t total = 0;
    
    for (int i=0; i  < len.size(); i++)
    {
        int bases = len[i];
        
        total += alignedSequenceSize(bases);
    }
    
    return total;
}


void PrealignmentFilter::destroy()
{
    
}

void PrealignmentFilter::initOpenCL(int platform_id)
{

    cl_uint spi = platform_id;
    cl_uint sdi = 0;

    if (m_verbose)
        cout << "[OCLFPGA] initialization (version compiled "  << __DATE__  << " "  << __TIME__ << ")" << endl;
    
    try
    {
        if(!setCwdToExeDir()) 
        {
            if (m_verbose)
                cout << "[OCLFPGA] ### ERROR ### Failed to change dir" << endl;
            return;
        }

        m_platform = selectPlatform(spi);

        if (m_verbose)
            cout << "[*] Platform Selected [OK] " << endl;
        
        m_deviceId = selectDevice(m_platform, sdi);

        if (m_verbose)
            cout << "[*] Device Selected [OK] " << endl;


        m_context = createContext(m_platform, m_deviceId);

        if (m_verbose)
            cout << "[*] Context Created [OK] " << endl;

        m_queue = createQueue(m_deviceId, m_context, 0);

        if (m_verbose)
            cout << "[*] Queue Created [OK] " << endl;

//        m_queue2 = createQueue(m_deviceId, m_context, 0);
//
//        if (m_verbose)
//            cout << "[*] Queue 2 Created [OK] " << endl;

        m_openCLFilesPath = ".";

        if (m_verbose)
            cout << "Default Path=  " << m_openCLFilesPath << endl;


//        initKernels();

    }
    catch (Error& err)
    {
        printf("%s\n", err.what());
        exit(0);
    }
}

void PrealignmentFilter::finalizeOpenCL()
{
    //finalizeKernels();
    cl_int err;
    
    err = clReleaseContext(m_context);   
    SAMPLE_CHECK_ERRORS(err);
    
    
}

void PrealignmentFilter::initKernels(string board, int memBanks, string openCLKernelType, int threshold, int patternLen, int textLen)
{
    cl_int ret;
    m_patternLen = patternLen;
    m_textLen = textLen;    
    m_board = board;
    m_memBanks = memBanks;

    //m_openCLKernelVersion = version;
    m_threshold = threshold;
    
    if (m_verbose)
    {
        printf("Reading Kernel files\n");
    }

    string platformName = getPlatformName(m_platform);
        
    printf("[OCLFPGA] Platform Name = %s\n", platformName.c_str());

    string bitstream_ext = "aocx";


    if (platformName.compare("Xilinx") == 0)
        bitstream_ext = "xclbin";

    uint64 t0, tf;

    perfCounter(&t0);

    // NVIDIA devices should work with binaries (PTX)

    const unsigned char* pKernels[10]; 
    size_t pKernelSizes[10];
    cl_int pStatus[10];

    memset(pKernels, 0, sizeof(pKernels));
    memset(pKernelSizes, 0, sizeof(pKernelSizes));

    int kernelCount = 0;

    int encoding = determineEncodingType(NULL);

    string sMemBanks = "";

    if (memBanks > 1)
        sMemBanks = format("m%d", memBanks); 

    std::string fullPath  = m_openCLFilesPath.c_str() + format("/fpga/%s/%s_e%d_%s_%d_%d_%d.%s", board.c_str(), openCLKernelType.c_str(), encoding, sMemBanks.c_str(), m_threshold, m_patternLen, m_textLen, bitstream_ext.c_str());

    printf("Opening %s\n", fullPath.c_str());

    setCwd(m_openCLFilesPath.c_str());

    int sysret = system("pwd");

    printf("Create Program From Binary...\n");

    m_program = createProgramFromBinary(m_context, fullPath.c_str(), &m_deviceId, 1); 

    printf("[OK]\n");

    printf("Compiling...\n");
    fflush(stdout);

    //ret = clBuildProgram(program, 1, &device, "-g -s", NULL, NULL);   // in Windows
    ret = clBuildProgram(m_program, 1, &m_deviceId, NULL, NULL, NULL);

    printf("[OK]\n");
    fflush(stdout);

    perfCounter(&tf);

    printf("[INFO] Kernel Compilation Time: %f\n", (float) secondsBetweenLaps(t0, tf));

    //if (ret == CL_BUILD_PROGRAM_FAILURE) 
    {
        // Determine the size of the log
        size_t log_size;
        ret = clGetProgramBuildInfo(m_program, m_deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        ret = clGetProgramBuildInfo(m_program, m_deviceId, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
    }

    SAMPLE_CHECK_ERRORS(ret);

    for (int i=0; i < kernelCount; i++)
    {
        free((void*)pKernels[i]);
    }

    fflush(stderr);
    fflush(stdout);

    //string allCode = m_addSourceCode + "\n" + m_mulSourceCode;

    m_kmerKernel = clCreateKernel(m_program, "kmer", &ret);
    SAMPLE_CHECK_ERRORS(ret);


        
    fflush(stderr);
    fflush(stdout);
}

void PrealignmentFilter::finalizeKernels()
{
    cl_int ret;
    
    
    
    ret = clReleaseKernel(m_kmerKernel);
    SAMPLE_CHECK_ERRORS(ret);
    
    ret = clReleaseProgram(m_program);
    SAMPLE_CHECK_ERRORS(ret);

}
