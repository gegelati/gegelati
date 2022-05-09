
#include <gtest/gtest.h>

#include "data/dataHandler.h"
#include "data/pointerWrapper.h"

TEST(PointerWrapperTest, Constructor)
{
    ASSERT_NO_THROW({
        Data::DataHandler* d = new Data::PointerWrapper<double>();
        delete d;
    }) << "Call to ArrayWrapper constructor failed.";
}