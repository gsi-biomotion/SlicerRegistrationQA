// Jacobian Filter Include:
#include "ConvertSlicerROIToRegion.h"

#include "itkImageFileWriter.h"
#include <itkVector.h>
#include <itkTransform.h>
#include <itkDisplacementFieldJacobianDeterminantFilter.h>
#include "itkRegionOfInterestImageFilter.h"
// #include <itkStatisticsImageFilter.h>
#include <itkLogImageFilter.h>
// #include "itkTransformFileReader.h"
// #include "itkTransformFileWriter.h"
// #include "itkDeformationFieldJacobianDeterminantFilter.h"

#include "itkPluginUtilities.h"

#include "JacobianFilterCLP.h"


#define DIMENSION 3
// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

template <class T>
int DoIt( int argc, char * argv[], T )
{
  PARSE_ARGS;

//   typedef    T OutputPixelType;
  
  typedef itk::Vector< float, 3 > InputPixelType;
  typedef float OutputPixelType;
  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;

  
  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

    
  typedef itk::RegionOfInterestImageFilter< InputImageType, InputImageType > RegionFilterType;
  typedef itk::DisplacementFieldJacobianDeterminantFilter< InputImageType > FilterType;
  typedef itk::LogImageFilter< OutputImageType, OutputImageType > LogType;
//   typedef itk::StatisticsImageFilter<OutputImageType> StatisticType;
  
//   typedef itk::TransformFileReader TransformReaderType;
//   
//   typedef itk::Transform< float > TransformType;
  
  
//   typedef itk::TransformFileReader TransformReaderType;
//   TransformReaderType::Pointer initialTransform;
//   initialTransform = TransformReaderType::New();
//   initialTransform->SetFileName( InputTransform.c_str() );
//   initialTransform->Update();
//   
//   TransformReaderType::TransformType::Pointer initial
//   = *(initialTransform->GetTransformList()->begin() );
//   
//   TransformType::OutputVectorType vector;
// //   vector=initial->TransformVector( InputImageType );
//   std::cout << initial->GetParameters() << std::endl;
  
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputVolume.c_str() );
  reader->Update();
  
  itk::ImageRegion<3> region = reader->GetOutput()->GetLargestPossibleRegion();
  
  if( fixedImageROI.size() == 6 )
		  {
		  std::vector<double> c(3, 0.0);
		  std::vector<double> r(3, 0.0);

		  // the input is a 6 element vector containing the center in RAS space
		  // followed by the radius in real world coordinates

		  // copy out center values
		  std::copy(fixedImageROI.begin(), fixedImageROI.begin() + 3,
			    c.begin() );
		  // copy out radius values
		  std::copy(fixedImageROI.begin() + 3, fixedImageROI.end(),
			    r.begin() );

		  // create lower point
		  itk::Point<double, 3> p1;
		  p1[0] = -c[0] + r[0];
		  p1[1] = -c[1] + r[1];
		  p1[2] = c[2] + r[2];

		  // create upper point
		  itk::Point<double, 3> p2;
		  p2[0] = -c[0] - r[0];
		  p2[1] = -c[1] - r[1];
		  p2[2] = c[2] - r[2];

	      //     if( DEBUG )
	      //       {
	      //       std::cout << "p1: " << p1 << std::endl;
	      //       std::cout << "p2: " << p2 << std::endl;
	      //       }

		  itk::ImageRegion<3> roiRegion =
		    convertPointsToRegion(p1, p2, reader->GetOutput());
		  region = roiRegion;
		  }
		  
		else if( fixedImageROI.size() > 1 &&
			fixedImageROI.size() < 6 )
		  {
		  std::cerr << "Number of parameters for ROI not as expected" << std::endl;
		  return EXIT_FAILURE;
		  }
		else if( fixedImageROI.size() > 6 )
		  {
		  std::cerr << "Multiple ROIs not supported" << std::endl;
		  return EXIT_FAILURE;
		  }
 
  typename RegionFilterType::Pointer regionFilter = RegionFilterType::New();
    regionFilter->SetInput(reader->GetOutput() );
    regionFilter->SetRegionOfInterest(region);
    regionFilter->Update();
    
    
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput( regionFilter->GetOutput() );
  filter->Update();
  itk::PluginFilterWatcher watchFilter(filter, "Calculating Jacobian Displacment Field.",
                                       CLPProcessInformation);
  
  if (enable_log)
  {
    typename LogType::Pointer logFilter = LogType::New();
    logFilter->SetInput( filter->GetOutput() );
    logFilter->Update();
    itk::PluginFilterWatcher watchLogFilter(logFilter, "Calculating log of Jacobian Displacment Field.",
					CLPProcessInformation);
    typename WriterType::Pointer writer = WriterType::New();  
    itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
    writer->SetFileName( outputVolume.c_str() );
    writer->SetInput( logFilter->GetOutput() );
    writer->SetUseCompression(1);
    writer->Update();
  }
  else {
    typename WriterType::Pointer writer = WriterType::New();
    itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
    writer->SetFileName( outputVolume.c_str() );
    writer->SetInput( filter->GetOutput() );
    writer->SetUseCompression(1);
    writer->Update();
  }


  
//   typename StatisticType::Pointer statistic = StatisticType::New();
//   itk::PluginFilterWatcher watchStatistic(statistic,
//                                        "Calculating Statistics",
//                                        CLPProcessInformation);
//   statistic->SetInput ( filter->GetOutput() );
//   statistic->Update();
//   mean=statistic->GetMean();
//   std::cout << "Mean: " << mean << std::endl;
//   std::cout << "Std.: " << statistic->GetSigma() << std::endl;
//   std::cout << "Max: " << statistic->GetMaximum() << std::endl;
//   std::cout << "Min.: " << statistic->GetMinimum() << std::endl;
  
  //TODO: Add statistics (min, max) and add logic in Widget
  std::cout << "Finished" << std::endl;
  return EXIT_SUCCESS;
}

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try
    {
    itk::GetImageType(inputVolume, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
    switch( componentType )
      {
      case itk::ImageIOBase::UCHAR:
        return DoIt( argc, argv, static_cast<unsigned char>(0) );
        break;
      case itk::ImageIOBase::CHAR:
        return DoIt( argc, argv, static_cast<char>(0) );
        break;
      case itk::ImageIOBase::USHORT:
        return DoIt( argc, argv, static_cast<unsigned short>(0) );
        break;
      case itk::ImageIOBase::SHORT:
        return DoIt( argc, argv, static_cast<short>(0) );
        break;
      case itk::ImageIOBase::UINT:
        return DoIt( argc, argv, static_cast<unsigned int>(0) );
        break;
      case itk::ImageIOBase::INT:
        return DoIt( argc, argv, static_cast<int>(0) );
        break;
      case itk::ImageIOBase::ULONG:
        return DoIt( argc, argv, static_cast<unsigned long>(0) );
        break;
      case itk::ImageIOBase::LONG:
        return DoIt( argc, argv, static_cast<long>(0) );
        break;
      case itk::ImageIOBase::FLOAT:
        return DoIt( argc, argv, static_cast<float>(0) );
        break;
      case itk::ImageIOBase::DOUBLE:
        return DoIt( argc, argv, static_cast<double>(0) );
        break;
      case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
      default:
        std::cout << "unknown component type" << std::endl;
        break;
      }
    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
