ALTER TABLE tracks DROP CONSTRAINT tracks_pkey;
ALTER TABLE tracks ADD PRIMARY KEY (yaid);
ALTER TABLE tracks DROP COLUMN id;
ALTER TABLE tracks RENAME COLUMN yaid TO id;