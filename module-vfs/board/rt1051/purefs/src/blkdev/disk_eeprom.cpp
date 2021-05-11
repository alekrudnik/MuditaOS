// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
#include <purefs/blkdev/disk_eeprom.hpp>
#include "bsp/eeprom/eeprom.hpp"
#include "board/rt1051/bsp/eeprom/M24256.hpp"
#include "bsp/BoardDefinitions.hpp"

namespace purefs::blkdev
{
    auto disk_eeprom::probe(unsigned flags) -> int
    {
        cpp_freertos::LockGuard lock(mutex);
        const auto error = bsp::eeprom::init();
        if (error) {
            return -EIO;
        }
        if (!bsp::eeprom::isPresent(bus_id)) {
            return -ENXIO;
        }
        return 0;
    }

    auto disk_eeprom::status() const -> media_status
    {
        return (bsp::eeprom::isPresent(bus_id)) ? (media_status::healthly) : (media_status::error);
    }

    auto disk_eeprom::get_info(info_type what, hwpart_t hwpart) const -> scount_t
    {
        cpp_freertos::LockGuard lock(mutex);
        if (hwpart > 0) {
            return -ERANGE;
        }
        switch (what) {
        case info_type::sector_size:
            return bsp::eeprom::eeprom_block_size(bus_id);
        case info_type::sector_count:
            return bsp::eeprom::eeprom_total_size(bus_id) / bsp::eeprom::eeprom_block_size(bus_id);
        case info_type::erase_block:
            return 0;
        default:
            return -ENOTSUP;
        }
    }

    auto disk_eeprom::write(const void *buf, sector_t lba, std::size_t count, hwpart_t hwpart) -> int
    {
        cpp_freertos::LockGuard lock(mutex);
        if (hwpart > 0) {
            return -ERANGE;
        }
        const size_t block_siz = bsp::eeprom::eeprom_block_size(bus_id);
        const size_t total_siz = bsp::eeprom::eeprom_total_size(bus_id);
        const auto addr        = lba * block_siz;
        const auto len         = count * block_siz;
        if (addr + len > total_siz) {
            return -ERANGE;
        }
        const auto nwr = bsp::eeprom::eeprom_write(bus_id, addr, reinterpret_cast<const char *>(buf), len);
        return (nwr != int(len)) ? (-ENXIO) : (0);
    }

    auto disk_eeprom::read(void *buf, sector_t lba, std::size_t count, hwpart_t hwpart) -> int
    {
        cpp_freertos::LockGuard lock(mutex);
        if (hwpart > 0) {
            return -ERANGE;
        }
        const size_t block_siz = bsp::eeprom::eeprom_block_size(bus_id);
        const size_t total_siz = bsp::eeprom::eeprom_total_size(bus_id);
        const auto addr        = lba * block_siz;
        const auto len         = count * block_siz;
        if (addr + len > total_siz) {
            return -ERANGE;
        }
        const auto nwr = bsp::eeprom::eeprom_read(bus_id, addr, reinterpret_cast<char *>(buf), len);
        return (nwr != int(len)) ? (-ENXIO) : (0);
    }
} // namespace purefs::blkdev
