// Steelrat scenario compiler
// 2014-07-25 Version for Windows compilers, Alemorf aleksey.f.morozov@gmail.com
// 2021-01-05 Version for Linux compilers, Alemorf aleksey.f.morozov@gmail.com

#include <stdexcept>
#include <string>
#include <string_view>
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


static unsigned utf8ToKoi7Chars(unsigned input)
{
    switch (input)
    {
        case 0x044E: return 0x60;
        case 0x0430: return 0x61;
        case 0x0431: return 0x62;
        case 0x0446: return 0x63;
        case 0x0434: return 0x64;
        case 0x0435: return 0x65;
        case 0x0444: return 0x66;
        case 0x0433: return 0x67;
        case 0x0445: return 0x68;
        case 0x0438: return 0x69;
        case 0x0439: return 0x6A;
        case 0x043A: return 0x6B;
        case 0x043B: return 0x6C;
        case 0x043C: return 0x6D;
        case 0x043D: return 0x6E;
        case 0x043E: return 0x6F;
        case 0x043F: return 0x70;
        case 0x044F: return 0x71;
        case 0x0440: return 0x72;
        case 0x0441: return 0x73;
        case 0x0442: return 0x74;
        case 0x0443: return 0x75;
        case 0x0436: return 0x76;
        case 0x0432: return 0x77;
        case 0x044C: return 0x78;
        case 0x044B: return 0x79;
        case 0x0437: return 0x7A;
        case 0x0448: return 0x7B;
        case 0x044D: return 0x7C;
        case 0x0449: return 0x7D;
        case 0x0447: return 0x7E;
        case 0x044A: return 0x7F;
        case 0x042E: return 0x60;
        case 0x0410: return 0x61;
        case 0x0411: return 0x62;
        case 0x0426: return 0x63;
        case 0x0414: return 0x64;
        case 0x0415: return 0x65;
        case 0x0424: return 0x66;
        case 0x0413: return 0x67;
        case 0x0425: return 0x68;
        case 0x0418: return 0x69;
        case 0x0419: return 0x6A;
        case 0x041A: return 0x6B;
        case 0x041B: return 0x6C;
        case 0x041C: return 0x6D;
        case 0x041D: return 0x6E;
        case 0x041E: return 0x6F;
        case 0x041F: return 0x70;
        case 0x042F: return 0x71;
        case 0x0420: return 0x72;
        case 0x0421: return 0x73;
        case 0x0422: return 0x74;
        case 0x0423: return 0x75;
        case 0x0416: return 0x76;
        case 0x0412: return 0x77;
        case 0x042C: return 0x78;
        case 0x042B: return 0x79;
        case 0x0417: return 0x7A;
        case 0x0428: return 0x7B;
        case 0x042D: return 0x7C;
        case 0x0429: return 0x7D;
        case 0x0427: return 0x7E;
        case 0x042A: return 0x7F;
    }
    return 0;
}

static std::string utf8ToKoi7(std::string_view input)
{
    std::string output;
    output.resize(input.size());
    char* out = (char*)output.data();
    for(auto inputCursor = input.begin(); inputCursor != input.end();)
    {
        char prefix = *inputCursor++;

        if ((prefix & 0x80) == 0)
        {
            *out++ = prefix;
            continue;
        }

        if ((~prefix) & 0x20)
        {
            char suffix = *inputCursor++;
            if (suffix == 0) throw std::runtime_error("Unsupported unicode char");
            int first5bit = prefix & 0x1F;
            first5bit <<= 6;
            int sec6bit = suffix & 0x3F;
            int unicode_char = first5bit + sec6bit;

            char c = (char)utf8ToKoi7Chars(unicode_char);
            if (c == 0) throw std::runtime_error("Unsupported unicode char");
            *out++ = c;
            continue;
        }

        throw std::runtime_error("Unsupported unicode char");
    }

    output.resize(out - output.data());
    return output;
}

static bool toUnsigned(unsigned& output, std::string_view input)
{
    const unsigned radix = 10;
    output = 0;
    for(char c : input)
    {
        if (c < '0' || c > '9')
            return false;
        auto next = output * radix + (c - '0');
        if (next < output) return false;
        output = next;
    }
    return true;
}

