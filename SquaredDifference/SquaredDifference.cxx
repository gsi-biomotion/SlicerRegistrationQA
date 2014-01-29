#include "itkImageFileWriter.h"


#include <itkSquaredDifferenceImageFilter.h>
#include <itkSqrtImageFilter.h>
#include <itkStatisticsImageFilter.h>

#include "itkPluginUtilities.h"

#include "SquaredDifferenceCLP.h"

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

  typedef    T InputPixelType;
  typedef    T OutputPixelType;

  typedef itk::Image<InputPixelType,  3> InputImageType;
  typedef itk::Image<OutputPixelType, 3> OutputImageType;
//   typedef itk::Image<double, 3> OutputImageTypeTmp;

  typedef itk::ImageFileReader<InputImageType>  ReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

    
  typedef itk::SquaredDifferenceImageFilter<InputImageType, InputImageType, OutputImageType> FilterType;
  typedef itk::SqrtImageFilter< OutputImageType, OutputImageType > SqrtFilterType;
  typedef itk::StatisticsImageFilter<OutputImageType> StatisticType;
    
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
 
  //Images need the same dimensions! - copy from substract values?
  
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1( reader1->GetOutput() );
  filter->SetInput2( reader2->GetOutput() );
  filter->Update();
  itk::PluginFilterWatcher watchFilter(filter, "Calculating Squared Difference",
                                       CLPProcessInformation);
  
  typename SqrtFilterType::Pointer sqrtfilter = SqrtFilterType::New();
  sqrtfilter->SetInput( filter->GetOutput() );
  sqrtfilter->Update();
  itk::PluginFilterWatcher watchSqrtFilter(sqrtfilter, "Calculating Square Root of Image Difference",
                                       CLPProcessInformation);

  typename WriterType::Pointer writer = WriterType::New();
  itk::PluginFilterWatcher watchWriter(writer,
                                       "Write Volume",
                                       CLPProcessInformation);
  writer->SetFileName( outputVolume.c_str() );
  writer->SetInput( sqrtfilter->GetOutput() );
  writer->SetUseCompression(1);
  writer->Update();
  
  typename StatisticType::Pointer statistic = StatisticType::New();
  itk::PluginFilterWatcher watchStatistic(statistic,
                                       "Calculating Statistigs",
                                       CLPProcessInformation);
  statistic->SetInput ( sqrtfilter->GetOutput() );
  statistic->Update();
  
  //TODO: How to include Mean into output?
//   sigma=statistic->GetMean();
  std::cout << "Mean: " << statistic->GetMean() << std::endl;
  std::cout << "Std.: " << statistic->GetSigma() << std::endl;
//   std::cout << "Min: " << statistic->GetMinimum() << std::endl;
//   std::cout << "Max: " << statistic->GetMaximum() << std::endl;
  
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

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}
