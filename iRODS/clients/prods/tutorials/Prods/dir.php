<?php
  require_once("/Path/to/Prods/src/Prods.inc.php");
  
  // make an iRODS account object for connection, assuming:
  // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
  $account = new RODSAccount("srbbrick15.sdsc.edu", 1247, "demouser", "demopass");
  
  //create an dir object, assuming the path is "/tempZone/home/demouser"
  $home=new ProdsDir($account,"/tempZone/home/demouser");
  
  //make a new child directory called "mydir" under home
  $mydir=$home->mkdir("mydir");
  
  //list home directory
  $children=$home->getAllChildren();
  //print each child's name
  echo "children are: \n";
  foreach($children as $child)
    echo $child->getName()."\n";
?>