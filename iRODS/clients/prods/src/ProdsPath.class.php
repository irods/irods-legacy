<?php

/**
 * PRODS class. Provides high level PRODS functionalities.
 * @author Sifang Lu <sifang@sdsc.edu>
 * @copyright Copyright &copy; 2007, TBD
 * @package Prods
 */

require_once("autoload.inc.php");

require_once(CLASS_DIR."/ProdsConfig.inc.php");

abstract class ProdsPath
{
  /**
	* string path 
	* @var string
	*/
	public $path_str;
	
  public $account;
  
  protected $usecache;
  protected $path_exists;
  
  protected $parent_path;
  protected $name;
  
  public function __construct(RODSAccount $account, $path_str)
  {
    $this->account=$account;
    
    // strip the tailing "/"
    while( ($path_str{strlen($path_str)-1}=='/')&&(strlen($path_str)>1) )
    {
      $path_str=substr($path_str,0,strlen($path_str)-1);  
    }
    $this->path_str=$path_str;
    
    if (PRODSPATH_USE_CACHE===false)
      $this->usecache=false;
    else
      $this->usecache=true;  
      
    $this->result_cached=false;
    
    $this->parent_path=dirname($this->path_str);
    $this->name=basename($this->path_str);
  }
  
  public function __toString()
  {
    return $this->account.$this->path_str;
  }
  
 /**
	* Whether a path exists on the server. 
	*/
  public function exists()
  {
    if ( isset($this->path_exists) )
      return $this->path_exists;
    else
    {
      $this->verify();
      return $this->path_exists;
    }
  } 
  
 /**
	* Verify if a path exist with server.
	*/
  abstract public function verify();
  
 /**
	* Get meta data of this path (file or dir).
	* @return array of RODSMeta. 
	*/
  public function getMeta()
  {
    if ($this instanceof ProdsFile)
      $type='d';
    else
    if ($this instanceof ProdsDir)  
      $type='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");
    $conn = RODSConnManager::getConn($this->account);
    $meta_array= $conn -> getMeta ($type,$this->path_str);
    RODSConnManager::releaseConn($conn);
    return $meta_array;
  }
    
 /**
  * update metadata to this path (file or dir)
  */
  public function updateMeta(RODSMeta $meta_old, RODSMeta $meta_new)
  {
    try {
      $this->rmMeta($meta_old);
    } catch (RODSException $e) {
      trigger_error("updateMeta(): Failed to deleted old metadata ($meta_old->name, $meta_old->value) with exception: $e",
            E_USER_WARNING);
    }
    $this->addMeta($meta_new);
  }
  
 /**
  * Add metadata to this path (file or dir)
  */
  public function addMeta(RODSMeta $meta)
  {
    if ($this instanceof ProdsFile)
      $type='d';
    else
    if ($this instanceof ProdsDir)  
      $type='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");
    
    $conn = RODSConnManager::getConn($this->account);
    $conn -> addMeta ($type,$this->path_str,$meta);
    RODSConnManager::releaseConn($conn);  
  }
  
 /**
  * remove metadata to this path (file or dir)
  */
  public function rmMeta(RODSMeta $meta)
  {
    if ($this instanceof ProdsFile)
      $type='d';
    else
    if ($this instanceof ProdsDir)  
      $type='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");
    
    $conn = RODSConnManager::getConn($this->account);
    $conn -> rmMeta ($type,$this->path_str,$meta);
    RODSConnManager::releaseConn($conn);  
  }
  
  /**
  * remove metadata to this path (file or dir)
  */
  public function rmMetaByID ($metaid)
  {
    if ($this instanceof ProdsFile)
      $type='d';
    else
    if ($this instanceof ProdsDir)  
      $type='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");
    
    $conn = RODSConnManager::getConn($this->account);
    $conn -> rmMetaByID ($type,$this->path_str,$metaid);
    RODSConnManager::releaseConn($conn);  
  }
  
