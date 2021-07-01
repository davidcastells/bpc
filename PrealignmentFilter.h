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
 * File:   FPGAKmerFilter.h
 * Author: dcr
 *
 * Created on February 20, 2020, 6:04 PM
 */

#ifndef PREALIGNMENTFILTER_H
#define PREALIGNMENTFILTER_H

#include "OpenCLUtils.h"
//#include "../benchmark/benchmark_utils.h"

#include <string>
#include <vector>
#include "edlib.h"

using namespace std;

class PrealignmentFilter 
{
public:
    PrealignmentFilter();
    virtual ~PrealignmentFilter();
    
public:
    void initOpenCL(int platform_id);
    void initKernels(string board, int memBanks, string openCLKernelType, int threshold, int patternLen, int textLen);
    void finalizeOpenCL();
    void finalizeKernels();
    void addInput(string pattern, string text);
    void computeAll(int realErrors);
 
   void destroy();
    void setVerbose(bool verbose);
    void setReportTime(bool reportTime);
   
    void invokeKernelSingleBuffer(unsigned char* pattern, unsigned int patternSize, unsigned int* workload, unsigned int tasks);
    void invokeKernelMultipleBuffers(unsigned char* pattern, unsigned int patternSize, unsigned char* workload, unsigned int tasks);
    
private:
    size_t countRequiredMemory(vector<int>& len);
 	   
    int determineEncodingType(size_t* requiredMemory);
    string decodeSequence(unsigned char* pattern, unsigned int offset, unsigned int basesLength);
    void encodeEntry0( unsigned char* pPattern, unsigned int offset, string pattern, string text );
    void encodeEntry1( unsigned char* pPattern, unsigned int offset, string pattern, string text );
    void encodeEntry2( unsigned char* pPattern, unsigned int offset, string pattern, string text );
    void encodeEntry3( unsigned char* pPattern, unsigned int offset, string pattern, string text );
 
    int recheckErrors(string& pattern,string& text);
    void*  allocMem(size_t size);
    void freeMem(void* p);

public:
    bool m_verbose;
    bool m_reportTime;

private:
    vector<string> m_basesPattern;
    vector<string> m_basesText;
    vector<int> m_basesPatternLength;
    vector<int> m_basesTextLength;

    int m_patternLen;
    int m_textLen;
    int m_memBanks;

    cl_platform_id m_platform;
    cl_context m_context;
    cl_device_id m_deviceId;
    cl_program m_program;
    cl_command_queue m_queue;
    cl_kernel m_kmerKernel;
    string m_openCLFilesPath;
    string m_board;
    
    cl_mem m_memPattern;
    cl_mem m_memPatternIdx;
    cl_mem m_memText;
    cl_mem m_memTextIdx;
    cl_mem m_memWorkload;

    int m_threshold;
    
    //int m_openCLKernelVersion;


};

#endif /* PREALIGNMENTFILTER_H */

