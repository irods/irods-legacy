curlGetStr { 
  msiCurlGetStr(*url, *outStr, *written); 
  writeLine("stdout", *outStr);
  writeLine("stdout", "--------------------------");
  writeLine("stdout", str(*written)++" bytes written");
}
INPUT *url="http://www.textfiles.com/art/dragon.txt"
OUTPUT ruleExecOut