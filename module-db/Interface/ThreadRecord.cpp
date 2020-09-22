#include "ThreadRecord.hpp"
#include "SMSRecord.hpp"
#include "ContactRecord.hpp"

#include <queries/messages/threads/QueryThreadGetByID.hpp>
#include <queries/messages/threads/QueryThreadGetByNumber.hpp>
#include <queries/messages/threads/QueryThreadGetByContactID.hpp>
#include <queries/messages/threads/QueryThreadRemove.hpp>

#include <cassert>
#include <log/log.hpp>

ThreadRecordInterface::ThreadRecordInterface(SmsDB *smsDb, ContactsDB *contactsDb)
    : smsDB(smsDb), contactsDB(contactsDb)
{}

bool ThreadRecordInterface::Add(const ThreadRecord &rec)
{
    auto ret = smsDB->threads.add(ThreadsTableRow{{.ID = rec.ID},
                                                  .date           = rec.date,
                                                  .msgCount       = rec.msgCount,
                                                  .unreadMsgCount = rec.unreadMsgCount,
                                                  .contactID      = rec.contactID,
                                                  .numberID       = rec.numberID,
                                                  .snippet        = rec.snippet,
                                                  .type           = rec.type});

    return ret;
}

bool ThreadRecordInterface::RemoveByID(uint32_t id)
{
    auto ret = smsDB->threads.removeById(id);
    if (ret == false) {
        return false;
    }

    SMSRecordInterface smsRecordInterface(smsDB, contactsDB);
    return smsRecordInterface.RemoveByField(SMSRecordField::ThreadID, std::to_string(id).c_str());
}

bool ThreadRecordInterface::Update(const ThreadRecord &rec)
{
    return smsDB->threads.update(ThreadsTableRow{{.ID = rec.ID},
                                                 .date           = rec.date,
                                                 .msgCount       = rec.msgCount,
                                                 .unreadMsgCount = rec.unreadMsgCount,
                                                 .contactID      = rec.contactID,
                                                 .numberID       = rec.numberID,
                                                 .snippet        = rec.snippet,
                                                 .type           = rec.type

    });
}

uint32_t ThreadRecordInterface::GetCount()
{
    return smsDB->threads.count();
}

uint32_t ThreadRecordInterface::GetCount(EntryState state)
{
    return smsDB->threads.count(state);
}

std::unique_ptr<std::vector<ThreadRecord>> ThreadRecordInterface::GetLimitOffset(uint32_t offset, uint32_t limit)
{
    auto records = std::make_unique<std::vector<ThreadRecord>>();

    auto ret = smsDB->threads.getLimitOffset(offset, limit);

    for (const auto &w : ret) {
        records->push_back(w);
    }

    return records;
}

std::unique_ptr<std::vector<ThreadRecord>> ThreadRecordInterface::GetLimitOffsetByField(uint32_t offset,
                                                                                        uint32_t limit,
                                                                                        ThreadRecordField field,
                                                                                        const char *str)
{
    auto records = std::make_unique<std::vector<ThreadRecord>>();

    ThreadsTableFields threadsField;
    switch (field) {
    case ThreadRecordField::ContactID: {
        threadsField = ThreadsTableFields::ContactID;
    } break;
    case ThreadRecordField::NumberID: {
        threadsField = ThreadsTableFields::NumberID;
    } break;
    default:
        LOG_ERROR("Invalid field type %u", static_cast<unsigned>(field));
        return records;
    }

    auto ret = smsDB->threads.getLimitOffsetByField(offset, limit, threadsField, str);
    for (const auto &w : ret) {
        records->push_back(w);
    }

    return records;
}

ThreadRecord ThreadRecordInterface::GetByID(uint32_t id)
{
    auto rec = smsDB->threads.getById(id);
    if (!rec.isValid()) {
        return ThreadRecord();
    }

    return ThreadRecord(rec);
}

