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
  
  public function findAcct($newacct)
  {
    foreach($this->accts as $acct)
    {
      if (true===$newacct->equals($acct))
      {
        $this->cur_acct=$acct;  
        return $acct;
      }
    }
    return NULL;  
  }
  
  // update account information.
  public function updateAcct($newacct)
  {
    $oldacct=$this->findAcct($newacct);
    if ($oldacct!=NULL)
    {
      if (!empty($newacct->pass))
      {
        $oldacct->pass=$newacct->pass;
      }
      
      if (!empty($newacct->zone))
      {
        $oldacct->zone=$newacct->zone;
      }
    }
    else
    {
      $this->add($newacct);
    }
  }  
}
?>