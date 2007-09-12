<?php
require_once("autoload.inc.php");
class RODSGenQueConds 
{
  private $cond;
  private $cond_kw;
  
 /**
  * default constructor.
  * @param string $name name of the field, which must be one defined in file 'RodsGenQueryNum.inc.php'.
  * @param string $attrs extra options used for operations such as order_by, the expected values are:
  *     - 'order_by_asc' order the result by this field, in ASCENDING order
  *     - 'order_by_desc' order the result by this field, in DESCENDING order
  */
  public function __construct(array $names=array(), array $ops=array(), 
    array $vals=array())    
  {
    require_once("RodsGenQueryNum.inc.php"); //load magic numbers
    require_once("RodsGenQueryKeyWd.inc.php"); //load magic keywords
    
    $this->cond=array('names'=>array(), 'sysnames'=>array(), 'values'=>array());
    $this->cond_kw=array('names'=>array(), 'sysnames'=>array(), 'values'=>array());
    
    for($i=0; $i<count($names); $i++)
    {
      $name=$names[$i];
      $op=$ops[$i];
      $val=$vals[$i];
      if (isset($GLOBALS['PRODS_GENQUE_NUMS']["$name"]))
      {
        $this->cond['names'][]=$name;
        $this->cond['sysnames'][]=$GLOBALS['PRODS_GENQUE_NUMS']["$name"];
        $this->cond['values'][]="$op '$val'";
      }
      else
      if (isset($GLOBALS['PRODS_GENQUE_KEYWD']["$name"]))
      {
        $this->cond_kw['names'][]=$name;
        $this->cond_kw['sysnames'][]=$GLOBALS['PRODS_GENQUE_KEYWD']["$name"];
        $this->cond_kw['values'][]="$op '$val'";  
      }
      else
      {
        throw new RODSException("General Query condition field name '$name' is not valid",
          'PERR_USER_INPUT_ERROR');
      }
    }
  }
  
 /**
  * Add a single select field.
  */
  public function add($name, $op, $val)
  {
    require_once("RodsGenQueryNum.inc.php"); //load magic numbers 
    require_once("RodsGenQueryKeyWd.inc.php"); //load magic keywords
    
    if (isset($GLOBALS['PRODS_GENQUE_NUMS']["$name"]))
    {
      $this->cond['names'][]=$name;
      $this->cond['sysnames'][]=$GLOBALS['PRODS_GENQUE_NUMS']["$name"];
      $this->cond['values'][]="$op '$val'";
    }
    else
    if (isset($GLOBALS['PRODS_GENQUE_KEYWD']["$name"]))
    {
      $this->cond_kw['names'][]=$name;
      $this->cond_kw['sysnames'][]=$GLOBALS['PRODS_GENQUE_KEYWD']["$name"];
      $this->cond_kw['values'][]="$op '$val'";  
    }
    else
    {
      throw new RODSException("General Query condition field name '$name' is not valid",
        'PERR_USER_INPUT_ERROR');
    }
  }
  
 /**
  * make a RP_InxValPair.
  */
  public function packetize()
  {
    return ( new RP_InxValPair(count($this->cond['names']), 
      $this->cond['sysnames'], $this->cond['values']) );
  }
  
 /**
  * make a RP_KeyValPair.
  */
  public function packetizeKW()
  {
    return ( new RP_KeyValPair(count($this->cond_kw['names']), 
      $this->cond_kw['sysnames'], $this->cond_kw['values']) );
  }
}
?>