<?php
ob_start();
require_once("../config.inc.php");

session_start();
if (!isset($_SESSION['acct_manager']))
  $_SESSION['acct_manager']= new RODSAcctManager();

$ruri="";
if (isset($_REQUEST['ruri']))
  $ruri=$_REQUEST['ruri'];
else
{
  header("HTTP/1.0 404 Not Found");
  die("Expected RODS URI not found!");
} 

$force_download=false;
if (isset($_REQUEST['force_download']))
  $force_download=true;

try {
  $file=ProdsFile::fromURI($ruri, false);
  if (empty($file->account->pass))
  {
    $acct=$_SESSION['acct_manager']->findAcct($file->account);
    if (empty($acct))
    {
      header("HTTP/1.0 403 forbidden");
      die('Authentication Required');
    }
    $file->account=$acct;
  }
  //$file->verify();
  if (!$file->exists())
  {
    header("HTTP/1.0 404 Not Found");
    die("Requested RODS URI: '$ruri' Not found on the RODS server!");
  }
  
  $filestats=$file->getStats();
  check_modified_header($filestats);
  
  if (ereg('Opera(/| )([0-9].[0-9]{1,2})', $_SERVER['HTTP_USER_AGENT']))
    $UserBrowser = "Opera";
  elseif (ereg('MSIE ([0-9].[0-9]{1,2})', $_SERVER['HTTP_USER_AGENT']))
    $UserBrowser = "IE";
  else
    $UserBrowser = '';
  require_once("../mimetype.inc.php");
  $fileext=array_pop(explode('.',$file->getName()));;
  $filemimetype='application/octet-stream';
  if ( (!empty($fileext)) && isset($GLOBALS['mimetypes'][$fileext]) ) 
    $filemimetype=$GLOBALS['mimetypes'][$fileext];
  if ($UserBrowser == "IE")
    $filemimetype=str_replace('octet-stream','octetstream',$filemimetype);
  @ob_end_clean(); /// decrease cpu usage extreme
  header('Content-Type: ' . $filemimetype);
  if ($force_download===true)
    header('Content-Disposition: attachment; filename="'.$file->getName().'"');
  else
    header('Content-Disposition: inline; filename="'.$file->getName().'"');
  header("Last-Modified: ".get_http_mdate($filestats));
  if ($filestats->size >0)
    header('Content-Length: '.$filestats->size);    
  header('Accept-Ranges: bytes');
  header("Cache-control: private");
  header('Pragma: private');  
  
  /////  multipart-download and resume-download
  $size=$filestats->size;
  
  if ($size<=0)
  {
    $range=0; 
  }
  else
  if(isset($_SERVER['HTTP_RANGE']))
  {
    list($a, $range) = explode("=",$_SERVER['HTTP_RANGE']);
    str_replace($range, "-", $range);
    $size2 = $size-1;
    $new_length = $size-$range;
    header("HTTP/1.1 206 Partial Content");
    header("Content-Length: $new_length");
    header("Content-Range: bytes $range$size2/$size");
  }
  else
  {
    $size2=$size-1;
    header("Content-Length: ".$size);
  }
  $chunksize = 1*(1024*1024);
  //$bytes_send = 0;
  $file->open("r");
  
  if(isset($_SERVER['HTTP_RANGE']))
    $file->seek($range);
  while( (($buffer = $file->read($chunksize))!=NULL) && 
    (connection_status()==0) )
  {
    print($buffer);//echo($buffer); // is also possible
    flush();
    //$bytes_send += strlen($buffer);
    //sleep(1);//// decrease download speed
  }
  $file->close();
  
  
} catch (Exception $e) {
  if ($e instanceof RODSException)
  {
    header("HTTP/1.0 500 Internal Server Error");
    header('Content-Type: text/html');
    header("Content-Length: ");
    echo '<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">'.
         '<HTML><HEAD>'.
         '<TITLE>500 Internal Server Error (iRODS Error: '.
            $e->getCode().' '.$e->getCodeAbbr().')</TITLE>'.
         '</HEAD><BODY>'.
         '<H1>iRODS Error:'.$e->getCode().'</H1>'.
         "$e".'<P>'.
         '<HR>'.
         '<ADDRESS>'.
             ' iRODS Server at '.$acct->host.
             ' Port: '.$acct->port.
             ' User: '.$acct->user.
         '</ADDRESS>'.
         '</BODY></HTML>';
    die ();
  }
  else
  {
    header("HTTP/1.0 500 Internal Server Error");
    header('Content-Type: text/html');
    header("Content-Length: ");
    echo '<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">'.
         '<HTML><HEAD>'.
         '<TITLE>500 Internal Server Error</TITLE>'.
         '</HEAD><BODY>'.
         '<H1>Internal Server Error:'.$e->getCode().'</H1>'.
         "$e".'<P>'.
         '<HR>'.
         '</BODY></HTML>';
    die ();
  }
}

function get_http_mdate($filestats)
{
    return gmdate("D, d M Y H:i:s",$filestats->mtime)." GMT";
}

function check_modified_header($filestats)
{
    // This function is based on code from http://ontosys.com/php/cache.html

    $headers=apache_request_headers();
    $if_modified_since=preg_replace('/;.*$/', '', $headers['If-Modified-Since']);
    if(!$if_modified_since)
        return;

    $gmtime=get_http_mdate($filestats);

    if (strtotime($if_modified_since) < strtotime($gmtime)) {
        header("HTTP/1.1 304 Not Modified");
        exit;
    }
}

?>