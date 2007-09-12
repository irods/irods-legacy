<?php

require_once(dirname(__FILE__)."/../autoload.inc.php");
class RP_GenQueryInp extends RODSPacket
{
  public function __construct($maxRows=500, $continueInx=0,
    RP_KeyValPair $KeyValPair_PI= NULL, 
    RP_InxIvalPair $InxIvalPair_PI=NULL, 
    RP_InxValPair $InxValPair_PI=NULL,
    $options=0, $partialStartIndex=0)
  {
    if (!isset($KeyValPair_PI)) $KeyValPair_PI=new KeyValPair_PI();
    if (!isset($InxIvalPair_PI)) $InxIvalPair_PI=new InxIvalPair_PI();
    if (!isset($InxValPair_PI)) $InxValPair_PI=new InxValPair_PI();
    
    $packlets=array("maxRows" => $maxRows, 'continueInx' => $continueInx,
      'partialStartIndex' => $partialStartIndex, 'options' => $options,
      'KeyValPair_PI' => $KeyValPair_PI, 'InxIvalPair_PI' => $InxIvalPair_PI,
      'InxValPair_PI' => $InxValPair_PI);  
    parent::__construct("GenQueryInp_PI",$packlets);
  }
     
}
?>