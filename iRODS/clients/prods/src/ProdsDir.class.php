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
  /**
  * @var RODSDirStats 
  */
  public  $stats;
  
  private $child_dirs;
  private $child_files;
  private $all_children;
  private $position;
  
 /**
	* Default Constructor.
	*
	* @param RODSAccount account iRODS account used for connection
	* @param string $path_str the path of this dir
	* @param boolean $verify whether verify if the path exsits
	* @param RODSDirStats $stats if the stats for this dir is already known, initilize it here.
	* @return a new ProdsDir
	*/
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
	* Verify if this dir exist with server. This function shouldn't be called directly, use {@link exists}
	*/
  protected function verify()
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
  * get next file or directory from the directory, where the internal iterator points to.
	* @return next file or directory from the directory. The file always come first and dir comes later. return false on failure
	*/
  public function getNextChild()
  {
    if (!$this->all_children)
      $this->all_children=$this->getAllChildren();
    if (($this->position>=count($this->all_children))||($this->position<0))
      return false;
    $names=array_keys($this->all_children);  
    $ret_val=$this->all_children[$names[$this->position]];
    $this->position++;
    return $ret_val;
  }
  
 /**
  * @return all children (files and dirs) of current dir
	*/
  public function getAllChildren()
  {
    $this->all_children=array();
    $this->all_children=array_merge($this->all_children,
      $this->getChildrenFiles());
    $this->all_children=array_merge($this->all_children,
      $this->getChildrenDirs());
    return $this->all_children;  
  }
  
 /**
  * Get children directories of this dir. 
	* @param $orderby An associated array specifying how to sort the result by attributes. See details in method {@link findDirs};
	* Note that if the current dir is root '/', it will not return '/' as its child, unlike iCommand's current behavior.
	* @return an array of ProdsDir
	*/
  public function getChildDirs(array $orderby=array(), $startingInx=0, 
    $maxresults=-1, &$total_num_rows=-1)
  {
    $terms=array("descendantOnly"=>true,"recursive"=>false);
    return $this->findDirs($terms,$total_num_rows,$startingInx,$maxresults,$orderby);  
  }
  
 /**
	* Get children files of this dir. 
	*
	* @param array $orderby An associated array specifying how to sort the result by attributes. See details in method {@link findFiles};
	* @param int $startingInx starting index of all files. default is 0.
	* @param int $maxresults max results returned. if negative, it returns all rows. default is -1
	* @param int &$total_num_rows number of all results
	* @param boolean $logical_file whether to return only logical files, if false, it returns all replica with resource name, if true, it returns only 1 logical file, with num_replica available in the stats. default is false.
	* @return an array of ProdsFile
	*/
  public function getChildFiles(array $orderby=array(), $startingInx=0, 
    $maxresults=-1, &$total_num_rows=-1, $logicalFile=false)
  {
    $terms=array("descendantOnly"=>true,"recursive"=>false, 'logicalFile'=>$logicalFile);
    return $this->findFiles($terms,$total_num_rows,$startingInx,$maxresults,$orderby); 
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
  * @param boolean $force_reload If stats already present in the object, and this flag is true, a force reload will be done.
  * @return RODSDirStats the stats object, note that if this object will not refresh unless $force_reload flag is used.
  */
  public function getStats($force_reload=false)
  {
    if ( ($force_reload===false)&&($this->stats) )
      return $this->stats;
    
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
  * -    'name' (string) - partial name of the target (file or dir)
  * -    'descendantOnly' (boolean) - whether to search among this directory's decendents. default is false.
  * -    'recursive'      (boolean) - whether to search recursively, among all decendents and their children. default is false. This option only works when 'descendantOnly' is true
  * -    'logicalFile'    (boolean) - whether to return logical file, instead of all replicas for each file. if true, the resource name for each file will be null, instead, num_replicas will be provided. default is false.
  * -    'smtime'         (int)     - start last-modified-time in unix timestamp. The specified time is included in query, in other words the search can be thought was "mtime >= specified time"
  * -    'emtime'         (int)     - end last-modified-time in unix timestamp. The specified time is not included in query, in other words the search can be thought was "mtime < specified time"
  * -    'owner'          (string)  - owner name of the file
  * -    'rescname'       (string)  - resource name of the file
  * -    'metadata' (array of RODSMeta) - array of metadata.
  * @param int &$total_count This value (passed by reference) returns the total potential count of search results
  * @param int $start starting index of search results.
  * @param int $limit up to how many results to be returned. If negative, give all results back.
  * @param array $sort_flds associative array with following keys:
  * -     'name'      - name of the file or dir
  * -     'size'      - size of the file
  * -     'mtime'     - last modified time
  * -     'ctime'     - creation time
  * -     'owner'     - owner of the file
  * -     'typename'  - file/data type 
  * -     'dirname'   - directory/collection name for the file
  * The results are sorted by specified array keys.
  * The possible array value must be boolean: true stands for 'asc' and false stands for 'desc', default is 'asc'  
  * @return array of ProdsPath objects (ProdsFile or ProdsDir).
  */
  public function findFiles(array $terms, &$total_count, $start=0, $limit=-1,
    array $sort_flds=array())
  {
    $flds=array("COL_DATA_NAME"=>NULL,"COL_D_DATA_ID"=>NULL,
        "COL_DATA_TYPE_NAME"=>NULL, "COL_D_RESC_NAME"=>NULL,
        "COL_DATA_SIZE"=>NULL,"COL_D_OWNER_NAME"=>NULL, "COL_D_OWNER_ZONE"=>NULL,
        "COL_D_CREATE_TIME"=>NULL, "COL_D_MODIFY_TIME"=>NULL,
        "COL_COLL_NAME"=>NULL, "COL_D_COMMENTS"=>NULL);
    
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
    
    $descendantOnly=false;
    $recursive=false;
    $logicalFile=false;
    $condition=new RODSGenQueConds();
    foreach($terms as $term_key => $term_val)
    {
      switch ($term_key)
      {
        case 'name':
          $condition->add('COL_DATA_NAME', 'like', '%'.$term_val.'%');
          break;
        case 'smtime':
          $condition->add('COL_D_MODIFY_TIME', '>=', $term_val);
          break;
        case 'emtime':
          $condition->add('COL_D_MODIFY_TIME', '<', $term_val);
          break;
        case 'owner':
          $condition->add('COL_D_OWNER_NAME', '=', $term_val);
          break;   
        case 'ownerzone':
          $condition->add('COL_D_OWNER_ZONE', '=', $term_val);
          break;  
        case 'rescname':
          $condition->add('COL_D_RESC_NAME', '=', $term_val);
        case 'metadata':
          $meta_array=$term_val;
          foreach($meta_array as $meta)
          {
            $condition->add('COL_META_DATA_ATTR_NAME', '=', $meta->name);
            if (isset($meta->op))
              $op=$meta->op;
            else
              $op='=';
            if ($op=='like')
              $value='%'.$meta->value.'%';
            else
              $value=$meta->value;  
            $condition->add('COL_META_DATA_ATTR_VALUE', $op, $value);
          }
          break;
        
        case 'descendantOnly':
          if (true===$term_val)
            $descendantOnly=true;
          break;
        
        case 'recursive':
          if (true===$term_val)
            $recursive=true;
          break;
          
        case 'logicalFile':
          if (true===$term_val)
            $logicalFile=true;
          break;  
          
        default:
          throw new RODSException("Term field name '$term_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      } 
    }
    
    if ($descendantOnly===true) 
    {
      if ($recursive===true)
        $condition->add('COL_COLL_NAME', 'like', $this->path_str.'%');
      else
        $condition->add('COL_COLL_NAME', '=', $this->path_str);    
    }
    
    if ($logicalFile===true)
    {
      $select->update('COL_D_RESC_NAME','count');
      $select->update('COL_DATA_SIZE','max');
      $select->update('COL_D_CREATE_TIME','min');
      $select->update('COL_D_MODIFY_TIME','max');
    }
    
    $conn = RODSConnManager::getConn($this->account);
    $results = $conn->query($select, $condition, $start, $limit);
    RODSConnManager::releaseConn($conn); 
    
    $total_count=$results->getTotalCount();
    $result_values=$results->getValues();
    $found=array();
    for($i=0; $i<$results->getNumRow(); $i++)
    {
      $resc_name= ($logicalFile===true)?NULL:$result_values['COL_D_RESC_NAME'][$i];
      $num_replica= ($logicalFile===true)? intval($result_values['COL_D_RESC_NAME'][$i]):NULL;
      $stats=new RODSFileStats(
          $result_values['COL_DATA_NAME'][$i],
          $result_values['COL_DATA_SIZE'][$i],
          $result_values['COL_D_OWNER_NAME'][$i],
          $result_values['COL_D_OWNER_ZONE'][$i],
          $result_values['COL_D_MODIFY_TIME'][$i],
          $result_values['COL_D_CREATE_TIME'][$i],
          $result_values['COL_D_DATA_ID'][$i],
          $result_values['COL_DATA_TYPE_NAME'][$i],
          $resc_name,
          $result_values['COL_D_COMMENTS'][$i],
          $num_replica
      );
      
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
  * query metadata, and find matching diretories.
  * @param array $terms an assositive array of search conditions, supported ones are:
  * -    'name' (string) - partial name of the target (file or dir)
  * -    'descendantOnly' (boolean) - whether to search among this directory's decendents. default is false.
  * -    'recursive'      (boolean) - whether to search recursively, among all decendents and their children. default is false. This option only works when 'descendantOnly' is true
  * -    'smtime'         (int)     - start last-modified-time in unix timestamp. The specified time is included in query, in other words the search can be thought was "mtime >= specified time"
  * -    'emtime'         (int)     - end last-modified-time in unix timestamp. The specified time is not included in query, in other words the search can be thought was "mtime < specified time"
  * -    'owner'          (string)  - owner name of the dir
  * -    'metadata' (array of RODSMeta) - array of metadata.
  * @param int &$total_count This value (passed by reference) returns the total potential count of search results
  * @param int $start starting index of search results.
  * @param int $limit up to how many results to be returned. If negative, give all results back.
  * @param array $sort_flds associative array with following keys:
  * -     'name'      - name of the dir
  * -     'mtime'     - last modified time
  * -     'ctime'     - creation time
  * -     'owner'     - owner of the dir
  * The results are sorted by specified array keys.
  * The possible array value must be boolean: true stands for 'asc' and false stands for 'desc', default is 'asc'  
  * @return array of ProdsPath objects (ProdsFile or ProdsDir).
  */
  public function findDirs(array $terms, &$total_count, $start=0, $limit=-1,
    array $sort_flds=array())
  {
    $flds=array("COL_COLL_NAME"=>NULL,"COL_COLL_ID"=>NULL,
        "COL_COLL_OWNER_NAME"=>NULL,
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
        
        case 'owner':
          if ($sort_fld_val===false)
            $flds['COL_COLL_OWNER_NAME']='order_by_desc';
          else
            $flds['COL_COLL_OWNER_NAME']='order_by_asc';
          break; 
        
        default:
          throw new RODSException("Sort field name '$sort_fld_key' is not valid",
            'PERR_USER_INPUT_ERROR');
          break;
      }
    }
    $select=new RODSGenQueSelFlds(array_keys($flds), array_values($flds));
    
    $descendantOnly=false;
    $recursive=false;
    $condition=new RODSGenQueConds();
    foreach($terms as $term_key => $term_val)
    {
      switch ($term_key)
      {
        case 'name':
          $condition->add('COL_COLL_NAME', 'like', '%'.$term_val.'%');
          break;
        case 'smtime':
          $condition->add('COL_COLL_MODIFY_TIME', '>=', $term_val);
          break;
        case 'emtime':
          $condition->add('COL_COLL_MODIFY_TIME', '<', $term_val);
          break;
        case 'owner':
          $condition->add('COL_COLL_OWNER_NAME', '=', $term_val);
          break;  
        case 'metadata':
          $meta_array=$term_val;
          foreach($meta_array as $meta)
          {
            $condition->add('COL_META_DATA_ATTR_NAME', '=', $meta->name);
            if (isset($meta->op))
              $op=$meta->op;
            else
              $op='=';
            if ($op=='like')
              $value='%'.$meta->value.'%';
            else
              $value=$meta->value;  
            $condition->add('COL_META_DATA_ATTR_VALUE', $op, $value);
          }
          break;
        
        case 'descendantOnly':
          if (true===$term_val)
            $descendantOnly=true;
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
    
    if ($descendantOnly===true) 
    {
      // eliminate '/' from children, if current path is already root
      if ($this->path_str=='/')
        $condition->add('COL_COLL_NAME', '<>', '/');
        
      if ($recursive===true)
        $condition->add('COL_COLL_PARENT_NAME', 'like', $this->path_str.'%');
      else
        $condition->add('COL_COLL_PARENT_NAME', '=', $this->path_str);    
    }
    
    $conn = RODSConnManager::getConn($this->account);
    $results = $conn->query($select, $condition, $start, $limit);
    RODSConnManager::releaseConn($conn); 
    
    $total_count=$results->getTotalCount();
    $result_values=$results->getValues();
    $found=array();
    for($i=0; $i<$results->getNumRow(); $i++)
    {
      $full_path=$result_values['COL_COLL_NAME'][$i];
      $acctual_name=basename($result_values['COL_COLL_NAME'][$i]);
      $stats=new RODSDirStats(
          $acctual_name,
          $result_values['COL_COLL_OWNER_NAME'][$i],
          $result_values['COL_COLL_OWNER_ZONE'][$i],
          $result_values['COL_COLL_MODIFY_TIME'][$i],
          $result_values['COL_COLL_CREATE_TIME'][$i],
          $result_values['COL_COLL_ID'][$i],
          $result_values['COL_COLL_COMMENTS'][$i]);
      
      $found[]=new ProdsDir($this->account, $full_path, false, $stats);
    }
    return $found;
  }    
}