#!/bin/csh -e
#!/bin/csh -ex
# gcov script for the clients directory
if ($#argv > 1) then
  echo "Too many arguments"
  echo "usage: gcovclients.csh [clean|help]"
  exit 1
else if ($#argv == 1) then
  if ($1 == "clean") then
    rm clients/*.gcov
    exit 0
  else if ($1 == "help") then
    echo "usage: gcovclients.csh [clean|help]"
    exit 0
  else
    echo "unknown input:" $1
    exit 1
  endif
endif

cd clients
gcov -o ../../clients/icommands/obj ../../clients/icommands/rulegen/lex.yy.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/rulegen/y.tab.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/rulegen/rulegen.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iphymv.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/irule.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ibun.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iscan.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ierror.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iqmod.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/imeta.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iget.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/imv.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iquest.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/irmtrash.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ipasswd.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iuserinfo.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/genOSAuth.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iexecmd.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ips.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iput.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ipwd.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/itrim.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iphybun.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/irsync.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ireg.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/irm.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iadmin.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/idbug.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ifsck.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iquota.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ienv.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iexit.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ihelp.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iqstat.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ichksum.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/icd.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ils.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/imiscsvrinfo.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ilsresc.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/isysmeta.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/idbo.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iqdel.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ixmsg.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/imkdir.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/ichmod.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/irepl.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/icp.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/iinit.c
gcov -o ../../clients/icommands/obj ../../clients/icommands/src/imcoll.c
gcov -o ../../clients/fuse/obj ../../clients/fuse/src/iFuseOper.c
gcov -o ../../clients/fuse/obj ../../clients/fuse/src/iFuseLib.c
gcov -o ../../clients/fuse/obj ../../clients/fuse/src/irodsFs.c
