#include "gpa.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cvconfig.h>

namespace cv
{

namespace
{

template<class Type>
constexpr float getT()
{
    return std::is_signed<Type>() ? -static_cast<float>(std::numeric_limits<Type>::min()) : std::fmaf(static_cast<float>(std::numeric_limits<Type>::max()), 0.5f, 0.5f);
}


template<class Type>
int getN(float sigma_range, float epsilon)
{
    float lam = getT<Type>() / sigma_range;

    lam *= lam;

    float p = std::log(std::exp(1) * lam);
    float q = -lam - std::log(epsilon);
    float t = q * std::exp(-1) / lam;

    float t_sq = t*t;

    // t + 1.5 * t^3 - (8/3) t^4 - t^2
    float W = t + std::fma(t_sq, 1.5f * t, std::fma((-8.f/3.f) * t_sq, t_sq, -t_sq));

    float N = std::min(std::max(q / W, 10.f), 300.f);

    if(sigma_range < 30.f)
    {
        for(int i=0; i<5; i++)
        {
            float lN = std::log(N);

            N -= std::fma(N, lN, - std::fma(p, N, -q)) / (lN + 1.f - p);
        }
    }

    if(std::isnan(N) || std::isinf(N))
        return -1;
    return std::ceil(N);
}

template<class Type>
inline int nppiEstN(Npp32f sigma_range, Npp32f epsilon)
{
    return sigma_range < 5.f ? 800 : sigma_range > 70 ? 10 : getN<Type>(sigma_range, epsilon);
}

inline int getKernelSize(const double& sigma)
{
    return cvCeil(4.f * sigma + 0.5f);
}

template<class Type>
void BilateralFilterGPAWorker_(Type& src, const int& sdepth, const float& midRange, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, Type& g, const double& eps)
{

    int N;

    Type H, F, G, P, Q, Fbar, tmp;

    Mat Hs;

    double inv_sqrt_n, sqrt_n;

    src.convertTo(src, CV_32F);

    if(flag == GPA_FLAG::GAUSSIAN)
    {
        int kernel_size = getKernelSize( sigma_spatial_or_box_width );

        Hs = getGaussianKernel(kernel_size, sigma_spatial_or_box_width);
    }
    else
    {
        int kernel_size =  2 * sigma_spatial_or_box_width + 1;

        Hs = Mat1f::ones(kernel_size, 1) / 3.f;
    }

    N = sdepth == CV_16S ? getN<short>(sigma_range, eps) : sdepth == CV_16U ? getN<ushort>(sigma_range, eps) : sdepth == CV_32F ? getN<float>(sigma_range, eps) : getN<uchar>(sigma_range, eps);


    H.create(src.size(), src.type());
    F.create(src.size(), src.type());

    // Compute H and F

    //H =  (src - 128.) x (1. / sigma_range)
    subtract(src, Scalar::all(128.), H);

    // F = exp(-0.5 x (H / sigma_range)^2)
    multiply(H, Scalar::all(1./sigma_range), H);
    multiply(H, H, F, -0.5);
    exp(F,F);


    G = Type::ones(F.size(), F.type());
    P = Type::zeros(G.size(), G.type());
    Q = P.clone();


    sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);

    for(int i=0; i<N;i++)
    {
        sqrt_n = std::sqrt(static_cast<double>(i) + 1.);
        inv_sqrt_n = 1. / sqrt_n;

        // In the following lines the multiplications are elementwise.

//      Q += G * Fbar
        multiply(G, Fbar, tmp);
        add(Q, tmp, Q);

//      F = H * F * inv_sqrt_n
        multiply(H, F, F, inv_sqrt_n);


        sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);

//      P += G * Fbar * sqrt_n
        multiply(G, Fbar, tmp, sqrt_n);
        add(P, tmp, P);

//      G += H * G * inv_sqrt_n
        multiply(H, G, tmp, inv_sqrt_n);
        add(G, tmp, G);
    }


    //Type g = 128. + sigma_range * (P / Q);

    divide(P, Q, g);
    multiply(g, Scalar::all(sigma_range), g);
    add(Scalar::all(midRange), g, g);

    g.convertTo(g, sdepth);
}

