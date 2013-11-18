UPDATE settings set version="2.1.0";
ALTER TABLE equipment ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE fermentable ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE hop ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE misc ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE style ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE yeast ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE water ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE mash ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE brewnote ADD COLUMN folder varchar(256) DEFAULT '';
ALTER TABLE recipe ADD COLUMN folder varchar(256) DEFAULT '';

