<?php
  // include the streamer class, which enable PHP core to recongnize "irods"
  // as a valid stream, just like file stream, socket stream, or HTTP stream.
  require_once("/Path/to/Prods/src/ProdsStreamer.class.php");
  
  // read and print the file "/tempZone/home/demouser/test.txt". assuming:
  // username: demouser, password: demopass, server: srbbrick15.sdsc.edu, port: 1247
  $str=file_get_contents("irods://demouser:demopass@srbbrick15.sdsc.edu:1247/tempZone/home/demouser/test.txt");
  var_dump ($str);
  
  // print the children of home dir "/tempZone/home/demouser"
  $children = scandir("irods://demouser:demopass@srbbrick15.sdsc.edu:1247/tempZone/home/demouser");
  var_dump ($children);
?>