#!/bin/csh -e
#!/bin/csh -ex
# gcov script for the server directory
if ($#argv > 1) then 
  echo "Too many arguments"
  echo "usage: gcovserver.csh [clean|help]"
  exit 1
else if ($#argv == 1) then
  if ($1 == "clean") then 
    rm server/*.gcov
    exit 0
  else if ($1 == "help") then
    echo "usage: gcovserver.csh [clean|help]"
    exit 0
  else
    echo "unknown input:" $1
    exit 1
  endif
endif

cd server
gcov -o ../../server/icat/obj ../../server/icat/src/icatGeneralUpdate.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatMidLevelHelpers.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatGeneralQuerySetup.c
# gcov -o ../../server/icat/obj ../../server/icat/src/m2icatd.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatGeneralQuery.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatHighLevelRoutines.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatLowLevelOracle.c
gcov -o ../../server/icat/obj ../../server/icat/src/rdaHighLevelRoutines.c
# gcov -o ../../server/icat/obj ../../server/icat/src/rodsEnvInitLib.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatMidLevelRoutines.c
gcov -o ../../server/icat/obj ../../server/icat/src/icatLowLevelOdbc.c
gcov -o ../../server/icat/obj ../../server/icat/src/dboHighLevelRoutines.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/msoFileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/tarStructFileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/structFileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/s3FileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/unixFileDriver.c
# gcov -o ../../server/drivers/obj ../../server/drivers/src/testWos.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/univMSSDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/fileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/wosFileDriver.c
gcov -o ../../server/drivers/obj ../../server/drivers/src/hpssFileDriver.c
# gcov -o ../../server/drivers/obj ../../server/drivers/src/haawStructFileDriver.c
# gcov -o ../../server/drivers/obj ../../server/drivers/test/hpsstest.c
gcov -o ../../server/core/obj ../../server/core/src/initServer.c
gcov -o ../../server/core/obj ../../server/core/src/miscServerFunct.c
gcov -o ../../server/core/obj ../../server/core/src/objDesc.c
gcov -o ../../server/core/obj ../../server/core/src/irodsXmsgServer.c
gcov -o ../../server/core/obj ../../server/core/src/irodsReServer.c
gcov -o ../../server/core/obj ../../server/core/src/readServerConfig.c
gcov -o ../../server/core/obj ../../server/core/src/fileOpr.c
gcov -o ../../server/core/obj ../../server/core/src/fileDriverNoOpFunctions.c
gcov -o ../../server/core/obj ../../server/core/src/xmsgLib.c
gcov -o ../../server/core/obj ../../server/core/src/collection.c
gcov -o ../../server/core/obj ../../server/core/src/rodsAgent.c
# gcov -o ../../server/core/obj ../../server/core/src/irodsNtServer.c
gcov -o ../../server/core/obj ../../server/core/src/rsIcatOpr.c
gcov -o ../../server/core/obj ../../server/core/src/dataObjOpr.c
gcov -o ../../server/core/obj ../../server/core/src/physPath.c
gcov -o ../../server/core/obj ../../server/core/src/rsLog.c
gcov -o ../../server/core/obj ../../server/core/src/rsRe.c
gcov -o ../../server/core/obj ../../server/core/src/rsApiHandler.c
gcov -o ../../server/core/obj ../../server/core/src/resource.c
gcov -o ../../server/core/obj ../../server/core/src/specColl.c
gcov -o ../../server/core/obj ../../server/core/src/reServerLib.c
gcov -o ../../server/core/obj ../../server/core/src/rodsServer.c
gcov -o ../../server/core/obj ../../server/core/src/objMetaOpr.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileStage.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileRead.c
gcov -o ../../server/api/obj ../../server/api/src/rsExecMyRule.c
gcov -o ../../server/api/obj ../../server/api/src/rsSyncMountedColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsGsiAuthRequest.c
gcov -o ../../server/api/obj ../../server/api/src/rsUnregDataObj.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileChksum.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileReaddir.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjGet.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileGet.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFilePut.c
gcov -o ../../server/api/obj ../../server/api/src/rsCollCreate.c
gcov -o ../../server/api/obj ../../server/api/src/rsGeneralAdmin.c
gcov -o ../../server/api/obj ../../server/api/src/rsAuthResponse.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjCreate.c
gcov -o ../../server/api/obj ../../server/api/src/rsOprComplete.c
gcov -o ../../server/api/obj ../../server/api/src/rsSendXmsg.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjOpen.c
gcov -o ../../server/api/obj ../../server/api/src/rsChkObjPermAndStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsModDataObjMeta.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetMiscSvrInfo.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileChmod.c
gcov -o ../../server/api/obj ../../server/api/src/rsKrbAuthRequest.c
gcov -o ../../server/api/obj ../../server/api/src/rsCloseCollection.c
gcov -o ../../server/api/obj ../../server/api/src/rsStructFileExtract.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataPut.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileGet.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjTrim.c
gcov -o ../../server/api/obj ../../server/api/src/rsDatabaseObjControl.c
gcov -o ../../server/api/obj ../../server/api/src/rsQuerySpecColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsRegDataObj.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjFsync.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileMkdir.c
gcov -o ../../server/api/obj ../../server/api/src/rsCollRepl.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileFstat.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetRescQuota.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileFstat.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileOpendir.c
gcov -o ../../server/api/obj ../../server/api/src/rsExecCmd.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetHostForGet.c
gcov -o ../../server/api/obj ../../server/api/src/rsRmCollOld.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjCreateAndStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsStructFileSync.c
gcov -o ../../server/api/obj ../../server/api/src/rsDatabaseRescOpen.c
gcov -o ../../server/api/obj ../../server/api/src/rsModAVUMetadata.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileMkdir.c
gcov -o ../../server/api/obj ../../server/api/src/rsRcvXmsg.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileOpen.c
gcov -o ../../server/api/obj ../../server/api/src/rsDatabaseRescClose.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetHostForPut.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjUnlink.c
gcov -o ../../server/api/obj ../../server/api/src/rsBulkDataObjPut.c
gcov -o ../../server/api/obj ../../server/api/src/rsSpecificQuery.c
gcov -o ../../server/api/obj ../../server/api/src/rsSimpleQuery.c
gcov -o ../../server/api/obj ../../server/api/src/rsL3FileGetSingleBuf.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileWrite.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileTruncate.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjTruncate.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileGetFsFreeSpace.c
gcov -o ../../server/api/obj ../../server/api/src/rsAuthCheck.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetXmsgTicket.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileStageToCache.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjPhymv.c
gcov -o ../../server/api/obj ../../server/api/src/rsBulkDataObjReg.c
gcov -o ../../server/api/obj ../../server/api/src/rsProcStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsPhyBundleColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileLseek.c
gcov -o ../../server/api/obj ../../server/api/src/rsReadCollection.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjRepl.c
gcov -o ../../server/api/obj ../../server/api/src/rsRuleExecMod.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsAuthRequest.c
gcov -o ../../server/api/obj ../../server/api/src/rsObjStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileCreate.c
gcov -o ../../server/api/obj ../../server/api/src/rsGenQuery.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileRead.c
gcov -o ../../server/api/obj ../../server/api/src/rsModAccessControl.c
gcov -o ../../server/api/obj ../../server/api/src/rsGeneralUpdate.c
gcov -o ../../server/api/obj ../../server/api/src/rsStreamClose.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileClosedir.c
gcov -o ../../server/api/obj ../../server/api/src/rsOpenCollection.c
gcov -o ../../server/api/obj ../../server/api/src/rsChkNVPathPerm.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetRemoteZoneResc.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjRsync.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileCreate.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjClose.c
gcov -o ../../server/api/obj ../../server/api/src/rsRmColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileClose.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileWrite.c
gcov -o ../../server/api/obj ../../server/api/src/rsStreamRead.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileRmdir.c
gcov -o ../../server/api/obj ../../server/api/src/rsGeneralRowInsert.c
gcov -o ../../server/api/obj ../../server/api/src/rsRuleExecSubmit.c
gcov -o ../../server/api/obj ../../server/api/src/rsGetTempPassword.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileUnlink.c
gcov -o ../../server/api/obj ../../server/api/src/rsRegColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsModColl.c
gcov -o ../../server/api/obj ../../server/api/src/rsStructFileBundle.c
gcov -o ../../server/api/obj ../../server/api/src/rsL3FilePutSingleBuf.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjRename.c
gcov -o ../../server/api/obj ../../server/api/src/rsFilePut.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataGet.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileClosedir.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileClose.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileUnlink.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjLseek.c
gcov -o ../../server/api/obj ../../server/api/src/rsRegReplica.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileRename.c
gcov -o ../../server/api/obj ../../server/api/src/rsEndTransaction.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjWrite.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileTruncate.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjPut.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjOpenAndStat.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileReaddir.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataCopy.c
gcov -o ../../server/api/obj ../../server/api/src/rsUserAdmin.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjRead.c
gcov -o ../../server/api/obj ../../server/api/src/rsRuleExecDel.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileFsync.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileRename.c
gcov -o ../../server/api/obj ../../server/api/src/rsStructFileExtAndReg.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjCopy.c
gcov -o ../../server/api/obj ../../server/api/src/rsUnbunAndRegPhyBunfile.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileOpendir.c
gcov -o ../../server/api/obj ../../server/api/src/rsGeneralRowPurge.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileRmdir.c
gcov -o ../../server/api/obj ../../server/api/src/rsPhyPathReg.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileOpen.c
gcov -o ../../server/api/obj ../../server/api/src/rsDataObjChksum.c
gcov -o ../../server/api/obj ../../server/api/src/rsSubStructFileLseek.c
gcov -o ../../server/api/obj ../../server/api/src/rsFileSyncToArch.c
# gcov -o ../../server/re/obj ../../server/re/src/reLib1.c
#  gcov -o ../../server/re/obj ../../server/re/src/reHelpers2.c
 gcov -o ../../server/re/obj ../../server/re/src/parser.c
#  gcov -o ../../server/re/obj ../../server/re/src/ChkDataObjAttr2.c
 gcov -o ../../server/re/obj ../../server/re/src/nre.reLib1.c
 gcov -o ../../server/re/obj ../../server/re/src/nre.reHelpers1.c
 gcov -o ../../server/re/obj ../../server/re/src/nre.reLib2.c
 gcov -o ../../server/re/obj ../../server/re/src/genQueryMS.c
 gcov -o ../../server/re/obj ../../server/re/src/reDBO.c
# gcov -o ../../server/re/obj ../../server/re/src/systemMS.c
 gcov -o ../../server/re/obj ../../server/re/src/datetime.c
# gcov -o ../../server/re/obj ../../server/re/src/reHelpers1.c
 gcov -o ../../server/re/obj ../../server/re/src/ruleAdminMS.c
 gcov -o ../../server/re/obj ../../server/re/src/printMS.c
 gcov -o ../../server/re/obj ../../server/re/src/sharedmemory.c
 gcov -o ../../server/re/obj ../../server/re/src/hashtable.c
 gcov -o ../../server/re/obj ../../server/re/src/icatGeneralMS.c
 gcov -o ../../server/re/obj ../../server/re/src/miscMS.c
# gcov -o ../../server/re/obj ../../server/re/src/reLib2.c
 gcov -o ../../server/re/obj ../../server/re/src/arithmetics.c
 gcov -o ../../server/re/obj ../../server/re/src/mailMS.c
 gcov -o ../../server/re/obj ../../server/re/src/cache.c
 gcov -o ../../server/re/obj ../../server/re/src/functions.c
 gcov -o ../../server/re/obj ../../server/re/src/reDataObjOpr.c
 gcov -o ../../server/re/obj ../../server/re/src/icatAdminMS.c
 gcov -o ../../server/re/obj ../../server/re/src/reSysDataObjOpr.c
 gcov -o ../../server/re/obj ../../server/re/src/typing.c
 gcov -o ../../server/re/obj ../../server/re/src/xmsgMS.c
 gcov -o ../../server/re/obj ../../server/re/src/nre.reHelpers2.c
 gcov -o ../../server/re/obj ../../server/re/src/reIn2p3SysRule.c
 gcov -o ../../server/re/obj ../../server/re/src/filesystem.c
 gcov -o ../../server/re/obj ../../server/re/src/extractAvuMS.c
 gcov -o ../../server/re/obj ../../server/re/src/reRDA.c
 gcov -o ../../server/re/obj ../../server/re/src/testMS.c
 gcov -o ../../server/re/obj ../../server/re/src/configuration.c
 gcov -o ../../server/re/obj ../../server/re/src/regExpMatch.c
 gcov -o ../../server/re/obj ../../server/re/src/rules.c
 gcov -o ../../server/re/obj ../../server/re/src/restructs.c
 gcov -o ../../server/re/obj ../../server/re/src/reNaraMetaData.c
 gcov -o ../../server/re/obj ../../server/re/src/sysBackupMS.c
 gcov -o ../../server/re/obj ../../server/re/src/reVariableMap.c
# gcov -o ../../server/re/obj ../../server/re/src/ruleAdmin.c
 gcov -o ../../server/re/obj ../../server/re/src/keyValPairMS.c
 gcov -o ../../server/re/obj ../../server/re/src/nre.systemMS.c
 gcov -o ../../server/re/obj ../../server/re/src/index.c
 gcov -o ../../server/re/obj ../../server/re/src/locks.c
 gcov -o ../../server/re/obj ../../server/re/src/reStruct.c
 gcov -o ../../server/re/obj ../../server/re/src/msiHelper.c
 gcov -o ../../server/re/obj ../../server/re/src/utils.c
 gcov -o ../../server/re/obj ../../server/re/src/reAutoReplicateService.c
 gcov -o ../../server/re/obj ../../server/re/src/region.c
 gcov -o ../../server/re/obj ../../server/re/src/conversion.c
# gcov -o ../../server/re/obj ../../server/re/src/juststubsMS.c
# gcov -o ../../server/re/obj ../../server/re/src/nre.genQueryMS.c
# ../../server/test/src/test_genu.c
# ../../server/test/src/test_chl.c
# ../../server/test/src/test_genq.c
# ../../server/test/src/test_cll.c
# ../../server/test/src/test_rda.c
# ../../server/test/src/reTest.c
# ../../server/test/s3/irodss3/src/puts3.c
# ../../server/test/s3/irodss3/src/gets3.c