ThreadRecord ThreadRecordInterface::GetByContact(uint32_t contact_id)
{
    auto ret =
        smsDB->threads.getLimitOffsetByField(0, 1, ThreadsTableFields::ContactID, std::to_string(contact_id).c_str());
    if (ret.size() == 0) {
        ThreadRecord re;
        re.contactID = contact_id;
        if (!Add(re)) {
            LOG_ERROR("There is no thread but we cant add it");
            return ThreadRecord();
        }

        ret = smsDB->threads.getLimitOffsetByField(
            0, 1, ThreadsTableFields::ContactID, std::to_string(contact_id).c_str());
        if (ret.size() == 0) {
            return ThreadRecord();
        }
    }

    return ThreadRecord(ret[0]);
}

ThreadRecord ThreadRecordInterface::GetByNumber(const utils::PhoneNumber::View &phoneNumber)
{
    auto contactInterface = ContactRecordInterface(contactsDB);
    auto match            = contactInterface.MatchByNumber(phoneNumber);

    if (!match.has_value()) {
        return ThreadRecord();
    }

    auto threadRec = GetLimitOffsetByField(0, 1, ThreadRecordField::NumberID, std::to_string(match->numberId).c_str());

    if (threadRec->size() == 0) {
        return ThreadRecord();
    }

    return threadRec->at(0);
}

std::unique_ptr<db::QueryResult> ThreadRecordInterface::runQuery(std::shared_ptr<db::Query> query)
{
    if (const auto localQuery = dynamic_cast<const db::query::ThreadsSearch *>(query.get())) {
        auto dbResult = smsDB->threads.getBySMSQuery(localQuery->text, localQuery->startingPosition, localQuery->depth);

        auto response = std::make_unique<db::query::ThreadsSearchResult>(dbResult.first, dbResult.second);
        response->setRequestQuery(query);
        return response;
    }

    if (const auto localQuery = dynamic_cast<const db::query::ThreadsGet *>(query.get())) {
        auto dbResult = smsDB->threads.getLimitOffset(localQuery->offset, localQuery->limit);

        auto response = std::make_unique<db::query::ThreadsGetResults>(dbResult);
        response->setRequestQuery(query);
        return response;
    }

    if (const auto local_query = dynamic_cast<const db::query::smsthread::MarkAsRead *>(query.get())) {
        auto response = runQueryImpl(local_query);
        response->setRequestQuery(query);
        return response;
    }

    if (const auto local_query = dynamic_cast<const db::query::ThreadGetByID *>(query.get())) {
        const auto ret = GetByID(local_query->id);
        auto response  = std::make_unique<db::query::ThreadGetByIDResult>(
            ret.isValid() ? std::optional<ThreadRecord>{ret} : std::nullopt);
        response->setRequestQuery(query);
        return response;
    }

    if (const auto local_query = dynamic_cast<const db::query::ThreadGetByNumber *>(query.get())) {
        auto response = std::make_unique<db::query::ThreadGetByNumberResult>(GetByNumber(local_query->getNumber()));
        response->setRequestQuery(query);
        return response;
    }

    if (const auto local_query = dynamic_cast<const db::query::ThreadGetByContactID *>(query.get())) {
        const auto thread = GetByContact(local_query->id);
        auto response     = std::make_unique<db::query::ThreadGetByContactIDResult>(
            thread.isValid() ? std::optional<ThreadRecord>(thread) : std::nullopt);
        response->setRequestQuery(query);
        return response;
    }

    if (const auto local_query = dynamic_cast<const db::query::ThreadRemove *>(query.get())) {
        const auto ret = RemoveByID(local_query->id);
        auto response  = std::make_unique<db::query::ThreadRemoveResult>(ret);
        response->setRequestQuery(query);
        return response;
    }

    return nullptr;
}

std::unique_ptr<db::query::smsthread::MarkAsReadResult> ThreadRecordInterface::runQueryImpl(
    const db::query::smsthread::MarkAsRead *query)
{
    using namespace db::query::smsthread;
    auto ret = false;

    auto record = GetByID(query->id);
    if (record.isValid()) {
        LOG_FATAL("query-read %d", static_cast<int>(query->read));
        record.unreadMsgCount = query->read == MarkAsRead::Read::True ? 0 : 1;
        ret                   = Update(record);
    }
    return std::make_unique<MarkAsReadResult>(ret);
}
