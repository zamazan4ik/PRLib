#ifndef BackgroundNormalization_Lepton_h__
#define BackgroundNormalization_Lepton_h__

#include "BackgroundNormalizationFilter.h"

namespace IPL
{
	namespace Filtration
	{

		class BackgroundNormalizationFilter_Lepton : public BackgroundNormalizationFilter {
		public:
			BackgroundNormalizationFilter_Lepton(ProcessingProfile& profile);
			virtual ~BackgroundNormalizationFilter_Lepton(void);

			/*!
			* \brief Concrete modification procedure.
			* \param[in] inputImage Image for modification.
			* \param[out] outputImage Modified image.
			*/
			virtual void Modify(const RasterImage& inputImage, RasterImage& outputImage) final;
		};

	} // namespace Filtration
} // namespace IPL


#endif // BackgroundNormalization_Lepton_h__