template<class Type>
void BilateralFilterGPA_(InputArray& _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, OutputArray _dst, const double& eps)
{
    int sdepth = _src.depth();
    float midRange = sdepth == CV_8U ? 128.f : sdepth == CV_16S ?  0.f :  sdepth == CV_16U ? 32768.f : 0.5f;
    int N;

    Type src, g;


    if constexpr (std::is_same<Type, Mat>())
    {
        src = _src.getMat();
    }
    else
    {
        src = _src.getUMat();
    }

    if(src.channels() > 1)
    {
        // Step 1) Split the interleave image into a set of planar channels.
        std::vector<Type> channels;
        Type alpha;
        bool is_alpha = src.channels() == 4;

        split(src, channels);

        // Step 2) Isolate the alpha channel.
        if(is_alpha)
        {
            alpha = channels.back();

            // Step 3) Remove the alpha channel.
            channels.pop_back();
        }

        // Step 4) Process the chromatic channels.
        for(Type& channel : channels)        
            BilateralFilterGPAWorker_(channel, sdepth, midRange, sigma_range, sigma_spatial_or_box_width, flag, channel, eps);


        // Step 5) reconstruct the image.
        if(is_alpha)
        {
            channels.emplace_back(alpha);
        }

        merge(channels, _dst);
    }
    else
    {
        BilateralFilterGPAWorker_(src, sdepth, midRange, sigma_range, sigma_spatial_or_box_width, flag, g, eps);

        _dst.assign(g);
    }
}


} // anonymous


void BilateralFilterGPA(InputArray _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, OutputArray _dst, const double& eps)
{
    CV_Assert_3(_src.isMat() || _src.isUMat(),
               (_src.channels() <= 4) && (_src.channels() != 2),
               ((_src.depth() == CV_8U) || (_src.depth() == CV_16U) || (_src.depth() == CV_16S) || (_src.depth() == CV_32F) )
               );


    if(_src.isMat())
    {
        BilateralFilterGPA_<Mat>(_src, sigma_range, sigma_spatial_or_box_width, flag, _dst, eps);
    }
    else
    {
        BilateralFilterGPA_<UMat>(_src, sigma_range, sigma_spatial_or_box_width, flag, _dst, eps);
    }
}



namespace cuda
{

#ifdef HAVE_CUDA
struct has_cuda : std::true_type {};
#else
struct has_cuda : std::false_type {};
#endif

namespace
{

    void cudaBilateralFilterGPA_(GpuMat& src, const int& sdepth, const float& midRange, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, GpuMat& dst, const double& eps, Stream& _stream)
    {
        GpuMat H, F, G, P, Q, Fbar, tmp, g;
        int N;
        double inv_sqrt_n, sqrt_n;

        src.convertTo(src, CV_32F);

        Mat Hs;

        if(flag == GPA_FLAG::GAUSSIAN)
        {
            int kernel_size = getKernelSize( sigma_spatial_or_box_width );

            Hs = getGaussianKernel(kernel_size, sigma_spatial_or_box_width);
        }
        else
        {
            int kernel_size =  2 * sigma_spatial_or_box_width + 1;

            Hs = Mat1f::ones(kernel_size, 1) / 3.f;
        }


        N = sdepth == CV_16S ? getN<short>(sigma_range, eps) : sdepth == CV_16U ? getN<ushort>(sigma_range, eps) : sdepth == CV_32F ? getN<float>(sigma_range, eps) : getN<uchar>(sigma_range, eps);

        H.create(src.size(), src.type());
        F.create(src.size(), src.type());

        // Compute H and F

        //H =  (src - midRange) x (1. / sigma_range)
        subtract(src, Scalar::all(128.), H, noArray(), -1, _stream);

        // F = exp(-0.5 x (H / sigma_range)^2)
        multiply(H, Scalar::all(1./sigma_range), H, 1., -1, _stream);
        multiply(H, H, F, -0.5, -1, _stream);
        exp(F,F, _stream);


        G = GpuMat(F.size(), F.type(), Scalar::all(1.));
        P = GpuMat(G.size(), G.type(), Scalar::all(0.));
        Q = P.clone();

        auto filter = createSeparableLinearFilter(src.type(), src.type(), Hs.t(), Hs);

        filter->apply(F, Fbar, _stream);

//        sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);

        for(int i=0; i<N;i++)
        {
            sqrt_n = std::sqrt(static_cast<double>(i) + 1.);
            inv_sqrt_n = 1. / sqrt_n;

            // In the following lines the multiplications are elementwise.

            // Q += G x Fbar
            multiply(G, Fbar, tmp, 1., -1, _stream);
            add(Q, tmp, Q, noArray(), -1, _stream);

            // F = H x F x inv_sqrt_n
            multiply(H, F, F, inv_sqrt_n, -1, _stream);

            // sepFilter2D(F, Fbar, CV_32F, Hs.t(), Hs);
            filter->apply(F, Fbar, _stream);

            // P += G x Fbar x sqrt_n
            multiply(G, Fbar, tmp, sqrt_n, -1, _stream);
            add(P, tmp, P, noArray(), -1, _stream);

            // G += H x G x inv_sqrt_n
            multiply(H, G, tmp, inv_sqrt_n, -1, _stream);
            add(G, tmp, G, noArray(), -1, _stream);
        }

        //Type g = midRange + sigma_range x (P / Q);
        divide(P, Q, g, 1., -1, _stream);
        multiply(g, Scalar::all(sigma_range), g, 1., -1, _stream);
        add(Scalar::all(midRange), g, g, noArray(), -1, _stream);

        g.convertTo(dst, sdepth);
    }

} // anonymous


