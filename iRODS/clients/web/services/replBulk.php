<?php
require_once("../config.inc.php");
session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

$ruri="";
if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
else
{
  $response=array('success'=> false,'errmsg'=>'Expected RURI not found');
  echo json_encode($response);
  exit(0);
} 

$resc="";
if (isset($_REQUEST['resc']))
  $resc=$_REQUEST['resc'];
else
{
  $response=array('success'=> false,'errmsg'=>'Expected resource name not found');
  echo json_encode($response);
  exit(0);
} 

$files=array();
$dirs=array();
if (isset($_REQUEST['files']))
  $files=$_REQUEST['files'];
if (isset($_REQUEST['dirs']))
  $dirs=$_REQUEST['dirs'];

if ( (empty($files))&&(empty($dirs)) )
{
  $response=array('success'=> false,'errmsg'=>'No files or collections specified');
  echo json_encode($response);
  exit(0);
}  

try {
  $account=RODSAccount::fromURI($ruri);
    
    $acct=$_SESSION['acct_manager']->findAcct($account);
    if (empty($acct))
    {
      if (empty($account->pass))
      {
        $response=array('success'=> false,'errmsg'=>'Authentication Required');
        echo json_encode($response);
        exit(0);
      }
      else
      {
        $_SESSION['acct_manager']->add($account);
      }
    }
    $account=$acct;
  
  $mservices=array();
  $input_params=array("*desc_resc" => $resc);
  
  $num_files=0;
  foreach ($files as $fileuri)
  {
    if (strlen($fileuri)>0)
    {
      $myfile=ProdsFile::fromURI($fileuri);
      $mservices[]=
        "msiDataObjRepl(*desc_file$num_files,*desc_resc,*outbuf)";
      $input_params["*desc_file$num_files"]=$myfile->getPath();
      $num_files++;
    }
  }
  
  $num_dirs=0;
  foreach ($dirs as $diruri)
  {
    if (strlen($diruri)>0)
    {
      $mydir=ProdsDir::fromURI($diruri);
      $mservices[]=
        "msiReplColl(*desc_dir$num_dirs,*desc_resc,backupMode,*outbuf)";
      $input_params["*desc_dir$num_dirs"]=$mydir->getPath();
      $num_dirs++;
    }
  }
  
  $rule_body='myTestRule||'.
    'delayExec(<PLUSET>0m</PLUSET>,'.
    implode("##", $mservices).
    ',nop)|nop';
  
  /* --debug code here in case when delayed exec not working */
  //$rule_body='myTestRule||'.implode("##", $mservices).'|nop';
  
    
  $rule=new ProdsRule($account,$rule_body,$input_params);
  $results=$rule->execute();   
  
  $response=array('success'=> true,'log'=>"$num_files files and $num_dirs scheduled for replication!");
  echo json_encode($response);
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> $e->getMessage(), 'errcode'=> $e->getCode());
  echo json_encode($response);
}

  
?>