ALTER TABLE tracks ADD COLUMN user_agent varchar(500);
ALTER TABLE tracks ADD COLUMN method varchar(10);
ALTER TABLE tracks ADD COLUMN referer varchar(100);
ALTER TABLE tracks ADD COLUMN host varchar(100);
ALTER TABLE tracks ADD COLUMN origin varchar(100);
