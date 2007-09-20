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
  $response=array('success'=> false,'log'=>'Expected RODS URI not found');
  echo json_encode($response);
  exit(0);
} 

$start= (isset($_REQUEST['start']))?$_REQUEST['start']:0;
$limit= (isset($_REQUEST['limit']))?$_REQUEST['limit']:500;
$sort= (isset($_REQUEST['sort']))?$_REQUEST['sort']:'name';
$dir= (isset($_REQUEST['dir']))?$_REQUEST['dir']:'ASC';
if ($dir=='ASC') $sort_order=true;
else $sort_order=false;  

$parent=ProdsDir::fromURI($ruri, false);
if (empty($parent->account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($parent->account);
  if (empty($acct))
  {
    $response=array('success'=> false,'log'=>'Authentication Required');
    echo json_encode($response);
    exit(0);
  }
  $parent->account=$acct;
}
 
  
$options=array();
if ( (isset($_REQUEST['name'])) && (strlen(trim($_REQUEST['name']))>0) )
  $options['name']=$_REQUEST['name'];
if ( isset($_REQUEST['descendantOnly']) && ($_REQUEST['descendantOnly']=='true') ) 
  $options['descendantOnly']=true;
else
  $options['descendantOnly']=false;
if ( isset($_REQUEST['recursive']) && ($_REQUEST['recursive']=='true') ) 
  $options['recursive']=true;
else
  $options['recursive']=false;

if ( isset($_REQUEST['smtime']) ) 
  $options['smtime']=(int)($_REQUEST['smtime']);
if ( isset($_REQUEST['emtime']) ) 
  $options['emtime']=(int)($_REQUEST['emtime']);
if ( (isset($_REQUEST['owner'])) && (strlen($_REQUEST['owner'])>0) )
  $options['owner']=($_REQUEST['owner']);
if ( (isset($_REQUEST['rescname'])) && (strlen($_REQUEST['rescname'])>0) )
  $options['rescname']=(int)($_REQUEST['rescname']);
if ( (isset($_REQUEST['metadata'])) && (strlen($_REQUEST['metadata'])>0) )
{
  $metadata_arr=json_decode(urldecode($_REQUEST['metadata']),true);
  $options['metadata']=array();
  foreach($metadata_arr as $meta)
  {
    $options['metadata'][]=new RODSMeta(
      $meta['name'],$meta['val'],NULL,NULL,$meta['op']);
  }
} 

if (count($options)<=0)
{
  $response=array('success'=> false,'log'=>'Search option not specified');
  echo json_encode($response);
  exit(0);
}

try {
  $total_count=0;
  $found_files=$parent->findFiles($options, $total_count,(int) $start,(int) $limit,array($sort => $sort_order));
  
  $arr=array();
  $arr['totalCount']=(int) $total_count;
  $arr['que_results']=array();
  $arr['success']=true;
  
  foreach($found_files as $file)
  {
    $filestats=array();
    $filestats['id']=$file->stats->id;
    $filestats['name']=$file->stats->name;
    $filestats['size']=$file->stats->size;
    $filestats['fmtsize']=format_size($file->stats->size);
    $filestats['mtime']=$file->stats->mtime;
    $filestats['ctime']=$file->stats->ctime;
    $filestats['owner']=$file->stats->owner;
    $filestats['rescname']=$file->stats->rescname;
    $filestats['typename']=$file->stats->typename;
    $filestats['type']=1;
    $filestats['dirname']=$file->getParentPath();
    $filestats['ruri']=$file->toURI();
    $arr['que_results'][]=$filestats;
  }
    
  $str= json_encode($arr);
  echo "($str)";
  
} catch (Exception $e) {
  $response=array('success'=> false,'errmsg'=> "$e", 'errcode' => $e->getCode());
  echo json_encode($response);
}

function format_size($rawSize) 
{
  if ($rawSize / 1099511627776 > 1) 
    return round($rawSize/1099511627776, 2) . ' TB';
  else if ($rawSize / 1073741824 > 1) 
    return round($rawSize/1073741824, 2) . ' GB';  
  else 
  if ($rawSize / 1048576 > 1) 
    return round($rawSize/1048576, 2) . ' MB'; 
  else if ($rawSize / 1024 > 1) 
    return round($rawSize/1024, 2) . ' KB'; 
  else 
    return round($rawSize, 1) . ' B ';
}    
?>