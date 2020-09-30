#pragma once

#include <cstdint>
#include <map>

namespace phonebookContactsMap
{
    constexpr uint32_t NO_MATCH_FOUND = 0;
}

enum class ContactQuerySection
{
    Favourites = 0,
    Mixed,
};

enum class ContactDisplayMode
{
    Regular = 0,
    SortedByLetter,
};

enum class LabelMarkerDisplayMode
{
    IncludeFavourites = 0,
    IgnoreFavourites,
};

struct ContactsMapData
{
    std::map<std::string, std::uint32_t> firstLetterDictionary;
    std::uint32_t favouritesCount;
    std::uint32_t itemCount;
};
