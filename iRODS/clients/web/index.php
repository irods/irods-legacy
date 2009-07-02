<?php
  
  die("this version of iRODS Browser has been deprecated. Please get the latest iRODS Browser from http://extrods.googlecode.com !");
  session_start();
  
  if (isset($_REQUEST['redirect']))
  {
    $parsed_redirect=parse_url('rods://'.$_REQUEST['redirect']);
    $host=$parsed_redirect['host'];
    $port=$parsed_redirect['port'];
    $user=$parsed_redirect['user'];
    $pass=$parsed_redirect['pass'];
    $init_path=$parsed_redirect['path'];
    $errmsg=isset($_REQUEST['errmsg'])?$_REQUEST['errmsg']:'';
     
    $loginform_js="generateLoginForm('form-login','$host',
      '$port','$user','$pass','$init_path','$errmsg');";
  }
  else
    $loginform_js="generateLoginForm('form-login');";
?>
<html>
<head>
  <title>iRODS Rich Web Client</title>
  <link rel="stylesheet" type="text/css" href="extjs/docs/resources/collapser.css" />
	<link rel="stylesheet" type="text/css" href="extjs/resources/css/ext-all.css" />
  <link rel="stylesheet" type="text/css" href="browse.css" />
  
    <script type="text/javascript" src="extjs/adapter/yui/yui-utilities.js"></script>     
    <script type="text/javascript" src="extjs/adapter/yui/ext-yui-adapter.js"></script>
    <script type="text/javascript" src="extjs/ext-all.js"></script>
    <script type="text/javascript" src="login.js"></script>
</head>
<body scroll="no" id="docs">
    
  
    <script type="text/javascript">
      Ext.onReady(function(){   
        layout = new Ext.BorderLayout(document.body, {
  	    north: {
  	        split:false,
  	        initialSize: 32,
  	        titlebar: false
  	    },
  	    center: {
  	        titlebar: false,
  	        autoScroll:true,
            closeOnTab: true
        }
  	    });
  
        layout.beginUpdate();
  	    layout.add('north', new Ext.ContentPanel('north', 'North'));
  	    layout.add('center', new Ext.ContentPanel('center', {title: 'RODS Web Client', closable: false, autoScroll:true, fitToFrame:true}));
  	    layout.endUpdate();
  	    
  	    Ext.QuickTips.init();
        // turn on validation errors beside the field globally
        Ext.form.Field.prototype.msgTarget = 'side';
  	    //generateLoginForm('form-login');
  	    <?php echo $loginform_js; ?>;
  	  });  
  	  
  	</script>     
    
  <div id ="container">
    <div id="north" class="x-layout-inactive-content">
      <h3> RODS Browser </h3> 
    </div>
  
    <div id="center" class="x-layout-inactive-content" style="margin:50px 0px; padding:0px;text-align:center;">
      <div style="width:320px;margin-top:100px;margin-left:auto;margin-right:auto;" > 
        <div class="x-box-tl"><div class="x-box-tr"><div class="x-box-tc"></div></div></div>
        <div class="x-box-ml"><div class="x-box-mr"><div class="x-box-mc" style="text-align:left;">
            <div id="form-login-err" style="color:red">&nbsp;</div>
            <h3 style="margin-bottom:5px;"> Sign on to iRODS </h3>
            <div id="form-login">
              
            </div>
        </div></div></div>
        <div class="x-box-bl"><div class="x-box-br"><div class="x-box-bc"></div></div></div>
      </div>
    </div>    
  </div>
</body>
</html>
