/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkBackscatterImageFilter_hxx
#define itkBackscatterImageFilter_hxx

#include <algorithm>
#include <cmath>

#include "itk_eigen.h"
#include ITK_EIGEN(Dense)
#include "itkMath.h"
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkImageSink.h"
#include "itkImageRegionSplitterDirection.h"

namespace itk
{

template <typename TInputImage, typename TOutputImage>
BackscatterImageFilter<TInputImage, TOutputImage>::BackscatterImageFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->DynamicMultiThreadingOff();
}

template <typename TInputImage, typename TOutputImage>
const ImageRegionSplitterBase *
BackscatterImageFilter<TInputImage, TOutputImage>::GetImageRegionSplitter() const
{
  m_RegionSplitter->SetDirection(m_Direction);
  return m_RegionSplitter.GetPointer();
}

template <typename TInputImage, typename TOutputImage>
void
BackscatterImageFilter<TInputImage, TOutputImage>::VerifyPreconditions() const
{
  Superclass::VerifyPreconditions();

  m_ThreadedInputMaskImage = this->GetInputMaskImage();

  if (this->GetDirection() >= ImageDimension)
  {
    itkExceptionMacro("Scan line direction must be a valid image dimension!");
  }

  if (this->GetSamplingFrequencyMHz() < itk::Math::eps)
  {
    itkExceptionMacro("RF sampling frequency was not set!");
  }
}

template <typename TInputImage, typename TOutputImage>
void
BackscatterImageFilter<TInputImage, TOutputImage>::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  // Initialize metric image
  this->GetOutput()->Allocate();
  this->GetOutput()->FillBuffer(0.0f);

  // Initialize iVars used in ComputeBackscatter()
  float nyquistFrequency = m_SamplingFrequencyMHz / 2;
  float numComponents = input->GetNumberOfComponentsPerPixel();
  m_FrequencyDelta = nyquistFrequency / numComponents;
  m_StartComponent = m_FrequencyBandStartMHz / m_FrequencyDelta;
  m_EndComponent = m_FrequencyBandEndMHz / m_FrequencyDelta;
  if (m_EndComponent == 0) // If m_FrequencyBandEndMHz is not set
  {
    m_EndComponent = numComponents - 1; // Use all components
  }
  m_ConsideredComponents = m_EndComponent - m_StartComponent + 1;
}

template <typename TInputImage, typename TOutputImage>
void
BackscatterImageFilter<TInputImage, TOutputImage>::ThreadedGenerateData(
  const OutputRegionType & regionForThread,
  ThreadIdType)
{
  if (regionForThread.GetNumberOfPixels() == 0)
  {
    return;
  }

  const InputImageType * input = this->GetInput();
  OutputImageType *      output = this->GetOutput();

  ImageLinearConstIteratorWithIndex<TInputImage> it(input, regionForThread);
  it.SetDirection(m_Direction);
  it.GoToBegin();

  // do the work
  while (!it.IsAtEnd())
  {
    while (!it.IsAtEndOfLine())
    {
      // Advance until an inclusion is found
      InputIndexType index = it.GetIndex();

      OutputPixelType estimatedBackscatter = ComputeBackscatter(index);

      // Record this backscatter for both pixels of the pair
      output->SetPixel(start, estimatedBackscatter);
      output->SetPixel(target, estimatedBackscatter);
      ++it;
    }
    it.NextLine();
  }
};

template <typename TInputImage, typename TOutputImage>
typename BackscatterImageFilter<TInputImage, TOutputImage>::OutputPixelType
BackscatterImageFilter<TInputImage, TOutputImage>::ComputeBackscatter(const InputIndexType & index) const
{
  using ScalarType = typename OutputPixelType::ValueType;

  // Get RF spectra frequency bins at start and end pixel positions
  auto           input = this->GetInput();
  InputPixelType sample = input->GetPixel(index);
  ScalarType     sum = 0;

  Eigen::Matrix<float, Eigen::Dynamic, 2> A(m_ConsideredComponents, 2);
  Eigen::Matrix<float, Eigen::Dynamic, 1> b(m_ConsideredComponents);
  for (unsigned i = 0; i < m_ConsideredComponents; i++)
  {
    A(i, 0) = 1;
    A(i, 1) = (1 + i + m_StartComponent) * m_FrequencyDelta; // x_i = frequency
    b(i) = sample[i + m_StartComponent];                     // y_i = intensity
    sum += sample[i + m_StartComponent];
  }

  // from https://eigen.tuxfamily.org/dox/group__LeastSquares.html
  Eigen::Matrix<float, 1, 2> lineFit = A.householderQr().solve(b);
  ScalarType                 frequencySlope = -lineFit(1);
  ScalarType                 frequencyIntercept = lineFit(0);

  OutputPixelType result{ sum / m_ConsideredComponents, frequencySlope, frequencyIntercept };

  return result;
}

template <typename TInputImage, typename TOutputImage>
void
BackscatterImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Image axis representing RF scanline: " << this->GetDirection() << std::endl;
  os << indent << "Sampling frequency (MHz): " << this->GetSamplingFrequencyMHz() << std::endl;
  os << indent << "Frequency band: [" << this->GetFrequencyBandStartMHz() << "," << this->GetFrequencyBandEndMHz()
     << "]" << std::endl;
}
} // end namespace itk
#endif // itkBackscatterImageFilter_hxx
