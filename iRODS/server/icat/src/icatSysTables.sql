
/*****************************************************************************
  These are the System Tables in the RODS Catalog 
    R_DATA_xxx            - Tables defining Data Info
    R_ZONE_xxx            - Tables defining Zone Info
    R_USER_xxx            - Tables defining User Info
    R_RESC_xxx            - Tables defining Resource Info
    R_COLL_xxx            - Tables defining Collection Info
    R_META_xxx            - Tables defining Metadata Info
    R_TOKN_xxx            - Tables defining Token Info
    R_RULE_xxx            - Tables defining Rules Info 
    R_OBJT_xxx            - Tables defining info applicable to multiple 
                                 first-class objects
  The column lengths are used as follows:
    ids                - bigint (64 bit)
    dates              - varchar(32)
    short strings      - varchar(250)
    long strings       - varchar(1000)
    very long strings  - varchar(2700)

  R_TOKN_MAIN table is like a meta table for holding all
    reserved keywords/tokens/systemic ontologies that are used by
    the other RODS table. For example, one may store information
    about the data_types in here instead of storing them in a 
    separate table. 
    
    Hence rows such as 
        token_id        = 1000
        token_namespace = 'data_type' 
        token_name      = 'gif image'
    or
        token_id        = 20
        token_namespace = 'access_type'
        token_name      = 'write'
        token_value     = '020' 
    will provide the keyword for the different token types.

    This is actually a multi-hierarchy table because what is used in the
    'token_namespace' column should be validated. To assist with that, 
    we use the string 'token_namespace' as a reserved keyword and use it to
    boot-strap the other tokens. Hence, on installation, there will be a 
    namespace called 'token_namespace' with the following token_names: 
         'data_type', 'object_type','zone_type','resc_type',
          'user_type','action_type','rulexec_type','access_type',
          'resc_class','coll_map', 'data_type_dot_ext', 'data_type_mime',
	  'auth_scheme_type'.

    On installation, each of the above mentioned namespaces will have  
    at least one token_name called 'generic' associated with them.
    On installation, other values might be populated for a few 
    token_namespaces.
    
    Whenever a new value for a token is introduced. the token_type is checked 
    to see if the token_namespace is a valid one, and the triple
    (token_namespace, token_name, token_value) is checked for uniqueness 
    before being added.

    Wheneever a token value is filled in any other table, it is checked 
    against the R_TOKN_MAIN table for validity.
*******************************************************/

