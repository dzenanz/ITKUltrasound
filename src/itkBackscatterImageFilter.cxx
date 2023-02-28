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
#include "itkBackscatterImageFilter.h"

namespace itk
{
/** Define how to print enumerations */
std::ostream &
operator<<(std::ostream & out, const BackscatterImageFilterEnums::BackscatterEstimateType value)
{
  return out << [value] {
    switch (value)
    {
      case BackscatterImageFilterEnums::BackscatterEstimateType::AVERAGE:
        return "BackscatterImageFilterEnums::BackscatterEstimateType::AVERAGE";
      case BackscatterImageFilterEnums::BackscatterEstimateType::SLOPE:
        return "BackscatterImageFilterEnums::BackscatterEstimateType::SLOPE";
      case BackscatterImageFilterEnums::BackscatterEstimateType::INTERCEPT:
        return "BackscatterImageFilterEnums::BackscatterEstimateType::INTERCEPT";
      default:
        return "INVALID VALUE FOR BackscatterImageFilterEnums::BackscatterEstimateType";
    }
  }();
}
} // namespace itk
