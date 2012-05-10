# This rule purge a cache resource *CacheRescName if the total size of this resource is 
# greater than *MaxSpAlwdTBs terabytes. The purge will be stopped once the requirement is met.
# The purge will occur on the collection *Collection and all its subcollections.
# The oldest copies in the cache will be cleared first.
# In the current setting, a single copy of the files will be kept. Hence, if a single copy
# exists only in *CacheRescName and not elsewhere, it won't be removed.
#
# Written by Jean-Yves Nief of CCIN2P3 and copyright assigned to Data Intensive Cyberinfrastructure Foundation
#
# usage example: irule -F rulepurgeDiskCache.r "*Collection='/tempZone/foo'" "*CacheRescName='demoResc'" *MaxSpAlwdTBs=5
#
purgeDiskCache {
	delay("<PLUSET>30s</PLUSET><EF>24h</EF>") {
		msiMakeGenQuery("sum(DATA_SIZE)", "RESC_NAME = '*CacheRescName'", *Query);
		msiExecGenQuery(*Query, *QueryOut);
		foreach (*QueryOut) {
			msiGetValByKey(*QueryOut, "DATA_SIZE", *TotalSize);
		}
		*usedSpace = double(*TotalSize);
		*MaxSpAlwd = *MaxSpAlwdTBs * 1024^4;
		if ( *usedSpace > *MaxSpAlwd ) then {
			*ContInxOld = 1;
			msiGetIcatTime(*Time,"unix");
			*Condition = "DATA_RESC_NAME = '*CacheRescName' AND COLL_NAME like '*Collection%'";
			msiMakeGenQuery("DATA_NAME, COLL_NAME, DATA_SIZE, order(DATA_CREATE_TIME)",*Condition,*Query2);
			msiExecGenQuery(*Query2,*List);
			msiGetContInxFromGenQueryOut(*List,*ContInxNew);
			# loop on the output list: 256 items at a time
			while ( *ContInxOld > 0 ) {
				foreach ( *List ) {
					msiGetValByKey(*List,"DATA_NAME",*D);
					msiGetValByKey(*List,"COLL_NAME",*C);
					msiGetValByKey(*List,"DATA_SIZE",*S);
					*usedSpace = *usedSpace - double(*S);
					if ( *usedSpace < *MaxSpAlwd ) {
						break;
					}
					msiDataObjTrim(*C/*D,"*CacheRescName","null","1","1",*status);
					writeLine("stdout","*C/*D on *CacheRescName has been purged");
				}
				if ( *usedSpace < *MaxSpAlwd ) { 
					break;
				}
				*ContInxOld = *ContInxNew;
				if( *ContInxOld > 0 ) { 
					msiGetMoreRows(*Query2,*List,*ContInxNew); 
				}
			}
		}
	}
}

input *MaxSpAlwdTBs = $1, *Collection = "/tempZone", *CacheRescName = "demoResc"
output ruleExecOut
