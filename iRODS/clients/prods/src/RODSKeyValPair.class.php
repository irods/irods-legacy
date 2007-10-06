<?php
/**
 * RODS keyval type class. This class is conresponding to iRODS's keyval type
 * @author Sifang Lu <sifang@sdsc.edu>
 * @copyright Copyright &copy; 2007, TBD
 * @package RODSConn
 */


require_once("autoload.inc.php");

class RODSKeyValPair
{
  private $keys;
  private $vals;
  
  public function __construct(array $arr=array())
  {  
    $this->keys=array_keys($arr);
    $this->vals=array_values($arr);
  }
  
  public function addPair ($key, $val)
  {
    $this->keys[]=$key;
    $this->vals[]=$val;
  }
  
  /**
   * Make a RP_KeyValPair
   * @return RP_KeyValPair a RP_KeyValPair object
   */
  public function makePacket()
  {
    return new RP_KeyValPair(count($this->keys),$this->keys, $this->vals);  
  } 
}

?>