#ifndef GPA_INTERNAL_HPP
#define GPA_INTERNAL_HPP

#include "utils.hpp"

#include <cmath>

#define CHECK_NPPI_RETURN(fun)\
    if (auto error = fun)\
        return error;

///
/// \brief getT
/// \return the minimum number that can be represented for the given type.
///
template<class Type>
constexpr float getT()
{
    if constexpr (std::is_same<Type, Npp8u>())
    {
        return 128.f;
    }
    else if constexpr(std::is_same<Type, Npp16u>())
    {
        return 32768.f;
    }
    else if constexpr(std::is_same<Type, Npp16s>())
    {
        return 0.f;
    }
    else
    {
        return 0.5f;
    }
}

///
/// \brief getN : return the number of iteration to do
///  based on the precision set.
/// \param sigma_range : standard deviation on the range
/// \param epsilon : accuracy to reach
/// \return Number of iteration to do in order to reach the accuracy.
///
template<class Type>
Npp32s getN(const Npp32f& sigma_range, const Npp32f& epsilon)
{
    Npp32f lam, q, W, N;

    if constexpr(std::is_integral<Type>() && std::is_signed<Type>())
    {
        lam = 0.f;
        q = -std::log(epsilon);
        N = 10.f;
    }
    else
    {
        Npp32f T = getT<Type>();

        lam = std::pow(T / sigma_range, 2.f);

        q = -lam - std::log(epsilon);
        Npp32f t = q * std::exp(-1) / lam;

        Npp32f t_sq = t*t;

        // t + 1.5 * t^3 - (8/3) t^4 - t^2
#ifdef FP_FAST_FMA
        W = t + std::fma(1.5f, t_sq * t, std::fma(t_sq * t_sq, (-8.f/3.f), -t_sq));
#else
        W = 1.5 * t_sq * t - (8.f/3.f) * t_sq * t_sq - t_sq;
#endif

        N = std::min(std::max(q / W, 10.f), 300.f);

    }

    if(sigma_range < 30.f)
    {
        Npp32f p;

        if constexpr(std::is_integral<Type>() && std::is_signed<Type>())
        {
            // Because lam is 0 in this case.
            p = 0.f;
        }
        else
        {
            p = std::log(std::exp(1) * lam);
        }

        // can happen if T is 0. That said the two if constexpr above should have made that case rare.
        if(std::isinf(p))
            p = 0.f;


        for(Npp32s i=0; i<5; i++)
        {
            Npp32f lN = std::log(N);

#ifdef FP_FAST_FMA
            N -= std::fma(N, lN, - std::fma(p, N, -q)) / (lN + 1.f - p);
#else
            N -= (N * lN - (p * N - q)) / (lN + 1.f - p);
#endif
        }
    }

    return static_cast<int>(std::ceil(N));
}


///
/// \brief generate_gaussian_kernel
/// \param d_kernel : separable kernel values, on the device memory
/// \param sigma : gaussian kernel standard deviation.
/// \param nFilterTaps : size of the kernel.
/// \return Status of the operations.
///
NppStatus generate_gaussian_kernel(Npp32f*& d_kernel, const Npp32f& sigma, const Npp32s& nFilterTaps);

///
/// \brief getDefaultStreamContext
/// \return the default stream context.
///
NppStreamContext getDefaultStreamContext();



#endif // GPA_INTERNAL_HPP