create table R_ZONE_MAIN
 (
   zone_id             bigint not null,
   zone_name           varchar(250) not null,
   zone_type_name      varchar(250) not null,
   zone_conn_string    varchar(1000),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_USER_MAIN
 (
   user_id             bigint not null,
   user_name           varchar(250) not null,
   user_type_name      varchar(250) not null,
   zone_name           varchar(250) not null,
   user_info           varchar(1000),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_RESC_MAIN
 (
   resc_id             bigint not null,
   resc_name           varchar(250) not null,
   zone_name           varchar(250) not null,
   resc_type_name      varchar(250) not null,
   resc_class_name     varchar(250) not null,
   resc_net            varchar(250) not null,
   resc_def_path       varchar(1000) not null,
   free_space          varchar(250),
   free_space_ts       varchar(32),
   resc_info           varchar(1000),
   r_comment           varchar(1000),
   resc_status         varchar(32),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_COLL_MAIN
 (
   coll_id             bigint not null,
   parent_coll_name    varchar(2700) not null,
   coll_name           varchar(2700) not null,
   coll_owner_name     varchar(250) not null,
   coll_owner_zone     varchar(250) not null,
   coll_map_id         bigint DEFAULT 0,
   coll_inheritance    varchar(1000),
   coll_type	       varchar(250) DEFAULT '0',
   coll_info1          varchar(2700) DEFAULT '0',
   coll_info2          varchar(2700) DEFAULT '0',
   coll_expiry_ts      varchar(32),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

/* 
  The data_is_dirty column is replStatus in the DataObjStatus structure.
  The data_status column is statusString (unused, currently).
*/
create table R_DATA_MAIN
 (
   data_id             bigint not null,
   coll_id             bigint not null,
   data_name           varchar(1000) not null,
   data_repl_num       INTEGER not null,
   data_version        varchar(250) DEFAULT '0',
   data_type_name      varchar(250) not null,
   data_size           bigint not null,
   resc_group_name     varchar(250),
   resc_name           varchar(250) not null,
   data_path           varchar(2700) not null,
   data_owner_name     varchar(250) not null,
   data_owner_zone     varchar(250) not null,
   data_is_dirty       INTEGER  DEFAULT 0,
   data_status         varchar(250),
   data_checksum       varchar(1000),
   data_expiry_ts      varchar(32),
   data_map_id         bigint DEFAULT 0,
   data_mode           varchar(32),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_META_MAIN
 (
   meta_id             bigint not null,
   meta_namespace      varchar(250),
   meta_attr_name      varchar(2700) not null,
   meta_attr_value     varchar(2700) not null,
   meta_attr_unit      varchar(250),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_TOKN_MAIN
 (
   token_namespace     varchar(250) not null,
   token_id            bigint not null,
   token_name          varchar(250) not null,
   token_value         varchar(250),
   token_value2        varchar(250),
   token_value3        varchar(250),
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_RULE_MAIN
 (
   rule_id bigint not null,
   rule_version varchar(250) DEFAULT '0',
   rule_base_name varchar(250) not null,
   rule_name varchar(250) not null DEFAULT 'null',
   rule_event varchar(2700) not null,
   rule_condition varchar(2700),
   rule_body varchar(2700) not null,
   rule_recovery varchar(2700) not null,
   rule_status INTEGER DEFAULT 1,
   rule_owner_name varchar(250) not null,
   rule_owner_zone varchar(250) not null,
   r_comment varchar(1000),
   create_ts varchar(32) ,
   modify_ts varchar(32)
 );

create table R_RULE_DVM
(
   dvm_id bigint not null,
   dvm_version varchar(250) DEFAULT '0',
   dvm_base_name varchar(250) not null,
   dvm_ext_var_name varchar(250) not null,
   dvm_condition varchar(2700),
   dvm_int_map_path varchar(2700) not null,
   dvm_status INTEGER DEFAULT 1,
   dvm_owner_name varchar(250) not null,
   dvm_owner_zone varchar(250) not null,
   r_comment varchar(1000),
   create_ts varchar(32) ,
   modify_ts varchar(32)
);

create table R_RULE_FNM
(
   fnm_id bigint not null,
   fnm_version varchar(250) DEFAULT '0',
   fnm_base_name varchar(250) not null,
   fnm_ext_func_name varchar(250) not null,
   fnm_int_func_name varchar(2700) not null,
   fnm_status INTEGER DEFAULT 1,
   fnm_owner_name varchar(250) not null,
   fnm_owner_zone varchar(250) not null,
   r_comment varchar(1000),
   create_ts varchar(32) ,
   modify_ts varchar(32)
);


create table R_RULE_EXEC
 (
   rule_exec_id        bigint not null,
   rule_name           varchar(2700) not null,
   rei_file_path       varchar(2700),
   user_name           varchar(250),
   exe_address         varchar(250),
   exe_time            varchar(32),
   exe_frequency       varchar(250),
   priority            varchar(32),
   estimated_exe_time  varchar(32),
   notification_addr   varchar(250),
   last_exe_time       varchar(32),
   exe_status          varchar(32),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_USER_GROUP
 (
   group_user_id       bigint not null,
   user_id             bigint not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_USER_SESSION_KEY
 (
   user_id             bigint not null,
   session_key         varchar(1000) not null,
   session_info        varchar(1000) ,
   auth_scheme         varchar(250) not null,
   session_expiry_ts   varchar(32) not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_USER_PASSWORD
 (
   user_id             bigint not null,
   rcat_password       varchar(250) not null,
   pass_expiry_ts      varchar(32) not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );



create table R_RESC_GROUP
 (
   resc_group_name     varchar(250) not null,
   resc_id             bigint not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_OBJT_METAMAP
 (
   object_id           bigint not null,
   meta_id             bigint not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_OBJT_ACCESS
 (
   object_id           bigint not null,
   user_id             bigint not null,
   access_type_id      bigint not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_OBJT_DENY_ACCESS
 (
   object_id           bigint not null,
   user_id             bigint not null,
   access_type_id      bigint not null,
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

create table R_OBJT_AUDIT
 (
   object_id           bigint not null,
   user_id             bigint not null,
   action_id           bigint not null,
   r_comment           varchar(1000),
   create_ts           varchar(32),
   modify_ts           varchar(32)
 );

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

/*
 Optional user authentication information,
 GSI DN(s) or Kerberos Principal name(s)
*/
create table R_USER_AUTH
(
   user_id             bigint not null,
   user_auth_name      varchar(1000),
   create_ts varchar(32)
);


create sequence R_ObjectId increment by 1 start with 10000;

create unique index idx_zone_main1 on R_ZONE_MAIN (zone_id);
create unique index idx_zone_main2 on R_ZONE_MAIN (zone_name);
create index idx_user_main1 on R_USER_MAIN (user_id);
create unique index idx_user_main2 on R_USER_MAIN (user_name,zone_name);
create index idx_resc_main1 on R_RESC_MAIN (resc_id);
create unique index idx_resc_main2 on R_RESC_MAIN (zone_name,resc_name);
create index idx_coll_main1 on R_COLL_MAIN (coll_id);
create unique index idx_coll_main2 on R_COLL_MAIN (parent_coll_name,coll_name);
create unique index idx_coll_main3 on R_COLL_MAIN (coll_name);
create index idx_data_main1 on R_DATA_MAIN (data_id);
create unique index idx_data_main2 on R_DATA_MAIN (coll_id,data_name,data_repl_num,data_version);
create unique index idx_meta_main1 on R_META_MAIN (meta_id);
create unique index idx_rule_main1 on R_RULE_MAIN (rule_id);
create unique index idx_rule_exec on R_RULE_EXEC (rule_exec_id);
create unique index idx_user_group1 on R_USER_GROUP (group_user_id,user_id);
create unique index idx_resc_logical1 on R_RESC_GROUP (resc_group_name,resc_id);
create unique index idx_objt_metamap1 on R_OBJT_METAMAP (object_id,meta_id);
create unique index idx_objt_access1 on R_OBJT_ACCESS (object_id,user_id);
create unique index idx_objt_daccs1 on R_OBJT_DENY_ACCESS (object_id,user_id);
