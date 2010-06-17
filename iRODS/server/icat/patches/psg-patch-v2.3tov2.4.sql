-- run these SQL statements, using the postgresql client psql 
--

ALTER TABLE r_resc_group ADD COLUMN resc_group_id bigint;
ALTER TABLE r_resc_group ALTER COLUMN resc_group_id SET STORAGE PLAIN;

-- Should generate the id for the existing resc_group ...
UPDATE r_resc_group SET resc_group_id=nextval('r_objectid') WHERE resc_group_id IS NULL;

ALTER TABLE r_resc_group ALTER COLUMN resc_group_id SET NOT NULL;


