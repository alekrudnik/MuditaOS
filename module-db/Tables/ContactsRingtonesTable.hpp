
/*
 * @file ContactsRingtonesTable.hpp
 * @author Mateusz Piesta (mateusz.piesta@mudita.com)
 * @date 28.05.19
 * @brief
 * @copyright Copyright (C) 2019 mudita.com
 * @details
 */

#pragma once

#include "Common/Common.hpp"
#include "Database/Database.hpp"
#include "Table.hpp"
#include "utf8/UTF8.hpp"
#include <string>
#include "Record.hpp"

struct ContactsRingtonesTableRow : public Record
{
    uint32_t contactID = DB_ID_NONE;
    UTF8 assetPath     = "";
    ContactsRingtonesTableRow(uint32_t ID = DB_ID_NONE, uint32_t contactID = 0, UTF8 assetPath = "")
        : contactID(contactID), assetPath(assetPath)
    {
        this->ID = ID;
    }
    ContactsRingtonesTableRow(uint32_t contactID, UTF8 assetPath) : contactID(contactID), assetPath(assetPath)
    {}
};

enum class ContactRingtonesTableFields
{
    AssetPath
};

class ContactsRingtonesTable : public Table<ContactsRingtonesTableRow, ContactRingtonesTableFields>
{
  public:
    ContactsRingtonesTable(Database *db);

    virtual ~ContactsRingtonesTable();

    bool create() override final;

    bool add(ContactsRingtonesTableRow entry) override final;

    bool removeById(uint32_t id) override final;

    bool update(ContactsRingtonesTableRow entry) override final;

    ContactsRingtonesTableRow getById(uint32_t id) override final;

    std::vector<ContactsRingtonesTableRow> getLimitOffset(uint32_t offset, uint32_t limit) override final;

    std::vector<ContactsRingtonesTableRow> getLimitOffsetByField(uint32_t offset,
                                                                 uint32_t limit,
                                                                 ContactRingtonesTableFields field,
                                                                 const char *str) override final;

    uint32_t count() override final;

    uint32_t countByFieldId(const char *field, uint32_t id) override final;

  private:
    const char *createTableQuery = "CREATE TABLE IF NOT EXISTS contact_ringtones("
                                   "_id              INTEGER PRIMARY KEY,"
                                   "contact_id       INTEGER,"
                                   "asset_path       TEXT NOT NULL,"
                                   "FOREIGN KEY(contact_id) REFERENCES contacts(_id) ON DELETE CASCADE"
                                   ");";
};
