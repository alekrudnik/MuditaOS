#pragma once

#include <module-db/Interface/EventsRecord.hpp>
#include <Common/Query.hpp>
#include <string>

namespace db::query::events
{
    class GetAllLimited : public Query
    {
      public:
        GetAllLimited(const uint32_t &offset, const uint32_t &limit);
        [[nodiscard]] auto debugInfo() const -> std::string override;

        uint32_t offset;
        uint32_t limit;
    };

    class GetAllLimitedResult : public QueryResult
    {
        std::unique_ptr<std::vector<EventsRecord>> records;

      public:
        GetAllLimitedResult(std::unique_ptr<std::vector<EventsRecord>> records);
        [[nodiscard]] auto getResult() -> std::unique_ptr<std::vector<EventsRecord>>;

        [[nodiscard]] auto debugInfo() const -> std::string override;
    };

} // namespace db::query::events