    void BilateralFilterGPA(InputArray _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, OutputArray _dst, const double& eps, Stream& _stream)
    {

        if constexpr(has_cuda())
        {
            int sdepth = _src.depth();
            float midRange = sdepth == CV_8U ? 128.f : sdepth == CV_16S ?  0.f :  sdepth == CV_16U ? 32768.f : 0.5f;
            int N;

            GpuMat src, alpha, H, F, G, P, Q, Fbar, tmp, g;

            double inv_sqrt_n, sqrt_n;


            src = _src.getGpuMat();

            if(src.channels() == 4)
            {
                // Step 1) Split the interleave image into a set of planar channels.
                std::vector<GpuMat> channels;

                split(src, channels, _stream);

                // Step 2) Isolate the alpha channel.
                alpha = channels.back();

                // Step 3) Reconstruct the interleaved image without the alpha channel.
                channels.pop_back();

                merge(channels, src);

                // Step 4) Process the chromatic channels.

                GpuMat dst;

                cudaBilateralFilterGPA_(src, sdepth, midRange, sigma_range, sigma_spatial_or_box_width, flag, dst, eps, _stream);

                // Step 5) reconstruct the image.

                split(dst, channels);

                channels.emplace_back(alpha);

                merge(channels, _dst, _stream);
            }
            else
            {
                _dst.create(_src.size(), _src.type());

                GpuMat& dst = _dst.getGpuMatRef();

                cudaBilateralFilterGPA_(src, sdepth, midRange, sigma_range, sigma_spatial_or_box_width, flag, dst, eps, _stream);
            }
        }
        else
        {
            CV_Error(cv::Error::GpuNotSupported, "The library is compiled without CUDA support");
        }
    }

} // cuda

} // cv


