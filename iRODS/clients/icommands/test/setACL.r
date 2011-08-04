setACL {
	msiSetACL("default", "admin:read", "rods", *path)
}
INPUT *path="/pho27/home/bob/doc/driver.txt"
OUTPUT ruleExecOut