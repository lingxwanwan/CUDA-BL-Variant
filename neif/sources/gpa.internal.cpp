#include "gpa.internal.hpp"

#include <vector>

#include <npp.h>

/// Modified from OpenCV. Used for its accuracy.
///
/// \brief get_gaussian_kernel_bit_exact
/// \param sigma : gaussian kernel standard deviation.
/// \param n : size of the kernel.
/// \param result : separable kernel values, on the host memory.
///
static void get_gaussian_kernel_bit_exact(const Npp64f sigma, const Npp32s& n, std::vector<Npp64f>& result)
{
    constexpr const static double sd_0_15 = 0.15;
    constexpr const static double sd_0_35 = 0.32;
    constexpr const static double sd_minus_0_125 = -0.125;

#ifdef FP_FAST_FMA
    double sigmaX = sigma > 0 ? double(sigma) : std::fma(double(n), sd_0_15, sd_0_35);
#else
    double sigmaX = sigma > 0 ? sigma : (static_cast<double>(n) * sd_0_15 + sd_0_35);
#endif
    double scale2X = sd_minus_0_125/(sigmaX*sigmaX);

    int n2_ = (n - 1) >> 1;

    std::vector<double> values;

    values.reserve(n2_ + 1);
    values.resize(values.capacity());

    double sum = 0.;
    for (int i = 0, x = 1 - n; i < n2_; ++i, x+=2)
    {
        double t = exp(static_cast<double>(x * x) * scale2X);
        values[i] = t;
        sum += t;
    }

#ifdef FP_FAST_FMA
    sum = std::fma(sum, 2., 1.);
#else
    sum = sum * 2. + 1.;
#endif

    if (!(n & 1) )
    {
        sum += 1.;
    }

    // normalize: sum(k[i]) = 1
    double mul1 = 1. / sum;

    result.reserve(n);
    result.resize(result.capacity());

    for (int i = 0, j = n - 1; i < n2_; ++i, --j )
    {
        double t = values[i] * mul1;
        result[i] = result[j] = t;
    }

    result[n2_] = 1. * mul1;

    if (!(n & 1))
        result[n2_ + 1] = result[n2_];

}

///
/// \brief generate_gaussian_kernel
/// \param d_kernel : separable kernel values, on the device memory
/// \param sigma : gaussian kernel standard deviation.
/// \param nFilterTaps : size of the kernel.
/// \return Status of the operations.
///
NppStatus generate_gaussian_kernel(Npp32f*& d_kernel, const Npp32f& sigma, const Npp32s& nFilterTaps)
{
    try
    {
        Npp32f h_kernel[nFilterTaps];

        auto convert_lambda = [](const double& v)->float{ return static_cast<float>(v);};

        std::vector<double> tmp;

        get_gaussian_kernel_bit_exact(sigma, nFilterTaps, tmp);

        std::transform(tmp.begin(), tmp.end(), h_kernel, convert_lambda);


        if(cudaMemcpy(d_kernel, h_kernel, nFilterTaps * sizeof(Npp32f), cudaMemcpyHostToDevice))
            return NPP_MEMCPY_ERROR;
        return NPP_SUCCESS;
    }
    catch(...)
    {
        return NPP_ERROR;
    }
}


NppStreamContext getDefaultStreamContext()
{
    NppStreamContext context;

    nppGetStreamContext(std::addressof(context));

    return context;
}
