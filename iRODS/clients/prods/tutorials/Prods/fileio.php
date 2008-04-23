<?php
  require_once("/Path/to/Prods/src/Prods.inc.php");
  
  // make an iRODS account object for connection, assuming:
  // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
  $account = new RODSAccount("srbbrick15.sdsc.edu", 1247, "demouser", "demopass");
  
  //create an file object for read, assuming the path is "/tempZone/home/demouser/test_read.txt"
  $myfile=new ProdsFile($account,"/tempZone/home/demouser/test_read.txt");
  
  //read and print out the file
  $myfile->open("r");
  echo "the file reads: <pre>";
  while($str=$myfile->read(4096))
    echo $str;
  echo "</pre>";
  //close the file pointer
  $myfile->close();
  
  //create an file object for write, assuming the path is "/tempZone/home/demouser/test_write.txt"
  $myfile=new ProdsFile($account,"/tempZone/home/demouser/test_write.txt");
  
  //write hello world to the file, onto "demoResc" as resource. Note that resource name is
  //required here by method open(), because iRODS needs to know which resource to write to.
  $myfile->open("w+","demoResc");
  $bytes=$myfile->write("Hello world!\n");
  //print the number of bytes writen
  echo "$bytes bytes written <br/>\n";
  $myfile->close();
?>