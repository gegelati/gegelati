
#include "data/constant.h"

Data::Constant::operator int32_t() const
{
    return this->value;
}

Data::Constant::operator double() const
{
    return (double)this->value;
}

bool Data::Constant::operator==(const Constant& other) const
{
    return this->value == other.value;
}

bool Data::Constant::operator!=(const Constant& other) const
{
    return this->value != other.value;
}
