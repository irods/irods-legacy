<?php
  session_start();
?>

<html>
<head>
  <title>iRODS Rich Web Client</title>
  <link rel="stylesheet" type="text/css" href="extjs/docs/resources/collapser.css" />
	<link rel="stylesheet" type="text/css" href="extjs/resources/css/ext-all.css" />
	<link rel="stylesheet" type="text/css" href="extjs/resources/css/xtheme-aero.css" />
  <link rel="stylesheet" type="text/css" href="browse.css" />
  
  <!-- LIBS -->     
  <script type="text/javascript" src="extjs/adapter/yui/yui-utilities.js"></script>     
  <script type="text/javascript" src="extjs/adapter/yui/ext-yui-adapter.js"></script>   
  <script type="text/javascript" src="extjs/adapter/ext/ext-base.js"></script>
    
  <!-- ENDLIBS -->
  <script type="text/javascript" src="extjs/ext-all-debug.js"></script>
  <script type="text/javascript" src="RODSTreeLoader.js"></script> 
  <script type="text/javascript">
    Ext.BLANK_IMAGE_URL='images/s.gif';
    Ext.SSL_SECURE_URL='blank.html';
    if (location.protocol=='https')
      Ext.isSecure=true;  
  </script>  
  
  <script type="text/javascript" src="history-experimental-min-2.2.2.js"></script> 
	<script type="text/javascript" src="browse.js">	</script>
