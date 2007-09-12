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
  }
  $resources_json="";  
  
  try {
  $conn= new RODSConn($collection->account);
  $conn->connect();
  $resources=$conn->getResources();
  
  $resources_arr_no_keys=array();
  foreach($resources as $resource)
  {
    $resources_arr_no_keys[]=array_values($resource);
  }
  
  $resources_with_status=array("success" => true, "total_count" => count($resources),
    "que_results" => $resources_arr_no_keys);
  $resources_json=json_encode($resources_with_status);
  $conn->disconnect();
  } catch (Exception $e) {
    $response=array('success'=> false,'error'=> $e->getMessage());
    $resources_json=json_encode($response);
  }
  
?>

<html>
<head>
  <title>Upload to <?php echo $ruri; ?> </title>
  <link rel="stylesheet" type="text/css" href="extjs/resources/css/ext-all.css" /> 
  <link rel="stylesheet" type="text/css" href="extjs/resources/css/ytheme-aero.css" />
  <link rel="stylesheet" type="text/css" href="extjs/examples/form/forms.css" />
  <link rel="stylesheet" type="text/css" href="uploadDialog.css" />
  
  <script type="text/javascript" src="extjs/adapter/yui/yui-utilities.js"></script>     
  <script type="text/javascript" src="extjs/adapter/yui/ext-yui-adapter.js"></script>
  <script type="text/javascript" src="extjs/ext-all-debug.js"></script>
  <script type="text/javascript" src="extjs/package/tabs/tabs.js"></script>
  <script type="text/javascript">
    var resource_arr= <?php echo $resources_json; ?>;
    var ruri= '<?php echo $ruri; ?>';
  </script> 
  <script type="text/javascript" src="uploadDialog.js"></script>
</head>

<body scroll="no" id="mainBody">
  
  <script type="text/javascript">
    Ext.EventManager.onDocumentReady(UploadForm.init, UploadForm, true);
  </script>  
  
  <div id="upload_dest_ruri" style="display:none;"><?php echo $ruri; ?></div>
  
  <div id="upload_form_tabs" class="xp"> </div>
  
  <!--
  <div style="width:580px;" > 
      <div class="x-box-tl"><div class="x-box-tr"><div class="x-box-tc"></div></div></div>
      <div class="x-box-ml"><div class="x-box-mr"><div class="x-box-mc"">
          <div id="form-upload-err" style="color:red">&nbsp;</div>
          <div id="form-upload">
            
          </div>
      </div></div></div>
      <div class="x-box-bl"><div class="x-box-br"><div class="x-box-bc"></div></div></div>
  </div>
  --> 
</body>

</html>   