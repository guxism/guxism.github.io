ALTER TABLE tracks ADD COLUMN yaid UUID DEFAULT uuid_generate_v4 ();
