#include <gtest/gtest.h>
#include <vector>

#include "dataHandlers/dataHandler.h"
#include "instructions/set.h"
#include "instructions/addPrimitiveType.h"
#include "environment.h"

TEST(Environment, Constructor){
	ASSERT_NO_THROW({
		std::vector<std::reference_wrapper<DataHandlers::DataHandler>> vect;
		Instructions::Set set;
		Environment e(set,vect,8);
	});
}