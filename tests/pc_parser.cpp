#include "cps/pc_compat/pc_loader.hpp"
#include <fstream>
#include <gtest/gtest.h>

namespace cps::utils::test {
    namespace {

        std::ifstream open_pc_test_file(const std::string & file_name) {
            return std::ifstream{std::string(std::getenv("CPS_TEST_DIR")) + file_name};
        }

        TEST(PcLoader, comments) {
            PcLoader pc_loader;
            std::ifstream input = open_pc_test_file("/cps-files/lib/pkgconfig/comments.pc");
            pc_loader.load(input);
            ASSERT_EQ(pc_loader.properties["Name"], "libfoo");
            ASSERT_EQ(pc_loader.properties["Description"], "This is an example library");
        }

        TEST(PcLoader, minimal) {
            PcLoader pc_loader;
            std::ifstream input = open_pc_test_file("/cps-files/lib/pkgconfig/minimal.pc");
            pc_loader.load(input);
            ASSERT_EQ(pc_loader.properties["Name"], "libfoo");
            ASSERT_EQ(pc_loader.properties["Description"], "An example library");
            ASSERT_EQ(pc_loader.properties["Version"], "1.0");
            ASSERT_EQ(pc_loader.properties["URL"], "https://example.com");
        }

        TEST(PcLoader, variables) {
            PcLoader pc_loader;
            std::ifstream input = open_pc_test_file("/cps-files/lib/pkgconfig/variables.pc");
            pc_loader.load(input);
            ASSERT_EQ(pc_loader.properties["Name"], "libfoo");
            ASSERT_EQ(pc_loader.properties["Description"], "an example library called libfoo");
            ASSERT_EQ(pc_loader.properties["Version"], "1.0");
            ASSERT_EQ(pc_loader.properties["URL"], "http://www.pkgconf.org");
            ASSERT_EQ(pc_loader.properties["Libs"], "-L/home/kaniini/pkg/lib -lfoo");
            ASSERT_EQ(pc_loader.properties["Libs.private"], "-lm");
            ASSERT_EQ(pc_loader.properties["Cflags"], "-I/home/kaniini/pkg/include/libfoo");
        }

    } // namespace
} // namespace cps::utils::test
