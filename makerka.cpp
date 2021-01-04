// Make RKA file for B2M emulator
// 2021-01-05 Alemorf aleksey.f.morozov@gmail.com

#include <stdexcept>
#include <string>
#include <iostream>
#include <stdio.h> // FILE
#include <sys/stat.h> // fstat
#include <unistd.h> // unlink
#include <assert.h>
#include <vector>
#include <map>
#include <string.h>

static std::string loadFile(const char* fileName, size_t maxFileSize = SIZE_MAX)
{
    FILE* file = fopen(fileName, "r");
    if (file == nullptr)
    {
        throw std::runtime_error(std::string("Can't open file ") + fileName);
    }

    struct stat buff;
    if (fstat(fileno(file), &buff) != 0)
    {
        auto result = fclose(file);
        assert(result == 0);
        throw std::runtime_error(std::string("Can't check file size ") + fileName);
    }

    if (buff.st_size > maxFileSize)
    {
        auto result = fclose(file);
        assert(result == 0);
        throw std::runtime_error(std::string("Too big file ") + fileName + ", current size "
                                 + std::to_string(buff.st_size) + ", max size " + std::to_string(maxFileSize));
    }

    std::string output;
    output.resize(buff.st_size);

    if (buff.st_size > 0)
    {
        if (fread(&output[0], 1, output.size(), file) != output.size())
        {
            auto result = fclose(file);
            assert(result == 0);
            throw std::runtime_error(std::string("Can't read file ") + fileName);
        }
    }

    auto result = fclose(file);
    assert(result == 0);
    return output;
}

static void saveFile(std::string_view fileName, const void* data, size_t size)
{
    FILE* file = fopen(fileName.data(), "w");
    if (file == nullptr)
    {
        throw std::runtime_error(std::string("Can't create file ") + fileName.data());
    }
    if (size != 0U)
    {
        if (fwrite(data, 1, size, file) != size)
        {
            auto result1 = fclose(file);
            assert(result1 == 0);
            auto result2 = unlink(fileName.data());
            assert(result2 == 0);
            throw std::runtime_error(std::string("Can't write file ") + fileName.data());
        }
    }
    auto result = fclose(file);
    assert(result == 0);
}

static uint16_t apogeySum(std::string_view data)
{
    uint16_t result = 0;
    if (data.size() > 0)
    {
        for (unsigned i = 0; i < data.size() - 1; i++)
            result += static_cast<unsigned char>(data[i]) * 257;
        result = (result & 0xFF00) + ((result + static_cast<unsigned char>(data.back())) & 0xFF);
    }
    return result;
}


#pragma pack(push, 1)

struct RkaFileHeader
{
    uint8_t startHigh, startLow;
    uint8_t endHigh, endLow;
};

struct RkaFileFooter
{
    uint8_t crcHigh, crcLow;
};

#pragma pack(pop)

int main(int argc, const char** argv)
{
    try
    {
        if (argc != 4)
        {
            std::cerr << "Usage: " << argv[0] << " <start address> <output file name> <input file name>" << std::endl;
            return 1;
        }

        const unsigned maxFileSize = 0x10000;

        // Load file
        std::string fileContents = loadFile(argv[3], maxFileSize);

        // Get start address
        char* incorrectStartValue = nullptr;
        const auto start = strtoul(argv[1], &incorrectStartValue, 0);
        if (incorrectStartValue[0] != 0 || start >= maxFileSize || fileContents.size() > maxFileSize - start)
        {
            throw std::runtime_error("Incorrect start address");
        }

        // Calc CRC
        const uint16_t crc = apogeySum(fileContents);

        // Allocate output buffer
        std::vector<uint8_t> output;
        output.resize(fileContents.size() + sizeof(RkaFileHeader) + sizeof(RkaFileFooter));

        // Write header
        RkaFileHeader* rkaFileHeader = reinterpret_cast<RkaFileHeader*>(&output[0]);
        const uint16_t end = start + fileContents.size() - 1;
        rkaFileHeader->startHigh = start >> 8;
        rkaFileHeader->startLow = start;
        rkaFileHeader->endHigh = end >> 8;
        rkaFileHeader->endLow = end;

        // Write body
        memcpy(&output[sizeof(RkaFileHeader)], fileContents.data(), fileContents.size());

        // Write footer
        RkaFileFooter* rkaFileFooter = reinterpret_cast<RkaFileFooter*>(&output[sizeof(RkaFileHeader) + fileContents.size()]);
        rkaFileFooter->crcHigh = crc >> 8;
        rkaFileFooter->crcLow = crc;
        fileContents.insert(fileContents.end(), reinterpret_cast<char*>(&rkaFileFooter), reinterpret_cast<char*>(&rkaFileFooter + 1));

        // Save file
        saveFile(argv[2], output.data(), output.size());
        return 0;

        // Excaption
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch(...)
    {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
