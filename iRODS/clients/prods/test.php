<?php

require_once("src/RODSConn.class.php");
require_once("src/RODSConnManager.class.php");

$account=new RODSAccount("localhost", 1247, "rods", "RODS", "tempZone");
$home_dir_str='/tempZone/home/rods';

try {

echo "--- trying to connect --- ".microtime()." <br/>\n";
$conn = RODSConnManager::getConn($account);
$que_result= $conn -> dirExists ($home_dir_str);
if ($que_result===true)
{
  echo "$home_dir_str exists! <br/>\n";
}
else
{
  echo "$home_dir_str doesn't exists! <br/>\n";
}

$que_result= $conn -> dirExists ($home_dir_str."/blah");
if ($que_result===true)
{
  echo "$home_dir_str/blah exists! <br/>\n";
}
else
{
  echo "$home_dir_str/blah doesn't exists! <br/>\n";
}
RODSConnManager::releaseConn($conn);  

$dir=new ProdsDir($account,"/tempZone/home/rods");

$childdirs=$dir->getChildDirs();
echo "Child dirs: <br/>\n";
foreach ($childdirs as $childdir)
{
  echo $childdir."<br/>\n";
}

$childfiles=$dir->getChildFiles();
echo "Child files: <br/>\n";
foreach ($childfiles as $childfile)
{
  echo $childfile." <br/>\n";
  
  /*
  $childfile->open("r");
  echo "desc=".$childfile->getL1desc()."<br/>\n";
  echo "<pre>".$childfile->read($childfile->stats->size)."</pre><br/>\n";
  $cur_offset=$childfile->seek(3);
  echo "offsetted '$cur_offset' bytes <br/> \n";
  echo "<pre>".$childfile->read($childfile->stats->size)."</pre><br/>\n";
  */
  
  //$childfile->close();
}

$myfile=new ProdsFile($account,"/tempZone/home/rods/test1");
$myfile->open("w+","demoResc");
$bytes=$myfile->write("Hello world from Sifang!\n");
echo "$bytes bytes written <br/>\n";
$myfile->close();

$myfile->open("r","demoResc",true);
$str=$myfile->read(200);
echo "the file reads: <pre>$str</pre>";
$myfile->close();


$file_src=new ProdsFile($account,"/tempZone/home/rods/test.php");
$file_dest=new ProdsFile($account,"/tempZone/home/rods/test.sifang.txt");
//$file_src->cpMeta($file_dest);
foreach($file_dest->getMeta() as $meta)
{
  echo "$file_dest->path_str: $meta->name; $meta->value; $meta->units <br/> \n";
}

$meta=array(new RODSMeta("test1","1"));
$files=ProdsPath::queryMeta($account,$meta);
foreach($files as $file)
  echo "$file <br/>\n";
echo "<hr/>";
$dirs=ProdsPath::queryMeta($account,$meta,1);
foreach($dirs as $dir)
  echo "$dir <br/>\n";

$file=new ProdsFile($account,"/tempZone/home/rods/test.php");
var_dump($file->getStats());


echo "<br/>--- connection successful! #2--- ".microtime()." <br/>\n";

} catch (RODSException $e) {
  echo ($e);
  echo $e->showStackTrace();
}

?>