setQuota {
	msiSetQuota(*type, *name, *resource, *value);
}
INPUT *type="user",*name="bob",*resource="total",*value="536870912000"
OUTPUT ruleExecOut