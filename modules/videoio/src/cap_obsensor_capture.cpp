// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

/*
* Copyright(C) 2022 by ORBBEC Technology., Inc.
* Authors:
*   Huang Zhenchang <yufeng@orbbec.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "precomp.hpp"

#include "cap_obsensor_capture.hpp"
#include "cap_obsensor/obsensor_stream_channel_interface.hpp"
#ifdef HAVE_OBSENSOR
namespace cv {
Ptr<IVideoCapture> create_obsensor_capture(int index)
{
    return makePtr<VideoCapture_obsensor>(index);
}

VideoCapture_obsensor::VideoCapture_obsensor(int index) : isOpened_(false)
{
    static const obsensor::StreamProfile colorProfile = { 640, 480, 30, obsensor::FRAME_FORMAT_MJPG };
    static const obsensor::StreamProfile depthProfile = {640, 480, 30, obsensor::FRAME_FORMAT_Y16};

    streamChannelGroup_ = obsensor::getStreamChannelGroup(index);
    if (!streamChannelGroup_.empty())
    {
        for (auto& channel : streamChannelGroup_)
        {
            auto streamType = channel->streamType();
            switch (streamType)
            {
            case obsensor::OBSENSOR_STREAM_COLOR:
                channel->start(colorProfile, [&](obsensor::Frame* frame) {
                    std::unique_lock<std::mutex> lk(frameMutex_);
                    colorFrame_ = Mat(1, frame->dataSize, CV_8UC1, frame->data).clone();
                    });
                break;
            case obsensor::OBSENSOR_STREAM_DEPTH:
                uint8_t data;
                channel->setProperty(obsensor::DEPTH_TO_COLOR_ALIGN, &data, 1);
                channel->start(depthProfile, [&](obsensor::Frame* frame) {
                    std::unique_lock<std::mutex> lk(frameMutex_);
                    depthFrame_ = Mat(frame->height, frame->width, CV_16UC1, frame->data, frame->width * 2).clone();
                    });
                uint32_t len;
                memset(&camParam_, 0, sizeof(camParam_));
                channel->getProperty(obsensor::CAMERA_PARAM, (uint8_t*)&camParam_, &len);
                break;
            default:
                break;
            }
        }
        isOpened_ = true;
    }
}

bool VideoCapture_obsensor::grabFrame()
{
    std::unique_lock<std::mutex> lk(frameMutex_);

    grabbedDepthFrame_ = depthFrame_;
    grabbedColorFrame_ = colorFrame_;

    depthFrame_.release();
    colorFrame_.release();

    return !grabbedDepthFrame_.empty() || !grabbedColorFrame_.empty();
}

bool VideoCapture_obsensor::retrieveFrame(int outputType, OutputArray frame)
{
    std::unique_lock<std::mutex> lk(frameMutex_);
    switch (outputType)
    {
    case CAP_OBSENSOR_DEPTH_MAP:
        if (!grabbedDepthFrame_.empty())
        {
            grabbedDepthFrame_.copyTo(frame);
            grabbedDepthFrame_.release();
            return true;
        }
        break;
    case CAP_OBSENSOR_BGR_IMAGE:
        if (!grabbedColorFrame_.empty())
        {
            auto mat = imdecode(grabbedColorFrame_, IMREAD_COLOR);
            grabbedColorFrame_.release();

            if (!mat.empty())
            {
                mat.copyTo(frame);
                return true;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

double VideoCapture_obsensor::getProperty(int propIdx) const {
    double rst = 0.0;
    propIdx = propIdx & (~CAP_OBSENSOR_GENERATORS_MASK);
    // int gen = propIdx & CAP_OBSENSOR_GENERATORS_MASK;
    switch (propIdx) {
    case CAP_PROP_OBSENSOR_INTRINSIC_FX:
        rst = camParam_.p1[0] / (int)(camParam_.p1[2] * 2 / 640 + 0.5);
        break;
    case CAP_PROP_OBSENSOR_INTRINSIC_FY:
        rst = camParam_.p1[1] / (int)(camParam_.p1[2] * 2 / 640 + 0.5);
        break;
    case CAP_PROP_OBSENSOR_INTRINSIC_CX:
        rst = camParam_.p1[2] / (int)(camParam_.p1[2] * 2 / 640 + 0.5);
        break;
    case CAP_PROP_OBSENSOR_INTRINSIC_CY:
        rst = camParam_.p1[3] / (int)(camParam_.p1[2] * 2 / 640 + 0.5);
        break;
    }
    return rst;
}

bool VideoCapture_obsensor::setProperty(int propIdx, double /*propVal*/)
{
    CV_LOG_WARNING(NULL, "Unsupported or read only property, id=" << propIdx);
    return false;
}

} // namespace cv::
#endif // HAVE_OBSENSOR