static std::string align(std::string_view input, unsigned padding)
{
    // Remove spaces
    std::string output;
    output.reserve(input.size());
    bool insertSpace = false;
    for(char c : input)
    {
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t')
        {
            insertSpace = true;
            continue;
        }
        if (insertSpace && !output.empty()) output += " ";
        output += c;
        insertSpace = false;
    }

    // Insert EOL
    static const unsigned screenWidth = 58;
    unsigned len = padding;
    unsigned prevLen = 0;
    char* prevSpace = nullptr;
    for(char& c : output)
    {
        len++;
        if (c == ' ')
        {
            if (len > screenWidth)
            {
                len -= prevLen + 1;
                if (prevSpace != nullptr)
                {
                    *prevSpace = 10;
                }
            }
            prevSpace = &c;
            prevLen = len;
        }
    }
    if (len > screenWidth)
    {
        if (prevSpace != nullptr)
        {
            *prevSpace = 10;
        }
    }

    return output;
}

static std::string processPage(std::string_view page, std::map<unsigned, unsigned>& pageNumbers)
{
    std::string out;
    std::string::size_type pos = 0;
    unsigned currentPadding = 0U;
    unsigned answerPadding = 0U;
    for(;;)
    {
        auto pos1 = page.find("{", pos);
        if (pos1 == page.npos)
        {
            if (pos != page.size())
            {
                out += align(utf8ToKoi7(page.substr(pos)), currentPadding);
                currentPadding = answerPadding;
                out.push_back(0);
            }
            break;
        }
        auto pos2 = page.find("}", pos1);
        if (pos2 == page.npos)
        {
            throw std::runtime_error("{ not closed");
        }
        const unsigned maxPageNumber = 254;
        unsigned pageNumber = maxPageNumber;
        auto labelText = page.substr(pos1 + 1, pos2 - pos1 - 1);
        if (labelText == "NEXT")
        {
            pageNumber = maxPageNumber;
        }
        else if (labelText == "GAMEOVER")
        {
            pageNumber = 0;
        }
        else if (toUnsigned(pageNumber, labelText))
        {
            auto i = pageNumbers.find(pageNumber);
            if (i == pageNumbers.end())
                throw std::runtime_error("Unkonwn label (" + std::to_string(pageNumber) + ")");
            pageNumber = i->second;
        }
        else
        {
            throw std::runtime_error("Incorrect number (" + std::string(labelText) + ")");
        }
        out += align(utf8ToKoi7(page.substr(pos, pos1 - pos)), currentPadding);
        currentPadding = answerPadding;
        out.push_back(0);
        out += (char)pageNumber;
        pos = pos2 + 1;
    }
    out += "\xFF";
    return out;
}

static std::string_view trim(std::string_view str)
{
    std::string_view::size_type l = 0;
    std::string_view::size_type r = str.size();
    while (l < r && str[l] == ' ')
    {
        l++;
    }
    while (r > 0 && str[r - 1] == ' ')
    {
        r--;
    }
    return str.substr(l, r - l);
}

static void convert(const char* outputFileName, const char* inputFileName)
{
    // Load file
    std::string fileContents = loadFile(inputFileName);

    // Split file
    std::map<unsigned, unsigned> pageNumbers;
    std::vector<std::string_view> pages;
    {
        unsigned pageCounter = 0;
        std::string_view::size_type pos = 0;
        std::string_view::size_type start = 0;
        for (;;)
        {
            auto pos1 = fileContents.find("\n", pos);

            std::string_view lineText = trim(std::string_view(fileContents).substr(pos,
                                               pos1 == fileContents.npos ? fileContents.npos : pos1 - pos));

            lineText = trim(lineText);

            if (!lineText.empty())
            {
                unsigned numberInLine = 0;
                if (toUnsigned(numberInLine, lineText))
                {
                    if (pageCounter > 0)
                    {
                        pages.push_back(std::string_view(fileContents).substr(start, pos - start));
                    }
                    pageNumbers[numberInLine] = pageCounter;
                    pageCounter++;
                    start = pos1 + 1;
                }
            }

            if (pos1 == fileContents.npos) break;
            pos = pos1 + 1;
        }

        if (pageCounter > 0)
        {
            pages.push_back(std::string_view(fileContents).substr(start));
        }
    }

    // Convert text
    std::string out;
    for(auto page : pages)
    {
        out += processPage(page, pageNumbers);
    }

    // Save file
    saveFile(outputFileName, out.data(), out.size());
}

int main(int argc, const char** argv)
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <output file name> <input file name>" << std::endl;
            return 1;
        }

        convert(argv[1], argv[2]);
        return 0;
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
