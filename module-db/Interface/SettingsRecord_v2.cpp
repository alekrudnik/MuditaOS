#include "SettingsRecord_v2.hpp"
#include "module-db/queries/settings/QuerySettingsGet_v2.hpp"
#include <sstream>
#include <cassert>

SettingsRecord_v2::SettingsRecord_v2(const SettingsTableRow_v2 &tableRow)
    : Record{tableRow.ID}, path{tableRow.path}, value{tableRow.value}
{}

std::ostream &operator<<(std::ostream &out, const SettingsRecord_v2 &rec)
{
    out << "settings:" << rec.ID << "|" << (rec.path) << "|" << rec.value << "|";

    return out;
}

SettingsRecordInterface_v2::SettingsRecordInterface_v2(SettingsDB *settingsDB) : settingsDB{settingsDB}
{}

SettingsRecord_v2 SettingsRecordInterface_v2::GetByPath(const std::string &path)
{
    return settingsDB->settings_v2.getByPath(path);
}

SettingsRecord_v2 SettingsRecordInterface_v2::GetByID(uint32_t id)
{
    return settingsDB->settings_v2.getById(id);
}

uint32_t SettingsRecordInterface_v2::GetCount()
{
    return settingsDB->settings_v2.count();
}

bool SettingsRecordInterface_v2::Add(const SettingsRecord_v2 &entry)
{
    return settingsDB->settings_v2.add({{.ID = 0}, .path = entry.getPath(), .value = entry.getValue<std::string>()});
}

bool SettingsRecordInterface_v2::RemoveByID(uint32_t id)
{
    auto entry = settingsDB->settings_v2.getById(id);
    if (!entry.isValid()) {
        return false;
    }
    return settingsDB->settings_v2.removeById(id);
}

bool SettingsRecordInterface_v2::Update(const SettingsRecord_v2 &recUpdated)
{
    auto recCurrent = GetByID(recUpdated.ID);
    if (!recCurrent.isValid()) {
        return false;
    }

    return settingsDB->settings_v2.update(SettingsTableRow_v2{
        {.ID = recCurrent.ID}, .path = recUpdated.getPath(), .value = recUpdated.getValue<std::string>()});
}

std::unique_ptr<db::QueryResult> SettingsRecordInterface_v2::runQuery(std::shared_ptr<db::Query> query)
{
    auto settingsQuery = dynamic_cast<db::query::settings::SettingsQuery *>(query.get());
    assert(settingsQuery);
    auto value = GetByPath(settingsQuery->getPath());
    return std::make_unique<db::query::settings::SettingsResult>(value);
}

std::unique_ptr<db::query::settings::SettingsResult> SettingsRecordInterface_v2::runQueryImpl(
    const db::query::settings::SettingsQuery *query)
{
    auto value = GetByPath(query->getPath());
    return std::make_unique<db::query::settings::SettingsResult>(value);
}
