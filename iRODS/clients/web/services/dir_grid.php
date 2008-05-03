<?php
require_once("../config.inc.php");

session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();
  
$ruri=(isset($_REQUEST['ruri']))?$_REQUEST['ruri']:'/';
$collection=ProdsDir::fromURI($ruri, false);

if (empty($collection->account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($collection->account);
  if (empty($acct))
  {
    $arr=array();
    $arr['success']=false;
    
    $arr['errors']=array();
    $arr['errmsg']="Please sign-on to access this collection/file";
    $arr['errcode']=RODSException::rodsErrAbbrToCode("USER_AUTH_STRING_EMPTY");
    
    $str= json_encode($arr);
    echo "($str)";
    exit (0);
  }
  $collection->account=$acct;
}  

$start= (isset($_REQUEST['start']))?$_REQUEST['start']:0;
$limit= (isset($_REQUEST['limit']))?$_REQUEST['limit']:500;
$sort= (isset($_REQUEST['sort']))?$_REQUEST['sort']:'name';
$dir= (isset($_REQUEST['dir']))?$_REQUEST['dir']:'ASC';
if ($dir=='ASC') $dirbit=0;
else $dirbit=1;  

$orderby=array("$sort" => $dirbit);

listFileJson($collection, $start, $limit, $orderby);

function listFileJson($dir, $start=0, $limit=500, $orderby=array())
{
  $arr=array();
  $arr['totalCount']=0;
  $arr['que_results']=array();
  $arr['success']=true;
  
  try {
  
    //$dir=ProdsDir::fromURI("rods://rods.tempZone:RODS@rt.sdsc.edu:1247/tempZone/home/rods", false);
    $arr['totalCount']=0;
    
    $childdirs=$dir->getChildDirs($orderby);
    
    $arr['totalCount']=$arr['totalCount']+count($childdirs);
    foreach ($childdirs as $childdir)
    {
      $childstats=array();
      $childstats['id']=$childdir->stats->id;
      $childstats['name']=$childdir->stats->name;
      $childstats['size']=-1;
      $childstats['fmtsize']="";
      $childstats['mtime']=$childdir->stats->mtime;
      $childstats['ctime']=$childdir->stats->ctime;
      $childstats['owner']=$childdir->stats->owner;
      $childstats['type']=0;
      $childstats['ruri']=$childdir->toURI();
      $arr['que_results'][]=$childstats;
    }
    
    $totalcount=0;
    //$childfiles=$dir->getChildFiles($orderby,0, -1, $totalcount, true);
    $childfiles=$dir->getChildFiles($orderby);
    $arr['totalCount']=$arr['totalCount']+count($childfiles);
    foreach ($childfiles as $childfile)
    {
      $childstats=array();
      $childstats['id']=$childfile->stats->id.'_'.$childfile->stats->rescname;
      $childstats['name']=$childfile->stats->name;
      $childstats['size']=$childfile->stats->size;
      $childstats['fmtsize']=format_size($childfile->stats->size);
      $childstats['mtime']=$childfile->stats->mtime;
      $childstats['ctime']=$childfile->stats->ctime;
      $childstats['owner']=$childfile->stats->owner;
      $childstats['rescname']=$childfile->stats->rescname;
      $childstats['num_replica']=$childfile->stats->num_replica;
      $childstats['typename']=$childfile->stats->typename;
      $childstats['type']=1;
      $childstats['ruri']=$childfile->toURI();
      $arr['que_results'][]=$childstats;
    }
    $_SESSION['acct_manager']->updateAcct($dir->account);
    $arr['que_results']=array_slice($arr['que_results'],$start,$limit);
    $str= json_encode($arr);
    echo "($str)";
  
  } catch (RODSException $e) {
    //echo ($e);
    //echo $e->showStackTrace();
    
    $arr=array();
    $arr['success']=false;
    $arr['errmsg']=$e->getCodeAbbr().":".$e->getMessage();
    $arr['errcode']=$e->getCode();
    $str= json_encode($arr);
    echo "($str)";
    exit (0);
  }
  
  
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