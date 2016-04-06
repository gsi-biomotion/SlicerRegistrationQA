
//Inverse Consistency Include:
#include "ConvertSlicerROIToRegion.h"

#include "itkImageFileWriter.h"
#include <itkImageRegionIterator.h>
#include <itkVectorLinearInterpolateImageFunction.h>
#include <itkContinuousIndex.h>
#include "itkRegionOfInterestImageFilter.h"
#include "itkPoint.h"
#include "itkIndex.h"


#include "itkPluginUtilities.h"

#include "InverseConsistencyCLP.h"

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

// 		typedef    T OutputPixelType;
                typedef itk::Vector< double, 3 > InputPixelType;
		//   typedef itk::Vector< float, 3 > OutputPixelType;
		typedef float OutputPixelType;
		typedef itk::Image<InputPixelType,  3> InputImageType;
		typedef itk::Image<OutputPixelType, 3> OutputImageType;

		typedef itk::ImageFileReader<InputImageType>  ReaderType;
		typedef itk::ImageFileWriter<OutputImageType> WriterType;
		typedef itk::VectorLinearInterpolateImageFunction<InputImageType, double> InterpolateType;
		typedef itk::ImageRegionIterator<OutputImageType> IteratorType;
		typedef itk::RegionOfInterestImageFilter< InputImageType, InputImageType > RegionFilterType;

		typename ReaderType::Pointer reader1 = ReaderType::New();
		itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 1", CLPProcessInformation);
		typename ReaderType::Pointer reader2 = ReaderType::New();
		itk::PluginFilterWatcher watchReader2(reader2, "Read Volume 2",CLPProcessInformation);
		reader1->SetFileName( inputVolume1.c_str() );
		reader2->SetFileName( inputVolume2.c_str() );
		reader2->ReleaseDataFlagOn();

		reader1->Update();
		reader2->Update();

		InputImageType::Pointer fixedImage = reader1->GetOutput();
		InputImageType::Pointer movingImage = reader2->GetOutput();

		// TODO: Check if you have everything: movingImage, fixedImage...
		// Add Comments
		
		itk::ImageRegion<3> region = fixedImage->GetLargestPossibleRegion();
		// If we have a bounding box mask (copied from MultiResolutonAffineRegistration)
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
		    convertPointsToRegion(p1, p2, reader1->GetOutput());
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

		typename InterpolateType::Pointer interpolator = InterpolateType::New();
		interpolator->SetInputImage(movingImage);

		typename OutputImageType::Pointer outputImage = OutputImageType::New();
		outputImage->SetRegions(region);
		outputImage->Allocate();

		const InputImageType::SpacingType& sp = fixedImage->GetSpacing();
		outputImage->SetSpacing( sp );

		const InputImageType::PointType origin = fixedImage->GetOrigin();
		outputImage->SetOrigin ( origin );


		itk::ContinuousIndex<double, 3> indexMoving;
		InputImageType::PointType fixedPoint;
		InputImageType::PointType fixedPointWarped;
		InputImageType::PointType movingPointWarped;
		//   OutputImageType::IndexType distance;
		//   InputImageType::PixelType vectorMoving;

		IteratorType imageIterator(outputImage,outputImage->GetRequestedRegion());
		IteratorType::PixelType distance;

		while(!imageIterator.IsAtEnd())
		{
			// Get the value of the current pixel
			//unsigned char val = imageIterator.Get();
			//std::cout << (int)val << std::endl;

			// Set the current pixel to white

			distance=0;
			//fixedImage->TransformIndexToPhysicalPoint(imageIterator.GetIndex(),fixedPoint); // Get phyisical point on fixed image
			outputImage->TransformIndexToPhysicalPoint(imageIterator.GetIndex(),fixedPoint); // Get phyisical point on fixed image

			//     if (isIn1)
			//     {
			itk::Index<3> indexFixed;
			fixedImage->TransformPhysicalPointToIndex(fixedPoint,indexFixed);
// 			fixedPointWarped = fixedPoint + imageIterator.Value(); // Sum point on fixed image with the vector at that point
			fixedPointWarped = fixedPoint + fixedImage->GetPixel(indexFixed);
			// 	bool isIn2 = movingImage->TransformPhysicalPointToContinuousIndex(fixedPointWarped,indexMoving); // Create continuous index on moving image
			bool isIn2 = movingImage->TransformPhysicalPointToContinuousIndex(fixedPointWarped,indexMoving); // Create continuous index on moving image
			if (isIn2)
			{
				// 	vectorMoving = interpolator->EvaluateAtContinuousIndex(indexMoving); // Interpolate to get vector at the continuous index position

				// Interpolate to get vector at the continuous index position and sum with fixedPointWarped.
				// In Ideal Inverse Consitency movingPointWarped=fixedPoint
				movingPointWarped = fixedPointWarped + interpolator->EvaluateAtContinuousIndex(indexMoving); // Calculate inverse consistency point (ideally it would be the same as fixedImage)
				distance = movingPointWarped.EuclideanDistanceTo(fixedPoint);
			}
			//     }
			imageIterator.Set(distance);
			++imageIterator;
		}

		//   std::cout << "Filter:" << std::endl;
		//   typename FilterType::Pointer filter = FilterType::New();
		//   filter->SetInput1( reader1->GetOutput() );
		//   filter->SetInput2( reader2->GetOutput() );
		//   filter->Update();
		//   itk::PluginFilterWatcher watchFilter(filter, "Adding two vector fields.",
		//                                        CLPProcessInformation);
		//   std::cout << "Writer:" << std::endl;
		typename WriterType::Pointer writer = WriterType::New();

		itk::PluginFilterWatcher watchWriter(writer,"Write Volume",CLPProcessInformation);
		writer->SetFileName( outputVolume.c_str() );
		writer->SetInput( outputImage );
		writer->SetUseCompression(1);
		writer->Update();

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
		itk::GetImageType(inputVolume1, pixelType, componentType);

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
