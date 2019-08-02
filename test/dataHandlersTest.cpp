#include <gtest/gtest.h>

#include "dataHandlers/dataHandler.h"

TEST(DataHandlers, Constructor) {
	DataHandlers::DataHandler* d;
	ASSERT_NO_THROW({
		d = new DataHandlers::DataHandler();
		}) << "Call to DataHandler constructor failed.";
	delete d;
}
