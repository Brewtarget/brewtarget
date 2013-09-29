CREATE TABLE settings( id integer primary key, version varchar(256) );
INSERT INTO settings VALUES(1,'2.0.2');
ALTER TABLE brewnote ADD COLUMN projected_ferm_points real DEFAULT 0.0;
UPDATE brewnote SET projected_ferm_points = -1.0;
