#!/bin/csh -e
#!/bin/csh -ex
# gcov script for the lib directory
if ($#argv > 1) then
  echo "Too many arguments"
  echo "usage: gcovlib.csh [clean|help]"
  exit 1
else if ($#argv == 1) then
  if ($1 == "clean") then
    rm lib/*.gcov
    exit 0
  else if ($1 == "help") then
    echo "usage: gcovlib.csh [clean|help]"
    exit 0
  else
    echo "unknown input:" $1
    exit 1
  endif
endif

cd lib
# gcov -o ../../lib/rbudp/obj ../../lib/rbudp/src/sendfile.c
gcov -o ../../lib/rbudp/obj ../../lib/rbudp/src/QUANTAnet_rbudpSender_c.c
gcov -o ../../lib/rbudp/obj ../../lib/rbudp/src/QUANTAnet_rbudpReceiver_c.c
# gcov -o ../../lib/rbudp/obj ../../lib/rbudp/src/recvfile.c
gcov -o ../../lib/rbudp/obj ../../lib/rbudp/src/QUANTAnet_rbudpBase_c.c
# ../../lib/isio/test3.c
gcov -o ../../lib/isio ../../lib/isio/src/isio.c
# ../../lib/isio/test1.c
# ../../lib/isio/test2.c
gcov -o ../../lib/fortran ../../lib/fortran/src/fortran_io.c
gcov -o ../../lib/md5/obj ../../lib/md5/src/md5Checksum.c
gcov -o ../../lib/md5/obj ../../lib/md5/src/md5c.c
gcov -o ../../lib/core/obj ../../lib/core/src/msParam.c
gcov -o ../../lib/core/obj ../../lib/core/src/mvUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/igsi.c
gcov -o ../../lib/core/obj ../../lib/core/src/rodsPath.c
gcov -o ../../lib/core/obj ../../lib/core/src/sockComm.c
gcov -o ../../lib/core/obj ../../lib/core/src/mkdirUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/cpUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/packStruct.c
gcov -o ../../lib/core/obj ../../lib/core/src/rodsLog.c
gcov -o ../../lib/core/obj ../../lib/core/src/rmUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/rcPortalOpr.c
gcov -o ../../lib/core/obj ../../lib/core/src/osauth.c
gcov -o ../../lib/core/obj ../../lib/core/src/ikrb.c
gcov -o ../../lib/core/obj ../../lib/core/src/trimUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/rcMisc.c
gcov -o ../../lib/core/obj ../../lib/core/src/ikrbGSSAPIWrapper.c
gcov -o ../../lib/core/obj ../../lib/core/src/phybunUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/getUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/rmtrashUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/clientLogin.c
gcov -o ../../lib/core/obj ../../lib/core/src/chksumUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/regUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/base64.c
gcov -o ../../lib/core/obj ../../lib/core/src/parseCommandLine.c
gcov -o ../../lib/core/obj ../../lib/core/src/getRodsEnv.c
gcov -o ../../lib/core/obj ../../lib/core/src/rsyncUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/obf.c
gcov -o ../../lib/core/obj ../../lib/core/src/procApiRequest.c
gcov -o ../../lib/core/obj ../../lib/core/src/fsckUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/bunUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/stringOpr.c
gcov -o ../../lib/core/obj ../../lib/core/src/scanUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/putUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/replUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/miscUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/phymvUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/lsUtil.c
gcov -o ../../lib/core/obj ../../lib/core/src/rcConnect.c
gcov -o ../../lib/core/obj ../../lib/core/src/mcollUtil.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileGetFsFreeSpace.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetXmsgTicket.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileCreate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjCreate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileSyncToArch.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSpecificQuery.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjWrite.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileOpendir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileRmdir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcChkObjPermAndStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjLseek.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjClose.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStructFileBundle.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjTrim.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileReaddir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileMkdir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileUnlink.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjRename.c
gcov -o ../../lib/api/obj ../../lib/api/src/apiDoc.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileClosedir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcModAccessControl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetHostForGet.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcAuthRequest.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileWrite.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGeneralUpdate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjRead.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileOpendir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileFstat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSimpleQuery.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileMkdir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileRead.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcExecMyRule.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjGet.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileOpen.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileRmdir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFilePut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcChkNVPathPerm.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjFsync.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGenQuery.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcKrbAuthRequest.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjUnlink.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileStageToCache.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileLseek.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcUnbunAndRegPhyBunfile.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileCreate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetRemoteZoneResc.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileGet.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRuleExecDel.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileClose.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcBulkDataObjReg.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcOprComplete.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGeneralRowPurge.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDatabaseRescOpen.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileWrite.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileTruncate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileRename.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileRead.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileGet.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileRename.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcPhyPathReg.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcProcStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRuleExecSubmit.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileFsync.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcCollCreate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileChmod.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStructFileExtAndReg.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDatabaseRescClose.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetHostForPut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStreamRead.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjRepl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjRsync.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileOpen.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileLseek.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRmColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileFstat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRuleExecMod.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSendXmsg.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDatabaseObjControl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcCloseCollection.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjCreateAndStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcReadCollection.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGsiAuthRequest.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcL3FileGetSingleBuf.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcBulkDataObjPut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjTruncate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjPut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStreamClose.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcUserAdmin.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileStage.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcUnregDataObj.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcL3FilePutSingleBuf.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcAuthCheck.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileClose.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFilePut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileTruncate.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcEndTransaction.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRmCollOld.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSubStructFileClosedir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcSyncMountedColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileChksum.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileUnlink.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcAuthResponse.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStructFileSync.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileReaddir.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcStructFileExtract.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataPut.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcPhyBundleColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjChksum.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcExecCmd.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetTempPassword.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjCopy.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcCollRepl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcFileStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcModDataObjMeta.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetRescQuota.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcModAVUMetadata.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataCopy.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcQuerySpecColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjOpen.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRegColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjPhymv.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcModColl.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGeneralRowInsert.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGeneralAdmin.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRcvXmsg.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRegDataObj.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataObjOpenAndStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcObjStat.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcDataGet.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcGetMiscSvrInfo.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcRegReplica.c
gcov -o ../../lib/api/obj ../../lib/api/src/rcOpenCollection.c
# ../../lib/test/src/l1test.c
# ../../lib/test/src/listcoll.c
# ../../lib/test/src/l1rm.c
# ../../lib/test/src/xmsgtest.c
# ../../lib/test/src/xmltest.c
# ../../lib/test/src/packtest.c
# ../../lib/test/src/l3structFile.c
# ../../lib/test/src/lowlevtest.c
# ../../lib/test/src/luketest.c
# ../../lib/test/src/iTestGenQuery.c
# ../../lib/test/src/testrule.c
# ../../lib/test/src/phptest.c
