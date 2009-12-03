exec iadmin mkuser test1 rodsuser

exec iadmin moduser test1 password test

exec iadmin mkuser test2 rodsuser

exec iadmin moduser test2 password test

exec iadmin mkresc test1-resc "unix file system" cache localhost "/opt/iRODS/iRODS/Vault"

exec iadmin mkresc test1-resc2 "unix file system" cache localhost "/opt/iRODS/iRODS/Vault2"

