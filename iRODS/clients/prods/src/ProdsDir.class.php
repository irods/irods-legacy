<?php
/**
 * PRODS directory class
 * @author Sifang Lu <sifang@sdsc.edu>
 * @copyright Copyright &copy; 2007, TBD
 * @package Prods
 */

require_once("autoload.inc.php");

class ProdsDir extends ProdsPath
{
  public  $stats;
  
  private $child_dirs;
  private $child_files;
  private $all_children;
  private $position;
  
  public function __construct(RODSAccount $account, $path_str, $verify=false, 
    RODSDirStats $stats=NULL)
  {
    $this->position=0;
    $this->stats=$stats;
    parent::__construct($account, $path_str);
    if ($verify===true)
    {
      if ($this->exists()===false)
      {
        throw new RODSException("Directory '$this' does not exist",
          'PERR_PATH_DOES_NOT_EXISTS');
      } 
    }
  }
  
 /**
	* Create a ProdsDir object from URI string.
	* @param string $path the URI Sting
	* @param boolean $verify whether verify if the path exsits
	* @return a new ProdsDir
	*/
  public static function fromURI($path, $verify=false)
  {
    if (0!=strncmp($path,"rods://",7))
      $path="rods://".$path;
    $url=parse_url($path);
    
    $host=isset($url['host'])?$url['host']:''; 
    $port=isset($url['port'])?$url['port']:'';   
    
    $user='';
    $zone='';
    if (isset($url['user']))
    {
      if (strstr($url['user'],".")!==false)
        list($user,$zone)=@explode(".",$url['user']);
      else
        $user=$url['user'];
    }  
    
    $pass=isset($url['pass'])?$url['pass']:'';
    
    $account=new RODSAccount($host, $port, $user, $pass, $zone);
    
    $path_str=isset($url['path'])?$url['path']:''; 
    if (empty($path_str))
      $path_str='/'; 
    
    return (new ProdsDir($account,$path_str,$verify));
  }  
  
 /**
	* Verify if this dir exist with server.
	*/
  public function verify()
  {
    $conn = RODSConnManager::getConn($this->account);
    $this->path_exists= $conn -> dirExists ($this->path_str);
    RODSConnManager::releaseConn($conn);  
  }
  
  
 /**
	* Resets the directory stream to the beginning of the directory.
	*/
  public function rewind()
  {
    $this->position=0;
  }
  
 /**
	* @return next file or directory from the directory. The file always come first and dir comes later. return false on failure
	*/
  public function getNextChild()
  {
    $children=$this->getAllChildren();
    if (($this->position>=count($children))||($this->position<0))
      return false;
    $names=array_keys($children);  
    $ret_val=$children[$names[$this->position]];
    $this->position++;
    return $ret_val;
  }
  
 /**
	* @return all children (files and dirs) of current dir
	*/
  public function getAllChildren()
  {
    /*
    if ( ($this->usecache===true) && (isset($this->all_children)) )
	  { 
      return $this->all_children;
    }
    */
    
    $this->all_children=array();
    $this->all_children=array_merge($this->all_children,
      $this->getChildFiles());
    $this->all_children=array_merge($this->all_children,
      $this->getChildDirs());
    return $this->all_children;  
  }
  
 /**
	* Get children directories of this dir. The result may be cached.
	* @param $orderby An associated array specifying how to sort the result by attributes. Each array key is the attribute, array val is 0 (assendent) or 1 (dessendent). The supported attributes are "name", "owner", "mtime". 
	* @return an array of ProdsDir
	*/
  public function getChildDirs(array $orderby=array(), $startingInx=0, 
    $maxresults=500, &$total_num_rows=-1)
  {
    /*
    if ( ($reset===false) && (isset($this->child_dirs)) )
	  { 
	    return $this->child_dirs;
	  }
	  */
	  
    $conn = RODSConnManager::getConn($this->account);
    $child_dirs_stats=$conn->getChildDirWithStats($this->path_str,$orderby,
      $startingInx, $maxresults, $total_num_rows);
    RODSConnManager::releaseConn($conn); 
    $this->child_dirs=array();
    foreach($child_dirs_stats as $stat)
    {
      if ($this->path_str=='/')
        $this->child_dirs[]=new ProdsDir($this->account, 
          "/".$stat->name,false,$stat);
      else
        $this->child_dirs[]=new ProdsDir($this->account, 
          $this->path_str."/".$stat->name,false,$stat);
    }
    return $this->child_dirs;
  }
  
  
  
