<?php
  require_once("config.inc.php");
  session_start();
  if (!isset($_SESSION['acct_manager']))
    $_SESSION['acct_manager']= new RODSAcctManager();
  if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
  else
  {
    echo "Error: RODS URI expected but not found!";
    exit(0);
  } 
  
  $pass='';
  $collection=ProdsDir::fromURI($ruri, false);
  if (empty($collection->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($collection->account);
    if (empty($acct))
    {
      echo "Error: RODS URI expected but not found!";
      exit(0);
    }
    $collection->account=$acct;
    $pass=$collection->account->pass;
  }
  else
    $pass=$collection->account->pass;
  
?>  
<html>
<head><title>Upload Applet</title>

<script>
document.cookie = 'password=rods,username=rods,host=client64-100.sdsc.edu,port=1247; expires=Thu, 2 Aug 2008 20:47:11 UTC; path=/';
</script>
</head>
<body>
 



<APPLET CODE="edu.sdsc.grid.gui.applet.UploadApplet.class" archive="applets/UploadApplet.jar" WIDTH="650" HEIGHT="200">
   <param name="ruri" value="http://<?php echo $ruri; ?>" />
   <param name="pass" value="<?php echo $pass; ?>" />
</APPLET>


</body>
</html>
