<?php
require_once("../config.inc.php");
require_once(PRODS_INC_PATH);

session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

$ruri=(isset($_REQUEST['node']))?$_REQUEST['node']:'/';
$ruri="rods:/".$ruri;
$collection=ProdsDir::fromURI($ruri, false);
if (empty($collection->account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($collection->account);
  if (empty($acct))
  {
    $arr=array();
    $arr['success']=false;
    $arr['errmsg']="You don't have permission for this directory! Please sign-on";
    $arr['errcode']=-99;
    $str= json_encode($arr);
    echo "($str)";
    exit (0);
  }
  $collection->account=$acct;
}  

$recursive= (isset($_REQUEST['recursive'])&&($_REQUEST['recursive']=='1'))? true:false;

try {

listChildDirJson($collection);
} catch (RODSException $e) {
    $arr=array();
    $arr['success']=false;
    $arr['errmsg']=$e->getMessage();
    $arr['errcode']=$e->getCode();
    $str= json_encode($arr);
    echo "($str)";
}


function listChildDirJson($dir)
{
  $arr=array();
    
  $childdirs=$dir->getChildDirs();
  foreach ($childdirs as $childdir)
  {
    $childstats=array();
    $childstats['id']=$childdir->getName();
    $childstats['text']=$childdir->getName();
    $childstats['leaf']=false;
    $childstats['cls']='folder';
    $arr[]=$childstats;
  }
  
    
  $str= json_encode($arr);
  echo "($str)";
}

function print_r_html($data,$return_data=false)
{
    $data = print_r($data,true);
    $data = str_replace( " ","&nbsp;", $data);
    $data = str_replace( "\r\n","<br>\r\n", $data);
    $data = str_replace( "\r","<br>\r", $data);
    $data = str_replace( "\n","<br>\n", $data);

    if (!$return_data)
        echo $data;   
    else
        return $data;
}


class ProdsDirTreeNode  
{
  public $dir;
  public $children;
  
  public function __construct(ProdsDir $dir)
  {
    $this->dir=$dir;
    $this->children=array();
  }
  
  
  public function expandChildDir($path_str)
  {
    $children_prods_dirs=$this->dir->getChildDirs();
    foreach($children_prods_dirs as  $children_prods_dir)
    {
      $this->children[]=new ProdsDirTreeNode($children_prods_dir);
    }
      
    if ($this->dir->path_str==$path_str) 
      return;
    
    foreach($this->children as $child_node)
    {
      if (0==strncmp($child_node->dir->path_str,$path_str,
        strlen($child_node->dir->path_str)))
      {
        $child_node->expandChildDir($path_str);
      }
    }
  }
  
  public function toArray()
  {
    $ret_val=array();
    $ret_val['id']=$this->dir->getName();
    $ret_val['text']=$this->dir->getName();
    $ret_val['leaf']=false;
    $ret_val['cls']='folder';
    $ret_val['childNodes']=array();
    foreach($this->children as $child)
    {
      $ret_val['childNodes'][]=$child->toArray();
    }
    return $ret_val;
  }
}
?>  