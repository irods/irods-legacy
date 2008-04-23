<?php
  require_once("/Path/to/Prods/src/Prods.inc.php");
  
  // make an iRODS account object for connection, assuming:
  // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
  $account = new RODSAccount("srbbrick15.sdsc.edu", 1247, "demouser", "demopass");
  
  //create an file object for read, assuming the path is "/tempZone/home/demouser/test_read.txt"
  $myfile=new ProdsFile($account,"/tempZone/home/demouser/test_read.txt");
  
  //create an metadata entry and associate it with the file
  $meta=new RODSMeta("myname","myvalue");
  $myfile->addMeta($meta);
  
  //get all metadata of the file, and print them
  //the output should look like "Name: Myname | Value: myvalue"
  $metadatas=$myfile->getMeta();
  foreach($metadatas as $meta)
    echo 'Name: '.$meta->name." | ".$meta->value."\n";
 ?>