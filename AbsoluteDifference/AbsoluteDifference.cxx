#include "itkImageFileWriter.h"

// Absolute Difference Include:
#include "ConvertSlicerROIToRegion.h"

#include "itkBSplineInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkPluginUtilities.h"
#include "itkAbsoluteValueDifferenceImageFilter.h"

#include "AbsoluteDifferenceCLP.h"

using namespace std;

// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace {

  template <class T>
  int DoIt( int argc, char * argv[], T ) {
	PARSE_ARGS;

	typedef    T InputPixelType;
	typedef    T OutputPixelType;

	typedef itk::Image<InputPixelType,  3> InputImageType;
	typedef itk::Image<OutputPixelType, 3> OutputImageType;

	typedef itk::ImageFileReader<InputImageType>  ReaderType;
	typedef itk::ImageFileWriter<OutputImageType> WriterType;
	
	typedef itk::RegionOfInterestImageFilter< InputImageType, InputImageType > RegionFilterType;
	typedef itk::BSplineInterpolateImageFunction<InputImageType> Interpolator;
	typedef itk::ResampleImageFilter<InputImageType, OutputImageType> ResampleType;
	typedef itk::AbsoluteValueDifferenceImageFilter<InputImageType, InputImageType, OutputImageType> FilterType;
	  
	typename ReaderType::Pointer reader1 = ReaderType::New();
	itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 1",
										  CLPProcessInformation);

	typename ReaderType::Pointer reader2 = ReaderType::New();
	itk::PluginFilterWatcher watchReader2(reader2,
										  "Read Volume 2",
										  CLPProcessInformation);
	
	reader1->SetFileName( inputVolume1.c_str() );
  //   reader1->ReleaseDataFlagOn();
	reader2->SetFileName( inputVolume2.c_str() );
  //   reader2->ReleaseDataFlagOn();
	
  //   reader1->Update();	// necessary to set resample->SetOutputParametersFromImage
  //   reader2->Update();
	
  //   itk::ImageRegion<3> region1 = reader1->GetOutput()->GetLargestPossibleRegion();
  //   itk::ImageRegion<3> region2 = reader2->GetOutput()->GetLargestPossibleRegion();
  //  
  //   // If we have a bounding box mask (copied from MultiResolutonAffineRegistration)
  //   if( fixedImageROI.size() == 6 )
  //     {
  //     std::vector<double> c(3, 0.0);
  //     std::vector<double> r(3, 0.0);
  // 
  //     // the input is a 6 element vector containing the center in RAS space
  //     // followed by the radius in real world coordinates
  // 
  //     // copy out center values
  //     std::copy(fixedImageROI.begin(), fixedImageROI.begin() + 3,
  //               c.begin() );
  //     // copy out radius values
  //     std::copy(fixedImageROI.begin() + 3, fixedImageROI.end(),
  //               r.begin() );
  // 
  //     // create lower point
  //     itk::Point<double, 3> p1;
  //     p1[0] = -c[0] + r[0];
  //     p1[1] = -c[1] + r[1];
  //     p1[2] = c[2] + r[2];
  // 
  //     // create upper point
  //     itk::Point<double, 3> p2;
  //     p2[0] = -c[0] - r[0];
  //     p2[1] = -c[1] - r[1];
  //     p2[2] = c[2] - r[2];
  // 
  // //     if( DEBUG )
  // //       {
  // //       std::cout << "p1: " << p1 << std::endl;
  // //       std::cout << "p2: " << p2 << std::endl;
  // //       }
  // 
  //     itk::ImageRegion<3> roiRegion1 =
  //       convertPointsToRegion(p1, p2, reader1->GetOutput());
  //       
  //     itk::ImageRegion<3> roiRegion2 =
  //       convertPointsToRegion(p1, p2, reader2->GetOutput());
  // 
  //     region1 = roiRegion1;
  //     region2 = roiRegion2;
  // 
  //     }
  //     
  //   else if( fixedImageROI.size() > 1 &&
  //            fixedImageROI.size() < 6 )
  //     {
  //     std::cerr << "Number of parameters for ROI not as expected" << std::endl;
  //     return EXIT_FAILURE;
  //     }
  //   else if( fixedImageROI.size() > 6 )
  //     {
  //     std::cerr << "Multiple ROIs not supported" << std::endl;
  //     return EXIT_FAILURE;
  //     }
	
  //     typename RegionFilterType::Pointer regionFilter1 = RegionFilterType::New();
  //     regionFilter1->SetInput(reader1->GetOutput() );
  //     regionFilter1->SetRegionOfInterest(region1);
  //     regionFilter1->Update();
  //   
  //     typename RegionFilterType::Pointer regionFilter2 = RegionFilterType::New();
  //     regionFilter2->SetInput(reader2->GetOutput() );
  //     regionFilter2->SetRegionOfInterest(region2);
  //     regionFilter2->Update();
	
	  
  //     // Copied from AddScalarVolumes module - resamples 2nd image to paramaters from 1st
  //     typename Interpolator::Pointer interp = Interpolator::New();
  // //     interp->SetInputImage(regionFilter2->GetOutput() );
  // 	interp->SetInputImage( reader2->GetOutput() );
  //     interp->SetSplineOrder( order );
  // 
  //     typename ResampleType::Pointer resample = ResampleType::New();
  // 	itk::PluginFilterWatcher watchResample(resample, "Resampling",
  // 					    CLPProcessInformation);
  // 	
  // 	//     resample->SetInput(regionFilter2->GetOutput() );
  // 	resample->SetInput( reader2->GetOutput() );
  // //     resample->SetOutputParametersFromImage(regionFilter1->GetOutput() );
  // 	resample->SetOutputParametersFromImage( reader1->GetOutput() );
  //     resample->SetInterpolator( interp );
  //     resample->SetDefaultPixelValue( 0 );
  //     resample->ReleaseDataFlagOn();
		
	  
	  // Calculate Absolute Difference
	  typename FilterType::Pointer filter = FilterType::New();
	  itk::PluginFilterWatcher watchFilter(filter, "Calculating Absolute Difference",
					  CLPProcessInformation);
  //     filter->SetInput1( regionFilter1->GetOutput() );
	  filter->SetInput1( reader1->GetOutput() );
	  filter->SetInput2( reader2->GetOutput() );
  //     filter->Update();

	// Write Output
	typename WriterType::Pointer writer = WriterType::New();
	itk::PluginFilterWatcher watchWriter(writer,
										"Write Volume",
										CLPProcessInformation);
	writer->SetFileName( outputVolume.c_str() );
	writer->SetInput( filter->GetOutput() );
	writer->SetUseCompression(1);
	writer->Update();  
	
	return EXIT_SUCCESS;
  }

} // end of anonymous namespace


int main( int argc, char * argv[] ){
  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  try {
    itk::GetImageType(inputVolume1, pixelType, componentType);

    // This filter handles all types on input, but only produces
    // signed types
	  switch( componentType ){
		case itk::ImageIOBase::UCHAR:
		  return DoIt( argc, argv, static_cast<unsigned char>(0) );
		  break;
		case itk::ImageIOBase::CHAR:
		  return DoIt( argc, argv, static_cast<char>(0) );
		  break;
		case itk::ImageIOBase::USHORT:
		  return DoIt( argc, argv, static_cast<unsigned int>(0) );
		  break;
		case itk::ImageIOBase::SHORT:
		  return DoIt( argc, argv, static_cast<int>(0) );
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

  catch( itk::ExceptionObject & excep ) {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
