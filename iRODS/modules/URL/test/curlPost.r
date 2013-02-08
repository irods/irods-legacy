curlPost {
	msiAddKeyVal(*postFields, "data", *data);
	msiCurlPost(*url, *postFields, *response);
	writeLine("stdout", "server response: "++*response);
}
INPUT *url="http://requestb.in/10ul3my1",*data="blah"
OUTPUT ruleExecOut