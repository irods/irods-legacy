-- run these SQL, using postgresql client psql 

create table R_SERVER_LOAD
(
     host_name  varchar(250) not null,
     resc_name  varchar(250) not null,
     cpu_used   INTEGER,
     mem_used   INTEGER,
     swap_used  INTEGER,
     runq_load  INTEGER,
     disk_space INTEGER,
     net_input  INTEGER,
     net_output INTEGER,
     create_ts  varchar(32)
);

create table R_SERVER_LOAD_DIGEST
(
     resc_name   varchar(250) not null,
     load_factor INTEGER,
     create_ts varchar(32)
);