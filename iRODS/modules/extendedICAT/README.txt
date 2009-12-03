The Extented ICAT module allows sites to add your own tables into the
iRODS ICAT system, making those tables queryable via the general-query
(rc/rsGenQuery) and updateable (insert or delete) via the
general-update (rc/rsGeneralUpdate).  'iquest' will also be aware of
your new columns for this user interface to the general-query.  Your
added tables can be used much like the built-in ICAT tables: queries
can be made on them via microservices, by clients, etc.  The
general-update can insert new rows with provided values, the current
irods time (determined by ICAT code), or a sequence value (ICAT code
using a DBMS sequence) (see lib/core/include/rodsGeneralUpdate.h).

To enable this:

1) Create an extendedICAT.h in this directory with the various
definitions of your tables, columns, and possibly links between
tables.  There are examples in the example1, example2, and example3
subdirectories.  See example3 for creating links (foreign keys)
between tables.  You can just copy the example .h and perhaps the
corresponding .sql file to this directory.

2) If you want, create a icatExtTables.sql and icatExtInserts.sql in
this directory.  If these exist, they will be invoked by irodssetup
during the installation process. You can also execute this SQL
manually (via, for example, psql) after installation, if you prefer.
For these too, there are examples in the example subdirectories.

3) After installation, enable this module by editing config/config.mk,
uncommenting out the line:
# EXTENDED_ICAT = 1

4) Then run 'make'.
