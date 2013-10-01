#ifndef IBUGUSLIDER_H
#define IBUGUSLIDER_H

#include "RangedSlider.h"

class IbuGuSlider : public RangedSlider
{
   Q_OBJECT
public:
   IbuGuSlider(QWidget* parent = 0);
   
   void setValue(double value);
};

#endif /*IBUGUSLIDER_H*/
