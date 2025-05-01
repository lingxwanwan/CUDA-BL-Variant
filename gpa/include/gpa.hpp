#ifndef GPA_HPP
#define GPA_HPP 1

#pragma once

#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <nppi_extra_image_filtering.hpp>
#include <cuda_gpabf.hpp>

namespace cv
{

///
/// \brief BilateralFilterGPA : implementation of the bilateral filter using Gaussian Polynomial Approximation (CPU and TAPI)
/// \param _src : source image (a cv::Mat or a cv::UMat). The input must have 1,3, or 4 channels and must of type CV_8U, CV_16U, CV_16s or CV_32F
/// \param sigma_range : variance of the range kernel.
/// \param sigma_spatial_or_box_width : sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param _dst : destination image (a cv::Mat or a cv::UMat)
/// \param eps
///
void BilateralFilterGPA(InputArray _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, OutputArray _dst, const double& eps = 1e-3);

namespace cuda
{

///
/// \brief ocvGPA : implementation of the bilateral filter using Gaussian Polynomial Approximation (CUDA)
/// \param _src : source image (a cv::cuda::GpuMat). The input must have 1,3, or 4 channels and must of type CV_8U, CV_16U, CV_16s or CV_32F
/// \param sigma_range : variance of the range kernel.
/// \param sigma_spatial_or_box_width : sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param _dst : destination image (a cv::cuda::GpuMat)
/// \param eps
///
void BilateralFilterGPA(InputArray _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, OutputArray _dst, const double& eps = 1e-3, Stream& _stream = Stream::Null());

} // cuda

} // cv

namespace nppi
{

///
/// \brief nppiGPA : implementation of the bilateral filter using Gaussian Polynomial Approximation (using nppi, use the libnppi_extra_image_filtering)
/// \param _src : source image. The input must have 1,3, or 4 channels and must of type CV_8U, CV_16U, CV_16s or CV_32F
/// \param sigma_range : variance of the range kernel.
/// \param sigma_spatial_or_box_width : sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param _dst : destination image. 
/// \param eps
///
void BilateralFilterGPA(const cv::cuda::GpuMat& _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, cv::cuda::GpuMat &_dst, const double& eps = 1e-3);

} // nppi

namespace cas
{

///
/// \brief nppiGPA : implementation of the bilateral filter using Gaussian Polynomial Approximation (using nppi, use the libnppi_extra_image_filtering)
/// \param _src : source image. The input must have 1,3, or 4 channels and must of type CV_8U, CV_16U, CV_16s or CV_32F
/// \param sigma_range : variance of the range kernel.
/// \param sigma_spatial_or_box_width : sigma_spatial_or_box_width: variance of the spatial kernel or width of the box kernel depending on the flag.
/// \param flag : Gaussian or Box filter to use in order to blur the data.
/// \param _dst : destination image.
/// \param eps
///
void BilateralFilterGPA(const cv::cuda::GpuMat& _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, cv::cuda::GpuMat &_dst, const double& eps = 1e-3);

} // cas

#endif
