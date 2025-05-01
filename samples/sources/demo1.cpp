#include "gpa.hpp"
#include "test.utils.hpp"
#include <regex>
#include <fstream>

using namespace cv;
using namespace test;

int main(int argc, char* argv[])
{

    auto [input_folder, input_filename, output_folder, flag, sigma_range, sigma_spatial_or_box_width, epsilon] = parseInputsDemo1(argc, argv);

    bool display = true;

    if(epsilon <= std::numeric_limits<double>::epsilon())
    {
        std::cout<<getCommandLineKeysDemo1() <<std::endl;
        return EXIT_SUCCESS;
    }

    // Check Inputs.
    if(!cv::utils::fs::exists(input_folder))
        CV_Error(cv::Error::StsError, "The specified input_folder does not exists!");

    if(!cv::utils::fs::exists(input_filename) && cv::utils::fs::exists(cv::utils::fs::join(input_folder, input_filename)))
        input_filename = cv::utils::fs::join(input_folder, input_filename);

    if(!cv::utils::fs::exists(input_filename) )
        CV_Error(cv::Error::StsError, "The specified input_filename does not exists!");

    // Prepare outputs
    if(!output_folder.empty())
    {
        if(!cv::utils::fs::exists(output_folder))
        {
            cv::utils::fs::createDirectories(output_folder);
        }

        display = false;
    }

    cv::Mat img_host = imread(input_filename, IMREAD_UNCHANGED);

    if((img_host.depth() != CV_8U) && (img_host.depth() != CV_16U) && (img_host.depth() != CV_16S) && (img_host.depth() != CV_32F))
    {
        std::cout<<"This example only support 8 bits per pixel, unsigned, 16 bits per pixels signed and unsigned, and single precision floating points, image type. Please retry with an image that respect these constraints"<<std::endl;
        return EXIT_SUCCESS;
    }

    cv::Mat dst;
    cv::UMat usrc;
    cv::cuda::GpuMat img_device;
    cv::cuda::Stream stream;

    if(img_host.empty())
        return EXIT_SUCCESS;

    if(display)
    {
        cv::imshow("Source", img_host);

        // First generate the reference.

        dst = test_fun<true, false, false>(img_host, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        // Prepare Test with OpenCL (TAPI).

        img_host.copyTo(usrc);

        test_fun<true, false, false>(usrc, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        // Prepare Test with CUDA.

        img_device.upload(img_host, stream);

        test_fun<true, false, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        // Prepare Test with NPPI.

        test_fun<true, true, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        // Prepare Test with CAS.

        test_fun<true, false, true>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        cv::waitKey(-1);
    }
    else
    {
        // Prepare a report.
        std::ofstream report(utils::fs::join(output_folder, "report.txt"));

        std::vector<std::tuple<String, Mat> > dsts;

        auto default_stream = std::cout.rdbuf(report.rdbuf());

        dsts.reserve(6);

        dsts.push_back(std::make_tuple("Source", img_host));

        // First generate the reference.

        dst = test_fun<false, false, false>(img_host, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        dsts.push_back(std::make_tuple("OpenCV_CPU", dst));

        // Prepare Test with OpenCL (TAPI).

        img_host.copyTo(usrc);

        auto udst = test_fun<false, false, false>(usrc, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        udst.copyTo(dst);

        dsts.push_back(std::make_tuple("OpenCV_OpenCL", dst));

        // Prepare Test with CUDA.

        img_device.upload(img_host, stream);

        auto gdst = test_fun<false, false, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        gdst.download(dst);

        dsts.push_back(std::make_tuple("OpenCV_CUDA_only", dst));

        // Prepare Test with NPPI.

        gdst = test_fun<false, true, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        gdst.download(dst);

        dsts.push_back(std::make_tuple("NPPI", dst));

        // Prepare Test with CAS.

        gdst = test_fun<false, false, true>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);

        gdst.download(dst);

        dsts.push_back(std::make_tuple("CUDA_custom", dst));

        // Save the report.
        report.close();

        // Reset the stream to its default value.
        std::cout.rdbuf(default_stream);


        auto [extensionless, extention] = get_extensionless_filename_and_extension(input_filename);

        // Save all the images.
        for(auto& [name, obj] : dsts)
            imwrite(utils::fs::join(output_folder, extensionless + "_" + name + "." + extention), obj);

    }




    return EXIT_SUCCESS;
}
