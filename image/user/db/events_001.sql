-- Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
-- For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

CREATE TABLE IF NOT EXISTS events(
                  _id INTEGER PRIMARY KEY,
                  name TEXT DEFAULT '',
                  start_date DATETIME,
                  end_date DATETIME,
                  duration INTEGER,
                  is_all_day BOOLEAN,
                  rrule TEXT DEFAULT ''
);

CREATE TABLE IF NOT EXISTS alarm_events(
                  _id INTEGER PRIMARY KEY,
                  event_id INTEGER,
                  music_tone TEXT,
                  enabled BOOLEAN,
                  snooze_duration INTEGER,
                  FOREIGN KEY (event_id) REFERENCES events (_id)
);

