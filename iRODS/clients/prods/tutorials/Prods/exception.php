<?php
  require_once("/Path/to/Prods/src/Prods.inc.php");
  
  try {
    // make an iRODS account object for connection, assuming:
    // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
    $account = new RODSAccount("srbbrick15.sdsc.edu", 1247, "demouser", "demopass");
    
    //create an dir object, which doesn't exist, to trigger the exception handler
    $dir=new ProdsPath($account,"/i-dont-exist");
  } catch (RODSException $e) { 
    echo "--- test failed! --- <br/>\n";
    echo ($e);
    echo $e->showStackTrace();
  }
?>