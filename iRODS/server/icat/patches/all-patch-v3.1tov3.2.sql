--- Depending on your ICAT DBMS type, run these SQL statements using 
---    the MySQL client mysql,
---    the Oracle client sqlplus,
---    or the PostgreSQL client psql,
--- to upgrade from a 3.1 ICAT to 3.2.

create table R_OBJT_FILESYSTEM_META
(
   object_id bigint not null,
   file_uid varchar(32),
   file_gid varchar(32),
   file_owner varchar(250),
   file_group varchar(250),
   file_mode varchar(32),
   file_ctime varchar(32),
   file_mtime varchar(32),
   file_source_path varchar(2700),
   create_ts varchar(32),
   modify_ts varchar(32)
);

create unique index idx_obj_filesystem_meta1 on R_OBJT_FILESYSTEM_META (object_id);

insert into R_TOKN_MAIN values ('resc_type',407,'direct access file system','','','','','1311740184','1311740184');
insert into R_TOKN_MAIN values ('resc_type',408,'ooici','','','','','1347482000','1347482000');
insert into R_TOKN_MAIN values ('resc_type',409,'opendap','','','','','1347482000','1347482000');
insert into R_TOKN_MAIN values ('resc_type',410,'pydap','','','','','1347482000','1347482000');
insert into R_TOKN_MAIN values ('resc_type',411,'erddap','','','','','1347482000','1347482000');
insert into R_TOKN_MAIN values ('resc_type',412,'tds','','','','','1347482000','1347482000');

insert into R_TOKN_MAIN values ('data_type',1705,'netcdf','','','','','1348000000','1348000000');
insert into R_TOKN_MAIN values ('data_type',1706,'hdf5','','','','','1348000000','1348000000');
insert into R_TOKN_MAIN values ('data_type',1707,'msso file','','|.mss|','','','1324000000','1324000000');
