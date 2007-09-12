<?php

class RODSFileStats
{
  public $name;
  public $size;
  public $owner;
  public $mtime;
  public $ctime;
  public $id;
  public $typename;  
  public $rescname;
  
  public function __construct($name,$size,$owner,$mtime,$ctime,$id,$typename,
    $rescname)
  {
    $this->name=$name;
    $this->size=$size;
    $this->owner=$owner;
    $this->mtime=$mtime;
    $this->ctime=$ctime;
    $this->id=$id;
    $this->typename=$typename;
    $this->rescname=$rescname;
  }
  
}  
     