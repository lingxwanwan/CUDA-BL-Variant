#include "gpa.hpp"
#include "test.utils.hpp"
#include <fstream>

using namespace cv;
using namespace cv::cuda;
using namespace test;

int main(int argc, char* argv[])
{

    auto [input_folder, output_folder, N, flag, sigma_range, sigma_spatial_or_box_width, epsilon] = parseInputsDemo2(argc, argv);

    // Check if epsilon is too low.
    if(epsilon <= std::numeric_limits<double>::epsilon())
    {
        std::cout<<getCommandLineKeysDemo2() <<std::endl;
        return EXIT_SUCCESS;
    }

    // Check Inputs.
    if(!cv::utils::fs::exists(input_folder))
        CV_Error(cv::Error::StsError, "The specified input_folder does not exists!");


    bool display = true;

    // Prepare outputs
    if(!output_folder.empty())
    {
        if(!cv::utils::fs::exists(output_folder))
        {
            cv::utils::fs::createDirectories(output_folder);
        }

        display = false;
    }

    std::vector filenames = getNImagesRandomly(input_folder, N);

    int i=-1;
    for(const auto& filename : filenames)
    {
        ++i;

        std::cout<<"Processing image: "<<(i+1)<<"/"<<filenames.size()<<std::endl;

        Mat img_host = imread(filename, IMREAD_UNCHANGED);

        if((img_host.depth() != CV_8U) && (img_host.depth() != CV_16U) && (img_host.depth() != CV_16S) && (img_host.depth() != CV_32F))
        {
            std::cout<<"This example only support 8 bits per pixel, unsigned, 16 bits per pixels signed and unsigned, and single precision floating points, image type. Please retry with an image that respect these constraints"<<std::endl;
            return EXIT_SUCCESS;
        }

        if(img_host.empty())
            continue;

        Mat dst;
        UMat usrc;
        GpuMat img_device;
        Stream stream;

        if(display)
        {
            imshow("Source " + std::to_string(i), img_host);

            // First generate the reference.

            dst = test_fun<true, false, false>(img_host, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon, i);

            // Prepare Test with OpenCL (TAPI).

            img_host.copyTo(usrc);

            test_fun<true, false, false>(usrc, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon, i);

            // Prepare Test with CUDA.

            img_device.upload(img_host, stream);

            test_fun<true, false, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon, i);

            // Prepare Test with NPPI.

            img_device.upload(img_host, stream);

            test_fun<true, true, false>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon, i);

            // Prepare Test with CAS.

            img_device.upload(img_host, stream);

            test_fun<true, false, true>(img_device, dst, sigma_range, sigma_spatial_or_box_width, flag, epsilon);
        }
        else
        {
            auto [extensionless, extention] = get_extensionless_filename_and_extension(filename);

            auto current_output_dir = utils::fs::join(output_folder, extensionless);

            if(!utils::fs::exists(current_output_dir))
                utils::fs::createDirectories(current_output_dir);

            // Prepare a report.
            std::ofstream report(utils::fs::join(current_output_dir, "report.txt"));

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

            // Save all the images.
            for(auto& [name, obj] : dsts)
                imwrite(utils::fs::join(current_output_dir, extensionless + "_" + name + "." + extention), obj);
        }
    }

    if(display)
        waitKey(-1);


    return EXIT_SUCCESS;
}

//#include <cstdlib>
//int main()
//{
//    return EXIT_SUCCESS;
//}
