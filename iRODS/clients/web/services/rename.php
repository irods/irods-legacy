<?php
require_once("../config.inc.php");
session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

if (!isset($_REQUEST['batch']))
{
  $response=array('success'=> false,'errmsg'=>"Expected rename batch not found");
  echo json_encode($response);
  exit(0);
}
$rename_batch=json_decode(urldecode($_REQUEST['batch']),true);
if (!is_array($rename_batch))
{
  $response=array('success'=> false,'errmsg'=>"Mal-formated rename batch");
  echo json_encode($response);
  exit(0);
}

$tasks_status=array();
$success=true;
foreach($rename_batch as $task)
{
  try{
     oneTask($task);
     $status=$task;
     $status['success']=true;
     $tasks_status[]=$status;   
  } catch (Exception $e) {
    $status=$task;
    $status['success']=false;
    $status['errcode']=$e->getCode();
    $status['errmsg']="$e";
    $tasks_status[]=$status;
    $success=false;
  }
}
$response=array('success'=> $success, 'batch_status' => $tasks_status);

echo json_encode($response); 
exit(0);

function oneTask ($task)
{
  if ($task['type']==0)
  {
    $this_path=ProdsDir::fromURI($task['src'], false);   
    $new_path=ProdsDir::fromURI($task['dest'], false);  
  }
  else
  {
    $this_path=ProdsFile::fromURI($task['src'], false);
    $new_path=ProdsFile::fromURI($task['dest'], false);
  }
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
  
  $this_path->rename($new_path->path_str);
}
 
?>