 /**
	* Get children files of this dir. The result may be cached.
	* @param $orderby An associated array specifying how to sort the result by attributes. Each array key is the attribute, array val is 0 (assendent) or 1 (dessendent). The supported attributes are "name", "size", "owner", "mtime". 
	* @return an array of ProdsFile
	*/
  public function getChildFiles(array $orderby=array(), $startingInx=0, 
    $maxresults=500, &$total_num_rows=-1)
  {
    /*
    if ( ($this->usecache===true) && (isset($this->child_files)) )
	  { 
	    return $this->child_files;
	  }
	  */
	  
	  $conn = RODSConnManager::getConn($this->account);
    $child_files_stats=$conn->getChildFileWithStats($this->path_str, $orderby,
      $startingInx, $maxresults, $total_num_rows);
    RODSConnManager::releaseConn($conn); 
    $this->child_files=array();
    foreach($child_files_stats as $stat)
    {
      $this->child_files[]=new ProdsFile($this->account,
        $this->path_str."/".$stat->name,false,$stat);
    }
    return $this->child_files;  
  }
  
 /**
  * Make a new directory under this directory
  * @param string $name full path of the new dir to be made on server
  * @return ProdsDir the new directory just created (or already exists)
  */
  public function mkdir($name)
  {
    $conn = RODSConnManager::getConn($this->account);
    $conn->mkdir($this->path_str."/$name");
    RODSConnManager::releaseConn($conn); 
    return (new ProdsDir($this->account, "$name"));
  }
  
 /**
  * remove this directory
  * @param boolean $recursive whether recursively delete all child files and child directories recursively.
  * @param boolean $force whether force delete the file/dir. If force delete, all files will be wiped physically. Else, they are moved to trash derectory.
  */
  public function rmdir($recursive=true,$force=false)
  {
    $conn = RODSConnManager::getConn($this->account);
    $conn->rmdir($this->path_str, $recursive, $force);
    RODSConnManager::releaseConn($conn); 
  }
  
 /**
  * get the dir stats
  */
  public function getStats()
  {
    if ( ($this->usecache===true) && (isset($this->stats)) )
	  { 
      return $this->stats;
    }
    
    $conn = RODSConnManager::getConn($this->account);
    $stats=$conn->getDirStats($this->path_str);
    RODSConnManager::releaseConn($conn); 
    
    if ($stats===false) $this->stats=NULL;
    else $this->stats=$stats;
    return $this->stats;
  }
  
