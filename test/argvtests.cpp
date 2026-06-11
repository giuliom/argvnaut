#include <gtest/gtest.h>

#include <argvnaut.h>

#define SUITE_NAME TestArgvnaut

// --- Positional arguments ---

TEST(SUITE_NAME, testOnePositional) 
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const char* argv[] = {"testprog", "file.txt"};
    const bool result = parser.parse(2, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "file.txt");
}

TEST(SUITE_NAME, testTwoPositionals)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addPositional("output", 2);
    const char* argv[] = {"testprog", "in.txt", "out.txt"};
    const bool result = parser.parse(3, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "in.txt");
    EXPECT_EQ(parser.getString("output").value(), "out.txt");
}

TEST(SUITE_NAME, testMissingRequiredPositional)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const char* argv[] = {"testprog"};
    const bool result = parser.parse(1, argv);
    EXPECT_FALSE(result);
}

// --- Flags ---

TEST(SUITE_NAME, testFlagLong)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const char* argv[] = {"testprog", "--verbose"};
    const bool result = parser.parse(2, argv);
    EXPECT_TRUE(result);
    EXPECT_TRUE(parser.getFlag("verbose"));
}

TEST(SUITE_NAME, testFlagShort)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const char* argv[] = {"testprog", "-v"};
    const bool result = parser.parse(2, argv);
    EXPECT_TRUE(result);
    EXPECT_TRUE(parser.getFlag("verbose"));
}

TEST(SUITE_NAME, testFlagDefault)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const char* argv[] = {"testprog"};
    const bool result = parser.parse(1, argv);
    EXPECT_TRUE(result);
    EXPECT_FALSE(parser.getFlag("verbose"));
}

// --- Options ---

TEST(SUITE_NAME, testOptionLong)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const char* argv[] = {"testprog", "--output", "file.txt"};
    const bool result = parser.parse(3, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionShort)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const char* argv[] = {"testprog", "-o", "file.txt"};
    const bool result = parser.parse(3, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionEqualssyntax)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o");
    const char* argv[] = {"testprog", "--output=file.txt"};
    const bool result = parser.parse(2, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "file.txt");
}

TEST(SUITE_NAME, testOptionDefault)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "default.txt");
    const char* argv[] = {"testprog"};
    const bool result = parser.parse(1, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("output").value(), "default.txt");
}

TEST(SUITE_NAME, testRequiredOptionMissing)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("output", "o", ArgvNaut::OptionType::STRING, "", true);
    const char* argv[] = {"testprog"};
    const bool result = parser.parse(1, argv);
    EXPECT_FALSE(result);
}

// --- Numeric options ---

TEST(SUITE_NAME, testIntOption)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER);
    const char* argv[] = {"testprog", "--count", "42"};
    const bool result = parser.parse(3, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getInt("count").value(), 42);
}

TEST(SUITE_NAME, testFloatOption)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("ratio", "r", ArgvNaut::OptionType::FLOAT);
    const char* argv[] = {"testprog", "--ratio", "3.14"};
    const bool result = parser.parse(3, argv);
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

    const char* argv[] = {"testprog", "-v", "--output", "out.txt", "in.txt", "--verbose"};
    const bool result = parser.parse(6, argv);
    EXPECT_TRUE(result);
    EXPECT_EQ(parser.getString("input").value(), "in.txt");
    EXPECT_EQ(parser.getString("output").value(), "out.txt");
    EXPECT_TRUE(parser.getFlag("verbose"));
}

// --- Error cases ---

TEST(SUITE_NAME, testUnknownFlag)
{
    ArgvNaut::Parser parser("testprog");
    const char* argv[] = {"testprog", "--unknown"};
    const bool result = parser.parse(2, argv);
    EXPECT_FALSE(result);
}

TEST(SUITE_NAME, testExtraPositional)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    const char* argv[] = {"testprog", "a.txt", "b.txt"};
    const bool result = parser.parse(3, argv);
    EXPECT_FALSE(result);
}

// --- has() ---

TEST(SUITE_NAME, testHas)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addOption("output", "o");
    const char* argv[] = {"testprog", "in.txt"};
    parser.parse(2, argv);
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

// --- Type validation at parse time ---

TEST(SUITE_NAME, testInvalidIntValueFailsParse)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER);
    const char* argv[] = {"testprog", "--count", "abc"};
    EXPECT_FALSE(parser.parse(3, argv));
    EXPECT_FALSE(parser.error().empty());
}

TEST(SUITE_NAME, testIntValueWithTrailingGarbageFailsParse)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER);
    const char* argv[] = {"testprog", "--count", "42abc"};
    EXPECT_FALSE(parser.parse(3, argv));
}

