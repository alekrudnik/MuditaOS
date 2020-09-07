#include "ContactsTable.hpp"
#include <log/log.hpp>

namespace ColumnName
{
    const uint8_t id         = 0;
    const uint8_t name_id    = 1;
    const uint8_t numbers_id = 2;
    const uint8_t ring_id    = 3;
    const uint8_t address_id = 4;
    const uint8_t type       = 5;
    const uint8_t speeddial  = 6;
}; // namespace ColumnName

ContactsTable::ContactsTable(Database *db) : Table(db)
{}

ContactsTable::~ContactsTable()
{}

bool ContactsTable::create()
{
    return db->execute(createTableQuery);
}

bool ContactsTable::add(ContactsTableRow entry)
{
    return db->execute("insert or ignore into contacts (name_id, numbers_id, ring_id, address_id, type, speeddial) "
                       " VALUES (%lu, '%q', %lu, %lu, %lu, '%q');",
                       entry.nameID,
                       entry.numbersID.c_str(),
                       entry.ringID,
                       entry.addressID,
                       entry.type,
                       entry.speedDial.c_str());
}

bool ContactsTable::removeById(uint32_t id)
{
    return db->execute("DELETE FROM contacts where _id = %u;", id);
}

bool ContactsTable::BlockByID(uint32_t id, bool shouldBeBlocked)
{
    return db->execute("UPDATE contacts SET blacklist=%lu WHERE _id=%lu", shouldBeBlocked ? 1 : 0, id);
}

bool ContactsTable::update(ContactsTableRow entry)
{
    return db->execute("UPDATE contacts SET name_id = %lu, numbers_id = '%q', ring_id = %lu, address_id = %lu, type "
                       " = %lu, speeddial = '%q' WHERE _id=%lu;",
                       entry.nameID,
                       entry.numbersID.c_str(),
                       entry.ringID,
                       entry.addressID,
                       entry.type,
                       entry.speedDial.c_str(),
                       entry.ID);
}

ContactsTableRow ContactsTable::getById(uint32_t id)
{
    auto retQuery = db->query("SELECT * FROM contacts WHERE _id= %lu;", id);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return ContactsTableRow();
    }

    return ContactsTableRow{
        (*retQuery)[ColumnName::id].getUInt32(),
        (*retQuery)[ColumnName::name_id].getUInt32(),
        (*retQuery)[ColumnName::numbers_id].getString(),
        (*retQuery)[ColumnName::ring_id].getUInt32(),
        (*retQuery)[ColumnName::address_id].getUInt32(),
        static_cast<ContactType>((*retQuery)[ColumnName::type].getUInt32()),
        (*retQuery)[ColumnName::speeddial].getString(),
    };
}

std::vector<ContactsTableRow> ContactsTable::Search(const std::string &primaryName,
                                                    const std::string &alternativeName,
                                                    const std::string &number)
{
    std::vector<ContactsTableRow> ret;

    if (primaryName.empty() && alternativeName.empty() && number.empty()) {
        return (ret);
    }

    std::string q = "select t1.*,t2.name_primary,t2.name_alternative from contacts t1 inner join contact_name "
                    "t2 "
                    "on t1._id=t2.contact_id inner join contact_number t3 on t1._id=t3.contact_id where ";

    if (!primaryName.empty()) {
        q += "t2.name_primary like '%%" + primaryName + "%%'";
        if (!alternativeName.empty())
            q += " or ";
    }

    if (!alternativeName.empty()) {
        q += "t2.name_alternative like '%%" + alternativeName + "%%'";
        if (!number.empty())
            q += " or ";
    }

    if (!number.empty())
        q += "t3.number_e164 like '%%" + number + "%%'";

    LOG_DEBUG("query: \"%s\"", q.c_str());
    auto retQuery = db->query(q.c_str());

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<ContactsTableRow>();
    }

    do {
        ret.push_back(ContactsTableRow{
            (*retQuery)[ColumnName::id].getUInt32(),
            (*retQuery)[ColumnName::name_id].getUInt32(),
            (*retQuery)[ColumnName::numbers_id].getString(),
            (*retQuery)[ColumnName::ring_id].getUInt32(),
            (*retQuery)[ColumnName::address_id].getUInt32(),
            static_cast<ContactType>((*retQuery)[ColumnName::type].getUInt32()),
            (*retQuery)[ColumnName::speeddial].getString(),
            (*retQuery)[ColumnName::speeddial + 1].getString(), // primaryName
            (*retQuery)[ColumnName::speeddial + 2].getString(), // alternativeName (WTF!)
        });
    } while (retQuery->nextRow());

    return ret;
}

