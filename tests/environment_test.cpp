#include "helpers/test_env.h"

#include <gtest/gtest.h>

TEST(TestEnvironment, HelpersCompileAndLink) {
	SUCCEED();
}

TEST(TestEnvironment, EphemeralPortAvailable) {
	const auto port = botf::test::ephemeralPort();
	ASSERT_TRUE(port.has_value());
	EXPECT_GT(*port, 0);
}
