<?php
  require_once("/Path/to/Prods/src/Prods.inc.php");
  
  // make an iRODS account object for connection, assuming:
  // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
  $account = new RODSAccount("srbbrick15.sdsc.edu", 1247, "demouser", "demopass");
  
  //create an dir object, assuming the path is "/tempZone/home/demouser"
  $home=new ProdsDir($account,"/tempZone/home/demouser");
  
  //search under home directory, recursively, by specified metadata:
  //  any file with metadata "myname" has a value of "myvalue"
  $meta=new RODSMeta("myname","myvalue",null,null,"=");  
  $files=$mydir=$home->findFiles(
    array( 
      'descendantOnly'=>true, //only search under this dir
      'recursive'=>true,      //search through all child dir as well
      'metadata'=> array($meta)
    )
  );
  
  // print the found files
  foreach($files as $file)
  {
    echo "Found file: ".$file->getPath()."\n";
  }
?>