namespace nppi
{

using namespace cv;
using namespace cv::cuda;

void BilateralFilterGPA(const GpuMat& _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, GpuMat& _dst, const double& eps)
{
    CV_Assert_2(
               (_src.channels() <= 4) && (_src.channels() != 2),
               ((_src.depth() == CV_8U) || (_src.depth() == CV_16U) || (_src.depth() == CV_16S) || (_src.depth() == CV_32F) )
               );

    if(_dst.empty())
        _dst.create(_src.size(), _src.type());

    int sdepth = _src.depth();
    int cn = _src.channels();

    if(sdepth == CV_8U)
    {
        if(cn == 1)
            nppiBilateralFilterByGaussianPolynomialApproximation_8u_C1R(_src.ptr(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr(), _dst.step, {_dst.cols, _dst.rows} );
        else if(cn == 3)
            nppiBilateralFilterByGaussianPolynomialApproximation_8u_C3R(_src.ptr(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr(), _dst.step, {_dst.cols, _dst.rows} );
        else
            nppiBilateralFilterByGaussianPolynomialApproximation_8u_AC4R(_src.ptr(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr(), _dst.step, {_dst.cols, _dst.rows} );
    }
    else if(sdepth == CV_16U)
    {
        if(cn == 1)
            nppiBilateralFilterByGaussianPolynomialApproximation_16u_C1R(_src.ptr<ushort>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<ushort>(), _dst.step, {_dst.cols, _dst.rows} );
        else if(cn == 3)
            nppiBilateralFilterByGaussianPolynomialApproximation_16u_C3R(_src.ptr<ushort>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<ushort>(), _dst.step, {_dst.cols, _dst.rows} );
        else
            nppiBilateralFilterByGaussianPolynomialApproximation_16u_AC4R(_src.ptr<ushort>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<ushort>(), _dst.step, {_dst.cols, _dst.rows} );
    }
    else if(sdepth == CV_16S)
    {
        if(cn == 1)
            nppiBilateralFilterByGaussianPolynomialApproximation_16s_C1R(_src.ptr<short>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<short>(), _dst.step, {_dst.cols, _dst.rows} );
        else if(cn == 3)
            nppiBilateralFilterByGaussianPolynomialApproximation_16s_C3R(_src.ptr<short>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<short>(), _dst.step, {_dst.cols, _dst.rows} );
        else
            nppiBilateralFilterByGaussianPolynomialApproximation_16s_AC4R(_src.ptr<short>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<short>(), _dst.step, {_dst.cols, _dst.rows} );
    }
    else
    {
        if(cn == 1)
            nppiBilateralFilterByGaussianPolynomialApproximation_32f_C1R(_src.ptr<float>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<float>(), _dst.step, {_dst.cols, _dst.rows} );
        else if(cn == 3)
            nppiBilateralFilterByGaussianPolynomialApproximation_32f_C3R(_src.ptr<float>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<float>(), _dst.step, {_dst.cols, _dst.rows} );
        else
            nppiBilateralFilterByGaussianPolynomialApproximation_32f_AC4R(_src.ptr<float>(), _src.step, sigma_range, sigma_spatial_or_box_width, flag, eps, _dst.ptr<float>(), _dst.step, {_dst.cols, _dst.rows} );
    }
}

} // nppi

namespace cas
{

using namespace cv;
using namespace cv::cuda;


void BilateralFilterGPA(const GpuMat& _src, const double& sigma_range, const double& sigma_spatial_or_box_width, const GPA_FLAG& flag, GpuMat& _dst, const double& eps)
{
    CV_Assert_2(
               (_src.channels() <= 4) && (_src.channels() != 2),
               ((_src.depth() == CV_8U) || (_src.depth() == CV_16U) || (_src.depth() == CV_16S) || (_src.depth() == CV_32F) )
               );

    if(_dst.empty() || _dst.size() != _src.size() || _dst.type() != _src.type())
    {
        _dst.create(_src.size(), _src.type());
    }

    switch (_src.type())
    {
    case CV_8UC1:
        gaussianApproximationBilateralFilter_8u_C1_Ctx(_src.ptr(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr(), _dst.step);
        break;

    case CV_8UC2:
        gaussianApproximationBilateralFilter_8u_C2_Ctx(_src.ptr(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr(), _dst.step);
        break;

    case CV_8UC3:
        gaussianApproximationBilateralFilter_8u_C3_Ctx(_src.ptr(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr(), _dst.step);
        break;

    case CV_8UC4:
        gaussianApproximationBilateralFilter_8u_C4_Ctx(_src.ptr(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr(), _dst.step);
        break;

    case CV_16UC1:
        gaussianApproximationBilateralFilter_16u_C1_Ctx(_src.ptr<ushort>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<ushort>(), _dst.step);
        break;

    case CV_16UC2:
        gaussianApproximationBilateralFilter_16u_C2_Ctx(_src.ptr<ushort>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<ushort>(), _dst.step);
        break;

    case CV_16UC3:
        gaussianApproximationBilateralFilter_16u_C3_Ctx(_src.ptr<ushort>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<ushort>(), _dst.step);
        break;

    case CV_16UC4:
        gaussianApproximationBilateralFilter_16u_C4_Ctx(_src.ptr<ushort>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<ushort>(), _dst.step);
        break;


    case CV_16SC1:
        gaussianApproximationBilateralFilter_16s_C1_Ctx(_src.ptr<short>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<short>(), _dst.step);
        break;

    case CV_16SC2:
        gaussianApproximationBilateralFilter_16s_C2_Ctx(_src.ptr<short>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<short>(), _dst.step);
        break;

    case CV_16SC3:
        gaussianApproximationBilateralFilter_16s_C3_Ctx(_src.ptr<short>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<short>(), _dst.step);
        break;

    case CV_16SC4:
        gaussianApproximationBilateralFilter_16s_C4_Ctx(_src.ptr<short>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<short>(), _dst.step);
        break;


    case CV_32FC1:
        gaussianApproximationBilateralFilter_32f_C1_Ctx(_src.ptr<float>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<float>(), _dst.step);
        break;

    case CV_32FC2:
        gaussianApproximationBilateralFilter_32f_C2_Ctx(_src.ptr<float>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<float>(), _dst.step);
        break;

    case CV_32FC3:
        gaussianApproximationBilateralFilter_32f_C3_Ctx(_src.ptr<float>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<float>(), _dst.step);
        break;

    case CV_32FC4:
        gaussianApproximationBilateralFilter_32f_C4_Ctx(_src.ptr<float>(), _src.step, _src.rows, _src.cols, sigma_range, sigma_spatial_or_box_width, flag, _dst.ptr<float>(), _dst.step);
        break;

    default:
        break;
    }
}

} // cas
