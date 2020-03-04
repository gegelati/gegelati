#include <gtest/gtest.h>

#include "data/hash.h"

TEST(DataHashTest, HashInt) {

	ASSERT_EQ(Data::Hash<int>()(1337), 10617510125527791323);
	ASSERT_EQ(Data::Hash<unsigned int>()(666), 11188042650382073989);
	ASSERT_EQ(Data::Hash<uint64_t>()(UINT64_MAX), 10157053723145373757);
}


TEST(DataHashTest, HashDouble) {
	ASSERT_EQ(Data::Hash<double>()(12.00), 12123769577132206109);
	ASSERT_EQ(Data::Hash<float>()((float)3.14), 2246715419936742613);
}

TEST(DataHashTest, HashNullptr) {
	nullptr_t t = NULL;
	ASSERT_EQ(Data::Hash<nullptr_t>()(t), 12161962213042174405);
}