std::vector<std::uint32_t> ContactsTable::GetIDsSortedByField(
    MatchType matchType, const std::string &name, std::uint32_t groupId, std::uint32_t limit, std::uint32_t offset)
{
    std::vector<std::uint32_t> ids;

    std::string query = "SELECT DISTINCT contacts._id FROM contacts";

    query += " INNER JOIN contact_name ON contact_name.contact_id == contacts._id";
    query += " LEFT JOIN contact_match_groups ON contact_match_groups.contact_id == contacts._id AND "
             "contact_match_groups.group_id = " +
             std::to_string(groupId);

    switch (matchType) {
    case MatchType::Name: {
        if (!name.empty()) {
            query += " WHERE contact_name.name_primary || ' ' || contact_name.name_alternative LIKE '" + name + "%%'";
            query += " OR contact_name.name_alternative || ' ' || contact_name.name_primary LIKE '" + name + "%%'";
        }
    } break;

    case MatchType::TextNumber: {
        if (!name.empty()) {
            query += " INNER JOIN contact_number ON contact_number.contact_id == contacts._id AND "
                     "contact_number.number_user LIKE '%%" +
                     name + "%%'";
        }
    } break;

    case MatchType::Group:
        query += " WHERE contact_match_groups.group_id == " + std::to_string(groupId);
        break;

    case MatchType::None:
        break;
    }

    query += " ORDER BY group_id DESC ";
    query += " , (contact_name.name_alternative IS NULL OR contact_name.name_alternative ='') ";
    query += " AND (contact_name.name_primary IS NULL OR contact_name.name_primary ='') ASC ";
    query += " , contact_name.name_alternative || ' ' || contact_name.name_primary ";

    if (limit > 0) {
        query += " LIMIT " + std::to_string(limit);
        query += " OFFSET " + std::to_string(offset);
    }

    query += " COLLATE NOCASE;";

    auto queryRet = db->query(query.c_str());
    if ((queryRet == nullptr) || (queryRet->getRowCount() == 0)) {
        return ids;
    }

    do {
        ids.push_back((*queryRet)[0].getUInt32());
    } while (queryRet->nextRow());

    return ids;
}

std::vector<ContactsTableRow> ContactsTable::getLimitOffset(uint32_t offset, uint32_t limit)
{
    auto retQuery = db->query("SELECT * from contacts ORDER BY name_id LIMIT %lu OFFSET %lu;", limit, offset);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<ContactsTableRow>();
    }

    std::vector<ContactsTableRow> ret;

    do {
        ret.push_back(ContactsTableRow{
            (*retQuery)[ColumnName::id].getUInt32(),                             // ID
            (*retQuery)[ColumnName::name_id].getUInt32(),                        // nameID
            (*retQuery)[ColumnName::numbers_id].getString(),                     // numbersID
            (*retQuery)[ColumnName::ring_id].getUInt32(),                        // ringID
            (*retQuery)[ColumnName::address_id].getUInt32(),                     // addressID
            static_cast<ContactType>((*retQuery)[ColumnName::type].getUInt32()), // type
            (*retQuery)[ColumnName::speeddial].getString(),                      // speed dial key
        });
    } while (retQuery->nextRow());

    return ret;
}

std::vector<ContactsTableRow> ContactsTable::getLimitOffsetByField(uint32_t offset,
                                                                   uint32_t limit,
                                                                   ContactTableFields field,
                                                                   const char *str)
{

    std::string fieldName;
    switch (field) {
    case ContactTableFields ::SpeedDial:
        fieldName = "speeddial";
        break;
    default:
        return std::vector<ContactsTableRow>();
    }

    auto retQuery = db->query("SELECT * from contacts WHERE %q='%q' ORDER BY name_id LIMIT %lu OFFSET %lu;",
                              fieldName.c_str(),
                              str,
                              limit,
                              offset);

    if ((retQuery == nullptr) || (retQuery->getRowCount() == 0)) {
        return std::vector<ContactsTableRow>();
    }

    std::vector<ContactsTableRow> ret;

    do {
        ret.push_back(ContactsTableRow{
            (*retQuery)[ColumnName::id].getUInt32(),
            (*retQuery)[ColumnName::name_id].getUInt32(),
            (*retQuery)[ColumnName::numbers_id].getString(),
            (*retQuery)[ColumnName::ring_id].getUInt32(),
            (*retQuery)[ColumnName::address_id].getUInt32(),
            static_cast<ContactType>((*retQuery)[5].getUInt32()),
            (*retQuery)[ColumnName::speeddial].getString(),
        });
    } while (retQuery->nextRow());

    return ret;
}

uint32_t ContactsTable::count()
{
    auto queryRet = db->query("SELECT COUNT(*) FROM contacts;");

    if (queryRet->getRowCount() == 0) {
        return 0;
    }

    return uint32_t{(*queryRet)[0].getUInt32()};
}

uint32_t ContactsTable::countByFieldId(const char *field, uint32_t id)
{
    auto queryRet = db->query("SELECT COUNT(*) FROM contacts WHERE %q=%lu;", field, id);

    if ((queryRet == nullptr) || (queryRet->getRowCount() == 0)) {
        return 0;
    }

    return uint32_t{(*queryRet)[0].getUInt32()};
}
