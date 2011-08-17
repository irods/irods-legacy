setQuota {
	# 500GB quota for a user
	msiSetQuota(*type, *name, *resource, *value);
}
INPUT *type="user",*name="joe",*resource="mydrive",*value="536870912000"
OUTPUT ruleExecOut