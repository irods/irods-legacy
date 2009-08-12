-- run these SQL statements, using the postgresql client psql 

alter table R_RESC_MAIN add column resc_status varchar(32);

create table R_USER_AUTH
(
   user_id             bigint not null,
   user_auth_name      varchar(1000)
);

insert into R_USER_AUTH ( user_id, user_auth_name ) select user_id, user_distin_name from r_user_main where user_distin_name <> '';

alter table R_USER_MAIN drop column user_distin_name;

