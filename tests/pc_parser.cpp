#include "cps/pc_compat/pc_loader.hpp"

#include <filesystem>
#include <fstream>
#include <variant>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

namespace cps::utils::test {
    using namespace cps::pc_compat;
    namespace fs = std::filesystem;

    namespace {

        std::ifstream open_pc_test_file(const fs::path & file_name) {
            return std::ifstream{std::string(std::getenv("CPS_TEST_DIR")) + file_name.string()};
        }

        void assert_string_value(const PcPropertyValue & property_value, std::string_view expected) {
            ASSERT_TRUE(std::holds_alternative<std::string>(property_value));
            ASSERT_EQ(std::get<std::string>(property_value), expected);
        }

        void assert_package_requirements(const PcPropertyValue & property_value, const PackageRequirements & expected) {
            ASSERT_TRUE(std::holds_alternative<PackageRequirements>(property_value));
            const auto & package_requirements = std::get<PackageRequirements>(property_value);
            ASSERT_EQ(package_requirements.size(), expected.size());
            for (size_t i = 0; i < package_requirements.size(); ++i) {
                ASSERT_EQ(package_requirements[i].package, expected[i].package);
                ASSERT_EQ(package_requirements[i].operation, expected[i].operation);
            }
        }

        TEST(PcLoader, comments) {
            PcLoader pc_loader;
            fs::path file_path = "/cps-files/lib/pkgconfig/pc-comments.pc";
            std::ifstream input = open_pc_test_file(file_path.string());
            pc_loader.load(input, file_path.parent_path().string());
            assert_string_value(pc_loader.properties["Name"], "libfoo");
            assert_string_value(pc_loader.properties["Description"], "This is an example library");
        }

        TEST(PcLoader, minimal) {
            PcLoader pc_loader;
            fs::path file_path = "/cps-files/lib/pkgconfig/pc-minimal.pc";
            std::ifstream input = open_pc_test_file(file_path.string());
            pc_loader.load(input, file_path.parent_path().string());
            assert_string_value(pc_loader.properties["Name"], "libfoo");
            assert_string_value(pc_loader.properties["Description"], "An example library");
            assert_string_value(pc_loader.properties["Version"], "1.0");
            assert_string_value(pc_loader.properties["URL"], "https://example.com");
        }

        TEST(PcLoader, variables) {
            PcLoader pc_loader;
            fs::path file_path = "/cps-files/lib/pkgconfig/pc-variables.pc";
            std::ifstream input = open_pc_test_file(file_path);
            pc_loader.load(input, file_path.parent_path());
            assert_string_value(pc_loader.properties["Name"], "libfoo");
            assert_string_value(pc_loader.properties["Description"], "an example library called libfoo");
            assert_string_value(pc_loader.properties["Version"], "1.0");
            assert_string_value(pc_loader.properties["URL"], "http://www.pkgconf.org");
            assert_string_value(pc_loader.properties["Libs"], "-L/home/kaniini/pkg/lib -lfoo");
            assert_string_value(pc_loader.properties["Libs.private"], "-lm");
            assert_string_value(pc_loader.properties["Cflags"], "-I/home/kaniini/pkg/include/libfoo");
        }

        TEST(PcLoader, requires) {
            PcLoader pc_loader;
            fs::path file_path = "/cps-files/lib/pkgconfig/pc-full.pc";
            std::ifstream input = open_pc_test_file(file_path);
            pc_loader.load(input, file_path.parent_path());
            assert_string_value(pc_loader.properties["Name"], "libfoo");
            assert_string_value(pc_loader.properties["Description"], "an example library called libfoo");
            assert_string_value(pc_loader.properties["Version"], "1.0");
            assert_string_value(pc_loader.properties["URL"], "http://www.pkgconf.org");
            assert_string_value(pc_loader.properties["Libs"], "-L/home/kaniini/pkg/lib -lfoo");
            assert_string_value(pc_loader.properties["Libs.private"], "-lm");
            assert_string_value(pc_loader.properties["Cflags"], "-I/home/kaniini/pkg/include/libfoo");
            assert_package_requirements(
                pc_loader.properties["Requires"],
                {PackageRequirement{.package = "libbar", .operation = VersionOperation::gt, .version = "2.0.0"}});
        }
    } // namespace
} // namespace cps::utils::test
