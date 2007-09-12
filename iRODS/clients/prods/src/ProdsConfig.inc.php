<?php
  
/**#@+
 * Constants
 */

/**
 * default RODS HOST name or ipaddr. This value is used, when server is not specified
 */
define("DEFAULT_RODS_HOST","localhost");

/**
 * default RODS port number. This value is used, when it is not specified
 */
define("DEFAULT_RODS_PORT",1247);

/**
 * default RODS port number. This value is used, when it is not specified
 */
define("DEFAULT_RODS_USER","public");

/**
 * default RODS port password. This value is used, when it is not specified
 */
define("DEFAULT_RODS_PASS","cando");

/**
 * default RODS zone name. This value is used, when it is not specified
 */
define("DEFAULT_RODS_ZONE","");

/**
 * Whether use cache for ProdsPath, and it's child classes. If cache is used,
 * the query results will be cached, and used for further queries. This option is
 * recommended to remain true for better performance. However, you may want to
 * if you experience an cache issue;
 */
define("PRODSPATH_USE_CACHE",true);

define("RODS_REL_VERSION",'rods0.5');
define("RODS_API_VERSION",'a');

/**#@-*/
  
  
?>