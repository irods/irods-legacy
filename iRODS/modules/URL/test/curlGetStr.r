curlGetStr { 
	msiCurlGetStr(*url, *outStr); 
	writeLine("stdout", *outStr);
}
INPUT *url="http://www.textfiles.com/art/dragon.txt"
OUTPUT ruleExecOut