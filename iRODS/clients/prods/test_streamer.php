<?php
  require_once("src/ProdsStreamer.class.php");
  $str=file_get_contents("rods://rods.tempZone:RODS@rt.sdsc.edu:1247/tempZone/home/rods/test.sifang2.txt");
  var_dump ($str);
  
  $children = scandir("rods://rods.tempZone:RODS@rt.sdsc.edu:1247/tempZone/home/rods");
  var_dump ($children);
?>
