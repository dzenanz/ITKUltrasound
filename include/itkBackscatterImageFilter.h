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
#ifndef itkBackscatterImageFilter_h
#define itkBackscatterImageFilter_h

#include <vector>

#include "itkImage.h"
#include "itkImageToImageFilter.h"
#include "itkMacro.h"
#include "itkNumericTraits.h"

namespace itk
{
/** \class BackscatterImageFilter
 * \brief Computes the estimated backscatter coefficient
 *
 * BackscatterImageFilter receives an input vector image representing
 * RF spectra. One image direction represents the direction of an
 * RF waveform emitted from an ultrasound probe. Remaining image
 * directions may represent directions in physical space, such as
 * where the probe contains multiple elements, or directions in
 * time as multiple samples are captured in a sweep with the probe.
 * Each pixel in the input image is a vector representing frequency
 * components at bins based on the sampling frequency.
 *
 * BackscatterImageFilter generates a scalar output image with
 * intensities representing backscatter estimates.
 *
 * There are three types of estimates available, see documentation for
 * EstimateType for detailed description.
 *
 * \sa MaskedImageToHistogramFilter
 * \sa Spectra1DImageFilter
 * \sa VariableLengthVector
 * \sa VectorImage
 *
 * \ingroup ITKImageStatistics
 * \ingroup Ultrasound
 */
template <typename TInputImage,
          typename TOutputImage = Image<float, TInputImage::ImageDimension>>
class ITK_TEMPLATE_EXPORT BackscatterImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BackscatterImageFilter);

  /** Standard class type aliases. */
  using Self = BackscatterImageFilter;
  using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(BackscatterImageFilter, ImageToImageFilter);

  /** Image type alias support */
  static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;

  using InputImageType = TInputImage;
  using InputImagePointer = typename TInputImage::Pointer;

  using OutputImageType = TOutputImage;
  using OutputImagePointer = typename TOutputImage::Pointer;

  using InputRegionType = typename TInputImage::RegionType;
  using InputSizeType = typename TInputImage::SizeType;
  using InputIndexType = typename TInputImage::IndexType;
  using InputPixelType = typename TInputImage::PixelType;

  using OutputRegionType = typename TOutputImage::RegionType;
  using OutputPixelType = typename TOutputImage::PixelType;

  /** RF sampling frequency */
  itkSetMacro(SamplingFrequencyMHz, float);
  itkGetConstMacro(SamplingFrequencyMHz, float);

  /** Low end of RF frequency band to use in backscatter analysis.
   * Must be a positive value. */
  itkSetMacro(FrequencyBandStartMHz, float);
  itkGetConstMacro(FrequencyBandStartMHz, float);

  /* High end of RF frequency band to use in backscatter analysis.
   * Must be a positive value.*/
  itkSetMacro(FrequencyBandEndMHz, float);
  itkGetConstMacro(FrequencyBandEndMHz, float);

  /** Estimate Type (0=average, 1=slope, 2=intercept).
   *
   * 0. (Default) Average of intensities of selected frequency components.
   * 1. Slope of a line fit to the selected frequency components.
   * 2. Negative intercept of a line fit to the selected frequency components.
   *    The intercept is negated because it is expected to be mostly negative.
   */
  itkSetMacro(EstimateType, unsigned int);
  itkGetConstMacro(EstimateType, unsigned int);

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

protected:
  BackscatterImageFilter();
  ~BackscatterImageFilter() override = default;

  void
  VerifyPreconditions() const override;

  /** Allocate buffers and other initializations before threaded execution. */
  void
  BeforeThreadedGenerateData() override;

  void
  DynamicThreadedGenerateData(const OutputRegionType & regionForThread) override;

  /** Compute backscatter for a pixel in the RF spectra vector image. */
  OutputPixelType
  ComputeBackscatter(const InputIndexType & index) const;

private:
  unsigned int m_EstimateType = 0;

  float m_SamplingFrequencyMHz = 0.0f;
  float m_FrequencyBandStartMHz = 0.0f;
  float m_FrequencyBandEndMHz = 0.0f;
  float m_FrequencyDelta = 0.0f;

  // Frequency band to consider for backscatter
  unsigned int m_StartComponent = 0;
  unsigned int m_EndComponent = 0;
  unsigned int m_ConsideredComponents = 1;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBackscatterImageFilter.hxx"
#endif

#endif
