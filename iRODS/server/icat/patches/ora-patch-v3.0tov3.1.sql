--- Run these SQL statements using the Oracle client sqlplus
--- to upgrade from a 3.0 Oracle ICAT to 3.1.
---
--- Note: the string 'tempZone' in the last insert should be changed
--- to your zone name.  You need to do this manually for updates but
--- this is handled automatically when installing a new ICAT as the
--- script modifies tempZone to your zone name.  For this change, we
--- should have used an 'update' instead of the 'drop' and 'insert'
--- but this comment is being added post-release.

create table R_TICKET_MAIN
(
   ticket_id           integer not null,
   ticket_string       varchar(100),
   ticket_type         varchar(20),
   user_id             integer not null,
   object_id           integer not null,
   object_type         varchar(16),
   uses_limit          int  DEFAULT 0,
   uses_count          int  DEFAULT 0,
   write_file_limit    int  DEFAULT 10,
   write_file_count    int  DEFAULT 0,
   write_byte_limit    int  DEFAULT 0,
   write_byte_count    int  DEFAULT 0,
   ticket_expiry_ts    varchar(32),
   restrictions        varchar(16),
   create_ts           varchar(32),
   modify_ts           varchar(32)
);

create table R_TICKET_ALLOWED_HOSTS
(
   ticket_id           integer not null,
   host                varchar(32)
);

create table R_TICKET_ALLOWED_USERS
(
   ticket_id           integer not null,
   user_name           varchar(250) not null
);

create table R_TICKET_ALLOWED_GROUPS
(
   ticket_id           integer not null,
   group_name          varchar(250) not null
);

create index idx_data_main6 on R_DATA_MAIN (data_path);

create unique index idx_ticket on R_TICKET_MAIN (ticket_string);
create unique index idx_ticket_host on R_TICKET_ALLOWED_HOSTS (ticket_id, host);
create unique index idx_ticket_user on R_TICKET_ALLOWED_USERS (ticket_id, user_name);
create unique index idx_ticket_group on R_TICKET_ALLOWED_GROUPS (ticket_id, group_name);


insert into R_TOKN_MAIN values ('data_type',1697,'gzipTar','','|.tar.gz|','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1698,'bzip2Tar','','|.tar.bz2|','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1699,'gzipFile','','|.gz|','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1700,'bzip2File','','|.bz2|','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1701,'zipFile','','|.zip|','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1702,'gzipTar bundle','','','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1703,'bzip2Tar bundle','','','','','1324000000','1324000000');
insert into R_TOKN_MAIN values ('data_type',1704,'zipFile bundle','','','','','1324000000','1324000000');

delete from R_RESC_MAIN where resc_id=9100 and resc_name='bundleResc';

insert into R_RESC_MAIN (resc_id, resc_name, zone_name, resc_type_name, resc_class_name,  resc_net, resc_def_path, free_space, free_space_ts, resc_info, r_comment, resc_status, create_ts, modify_ts) values (9100, 'bundleResc', 'tempZone', 'unix file system', 'bundle', 'localhost', '/bundle', '', '', '', '', '', '1250100000','1250100000');
