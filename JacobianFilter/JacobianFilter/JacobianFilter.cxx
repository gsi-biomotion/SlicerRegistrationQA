#include "itkImageFileWriter.h"
#include <itkVector.h>
#include <itkTransform.h>

#include <itkDisplacementFieldJacobianDeterminantFilter.h>
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

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput( reader->GetOutput() );
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
