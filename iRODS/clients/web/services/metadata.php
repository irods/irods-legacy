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
  $response=array('success'=> false,'errmsg'=>'Expected RODS URI not found');
  echo json_encode($response);
  exit(0);
} 

$type=NULL;
if (isset($_REQUEST['type']))
  $type=$_REQUEST['type'];
if ($type==NULL)
{
  $response=array('success'=> false,'errmsg'=>'Expected type not specified');
  echo json_encode($response);
  exit(0);
}  

$action=NULL;
$mod_batch=array();
if (isset($_REQUEST['action']))
{
  $action=$_REQUEST['action'];
  if ($action=='mod')
  {
    if (!isset($_REQUEST['batch']))
    {
      $response=array('success'=> false,'errmsg'=>"Expected Mod batch not found");
      echo json_encode($response);
      exit(0);
    }
    $mod_batch=json_decode(urldecode($_REQUEST['batch']),true);
    if (!is_array($mod_batch))
    {
      $response=array('success'=> false,'errmsg'=>"Mal-formated Mod batch");
      echo json_encode($response);
      exit(0);
    }
  }
} 
if ($action==NULL)
{
  $response=array('success'=> false,'errmsg'=>'Expected type not specified');
  echo json_encode($response);
  exit(0);
}  

try {
  
  if ($type==0)
    $this_path=ProdsDir::fromURI($ruri, false);
  else
    $this_path=ProdsFile::fromURI($ruri, false);  
  if (empty($parent->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($this_path->account);
    if (empty($acct))
    {
      $response=array('success'=> false,'errmsg'=>'Authentication Required');
      echo json_encode($response);
      exit(0);
    }
    $this_path->account=$acct;
  }
  if (empty($this_path->account->zone))
  {
    $this_path->account->getUserInfo();
  }  
  
  switch ($action)
  {
    case "get":
      $metadata_objs=$this_path->getMeta();
      $metadata_array=array();
      foreach($metadata_objs as $metadata_obj)
      {
        $metadata_array_entry=array();
        $metadata_array_entry['id']=$metadata_obj->id;
        $metadata_array_entry['name']=$metadata_obj->name;
        $metadata_array_entry['value']=$metadata_obj->value;
        $metadata_array_entry['unit']=$metadata_obj->units;
        $metadata_array_entry['isnew']=false;
        $metadata_array[]=$metadata_array_entry;
      }
      $response=array('success'=> true, 'totalCount'=>count($metadata_array),
        'que_results'=>$metadata_array);
      break;
    case "mod":
      modMetadata($mod_batch, $this_path);
      $response=array('success'=> true, 'msg'=>"all done"); 
      break;
    default:
      $response=array('success'=> false, 'errmsg'=>"action type '$action' not supported"); 
  }
  echo json_encode($response);
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> "$e", 'errcode'=> $e->getCode());
  echo json_encode($response);
}

function modMetadata($mod_batch, ProdsPath $target_path)
{
  foreach($mod_batch as $mod_instrction)
  {
    $op_type=$mod_instrction['op'];
    switch($op_type)
    {
      case 'add':
        $target_path->addMeta(new RODSMeta(
          $mod_instrction['target']['name'],
          $mod_instrction['target']['value'],
          $mod_instrction['target']['unit']
        ));
        break;
      case 'delbyid':
        $target_path->rmMetaByID($mod_instrction['id']);
        break;
      case 'updatebyid':
        $target_path->rmMetaByID($mod_instrction['target']['id']);
        $target_path->addMeta( new RODSMeta(
            $mod_instrction['target']['name'],
            $mod_instrction['target']['value'],
            $mod_instrction['target']['unit'])
        );
        break;
      default:
        $response=array('success'=> false,'errmsg'=>"op type '$op_type' no supported");
        echo json_encode($response);
        exit(0);  
    }
  }    
}
  
?>