/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <string>

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTestingMacros.h"

#include "itkBackscatterImageFilter.h"

int
itkBackscatterImageFilterTest(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Usage: " << argv[0];
    std::cerr << " spectraImage outputImage <numWorkUnits>";
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  using RealType = float;
  const unsigned int Dimension = 3;

  using SpectraImageType = itk::VectorImage<RealType, Dimension>;
  using OutputImageType = itk::Image<RealType, Dimension>;

  SpectraImageType::Pointer inputImage = itk::ReadImage<SpectraImageType>(std::string(argv[1]));

  const std::string outputImagePath = argv[2];

  unsigned int numWorkUnits = (argc > 3 ? std::stoi(argv[3]) : 1);

  // Initialize the filter
  using BackscatterFilterType = itk::BackscatterImageFilter<SpectraImageType, OutputImageType, MaskImageType>;
  BackscatterFilterType::Pointer backscatterFilter = BackscatterFilterType::New();

  backscatterFilter->SetInput(inputImage);
  ITK_TEST_SET_GET_VALUE(inputImage, backscatterFilter->GetInput());

  // Bad scanline direction produces an error
  backscatterFilter->SetScanDirection(Dimension);
  ITK_TRY_EXPECT_EXCEPTION(backscatterFilter->Update());

  backscatterFilter->SetScanDirection(0);
  ITK_TEST_SET_GET_VALUE(0, backscatterFilter->GetScanDirection());

  // Missing sampling frequency produces an error
  ITK_TRY_EXPECT_EXCEPTION(backscatterFilter->Update());

  backscatterFilter->SetSamplingFrequencyMHz(60);
  ITK_TEST_SET_GET_VALUE(60.0, backscatterFilter->GetSamplingFrequencyMHz());
  ITK_TRY_EXPECT_NO_EXCEPTION(backscatterFilter->UpdateOutputInformation());

  backscatterFilter->SetFrequencyBandStartMHz(5.0);
  ITK_TEST_SET_GET_VALUE(5.0, backscatterFilter->GetFrequencyBandStartMHz());
  backscatterFilter->SetFrequencyBandEndMHz(20.0);
  ITK_TEST_SET_GET_VALUE(20.0, backscatterFilter->GetFrequencyBandEndMHz());

  backscatterFilter->SetNumberOfWorkUnits(numWorkUnits);

  ITK_EXERCISE_BASIC_OBJECT_METHODS(backscatterFilter, BackscatterImageFilter, ImageToImageFilter);

  // Run
  ITK_TRY_EXPECT_NO_EXCEPTION(backscatterFilter->Update());

  // Verify output
  itk::WriteImage(backscatterFilter->GetOutput(), outputImagePath, false);

  // Now boil it down to a single backscatter value
  using HistogramFilterType = itk::Statistics::ImageToHistogramFilter<OutputImageType>;
  auto histogramFilter = HistogramFilterType::New();
  histogramFilter->SetInput(backscatterFilter->GetOutput());
  histogramFilter->SetMarginalScale(10);

  HistogramFilterType::HistogramSizeType histogramSize{ 1 };
  histogramSize[0] = 1e5;
  histogramFilter->SetHistogramSize(histogramSize);
  histogramFilter->Update();
  auto histogram = histogramFilter->GetOutput();

  // We will use median as a robust estimate of the mean
  float median = histogram->Quantile(0, 0.50);
  std::cout << "Median backscatter: " << median << " dB/(MHz*cm)" << std::endl;

  if (!std::isfinite(median))
  {
    std::cerr << "The median backscatter if not a finite number! It is: " << median << std::endl;
    return EXIT_FAILURE;
  }

  if (argc > 9) // Expected value is provided on the command line
  {
    float expectedBackscatter = std::stof(argv[9]);
    if (!itk::Math::FloatAlmostEqual(expectedBackscatter, median, 4, 1e-4))
    {
      std::cerr << "Regression test failure: the expected backscatter is: " << expectedBackscatter << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
