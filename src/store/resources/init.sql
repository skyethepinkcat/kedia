PRAGMA foreign_keys = ON;

CREATE TABLE media (
  media_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  in_use BOOLEAN,
  icon BLOB
);

CREATE TABLE file (
  file_id INTEGER NOT NULL PRIMARY KEY,
  file_checksum TEXT
);

CREATE TABLE series (media_id INTEGER NOT NULL PRIMARY KEY,
                     series_title TEXT NOT NULL,
                     FOREIGN KEY (media_id) REFERENCES media(media_id) ON DELETE CASCADE
);

CREATE TABLE season (
  media_id INTEGER NOT NULL PRIMARY KEY,
  series_id INTEGER NOT NULL,
  season_num INTEGER,
  is_bonus BOOLEAN NOT NULL,
  season_title TEXT,
  FOREIGN KEY (series_id) REFERENCES series(media_id),
  FOREIGN KEY (media_id) REFERENCES media(media_id) ON DELETE CASCADE
);

CREATE TABLE episode (
  media_id INTEGER NOT NULL PRIMARY KEY,
  season_id INTEGER NOT NULL,
  episode_num FLOAT NOT NULL,
  episode_title TEXT,
  FOREIGN KEY (season_id) REFERENCES season(media_id),
  FOREIGN KEY (media_id) REFERENCES media(media_id) ON DELETE CASCADE
);

CREATE TABLE media_file (
  media_id INTEGER NOT NULL,
  file_id INTEGER NOT NULL,
  FOREIGN KEY (media_id) REFERENCES media(media_id) ON DELETE CASCADE,
  FOREIGN KEY (file_id) REFERENCES file(file_id) ON DELETE CASCADE,
  PRIMARY KEY (media_id, file_id)
);

CREATE TABLE drive (
  drive_id INTEGER NOT NULL PRIMARY KEY,
  drive_uuid TEXT NOT NULL,
  drive_name TEXT,
  drive_path TEXT NOT NULL,
  drive_removeable BOOLEAN NOT NULL,
  drive_remote BOOLEAN NOT NULL,
  drive_rank INTEGER,
  drive_video_directory TEXT
);

CREATE TABLE file_location (
  file_location_path TEXT NOT NULL,
  file_id INTEGER NOT NULL,
  drive_id INTEGER NOT NULL,
  PRIMARY KEY(file_id, drive_id)
);

