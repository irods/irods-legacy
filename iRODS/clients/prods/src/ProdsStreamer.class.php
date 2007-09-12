<?php

/**
 * PRODS class
 * @author Sifang Lu <sifang@sdsc.edu>
 * @copyright Copyright &copy; 2007, TBD
 * @package Prods
 */

require_once("autoload.inc.php");

class ProdsStreamer
{
 /**
	* current postion of the file or dir
	*
	* @access private
	*/
  private $postion;
  
  /**
	 * Name of the directory/collection specified in the URI to opendir().
	 *
	 * @access private
	 */
	private $dir;
	
	/**
	 * Name of the file specified in the URI to fopen().
	 *
	 * @access private
	 */
	private $file;

	
	/**
	 * opendir() handler.
	 *
	 * @access private
	 */
	public function dir_opendir ($path, $options) 
	{
		try {
		  $this->dir=ProdsDir::fromURI($path,true);
		  return true;
		} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}
	}

	/**
	 * readdir() handler.
	 *
	 * @access private
	 */
	public function dir_readdir () {
	  try {
  		$child=$this->dir->getNextChild();
  		if ($child===false) return false;
  		return $child->getName();
  	} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}
	}

	/**
	 * rewinddir() handler.
	 *
	 * @access private
	 */
	public function dir_rewinddir () {
	  try {
		  $this->dir->rewind();
		  return true;
		} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}
	}

	/**
	 * closedir() handler.
	 *
	 * @access private
	 */
	public function dir_closedir () {
		try {
		  $this->dir->rewind();
		  return true;
		} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}
	}

	/**
	 * fopen() handler.
	 *
	 * @access private
	 */
	public function stream_open ($path, $mode, $options, &$opened_path) {
		
		// get rid of tailing 'b', if any.
		if ( ($mode{strlen($mode)-1}=='b')&&(strlen($mode)>1) )
		  $mode=substr($mode,0,strlen($mode)-1);
		
		try {
		  $this->file=ProdsFile::fromURI($path);
		  $this->file->open($mode);
		  return true;
		} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}
	}

	/**
	 * fread() and fgets() handler.
	 *
	 * @access private
	 */
	public function stream_read ($count) {
		if (in_array ($this->file->getOpenMode, array ('w', 'a', 'x'))) {
			return false;
		}
		try {
  		$ret = $this->file->read($count);
  		$this->position=$this->file->tell();
  		return $ret;
  	} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}	
	}

	/**
	 * fwrite() handler.
	 *
	 * @access private
	 */
	public function stream_write ($data) {
		if ($this->_mode =='r') {
			return false;
		}
		try {
  		$ret = $this->file->write($date);
  		$this->position=$this->file->tell();
  		return $ret;
  	} catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		}	
	}

	/**
	 * ftell() handler.
	 *
	 * @access private
	 */
	function stream_tell () {
		return $this->position;
	}

	/**
	 * feof() handler.
	 *
	 * @access private
	 */
	function stream_eof () {
	  try {
	    $stats=$this->file->getStats();
	    return $this->position >= $stats->size;
	  } catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return true;
		} 
	}

	/**
	 * fstat() handler.
	 *
	 * @access private
	 */
	function stream_stat () {
	  
	  try {
	    $stats=$this->file->getStats();
	    return array (
  			-1, -1, -1, -1, -1, -1, $stats->size, time (), $stats->mtime, $stats->ctime, -1, -1,
  			'dev' => -1,
  			'ino' => -1,
  			'mode' => -1,
  			'nlink' => -1,
  			'uid' => -1,
  			'gid' => -1,
  			'rdev' => -1,
  			'size' => $stats->size,
  			'atime' => time (),
  			'mtime' => $stats->mtime,
  			'ctime' => $stats->ctime,
  			'blksize' => -1,
  			'blocks' => -1,
  		);
	  } catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return array (
  			-1, -1, -1, -1, -1, -1, -1, time (), time (), time (), -1, -1,
  			'dev' => -1,
  			'ino' => -1,
  			'mode' => -1,
  			'nlink' => -1,
  			'uid' => -1,
  			'gid' => -1,
  			'rdev' => -1,
  			'size' => -1,
  			'atime' => time (),
  			'mtime' => time (),
  			'ctime' => time (),
  			'blksize' => -1,
  			'blocks' => -1,
  		);
		} 
	}

	/**
	 * fseek() handler.
	 *
	 * @access private
	 */
	function stream_seek ($offset, $whence) {
		try {
	    $this->file->seek($offset, $whence);
	    return true;
	  } catch (Exception $e) {
		  trigger_error("Got an exception:$e", E_USER_WARNING);
		  return false;
		} 
	}

	/**
	 * fflush() handler.  Please Note: This method must be called for any
	 * changes to be committed to the repository.
	 *
	 * @access private
	 */
	function stream_flush () {
    return true;
	}

	/**
	 * fclose() handler.
	 *
	 * @access private
	 */
	function stream_close () {
		$this->position = 0;
		$this->file = null;
		$this->dir = null;
	}

}

stream_wrapper_register('rods', 'ProdsStreamer')
		or die ('Failed to register protocol:rods');
?>