TEST(SUITE_NAME, testBooleanOption)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("enabled", "e", ArgvNaut::OptionType::BOOLEAN);
    const char* argv[] = {"testprog", "--enabled", "true"};
    EXPECT_TRUE(parser.parse(3, argv));
    EXPECT_TRUE(parser.getFlag("enabled"));
}

TEST(SUITE_NAME, testInvalidDefaultValueFailsParse)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER, "notanumber");
    const char* argv[] = {"testprog"};
    EXPECT_FALSE(parser.parse(1, argv));
}

// --- Negative numbers and -- separator ---

TEST(SUITE_NAME, testNegativeNumberPositional)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("offset", 1);
    const char* argv[] = {"testprog", "-5"};
    EXPECT_TRUE(parser.parse(2, argv));
    EXPECT_EQ(parser.getInt("offset").value(), -5);
}

TEST(SUITE_NAME, testNegativeOptionValue)
{
    ArgvNaut::Parser parser("testprog");
    parser.addOption("offset", "o", ArgvNaut::OptionType::INTEGER);
    const char* argv[] = {"testprog", "--offset", "-3"};
    EXPECT_TRUE(parser.parse(3, argv));
    EXPECT_EQ(parser.getInt("offset").value(), -3);
}

TEST(SUITE_NAME, testDoubleDashSeparator)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addFlag("verbose", "v");
    const char* argv[] = {"testprog", "--", "--verbose"};
    EXPECT_TRUE(parser.parse(3, argv));
    EXPECT_EQ(parser.getString("input").value(), "--verbose");
    EXPECT_FALSE(parser.getFlag("verbose"));
}

// --- char** overload ---

TEST(SUITE_NAME, testParseNonConstArgv)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    char prog[] = "testprog";
    char flag[] = "--verbose";
    char* argv[] = {prog, flag};
    EXPECT_TRUE(parser.parse(2, argv));
    EXPECT_TRUE(parser.getFlag("verbose"));
}

// --- Optional positionals and position validation ---

TEST(SUITE_NAME, testOptionalPositionalMissing)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addPositional("output", 2, false);
    const char* argv[] = {"testprog", "in.txt"};
    EXPECT_TRUE(parser.parse(2, argv));
    EXPECT_TRUE(parser.has("input"));
    EXPECT_FALSE(parser.has("output"));
}

TEST(SUITE_NAME, testPositionalGapFailsParse)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 2); // gap: no position 1
    const char* argv[] = {"testprog", "in.txt"};
    EXPECT_FALSE(parser.parse(2, argv));
}

TEST(SUITE_NAME, testPositionZeroRejected)
{
    ArgvNaut::Parser parser("testprog");
    EXPECT_FALSE(parser.addPositional("input", 0));
}

// --- Error messages, usage and help ---

TEST(SUITE_NAME, testErrorMessageSetOnFailure)
{
    ArgvNaut::Parser parser("testprog");
    const char* argv[] = {"testprog", "--unknown"};
    EXPECT_FALSE(parser.parse(2, argv));
    EXPECT_NE(parser.error().find("--unknown"), std::string::npos);
}

TEST(SUITE_NAME, testErrorClearedOnSuccess)
{
    ArgvNaut::Parser parser("testprog");
    parser.addFlag("verbose", "v");
    const char* bad[] = {"testprog", "--unknown"};
    EXPECT_FALSE(parser.parse(2, bad));
    const char* good[] = {"testprog", "-v"};
    EXPECT_TRUE(parser.parse(2, good));
    EXPECT_TRUE(parser.error().empty());
}

TEST(SUITE_NAME, testUsage)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1);
    parser.addPositional("output", 2, false);
    parser.addFlag("verbose", "v");
    EXPECT_EQ(parser.usage(), "Usage: testprog [options] <input> [output]");
}

TEST(SUITE_NAME, testHelpContainsArguments)
{
    ArgvNaut::Parser parser("testprog");
    parser.addPositional("input", 1, true, "Input file");
    parser.addFlag("verbose", "v", "Enable verbose output");
    parser.addOption("count", "n", ArgvNaut::OptionType::INTEGER, "1", false, "Number of iterations");
    const std::string help = parser.help();
    EXPECT_NE(help.find("input"), std::string::npos);
    EXPECT_NE(help.find("-v, --verbose"), std::string::npos);
    EXPECT_NE(help.find("Enable verbose output"), std::string::npos);
    EXPECT_NE(help.find("--count <value>"), std::string::npos);
    EXPECT_NE(help.find("(default: 1)"), std::string::npos);
}