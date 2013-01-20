curlGetObj { 
  msiCurlGetObj(*url, *destObj, *written); 
  writeLine("stdout", str(*written)++" bytes written");
}
INPUT *url="http://www.textfiles.com/art/ferrari.art",*destObj="/pho27/home/rods/ferrari.art"
OUTPUT ruleExecOut