 /**
  * copy meta data between two path (file or dir)
  */
  public function cpMeta(ProdsPath $dest)
  {
    if ($this instanceof ProdsFile)
      $type_src='d';
    else
    if ($this instanceof ProdsDir)  
      $type_src='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");
        
    if ($dest instanceof ProdsFile)
      $type_dest='d';
    else
    if ($dest instanceof ProdsDir)  
      $type_dest='c';
    else
       throw new RODSException("Unsupported data type:".get_class($this),
        "PERR_INTERNAL_ERR");    
    
    $conn = RODSConnManager::getConn($this->account);
    $conn -> cpMeta ($type_src,$type_dest,$this->path_str,$dest->path_str);
    RODSConnManager::releaseConn($conn);  
  }
  
 /**
  * query metadata, and find matching files.
  * @param RODSAccount $account accout of RODS server
  * @param array $meta_array array of RODSMeta, each represent one meta query. For instance one query could be ("foo","bar",">"). It means check any files with metadata name "foo", with value greater than "bar".
  * @param integer $ret_type. If 0, return matching files; if 1, return matching directories
  * @param string $parent_dir_str. If this is set, it would only search the files directly under this directory.
  * @return array of prodsFile.
  */
  public static function queryMeta(RODSAccount $account,
    array $meta_array, $ret_type=0, $parent_dir_str=NULL)
  {
    if ($ret_type==0)
      $select=array("COL_COLL_NAME","COL_DATA_NAME");
    else
      $select=array("COL_COLL_NAME");
      
    $condition=array();
    foreach($meta_array as $meta)
    {
      $condition[]=new RODSQueryCondition
        ("COL_META_DATA_ATTR_NAME",$meta->name);
      $condition[]=new RODSQueryCondition
        ("COL_META_DATA_ATTR_VALUE",$meta->value,$meta->op);
    } 
    if (!empty($parent_dir_str))
    {
      $condition[]=new RODSQueryCondition("COL_COLL_NAME",parent_dir_str);
    }
    
    $conn = RODSConnManager::getConn($account);
    $genque_result= $conn -> genQuery ($select,$condition);
    RODSConnManager::releaseConn($conn);    
    
    if ($genque_result===false)
    {
      return array();
    }
    
    $ret_val=array();
    for($i=0;$i < count($genque_result["COL_COLL_NAME"]);$i++)
    {
      if ($ret_type==0)
        $ret_val[]=new ProdsFile($account,
          $genque_result["COL_COLL_NAME"][$i]."/".
          $genque_result["COL_DATA_NAME"][$i]);
      else
        $ret_val[]=new ProdsDir($account,
          $genque_result["COL_COLL_NAME"][$i]);
    }
    return $ret_val;
  }
  
 /**
  * rename this path (file of dir)
  * @param string $new_path_str new path string to be renamed to.
  */
  public function rename($new_path_str)
  {
    if ($this instanceof ProdsFile)
      $type=0;
    else
      $type=1;
    $conn = RODSConnManager::getConn($this->account);
    $conn->rename($this->path_str, $new_path_str,$type);
    RODSConnManager::releaseConn($conn); 
    $this->path_str=$new_path_str;
    $this->parent_path=dirname($this->path_str);
    $this->name=basename($this->path_str);
  } 
  
 /**
  * Get name of this path. note that this is not the full path. for instance if path is "/foo/bar", the name is "bar"
  * @return string name of the path.
  */
  public function getName()
  {
    return $this->name;
  } 
  
 /**
  * Get string form of this path. note that this is the full path. 
  * @return string form of the path.
  */
  public function getPath()
  {
    return $this->path_str;
  }
  
 /**
  * Get parent's path of this path. 
  * @return string parent's path.
  */
  public function getParentPath()
  {
    return $this->parent_path;  
  }
  
  public function toURI()
  {
    return $this->account->user."@".$this->account->host.":".$this->account->port.$this->path_str;    
  }
}  
?>