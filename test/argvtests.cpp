#include <gtest/gtest.h>

#include <argvnaut.h>

#define SUITE_NAME TestArgvnaut

TEST(SUITE_NAME, testOneArg) 
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const bool result = parser.parse(2, (const char*[]){"testprog", "file.txt"});
    EXPECT_TRUE(result);

}