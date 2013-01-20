curlPost { 
	msiCurlPost(*url, *data, *status); 
	writeLine("stdout", *status);
}
INPUT *url="http://requestb.in/mldh1fmm",*data="blah"
OUTPUT ruleExecOut