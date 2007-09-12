<?php
  define("PRODS_INC_PATH", dirname(__FILE__)."/../prods/src/Prods.inc.php");
  
  /*
  define("DEFAULT_RODS_HOST","rt.sdsc.edu");
  define("DEFAULT_RODS_PORT",1247);
  define("DEFAULT_RODS_USER","rods");
  define("DEFAULT_RODS_PASS","RODS");
  */
  require_once(PRODS_INC_PATH);
  require_once("RODSAcctManager.class.php");
?>