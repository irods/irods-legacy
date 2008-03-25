<?php
  define("PRODS_INC_PATH", dirname(__FILE__)."/../prods/src/Prods.inc.php");
  
  // automatically extract exif headers from tiff and jpeg.
  // note that this option requirs PHP EXIF package to be configured at install time:
  // http://us2.php.net/exif
  define("AUTO_EXTRACT_EXIF", false);
  
  /*
  define("DEFAULT_RODS_HOST","rt.sdsc.edu");
  define("DEFAULT_RODS_PORT",1247);
  define("DEFAULT_RODS_USER","rods");
  define("DEFAULT_RODS_PASS","RODS");
  */
  require_once(PRODS_INC_PATH);
  require_once("RODSAcctManager.class.php");
?>