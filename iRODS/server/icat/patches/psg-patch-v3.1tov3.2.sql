--- Run these SQL statements using the PostgreSQL client psql
--- to upgrade from a 3.1 Postgres ICAT to 3.2.

alter table R_DATA_MAIN add column data_reg_user_id INTEGER DEFAULT 0;
