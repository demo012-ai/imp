// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#ifndef OPENCV_3D_VOLUME_IMPL_HPP
#define OPENCV_3D_VOLUME_IMPL_HPP

#include <iostream>

#include "../precomp.hpp"

namespace cv
{

class Volume::Impl
{
private:

public:
    Impl(VolumeSettings settings);
    virtual ~Impl() {};

    virtual void integrate(OdometryFrame frame, InputArray pose) = 0;
    virtual void raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const = 0;

    virtual void fetchNormals() const = 0;
    virtual void fetchPointsNormals() const = 0;

    virtual void reset() = 0;
    virtual int getVisibleBlocks() const = 0;
    virtual size_t getTotalVolumeUnits() const = 0;

public:
    const float voxelSize;
    const float voxelSizeInv;
    const float raycastStepFactor;
    Affine3f pose;

};


class TsdfVolume : public Volume::Impl
{
public:
    TsdfVolume(VolumeSettings settings);
    ~TsdfVolume();

    virtual void integrate(OdometryFrame frame, InputArray pose) override;
    virtual void raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const override;

    virtual void fetchNormals() const override;
    virtual void fetchPointsNormals() const override;

    virtual void reset() override;
    virtual int getVisibleBlocks() const override;
    virtual size_t getTotalVolumeUnits() const override;


    float interpolateVoxel(const cv::Point3f& p) const;
    Point3f getNormalVoxel(const cv::Point3f& p) const;


public:
    Point3i volResolution;

    Point3f volSize;
    float truncDist;
    Vec4i volDims;
    Vec8i neighbourCoords;

    VolumeSettings settings;
    Vec4i volStrides;
    Vec6f frameParams;
    Mat pixNorms;
    // See zFirstMemOrder arg of parent class constructor
    // for the array layout info
    // Consist of Voxel elements
    Mat volume;
};


class HashTsdfVolume : public Volume::Impl
{
public:
    HashTsdfVolume(VolumeSettings settings);
    ~HashTsdfVolume();

    virtual void integrate(OdometryFrame frame, InputArray pose) override;
    virtual void raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const override;

    virtual void fetchNormals() const override;
    virtual void fetchPointsNormals() const override;

    virtual void reset() override;
    virtual int getVisibleBlocks() const override;
    virtual size_t getTotalVolumeUnits() const override;
private:
    VolumeSettings settings;
};


class ColorTsdfVolume : public Volume::Impl
{
public:
    ColorTsdfVolume(VolumeSettings settings);
    ~ColorTsdfVolume();

    virtual void integrate(OdometryFrame frame, InputArray pose) override;
    virtual void raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const override;

    virtual void fetchNormals() const override;
    virtual void fetchPointsNormals() const override;

    virtual void reset() override;
    virtual int getVisibleBlocks() const override;
    virtual size_t getTotalVolumeUnits() const override;
private:
    VolumeSettings settings;
};


Volume::Volume()
{
    VolumeSettings settings;
    this->impl = makePtr<TsdfVolume>(settings);
}
Volume::Volume(VolumeType vtype, VolumeSettings settings)
{
    std::cout << "Volume::Volume()" << std::endl;

    switch (vtype)
    {
    case VolumeType::TSDF:
        std::cout << "case VolumeType::TSDF" << std::endl;
        this->impl = makePtr<TsdfVolume>(settings);
        break;
    case VolumeType::HashTSDF:
        this->impl = makePtr<HashTsdfVolume>(settings);
        break;
    case VolumeType::ColorTSDF:
        this->impl = makePtr<ColorTsdfVolume>(settings);
        break;
    default:
        //CV_Error(Error::StsInternal,
        //	"Incorrect OdometryType, you are able to use only { ICP, RGB, RGBD }");
        std::cout << "Incorrect OdometryType, you are able to use only { ICP, RGB, RGBD }" << std::endl;
        break;
    }
}
Volume::~Volume() {}

void Volume::integrate(OdometryFrame frame, InputArray pose) { this->impl->integrate(frame, pose); }
void Volume::raycast(const Matx44f& cameraPose, int height, int width, OutputArray _points, OutputArray _normals) const { this->impl->raycast(cameraPose, height, width, _points, _normals); }
void Volume::fetchNormals() const { this->impl->fetchNormals(); }
void Volume::fetchPointsNormals() const { this->impl->fetchPointsNormals(); }
void Volume::reset() { this->impl->reset(); }
int Volume::getVisibleBlocks() const { return this->impl->getVisibleBlocks(); }
size_t Volume::getTotalVolumeUnits() const { return this->impl->getTotalVolumeUnits(); }


}

#endif // !OPENCV_3D_VOLUME_IMPL_HPP
