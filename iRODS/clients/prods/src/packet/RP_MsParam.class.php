<?php

require_once(dirname(__FILE__)."/../autoload.inc.php");
class RP_MsParam extends RODSPacket
{
  public function __construct($label='', RODSPacket $inOutStruct=null, 
    RP_BinBytesBuf $BinBytesBuf_PI=null )
  {
    if (!isset($BinBytesBuf_PI)) $BinBytesBuf_PI=new RP_BinBytesBuf();
    if (!isset($inOutStruct)) $inOutStruct=new RODSPacket();
    
    $packlets=array("label" => $label, "type" => $inOutStruct->type, 
      $inOutStruct->type => $inOutStruct, "BinBytesBuf_PI" => $BinBytesBuf_PI);  
    parent::__construct("MsParam_PI",$packlets);
  }
     
}
?>