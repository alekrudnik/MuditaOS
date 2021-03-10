// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "personalization_data_integration/PersonalizationFileValidation.hpp"
#include <filesystem>

namespace phone_personalization
{

    class PersonalizationFileParserTestWrapper : public PersonalizationFileParser
    {
      public:
        explicit PersonalizationFileParserTestWrapper(const std::filesystem::path &filePath)
            : PersonalizationFileParser(filePath){};

        std::string test_loadFileContent()
        {
            return loadFileContent();
        }
    };

    class PersonalizationFileValidatorWrapper : public PersonalizationFileValidator
    {
      public:
        explicit PersonalizationFileValidatorWrapper(const json11::Json &jsonObj, const std::filesystem::path &filePath)
            : PersonalizationFileValidator(jsonObj, filePath){};

        bool test_validateFileAndCRC()
        {
            return validateFileCRC();
        }

        bool test_validateJsonObject()
        {
            return validateJsonObject();
        }

        ParamModel test_validateByFormat(const std::string &key)
        {
            validateByFormat(key);
            auto param = getParamsModel()[key];
            return param;
        }

        ParamModel test_validateByValues(const std::string &key)
        {
            validateByValues(key);
            auto param = getParamsModel()[key];
            return param;
        }
    };

    TEST_CASE("Personalization File Validation")
    {

        SECTION("Check PersonalizationFileParser class")
        {

            SECTION("Valid case")
            {
                auto path = std::filesystem::current_path();
                path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                auto fileParserTestWrapper = PersonalizationFileParserTestWrapper(path);

                auto expectedSerialNumber = "SN123456789098";
                auto expectedCaseColour   = "black";

                auto content = fileParserTestWrapper.test_loadFileContent();
                REQUIRE(!content.empty());

                auto parser  = PersonalizationFileParser(path);
                auto jsonObj = parser.parseJson();
                REQUIRE(jsonObj != nullptr);

                auto serialNumber = jsonObj[param::serial_number::key].string_value();
                auto caseColour   = jsonObj[param::case_colour::key].string_value();

                REQUIRE(serialNumber == expectedSerialNumber);
                REQUIRE(caseColour == expectedCaseColour);
            }

            SECTION("Wrong json format")
            {
                auto path = std::filesystem::current_path();
                path += "/testfiles/data/personalization_files/invalid/invalid_json_format.json";

                auto fileParserTestWrapper = PersonalizationFileParserTestWrapper(path);

                auto content = fileParserTestWrapper.test_loadFileContent();
                REQUIRE(!content.empty());

                auto parser  = PersonalizationFileParser(path);
                auto jsonObj = parser.parseJson();
                REQUIRE(jsonObj == nullptr);
            }

            SECTION("File doesn't exist")
            {
                auto path = std::filesystem::current_path();
                path += "/testfiles/data/personalization_files/invalid/not_existing.json";

                auto fileParserTestWrapper = PersonalizationFileParserTestWrapper(path);

                auto content = fileParserTestWrapper.test_loadFileContent();
                REQUIRE(content.empty());

                auto parser  = PersonalizationFileParser(path);
                auto jsonObj = parser.parseJson();
                REQUIRE(jsonObj == nullptr);
            }
        }

        SECTION("Check PersonalizationFileValidator class")
        {

            SECTION("Check test validateFileAndCRC")
            {
                SECTION("Valid case")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateFileAndCRC();

                    REQUIRE(result);
                }

                SECTION("File does not exist")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/not_existing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateFileAndCRC();

                    REQUIRE(!result);
                }

                SECTION("Wrong crc")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_crc.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateFileAndCRC();

                    REQUIRE(!result);
                }
            }

            SECTION("Check test validateFileAndItsContent")
            {
                SECTION("Valid case")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateJsonObject();

                    REQUIRE(result);
                }

                SECTION("File does not exist")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/not_existing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateJsonObject();

                    REQUIRE(!result);
                }

                SECTION("Wrong json format")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_json_format.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto result    = validator.test_validateJsonObject();

                    REQUIRE(!result);
                }
            }

            SECTION("Check validation of serial number")
            {
                SECTION("Valid case 1")
                {

                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByFormat(phone_personalization::param::serial_number::key);

                    REQUIRE(serialNumber.isValid);
                }

                SECTION("Valid case 2")
                {

                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_white.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByFormat(phone_personalization::param::serial_number::key);

                    REQUIRE(serialNumber.isValid);
                }

                SECTION("Invalid (validate serial number by values)")
                {

                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_white.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByValues(phone_personalization::param::serial_number::key);

                    REQUIRE(!serialNumber.isValid);
                }

                SECTION("Empty serial field")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/personalization_empty_serial.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByFormat(phone_personalization::param::serial_number::key);

                    REQUIRE(!serialNumber.isValid);
                }

                SECTION("serial field is missing")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/serial_number_missing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByFormat(phone_personalization::param::serial_number::key);

                    REQUIRE(!serialNumber.isValid);
                }

                SECTION("Wrong serial number format")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_serial_number.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto serialNumber =
                        validator.test_validateByFormat(phone_personalization::param::serial_number::key);

                    REQUIRE(!serialNumber.isValid);
                }
            }

            SECTION("Check validation of case colour")
            {
                SECTION("Valid cases")
                {
                    SECTION("case colour black")
                    {
                        auto path = std::filesystem::current_path();
                        path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                        auto parser  = PersonalizationFileParser(path);
                        auto jsonObj = parser.parseJson();

                        auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                        auto caseColour =
                            validator.test_validateByValues(phone_personalization::param::case_colour::key);

                        REQUIRE(caseColour.isValid);
                    }
                }

                SECTION("case colour white")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/valid/personalization_white.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator  = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto caseColour = validator.test_validateByValues(phone_personalization::param::case_colour::key);

                    REQUIRE(caseColour.isValid);
                }

                SECTION("case colour is missing")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/case_colour_missing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator  = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto caseColour = validator.test_validateByValues(phone_personalization::param::case_colour::key);

                    REQUIRE(!caseColour.isValid);
                }

                SECTION("Invalid case")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/personalization_yellow.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator  = PersonalizationFileValidatorWrapper(jsonObj, path);
                    auto caseColour = validator.test_validateByValues(phone_personalization::param::case_colour::key);

                    REQUIRE(!caseColour.isValid);
                }
            }
        }

        SECTION("Check Personalization data validation")
        {
            SECTION("Valid file 1")
            {
                auto path = std::filesystem::current_path();
                path += "/testfiles/data/personalization_files/valid/personalization_black.json";

                auto expectedSerialNumber = "SN123456789098";
                auto expectedCaseColour   = "black";

                auto parser  = PersonalizationFileParser(path);
                auto jsonObj = parser.parseJson();

                auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                REQUIRE(validator.validate());

                auto paramsData = PersonalizationData();
                paramsData.setParamsFromJson(jsonObj);

                paramsData.setInvalidParamsAsDefault(validator.getParamsModel());

                REQUIRE(paramsData.getSerialNumber() == expectedSerialNumber);
                REQUIRE(paramsData.getCaseColour() == expectedCaseColour);
            }

            SECTION("Valid file 2")
            {
                auto path = std::filesystem::current_path();
                path += "/testfiles/data/personalization_files/valid/personalization_white.json";

                auto expectedSerialNumber = "SN123456789098";
                auto expectedCaseColour   = "white";

                auto parser  = PersonalizationFileParser(path);
                auto jsonObj = parser.parseJson();

                auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                REQUIRE(validator.validate());

                auto paramsData = PersonalizationData();
                paramsData.setParamsFromJson(jsonObj);

                paramsData.setInvalidParamsAsDefault(validator.getParamsModel());

                REQUIRE(paramsData.getSerialNumber() == expectedSerialNumber);
                REQUIRE(paramsData.getCaseColour() == expectedCaseColour);
            }

            SECTION("Invalid file")
            {

                SECTION("File does not exist")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/not_existing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Invalid crc")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_crc.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Invalid format")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_json_format.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Empty serial number")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/personalization_empty_serial.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Invalid serial number")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/invalid_serial_number.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Serial number is missing")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/serial_number_missing.json";

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(!validator.validate());
                }

                SECTION("Invalid case colour")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/personalization_yellow.json";

                    auto expectedSerialNumber = "SN123456789098";
                    auto expectedCaseColour   = param::case_colour::default_value;

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(validator.validate());

                    auto paramsData = PersonalizationData();
                    paramsData.setParamsFromJson(jsonObj);

                    paramsData.setInvalidParamsAsDefault(validator.getParamsModel());

                    REQUIRE(paramsData.getSerialNumber() == expectedSerialNumber);
                    REQUIRE(paramsData.getCaseColour() == expectedCaseColour);
                }

                SECTION("Case colour is missing")
                {
                    auto path = std::filesystem::current_path();
                    path += "/testfiles/data/personalization_files/invalid/case_colour_missing.json";

                    auto expectedSerialNumber = "SN123456789098";
                    auto expectedCaseColour   = param::case_colour::default_value;

                    auto parser  = PersonalizationFileParser(path);
                    auto jsonObj = parser.parseJson();

                    auto validator = PersonalizationFileValidatorWrapper(jsonObj, path);
                    REQUIRE(validator.validate());

                    auto paramsData = PersonalizationData();
                    paramsData.setParamsFromJson(jsonObj);

                    paramsData.setInvalidParamsAsDefault(validator.getParamsModel());

                    REQUIRE(paramsData.getSerialNumber() == expectedSerialNumber);
                    REQUIRE(paramsData.getCaseColour() == expectedCaseColour);
                }
            }
        }
    }
} // namespace phone_personalization
