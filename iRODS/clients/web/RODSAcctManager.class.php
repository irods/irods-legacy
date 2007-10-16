<?php

require_once(PRODS_INC_PATH);

class RODSAcctManager
{
  private $accts;
  private $cur_acct;
  
  public function __construct()
  {
    $this->accts=array();
    $this->cur_acct=NULL;
  }
  
  public function add(RODSAccount $newacct)
  {
    foreach($this->accts as &$acct)
    {
      if ($newacct->equals($acct))
      {
        $acct=$newacct;
        $this->cur_acct=$newacct;
        return;
      }
    }
    $this->accts[]=$newacct;
    $this->cur_acct=$newacct;
  }
  
  public function getCurAcct()
  {
    return $this->cur_acct;  
  }
  
  public function findAcct(RODSAccount $newacct)
  {
    $index=$this->findAcctIndex($newacct);
    
    //echo ("index=$index\n");
    if ($index<0)
      return NULL;
    else
    {
      return $this->accts[$index];
    }
  }
  
  public function findAcctIndex(RODSAccount $newacct)
  {
    for($i=0; $i<count($this->accts); $i++ )
    {
      $acct=$this->accts[$i];
      if (true==$newacct->equals($acct))
      {
        $this->cur_acct=$acct;  
        return $i;
      }
    }
    return -1;  
  }
  
  // update account information.
  public function updateAcct(RODSAccount $newacct)
  {
    $oldacct_index=$this->findAcctIndex($newacct);
    if ($oldacct_index>=0)
    {
      if (!empty($newacct->pass))
      {
        $this->accts[$oldacct_index]->pass=$newacct->pass;
      }
      
      if (!empty($newacct->zone))
      {
        $this->accts[$oldacct_index]->zone=$newacct->zone;
      }
    }
    else
    {
      $this->add($newacct);
    }
  }  
}
?>