</head>
<!-- <body scroll="no" id="body"> -->
<body id="body">  
  <div id="loading-mask" style="width:100%;height:100%;background:#c3daf9;position:absolute;z-index:20000;left:0;top:0;">&#160;</div>
  <div id="loading">
    <div class="loading-indicator"><img src="extjs/resources/images/default/grid/loading.gif" style="width:16px;height:16px;" align="absmiddle">&#160;Connecting ...</div>
  </div>
  
  <script type="text/javascript">
    var bookmarkedRURI = YAHOO.util.History.getBookmarkedState( "ruri" ); 
    var queryRURI = YAHOO.util.History.getQueryStringParameter( "ruri" );
    var defaultRURI = 'rods.tempZone:RODS@rt.sdsc.edu:1247/tempZone/home'; 
    var initialRURI = bookmarkedRURI || queryRURI || defaultRURI; 
    
    var browser=RodsBrowser(initialRURI, <?php echo '"'.session_id().'"'; ?>);
    //Ext.onReady(browser.init, browser);
    Ext.onReady(function(){  
      Ext.QuickTips.init();
      browser.init();
    });  
    
    // Register our only module. Module registration MUST take
    // place before calling YAHOO.util.History.initialize.
    YAHOO.util.History.register( "ruri", initialRURI, function( state ) {
        // This is called after calling YAHOO.util.History.navigate, or after the user
        // has trigerred the back/forward button. We cannot discrminate between
        // these two situations.
        browser.gridGoTo( state );
    } );
    
    function initializeHistory() {
        
        // This is the tricky part... The window's onload handler is called when the
        // user comes back to your page using the back button. In this case, the
        // actual section that needs to be loaded corresponds to the last section
        // visited before leaving the page, and not the initial section. This can
        // be retrieved using getCurrentState:
        var currentRURI = YAHOO.util.History.getCurrentState( "ruri" );
        if ( location.hash.substr(1).length > 0 ) {
            browser.gotoRURI( currentRURI );
        }
    }
    
    // Subscribe to this event before calling YAHOO.util.History.initialize,
    // or it may never get fired! Note that this is guaranteed to be fired
    // after the window's onload event.
    YAHOO.util.History.onLoadEvent.subscribe( function() {
        initializeHistory();
    } );
    
    // The call to YAHOO.util.History.initialize should ALWAYS be from within
    // a script block located RIGHT AFTER the opening body tag (this seems to prevent
    // an edge case bug on IE - IE seems to sometimes forget the history when
    // coming back to a page, and the back - or forward button depending on the
    // situation - is disabled...)
    try {
        YAHOO.util.History.initialize();
    } catch ( e ) {
        // The only exception that gets thrown here is when the browser is not A-grade.
        // Since scripting is enabled, we still try to provide the user with a better
        // experience using AJAX. The only caveat is that the browser history will not work.
        initializeHistory();
    }
    
    removeLoadingMasks();
  </script>
  
  <div id ="container">
    <div id="tree-div" style="overflow:auto; height:300px;width:250px;border:1px solid #c3daf9;"></div>
    <div id="west" class="x-layout-inactive-content">
      Hi. I'm the west panel.
    </div>
    <div id="north" class="x-layout-inactive-content">
      <div id="login-out" style="text-align:right;"></div>
    </div>
  
    <div id="center2" class="x-layout-inactive-content"></div>
    <div id="rods-browser-grid" style="border:1px solid #99bbe8;overflow: hidden; width: 665px; height: 300px;"> </div>
    
  </div>
  
  <div id="file-search-result-dlg" style="visibility:hidden;">
    <div id= "file-search-result-dlg-hd" class="x-dlg-hd">File Search Results</div>
    <div id= "file-search-result-dlg-bd" class="x-dlg-bd">
        <div id= "file-search-result-grid-center" class="x-layout-inactive-content" style="padding:10px;"></div> 
        <div id= "file-search-result-grid"></div>    
    </div>
  </div>
  
  <div id="search-dlg" style="visibility:hidden;">
    <div id= "search-dlg-hd" class="x-dlg-hd">Advanced Search</div>
    <div id= "search-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
    </div>
  </div>
  
  <div id="file-dlg" style="visibility:hidden;">
    <!-- <div id= "file-dlg-hd" class="ydlg-hd">File Viewer</div> -->
    <div id= "file-dlg-hd" class="x-dlg-hd">File Viewer</div>
    <!-- <div class="ydlg-bd"> -->
    <div id= "file-dlg-bd" class="x-dlg-bd">
        <div id="tab-main-toolbar"></div> 
        <div id="fileviewer-tab-main" class="x-layout-inactive-content" 
            style="padding:10px;">
             
        </div>
        
        <div id="fileviewer-tab-meta" class="x-layout-inactive-content" 
            style="padding:10px;"></div>
        <div id= "fileviewer-tab-meta-grid">Metadata</div> 
        
        <div id="fileviewer-tab-repl" class="x-layout-inactive-content" 
            style="padding:10px;"> </div>
        <div id= "fileviewer-tab-repl-grid">Replicas</div> 
        
        <div id="fileviewer-tab-more" class="x-layout-inactive-content" 
            style="padding:10px;">
            Under developement 
        </div>
    </div>
  </div>
  
  <div id="repl-dlg" style="visibility:hidden;">
    <div id= "repl-dlg-hd" class="x-dlg-hd">Replicate to following resource</div>
    <div id= "repl-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
      <div id= "repl-dlg-bd-form"></div>     
    </div>
  </div>
  
  <div id="repl-bulk-dlg" style="visibility:hidden;">
    <div id= "repl-bulk-dlg-hd" class="x-dlg-hd">Replicate files/collections to resource</div>
    <div id= "repl-bulk-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
      <div id= "repl-bulk-dlg-bd-desc" style="padding:10px;"></div>
      <div id= "repl-bulk-dlg-bd-form" style="padding:10px;"></div>     
    </div>
  </div>
  
  <div id="new-file-dlg" style="visibility:hidden;">
    <div id= "new-file-dlg-hd" class="x-dlg-hd">Create New File</div>
    <div id= "new-file-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
      <div id= "new-file-dlg-bd-form"></div>     
    </div>
  </div>
  
  <div id="file-upload-dlg" style="visibility:hidden;">
    <div id= "file-upload-dlg-hd" class="x-dlg-hd">Upload a single file</div>
    <div id= "file-upload-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
      <div id= "file-upload-dlg-bd-form"></div>     
    </div>
  </div>
  
  <div id="upload-applet-dlg" style="visibility:hidden;">
    <div id= "upload-applet-dlg-hd" class="x-dlg-hd">Upload files and directories with JAVA applet</div>
    <div id= "upload-applet-dlg-bd" class="x-dlg-bd" style="padding:10px;background:#c4d2e3;border:0 none;">
      <div id= "upload-applet-dlg-bd-main" style="padding:10px;background:#c4d2e3;border:0 none;"></div>     
    </div>
  </div>
  
  <div id="metadata-dlg" style="visibility:hidden;">
    <div id= "metadata-dlg-hd" class="x-dlg-hd">User Defined Metadata</div>
    <div id= "metadata-dlg-bd" class="x-dlg-bd">
      <div id= "metadata-grid-center" class="x-layout-inactive-content" style="padding:10px;"></div> 
      <div id= "metadata-grid"></div>
    </div>
  </div>
  
  <iframe id="download-frame" name="download-frame" style="display:none;width=1px;height=1px"></iframe>
</body>
</html>
