#ifndef BaseDeblurFilter_OpenCV_h__
#define BaseDeblurFilter_OpenCV_h__

#include "BaseDeblurFilter.h"

namespace IPL
{
	namespace Filtration
	{

		class BaseDeblurFilter_OpenCV :
			public BaseDeblurFilter
		{
		public:
			BaseDeblurFilter_OpenCV(ProcessingProfile& profile);
			virtual ~BaseDeblurFilter_OpenCV(void);

		protected:
			//! Implementation of deblur filtration procedure.
			virtual void Deblur(const RasterImage& inputImage, RasterImage& outputImage);
		};

	} // namespace Filtration
} // namespace IPL

#endif // BaseDeblurFilter_OpenCV_h__
