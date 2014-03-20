#include "itkImageFileWriter.h"
#include <itkImageRegionIterator.h>
#include <itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.h>
#include <itkContinuousIndex.h>
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

  typedef itk::Vector< double, 3 > InputPixelType;
//   typedef itk::Vector< float, 3 > OutputPixelType;
  typedef float OutputPixelType;
  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;
    
  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  typedef itk::VectorLinearInterpolateNearestNeighborExtrapolateImageFunction<InputImageType, double> InterpolateType;
  typedef itk::ImageRegionIterator<OutputImageType> IteratorType;

    
  typename ReaderType::Pointer reader1 = ReaderType::New();
  itk::PluginFilterWatcher watchReader1(reader1, "Read Volume 1",
                                        CLPProcessInformation);
  typename ReaderType::Pointer reader2 = ReaderType::New();
  itk::PluginFilterWatcher watchReader2(reader2,
                                        "Read Volume 2",
                                        CLPProcessInformation);
  reader1->SetFileName( inputVolume1.c_str() );
  reader2->SetFileName( inputVolume2.c_str() );
  reader2->ReleaseDataFlagOn();

  reader1->Update();
  reader2->Update();
  
  InputImageType::Pointer fixedImage = reader1->GetOutput();
  InputImageType::Pointer movingImage = reader2->GetOutput();
  
  // TODO: Check if you have everything: movingImage, fixedImage...
  // Add Comments
  
  typename InterpolateType::Pointer interpolator = InterpolateType::New();
  interpolator->SetInputImage(movingImage);
  
  typename OutputImageType::Pointer outputImage = OutputImageType::New();
  outputImage->SetRegions(fixedImage->GetLargestPossibleRegion());
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
    fixedImage->TransformIndexToPhysicalPoint(imageIterator.GetIndex(),fixedPoint); // Get phyisical point on fixed image
    
//     if (isIn1)
//     {
      fixedPointWarped = fixedPoint + imageIterator.Value(); // Sum point on fixed image with the vector at that point
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
  
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
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
