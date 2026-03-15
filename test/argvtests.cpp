#include <gtest/gtest.h>

#include <argvnaut.h>

#define SUITE_NAME TestArgvnaut

// --- Positional arguments ---

TEST(SUITE_NAME, testOnePositional) 
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const bool result = parser.parse(2, (const char*[]){"testprog", "file.txt"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "file.txt");
}

TEST(SUITE_NAME, testTwoPositionals)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addPositional("output", 2);
    const bool result = parser.parse(3, (const char*[]){"testprog", "in.txt", "out.txt"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "in.txt");
    EXPECT_EQ(parser.getString("output").value(), "out.txt");
}

TEST(SUITE_NAME, testMissingRequiredPositional)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const bool result = parser.parse(1, (const char*[]){"testprog"});
    EXPECT_FALSE(result);
}

// --- Flags ---

TEST(SUITE_NAME, testFlagLong)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const bool result = parser.parse(2, (const char*[]){"testprog", "--verbose"});
    EXPECT_TRUE(result);
    EXPECT_TRUE(parser.getFlag("verbose"));
}

TEST(SUITE_NAME, testFlagShort)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const bool result = parser.parse(2, (const char*[]){"testprog", "-v"});
    EXPECT_TRUE(result);
    EXPECT_TRUE(parser.getFlag("verbose"));
}

TEST(SUITE_NAME, testFlagDefault)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const bool result = parser.parse(1, (const char*[]){"testprog"});
    EXPECT_TRUE(result);
    EXPECT_FALSE(parser.getFlag("verbose"));
}

// --- Options ---

TEST(SUITE_NAME, testOptionLong)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const bool result = parser.parse(3, (const char*[]){"testprog", "--output", "file.txt"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionShort)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const bool result = parser.parse(3, (const char*[]){"testprog", "-o", "file.txt"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionEqualssyntax)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const bool result = parser.parse(2, (const char*[]){"testprog", "--output=file.txt"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionDefault)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "default.txt");
    const bool result = parser.parse(1, (const char*[]){"testprog"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "default.txt");
}

TEST(SUITE_NAME, testRequiredOptionMissing)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "", true);
    const bool result = parser.parse(1, (const char*[]){"testprog"});
    EXPECT_FALSE(result);
}

// --- Numeric options ---

TEST(SUITE_NAME, testIntOption)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER);
    const bool result = parser.parse(3, (const char*[]){"testprog", "--count", "42"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getInt("count").value(), 42);
}

TEST(SUITE_NAME, testFloatOption)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("ratio", "r", ArgvNaut::OptionType::FLOAT);
    const bool result = parser.parse(3, (const char*[]){"testprog", "--ratio", "3.14"});
    EXPECT_TRUE(result);
    EXPECT_NEAR(parser.getFloat("ratio").value(), 3.14f, 0.001f);
}

// --- Mixed arguments ---

TEST(SUITE_NAME, testMixedArgs)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addFlag("verbose", "v");
    parser.addOption("output", "o");

    const bool result = parser.parse(6, (const char*[]){"testprog", "-v", "--output", "out.txt", "in.txt", "--verbose"});
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "in.txt");
    EXPECT_EQ(parser.getString("output").value(), "out.txt");
    EXPECT_TRUE(parser.getFlag("verbose"));
}

// --- Error cases ---

TEST(SUITE_NAME, testUnknownFlag)
{
    ArgvNaut::Parser parser("testprog");
    const bool result = parser.parse(2, (const char*[]){"testprog", "--unknown"});
    EXPECT_FALSE(result);
}

TEST(SUITE_NAME, testExtraPositional)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const bool result = parser.parse(3, (const char*[]){"testprog", "a.txt", "b.txt"});
    EXPECT_FALSE(result);
}

// --- has() ---

TEST(SUITE_NAME, testHas)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addOption("output", "o");
    parser.parse(2, (const char*[]){"testprog", "in.txt"});
    EXPECT_TRUE(parser.has("input"));
    EXPECT_FALSE(parser.has("output"));
}

// --- Duplicate registration ---

TEST(SUITE_NAME, testDuplicatePositionalName)
{
    ArgvNaut::Parser parser("testprog");
    EXPECT_TRUE(parser.addPositional("input", 1));
    EXPECT_FALSE(parser.addPositional("input", 2));
}

TEST(SUITE_NAME, testDuplicatePositionalPosition)
{
    ArgvNaut::Parser parser("testprog");
    EXPECT_TRUE(parser.addPositional("input", 1));
    EXPECT_FALSE(parser.addPositional("output", 1));
}