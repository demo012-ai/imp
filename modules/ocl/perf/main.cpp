/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Multicoreware, Inc., all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors as is and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "perf_precomp.hpp"

#define DUMP_INFO_STDOUT(propertyDisplayName, propertyValue) \
    do { \
        std::cout << (propertyDisplayName) << ": " << (propertyValue) << std::endl; \
    } while (false)\

#define DUMP_INFO_XML(propertyXMLName, propertyValue) \
    do { \
        std::stringstream ss; ss << propertyValue; \
        ::testing::Test::RecordProperty((propertyXMLName), ss.str()); \
    } while (false)\

#define DUMP_DEVICES_INFO_STDOUT(deviceType, deviceIndex, deviceName, deviceVersion) \
    do { \
        std::cout << "        " << (deviceType) << " " << (deviceIndex) << " : " << (deviceName) << " : " << deviceVersion << std::endl; \
    } while (false)\

#define DUMP_DEVICES_INFO_XML(deviceType, deviceIndex, deviceName, deviceVersion) \
    do { \
        std::stringstream ss; \
        ss << " " << deviceIndex << " : " << deviceName << " : " << deviceVersion; \
        ::testing::Test::RecordProperty((deviceType), ss.str()); \
    } while (false)\

#include "opencv2/ocl/private/opencl_dumpinfo.hpp"

static const char * impls[] =
{
    IMPL_OCL,
    IMPL_PLAIN,
#ifdef HAVE_OPENCV_GPU
    IMPL_GPU
#endif
};


int main(int argc, char ** argv)
{
    ::perf::TestBase::setPerformanceStrategy(::perf::PERF_STRATEGY_SIMPLE);

    CV_PERF_TEST_MAIN_INTERNALS(ocl, impls, dumpOpenCLDevice())
}
