<?php
require_once("../config.inc.php");
session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

$upload_error_msg=array(
        0=>"There is no error, the file uploaded with success",
        1=>"The uploaded file exceeds the upload_max_filesize directive (".ini_get("upload_max_filesize").") in php.ini",
        2=>"The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form",
        3=>"The uploaded file was only partially uploaded",
        4=>"No file was uploaded",
        6=>"Missing a temporary folder",
        7=>"Failed to write file to disk.",
        8=>"File upload stopped by extension."
);

/*
$data .= '<br>' . date('Y-m-d H:i:s') . '$_FILES<pre>' . print_r($_FILES, true) . "</pre>";
$data .= '<br>' . date('Y-m-d H:i:s') . '$_POST<pre>' . print_r($_POST, true) . "</pre>";

$fp = fopen(date('Y-m-d') .".txt", "a");
fwrite($fp, $data);
fclose($fp);
*/

$ruri="";
if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
else
{
  $response=array('success'=> false,'errmsg'=>'Expected RODS URI not found');
  echo json_encode($response);
  exit(0);
} 

$resource="";
if (isset($_REQUEST['resource']))
  $resource=$_REQUEST['resource'];
else
{
  $response=array('success'=> false,'errmsg'=>'Resource not set');
  echo json_encode($response);
  exit(0);
}
   

$collection=ProdsDir::fromURI($ruri, false);
if (empty($collection->account->pass))
{
  $acct=$_SESSION['acct_manager']->findAcct($collection->account);
  if (empty($acct))
  {
    $response=array('success'=> false,'errmsg'=>'Authentication Required');
    echo json_encode($response);
    exit(0);
  }
  $collection->account=$acct;
}
if (empty($collection->account->zone))
{
  $collection->account->getUserInfo();
}  

$response=array('success'=> false,'errmsg'=>'Unknow errors', 'errcode'=>-1);
$error=false;
$filelist=array();
$numfiles=0;

try {
  foreach($_FILES as $srcfile)
  {
    $filename=($srcfile['name']);
    if (empty($filename))  
      continue;
    
    if ($srcfile['error']!=UPLOAD_ERR_OK)
    {
      if (isset($upload_error_msg[$srcfile['error']]))
       $response=array('success'=> false,'errmsg'=>$upload_error_msg[$srcfile['error']],'errcode'=>$srcfile['error']);
      else 
       $response=array('success'=> false,'errmsg'=>'Unknow errors', 'errcode'=>$srcfile['error']);
      $error=true;
      break;
    }    
    
    $tempuploadfilepath=tempnam(dirname($srcfile['tmp_name']),'RODS_Web_Upload');
    move_uploaded_file($srcfile['tmp_name'], $tempuploadfilepath);
    
    //copy($srcfile['tmp_name'], $srcfile['name']);
    $destfile=new ProdsFile($collection->account,
      $collection->path_str."/".$filename);
    /*
    $response=array('success'=> false,'log'=> ''.$collection->account); 
    echo json_encode($response);
    exit(0); 
    */
    $destfile->open('w',$resource);
    $destfile->write(file_get_contents($tempuploadfilepath));
    $destfile->close();
    if (AUTO_EXTRACT_EXIF === true)
      extactExif($tempuploadfilepath,$destfile);
    $numfiles++; 
    unlink($tempuploadfilepath); 
  }
  
  if ($error===false)
  {
    $response=array('success'=> true,'msg'=>"Uploaded file successfully.");
  }
  
  echo json_encode($response);
} catch (Exception $e) {
  $response=array('success'=> false, 'errmsg'=> $e->getMessage(), 'errcode'=>$e->getCode());
  echo json_encode($response);
}

function extactExif($localfile, $remoteRODSfile)
{
  $exif = exif_read_data($localfile, 'EXIF');
  if ($exif===false) return;
  
  foreach ($exif as $name => $val) {
    
    if ($name=='THUMBNAIL')
    {
      foreach ($val as $tname => $tval) 
        $remoteRODSfile->addMeta(new RODSMeta(
          'EXIF.THUMBNAIL.'.$tname, $tval, ''));
    }
    else
    if ($name=='COMPUTED')
    {
      foreach ($val as $cname => $cval) 
      {
        if ($cname=='html') 
        {
          //skip html tag, because there is a irods server bug that corrupting string with 
          //double quotes: 'COMPUTED.html: width="3264" height="2448"'  
        }
        else
          $remoteRODSfile->addMeta(new RODSMeta(
            'EXIF.COMPUTED.'.$cname, $cval, ''));
      }
    }
    else
    if ($name=='MakerNote')
    {
      //skip makernote  
    } 
    else
    if ($name=='ComponentsConfiguration')
    {
      //skip ComponentsConfiguration, because there is a irods server bug that corrupting string with 
         
    }  
    else  
      $remoteRODSfile->addMeta(new RODSMeta(
        'EXIF.'.$name, $val, ''));
  }
}
?>