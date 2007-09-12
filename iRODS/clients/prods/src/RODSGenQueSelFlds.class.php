<?php
require_once("autoload.inc.php");
class RODSGenQueSelFlds 
{
  private $names;
  private $indexes;
  private $attrs;
  
 /**
  * default constructor.
  * @param string $name name of the field, which must be one defined in file 'RodsGenQueryNum.inc.php'.
  * @param string $attrs extra options used for operations such as order_by, the expected values are:
  *     - 'order_by_asc' order the result by this field, in ASCENDING order
  *     - 'order_by_desc' order the result by this field, in DESCENDING order
  */
  public function __construct(array $names=array(), array $attrs=array())    
  {
    require_once("RodsGenQueryNum.inc.php"); //load magic numbers
    
    $this->names=$names;
    $this->attrs=array();
    $this->indexes=array();
    
    for($i=0; $i<count($names); $i++)
    {
      $name=$names[$i];
      if(!isset($GLOBALS['PRODS_GENQUE_NUMS']["$name"]))
      {
        throw new RODSException("General Query select field name '$name' is not valid",
          'PERR_USER_INPUT_ERROR');
      }
      $this->indexes[]=$GLOBALS['PRODS_GENQUE_NUMS']["$name"];
      
      if (empty($attrs[$i]))
        $this->attrs[]=1;
      else
      if ($attrs[$i]=='order_by_asc')
      {
        $this->attrs[]=$GLOBALS['PRODS_GENQUE_NUMS']['ORDER_BY'];
      }
      else
      if ($attrs[$i]=='order_by_desc')
      {
        $this->attrs[]=$GLOBALS['PRODS_GENQUE_NUMS']['ORDER_BY_DESC'];
      }
      else
      {
        $attr=$attrs[$i];
        throw new RODSException("Unexpected attribute: '$attr'",
            'PERR_USER_INPUT_ERROR');
      }  
    }
    
  }
  
 /**
  * Add a single select field.
  */
  public function add($name, $attr=NULL)
  {
    require_once("RodsGenQueryNum.inc.php"); //load magic numbers 
    if(!isset($GLOBALS['PRODS_GENQUE_NUMS']["$name"]))
    {
      throw new RODSException("General Query select field name '$name' is not valid",
          'PERR_USER_INPUT_ERROR');
    }
    $this->indexes[]=$GLOBALS['PRODS_GENQUE_NUMS']["$name"];
    $this->names[]=$name;
    
    if (empty($attrs))
      $this->attrs[]=1;
    else
    if ($attr=='order_by_asc')
    {
      $this->attrs[]=$GLOBALS['PRODS_GENQUE_NUMS']['ORDER_BY'];
    }
    else
    if ($attr=='order_by_desc')
    {
      $this->attrs[]=$GLOBALS['PRODS_GENQUE_NUMS']['ORDER_BY_DESC'];
    }
    else
    {
      throw new RODSException("Unexpected attribute: '$attr'",
          'PERR_USER_INPUT_ERROR');
    }  
  }
  
 /**
  * make a RP_InxIvalPair.
  */
  public function packetize()
  {
    return (new RP_InxIvalPair(count($this->names), $this->indexes,
      $this->attrs));
    
  }
  
  public function getIndexes()
  {
    return $this->indexes;
  }
  
  public function getAttrs()
  {
    return $this->attrs;
  }
  
  public function getCount()
  {
    return count($this->names);
  }
  
}
?>