 /**
  * query metadata, and find matching files.
  * @param array $terms an assositive array of search conditions, supported ones are:
  *     'name' (string) - partial name of the target (file or dir)
  *     'descendentOnly' (boolean) - whether to search among this directory's decendents. default is false.
  *     'recursive'      (boolean) - whether to search recursively, among all decendents and their children. default is false. This option only works when 'descendentOnly' is true
  *     'meta' (array of RODSMeta) - array of metadata.
  * @param int &$total_count This value (passed by reference) returns the total potential count of search results
  * @param int $start starting index of search results.
  * @param int $limit up to how many results to be returned. If negative, give all results back.
  * @param array $sort_flds associative array with following keys:
  *      'name'      - name of the file or dir
  *      'size'      - size of the file
  *      'mtime'     - last modified time
  *      'ctime'     - creation time
  *      'typename'  - file/data type 
  *      'rescname'  - resource name
  *      'dirname'   - directory/collection name for the file
  *     The results are sorted by specified array keys.
  *     The possible array value must be boolean: true stands for 'asc' and false stands for 'desc', default is 'asc'  
  * @return array of ProdsPath objects (ProdsFile or ProdsDir).
  */
  public function findFiles(array $terms, &$total_count, $start=0, $limit=-1,
    array $sort_flds=array())
  {
    $flds=array("COL_DATA_NAME"=>NULL,"COL_D_DATA_ID"=>NULL,
        "COL_DATA_TYPE_NAME"=>NULL,"COL_D_RESC_NAME"=>NULL,
        "COL_DATA_SIZE"=>NULL,"COL_D_OWNER_NAME"=>NULL,
        "COL_D_CREATE_TIME"=>NULL, "COL_D_MODIFY_TIME"=>NULL,
        "COL_COLL_NAME"=>NULL);
    
    foreach($sort_flds as $sort_fld_key => $sort_fld_val)
    {
      switch($sort_fld_key)
      {
        case 'name':
          if ($sort_fld_val===false)
            $flds['COL_DATA_NAME']='order_by_desc';
          else
            $flds['COL_DATA_NAME']='order_by_asc';
          break;
        
        case 'size':
          if ($sort_fld_val===false)
            $flds['COL_DATA_SIZE']='order_by_desc';
          else
            $flds['COL_DATA_SIZE']='order_by_asc';
          break; 
        
        case 'mtime':
          if ($sort_fld_val===false)
            $flds['COL_D_MODIFY_TIME']='order_by_desc';
          else
            $flds['COL_D_MODIFY_TIME']='order_by_asc';
          break; 
        
        case 'ctime':
          if ($sort_fld_val===false)
            $flds['COL_D_CREATE_TIME']='order_by_desc';
          else
            $flds['COL_D_CREATE_TIME']='order_by_asc';
          break; 
        
        case 'typename':
          if ($sort_fld_val===false)
            $flds['COL_DATA_TYPE_NAME']='order_by_desc';
          else
            $flds['COL_DATA_TYPE_NAME']='order_by_asc';
          break; 
        
        case 'owner':
          if ($sort_fld_val===false)
            $flds['COL_D_OWNER_NAME']='order_by_desc';
          else
            $flds['COL_D_OWNER_NAME']='order_by_asc';
          break; 
        
        case 'rescname':
          if ($sort_fld_val===false)
            $flds['COL_D_RESC_NAME']='order_by_desc';
          else
            $flds['COL_D_RESC_NAME']='order_by_asc';
          break;      
          
        case 'dirname':
          if ($sort_fld_val===false)
            $flds['COL_COLL_NAME']='order_by_desc';
          else
            $flds['COL_COLL_NAME']='order_by_asc';
          break;  
              
        default:
          throw new RODSException("Sort field name '$sort_fld_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      }
    }    
    
    $select=new RODSGenQueSelFlds(array_keys($flds), array_values($flds));
    
    $descendentOnly=false;
    $recursive=false;
    $condition=new RODSGenQueConds();
    foreach($terms as $term_key => $term_val)
    {
      switch ($term_key)
      {
        case 'name':
          $condition->add('COL_DATA_NAME', 'like', '%'.$term_val.'%');
          break;
        
        case 'meta':
          $meta_array=$term_val;
          foreach($meta_array as $meta)
          {
            $condition->add('COL_META_DATA_ATTR_NAME', '=', $meta->name);
            if (isset($meta->op))
              $op=$meta->op;
            else
              $op='=';
            $condition->add('COL_META_DATA_ATTR_VALUE', $op, $meta->value);
          }
          break;
        
        case 'descendentOnly':
          if (true===$term_val)
            $descendentOnly=true;
          break;
        
        case 'recursive':
          if (true===$term_val)
            $recursive=true;
          break;
          
        default:
          throw new RODSException("Term field name '$term_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      } 
    }
    
    if ($descendentOnly===true) 
    {
      if ($recursive===true)
        $condition->add('COL_COLL_NAME', 'like', $this->path_str.'%');
      else
        $condition->add('COL_COLL_NAME', '=', $this->path_str);    
    }
    
    $conn = RODSConnManager::getConn($this->account);
    $results = $conn->query($select, $condition, $start, $limit);
    RODSConnManager::releaseConn($conn); 
    
    $total_count=$results->getTotalCount();
    $result_values=$results->getValues();
    $found=array();
    for($i=0; $i<$results->getNumRow(); $i++)
    {
      $stats=new RODSFileStats(
          $result_values['COL_DATA_NAME'][$i],
          $result_values['COL_DATA_SIZE'][$i],
          $result_values['COL_D_OWNER_NAME'][$i],
          $result_values['COL_D_MODIFY_TIME'][$i],
          $result_values['COL_D_CREATE_TIME'][$i],
          $result_values['COL_D_DATA_ID'][$i],
          $result_values['COL_DATA_TYPE_NAME'][$i],
          $result_values['COL_D_RESC_NAME'][$i]);
      
      if ($result_values['COL_COLL_NAME'][$i]=='/')
        $full_path='/'.$result_values['COL_DATA_NAME'][$i];
      else
        $full_path=$result_values['COL_COLL_NAME'][$i].'/'.
          $result_values['COL_DATA_NAME'][$i];
      $found[]=new ProdsFile($this->account, $full_path, false, $stats);
    }
    return $found;
  }
  
 /**
  * This function is still under developement!!!!
  * query metadata, and find matching directoris.
  * @param array $terms an assositive array of search conditions, supported ones are:
  *     'name' (string) - partial name of the target (file or dir)
  *     'descendentOnly' (boolean) - whether to search among this directory's decendents. default is false.
  *     'recursive'      (boolean) - whether to search recursively, among all decendents and their children. default is false. This option only works when 'descendentOnly' is true
  *     'meta' (array of RODSMeta) - array of metadata.
  * @param int &$total_count This value (passed by reference) returns the total potential count of search results
  * @param int $start starting index of search results.
  * @param int $limit up to how many results to be returned. If negative, give all results back.
  * @param array $sort_flds associative array with following keys:
  *      'name'      - name of the file or dir
  *      'mtime'     - last modified time
  *      'ctime'     - creation time
  *     The results are sorted by specified array keys.
  *     The possible array value must be boolean: true stands for 'asc' and false stands for 'desc', default is 'asc'  
  * @return array of ProdsPath objects (ProdsFile or ProdsDir).
  */
  public function findDirs(array $terms, &$total_count, $start=0, $limit=-1,
    array $sort_flds=array())
  {
    $flds=array("COL_COLL_NAME"=>NULL,"COL_COLL_ID"=>NULL,
        "COL_COLL_OWNER_NAME"=>NULL,"COL_COLL_OWNER_ZONE"=>NULL,
        "COL_COLL_CREATE_TIME"=>NULL,"COL_COLL_MODIFY_TIME"=>NULL,
        "COL_COLL_COMMENTS"=>NULL);
    
    foreach($sort_flds as $sort_fld_key => $sort_fld_val)
    {
      switch($sort_fld_key)
      {
        case 'name':
          if ($sort_fld_val===false)
            $flds['COL_COLL_NAME']='order_by_desc';
          else
            $flds['COL_COLL_NAME']='order_by_asc';
          break;
        
        case 'mtime':
          if ($sort_fld_val===false)
            $flds['COL_COLL_MODIFY_TIME']='order_by_desc';
          else
            $flds['COL_COLL_MODIFY_TIME']='order_by_asc';
          break; 
        
        case 'ctime':
          if ($sort_fld_val===false)
            $flds['COL_COLL_CREATE_TIME']='order_by_desc';
          else
            $flds['COL_COLL_CREATE_TIME']='order_by_asc';
          break; 
        
        default:
          throw new RODSException("Sort field name '$sort_fld_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      }
    }    
    
    $select=new RODSGenQueSelFlds(array_keys($flds), array_values($flds));
    
    $descendentOnly=false;
    $recursive=false;
    $condition=new RODSGenQueConds();
    foreach($terms as $term_key => $term_val)
    {
      switch ($term_key)
      {
        case 'name':
          $condition->add('COL_DATA_NAME', 'like', '%'.$term_val.'%');
          break;
        
        case 'meta':
          $meta_array=$term_val;
          foreach($meta_array as $meta)
          {
            $condition->add('COL_META_DATA_ATTR_NAME', '=', $meta->name);
            if (isset($meta->op))
              $op=$meta->op;
            else
              $op='=';
            $condition->add('COL_META_DATA_ATTR_VALUE', $op, $meta->value);
          }
          break;
        
        case 'descendentOnly':
          if (true===$term_val)
            $descendentOnly=true;
          break;
        
        case 'recursive':
          if (true===$term_val)
            $recursive=true;
          break;
          
        default:
          throw new RODSException("Term field name '$term_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      } 
    }
    
    if ($descendentOnly===true) 
    {
      if ($recursive===true)
        $condition->add('COL_COLL_NAME', 'like', $this->path_str.'%');
      else
        $condition->add('COL_COLL_NAME', '=', $this->path_str);    
    }
    
    $conn = RODSConnManager::getConn($this->account);
    $results = $conn->query($select, $condition, $start, $limit);
    RODSConnManager::releaseConn($conn); 
    
    $total_count=$results->getTotalCount();
    $result_values=$results->getValues();
    $found=array();
    for($i=0; $i<$results->getNumRow(); $i++)
    {
      $stats=new RODSFileStats(
          $result_values['COL_DATA_NAME'][$i],
          $result_values['COL_DATA_SIZE'][$i],
          $result_values['COL_D_OWNER_NAME'][$i],
          $result_values['COL_D_MODIFY_TIME'][$i],
          $result_values['COL_D_CREATE_TIME'][$i],
          $result_values['COL_D_DATA_ID'][$i],
          $result_values['COL_DATA_TYPE_NAME'][$i],
          $result_values['COL_D_RESC_NAME'][$i]);
      
      if ($result_values['COL_COLL_NAME'][$i]=='/')
        $full_path='/'.$result_values['COL_DATA_NAME'][$i];
      else
        $full_path=$result_values['COL_COLL_NAME'][$i].'/'.
          $result_values['COL_DATA_NAME'][$i];
      $found[]=new ProdsFile($this->account, $full_path, false, $stats);
    }
    return $found;
  }
  
  public function findDir(array $terms, $start=0, $limit=500,
    array $sort_fld=array())
  {
    if ($type==0)
      $select=array("COL_COLL_NAME");
    else
      $select=array("COL_COLL_NAME","COL_DATA_NAME");
    
    $condition=array();
    foreach($terms as $term_key => $term_val)
    {
      switch ($term_key)
      {
        case 'name':
          if ($type==0)
            $condition[]=new RODSQueryCondition
              ("COL_COLL_NAME",'%'.$term_val.'%', 'like');
          else
            $condition[]=new RODSQueryCondition
              ("COL_DATA_NAME",'%'.$term_val.'%', 'like');
          break;
        
        case 'meta':
          $meta_array=$term_val;
          foreach($meta_array as $meta)
          {
            $condition[]=new RODSQueryCondition
              ("COL_META_DATA_ATTR_NAME",$meta->name);
            $condition[]=new RODSQueryCondition
              ("COL_META_DATA_ATTR_VALUE",$meta->value,$meta->op);
          }
          break;
        
        case 'descendentOnly':
          if (false!==$term_val)
          {
            if ($type==0)
              $condition[]=new RODSQueryCondition
                ("COL_COLL_PARENT_NAME", $this->path_str.'%', 'like');
            else
              $condition[]=new RODSQueryCondition
                ("COL_COLL_NAME", $this->path_str.'%', 'like');
          }
          break;
          
        default:
          break;
      } 
    }
    $conn = RODSConnManager::getConn($this->account);
    $results = $conn->genQuery($select, $condition, array(), $start, $limit, 
      false );
    RODSConnManager::releaseConn($conn); 
    
    
    
    return $ret_val;
  }
  
}