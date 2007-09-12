<?php
session_start();
//session_write_close();
session_unset();
session_destroy();
$_SESSION = array();
header( 'Location: index.php' ) ;
?>