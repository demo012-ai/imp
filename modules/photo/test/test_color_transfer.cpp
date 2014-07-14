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
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
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
// This software is provided by the copyright holders and contributors "as is" and
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


#include "test_precomp.hpp"
#include "opencv2/photo.hpp"
#include <string>

using namespace cv;
using namespace std;


TEST(Photo_colorTransfer, regression)
{
        string folder = string(cvtest::TS::ptr()->get_data_path()) + "colorTransfer/";
        string original_path_src = folder + "color_image_src_1.png";
        string original_path_dst = folder + "color_image_dst_1.png";

        Mat source = imread(original_path_src, IMREAD_COLOR);
        Mat target = imread(original_path_dst, IMREAD_COLOR);

        ASSERT_FALSE(source.empty()) << "Could not load input source image " << original_path_src;
        ASSERT_FALSE(target.empty()) << "Could not load input target image " << original_path_dst;
        ASSERT_FALSE(source.channels()!=3) << "Load color input source image " << original_path_src;
        ASSERT_FALSE(target.channels()!=3) << "Load color input target image " << original_path_dst;

        Mat color_transfer;
        colorTransfer(source, target, color_transfer);

        imwrite(folder + "color_transfer.png",color_transfer);

}
