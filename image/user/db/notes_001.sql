CREATE TABLE IF NOT EXISTS notes(
    _id INTEGER PRIMARY KEY,
    date INTEGER,
    snippet TEXT DEFAULT '',
    path TEXT DEFAULT ''
);
