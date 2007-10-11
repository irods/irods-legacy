<?php
require_once("autoload.inc.php");
class RODSGenQueConds 
{
  private $cond;
  private $cond_kw;
  
 /**
  * default constructor. It take names, ops, and vals.
  * suppose name='foo' op='>=' and val='0', then the triplex means
  * "foo >= 0" as one iRODS general query condition.
  * @param array (of string) $names names of the field, which must be one defined in file 'RodsGenQueryNum.inc.php'.
  * @param array (of string) $ops logical operator, such as '=' 'like' '>'
  * @param array (of string) $vals value of the filed
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