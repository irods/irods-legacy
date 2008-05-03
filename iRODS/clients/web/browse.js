function getCurrentRURI()
{
  return browser.getCurrentRURI();
}

function refreshTreeNodeByRURI(new_ruri)
{
  browser.refreshTreeNodeByRURI(new_ruri);
}

function refreshGrid()
{
  browser.refreshGrid();
}

function removeLoadingMasks()
{
  var loading = Ext.get('loading');
  var mask = Ext.get('loading-mask');
	mask.setOpacity(.8);
	mask.shift({
		xy:loading.getXY(),
		width:loading.getWidth(),
		height:loading.getHeight(),
		remove:true,
		duration:1,
		opacity:.3,
		easing:'bounceOut',
		callback : function(){
			loading.fadeOut({duration:.2,remove:true});
		}
	});
}

// pluggable renders
function renderName(value, p, record){
  if (record.data['type']==0) 
    return String.format(
      '<span class="x-grid-col-objtype-dir">{0}</span>', value);
  else
  if (record.data['type']==1)
    return String.format(
      '<span class="x-grid-col-objtype-generic-file">{0}</span>', value);
  else
    return value;
}

function renderFmtSize(value,p,record){
  if ( (!value)||(value<0) )
    return '';
    
  var rawSize=value;
  if (rawSize / 1099511627776 > 1) 
    return Math.round(rawSize*100/1099511627776)/100 + ' TB';
  else if (rawSize / 1073741824 > 1) 
    return Math.round(rawSize*100/1073741824)/100 + ' GB';  
  else 
  if (rawSize / 1048576 > 1) 
    return Math.round(rawSize*100/1048576)/100 + ' MB'; 
  else if (rawSize / 1024 > 1) 
    return Math.round(rawSize*100/1024)/100 + ' KB'; 
  else 
    return rawSize + ' B ';
}

function renderTime(value){
  try {
    return value.dateFormat('F j, Y, g:i a');
  } catch (e) {
    return 'Invalid Time';
  }
}

function jsonErrorResponseHandler(conn, r, options) {
  
  try {
    var response = Ext.util.JSON.decode(r.responseText);
  	if (response && response.success == false) {
  		Ext.Msg.alert("Error", response.errmsg);
  	}
  } catch (e) {
    alert("Invalid server response:"+r.responseText+'<br/>Exception:'+e);
  }   
	
}

function generalRODSHttpRequestHandler(options,success,response)
{
  if (options.first_calback)
    options.first_calback();
  
  if (success!=true)
  {
    alert("HTTP error ("+response.status+"): "+response.statusText);
  }
  else
  {   
    try {
      var response = Ext.util.JSON.decode(response.responseText);
    } catch (e) {
      alert("Invalid server response:"+response.responseText+'<br/>Exception:'+e);
      return;
    }   
    	
    if (response && response.success == false) {
      Ext.Msg.alert("Error "+response.errcode+" :", response.errmsg);
    }
    else
    {
      if (options.success_calback)
      {
        Ext.callback(options.success_calback, options.scope);
      }
    }
      
  }  
}
    
function RODSResourceBox()
{
  return {
    init: function(init_ruri)
    {  
      this.resources = new Ext.data.Store({
          proxy: new Ext.data.HttpProxy({
              url: 'services/serverQuery.php?action=resources'
          }),
          
          // create reader that reads the Topic records
          reader: new Ext.data.JsonReader({
              successProperty: 'success',
              root: 'que_results',
              totalProperty: 'totalCount',
              id: 'id'
          }, [
              {name: 'id', mapping: 'id'},
              {name: 'name', mapping: 'name'},
              {name: 'type', mapping: 'type'},
              {name: 'zone', mapping: 'zone'},
              {name: 'class', mapping: 'class'},
              {name: 'loc', mapping: 'loc'},
              {name: 'info', mapping: 'info'},
              {name: 'comment', mapping: 'comment'},
              {name: 'vault_path', mapping: 'vault_path'},
              {name: 'free_space', mapping: 'free_space'},
              {name: 'ctime', mapping: 'ctime', type: 'date', dateFormat: 'timestamp'},
              {name: 'mtime', mapping: 'mtime', type: 'date', dateFormat: 'timestamp'}
          ]),
    
          // turn off remote sorting
          remoteSort: false
      });
      
      this.ruri=init_ruri.substring(0,init_ruri.indexOf('/'));
      
      this.resources.on('beforeload', function() {
        this.resources.baseParams = {'ruri': this.ruri};
      }, this);
      
      this.resources.proxy.getConnection().
        on('requestcomplete', jsonErrorResponseHandler);
      
      this.box = new Ext.form.ComboBox({
               fieldLabel: 'Resource',
               store: this.resources, 
               displayField:'name',
               valueFiled: 'name',
               emptyText:'Select a Resource...',
               selectOnFocus:true,
               allowBlank:false,
               hiddenName: 'resource',
               triggerAction: 'all',
               forceSelection:true
      });
      
      this.box.store.on("load",function(store){
        this.clearValue();
        this.setValue(store.getAt(0).data.name);
      },this.box);
      
      this.box.store.load();
    }, //end of function init
    
    // update RURI, and reload, if needed.
    updateRURI: function (new_ruri)
    {
      var newacct=new_ruri.substring(0,new_ruri.indexOf('/'));
        
      if (this.ruri!=newacct) // if acct has changed
      {
        this.ruri=newacct;
        this.resources.load();
      }
    }
  }
}

function RODSFileSearchDialog()
{
  var adv_search_dlg, file_viewer, adv_search_form, adv_search_flds, 
    result_dlg, gridpanel, grid, grid_view, ds, num_newrows, ruri, 
    descendantOnly, partial_name, metaname_store;
  
  function jsonErrorResponseHandler(conn, r, options) {
    
    try {
      var response = Ext.util.JSON.decode(r.responseText);
    	if (response && response.success == false) {
    		Ext.Msg.alert("Error", response.errmsg);
    	}
    } catch (e) {
      alert("Invalid server response:"+r.responseText+'<br/>Exception:'+e);
    }   
  	
  }
  
  return {
    init : function(adv_search_dlg_container,
      result_dlg_container, result_grid_container, file_viewer) {  
      
      this.file_viewer=file_viewer;
      this.descendantOnly=false;
      
      // the column model has information about grid columns
      // dataIndex maps the column to the specific data field in
      // the data store (created below)
      var cm = new Ext.grid.ColumnModel([{
             id: "dir_grid_col_name",
             header: "Name",
             dataIndex: 'name',
             width: 100,
             renderer: renderName,
             //renderer: renderTopic,
             //locked: true, //lock the column
             css: 'white-space:normal;'
          },{
             header: "Collection",
             dataIndex: 'dirname',
             width: 400,
             align: 'left'
          },{
             header: "Owner",
             dataIndex: 'owner',
             width: 100,
             hidden: true
          },{
             header: "Resource",
             dataIndex: 'rescname',
             width: 100,
             hidden: false 
          },{
             header: "Type",
             dataIndex: 'typename',
             width: 100,
             hidden: true      
          },{
             header: "Size",
             dataIndex: 'size',
             width: 50,
             renderer: renderFmtSize,
             align: 'right'
           },{
             header: "Date Created",
             dataIndex: 'ctime',
             width: 150,
             renderer: renderTime,
             align: 'right',
             hidden: true   
          },{
             header: "Date Modified",
             dataIndex: 'mtime',
             width: 200,
             renderer: renderTime,
             align: 'right'
          }]);
      
      // by default columns are sortable
      cm.defaultSortable = true;
      
      // create the Data Store
      this.ds = new Ext.data.Store({
          proxy: new Ext.data.HttpProxy({
              url: 'services/search.php'
          }),
          
          // create reader that reads the Topic records
          reader: new Ext.data.JsonReader({
              successProperty: 'success',
              root: 'que_results',
              totalProperty: 'totalCount'
          }, [
              {name: 'name', mapping: 'name'},
              {name: 'dirname', mapping: 'dirname'},
              {name: 'ruri', mapping: 'ruri'},
              {name: 'owner', mapping: 'owner'},
              {name: 'rescname', mapping: 'rescname'},
              {name: 'size', mapping: 'size', type: 'int'},
              {name: 'fmtsize', mapping: 'fmtsize'},
              {name: 'type', mapping: 'type'},
              {name: 'typename', mapping: 'typename'},
              {name: 'ctime', mapping: 'ctime', type: 'date', dateFormat: 'timestamp'},
              {name: 'mtime', mapping: 'mtime', type: 'date', dateFormat: 'timestamp'}
          ]),

          // turn on remote sorting
          remoteSort: true
      });
      this.ds.setDefaultSort('name', 'desc'); 
      
      this.ds.proxy.getConnection().on('requestcomplete', jsonErrorResponseHandler);
      
      // create the search result grid
      this.grid = new Ext.grid.Grid(result_grid_container, {
          ds: this.ds,
          cm: cm,
          selModel: new Ext.grid.RowSelectionModel({singleSelect:true}),
          enableColLock:false,
          autoWidth:true,
          autoExpandColumn:'dir_grid_col_name'
      });
      
      this.grid.addListener("rowdblclick",function (grid, rowIndex, e) {
        var sm = grid.getSelectionModel();
        var record = sm.getSelected();
        this.file_viewer.view(record, e.getTarget());
      },this);
      
      // render it
      this.grid.render();
      
      // configure footer paging bar
      var gridFoot = this.grid.getView().getFooterPanel(true);
      // add a paging toolbar to the grid's footer
      var paging = new Ext.PagingToolbar(gridFoot, this.ds, {
          pageSize: 100,
          displayInfo: true,
          displayMsg: 'Displaying Files {0} - {1} of {2}',
          emptyMsg: "No file found"
      });
      
      this.gridpanel = new Ext.GridPanel(this.grid, 
        {autoCreate: true, fitToFrame: true, autoScroll: true, title: 'Search Results'} );
      
      this.result_dlg= new Ext.LayoutDialog(result_dlg_container, { 
              width:800,
              height:600,
              shadow:true,
              minButtonWidth:10,
              minWidth:300,
              minHeight:300,
              proxyDrag: true,
              modal: true,
              center: {
    	          autoScroll:true,
    	          //tabPosition: 'top',
    	          closeOnTab: true,
    	          alwaysShowTabs: false,
    	          titlebar: false
    	        }
      });
      var layout = this.result_dlg.getLayout();
      layout.beginUpdate();
      layout.add("center", this.gridpanel);
      layout.endUpdate();
      this.result_dlg.addKeyListener(27, this.result_dlg.hide, this.result_dlg); // ESC can also close the dialog
      
      this.adv_search_form=new Ext.form.Form();
      this.adv_search_flds={};
      this.adv_search_flds['name']=new Ext.form.TextField({
                    width:400,
                    name: 'name',
                    fieldLabel: 'Name',
                    emptyText: 'Name or Partial Name, case sensitive'
                });
      
      this.adv_search_flds['mtime_since_now']=new Ext.form.ComboBox({
                    width:200,
                    name: 'mtime_since_now',
                    fieldLabel: 'Modified Within',
                    emptyText: 'Any Time',
                    forceSelection:true,
                    store: new Ext.data.SimpleStore({
                        fields: ['name', 'val'],
                        data : [  
                                 ['24 hours', 24*3600], 
                                 ['7 days',   7*24*3600], 
                                 ['30 days',  30*24*3600],
                                 ['3 month',  3*30*24*3600],
                                 ['6 month',  6*30*24*3600],
                                 ['One Year', 365*24*3600]
                               ]  
                    }),          
                    displayField: 'name',
                    valueField: 'val', 
                    mode: 'local',
                    triggerAction: 'all',
                    editable: false         
                });              
      
      this.adv_search_flds['owner']=new Ext.form.TextField({
                    width:200,
                    name: 'owner',
                    fieldLabel: 'Owner',
                    emptyText: 'Owner of the file'
                });
      
      this.adv_search_flds['rsrc']=new Ext.form.TextField({
                    width:200,
                    name: 'rsrc',
                    fieldLabel: 'Resource',
                    emptyText: 'Resource of the file'
                });
      
      this.adv_search_flds['descendentOnly']= new Ext.form.Checkbox ({
         	fieldLabel: 'Only', checked: false, boxLabel: 'Under Current Collection'});
      
      this.adv_search_flds['cwd']= new Ext.form.TextField({
                    width:400,
                    name: 'cwd',
                    fieldLabel: 'Current Collection',
                    disabled: true
                });
      
      this.adv_search_form.fieldset(
        {legend:'Attributes', labelWidth:120},
        this.adv_search_flds['name'],
        this.adv_search_flds['mtime_since_now'],
        this.adv_search_flds['owner'],
        this.adv_search_flds['rsrc'],
        this.adv_search_flds['descendentOnly'],
        this.adv_search_flds['cwd']
      );
      
      //Create meta data fields
      this.metaname_store= new Ext.data.Store({
            proxy: new Ext.data.HttpProxy({
                url: 'services/serverQuery.php'
            }),
            
            // create reader that reads the Topic records
            reader: new Ext.data.JsonReader({
                successProperty: 'success',
                root: 'que_results',
                totalProperty: 'totalCount'
            }, [
                {name: 'metaname', mapping: 'name'}
            ]),
  
            // turn off remote sorting
            remoteSort: false
      });
      this.metaname_store.on('beforeload', function() {
          this.metaname_store.baseParams = {'ruri': this.ruri, 
            'action':'metadataname_for_files'};
      }, this);
        
      this.metaname_store.proxy.getConnection().
          on('requestcomplete', jsonErrorResponseHandler);
      
      this.metaop_store = new Ext.data.SimpleStore({
                        fields: ['opname', 'opval'],
                        data : [  
                                 ['=', '='], 
                                 ['>', '>'], 
                                 ['>=', '>='],
                                 ['<',  '<'],
                                 ['<=',  '<='],
                                 ['like', 'like']
                               ]  
                    });          
       
      this.adv_search_flds['metaname']=Array();
      this.adv_search_flds['metaop']=Array();
      this.adv_search_flds['metaval']=Array();
      
      for (var i=0; i<5; i++)
      {
        this.adv_search_flds['metaname'][i]=new Ext.form.ComboBox({
                    width:150,
                    name: 'metaname'+i,
                    fieldLabel: 'Name',
                    emptyText: 'Name',
                    store: this.metaname_store,
                    displayField: 'metaname',
                    valueFiled: 'metaname',
                    triggerAction: 'all'
                });
        this.adv_search_flds['metaop'][i]=new Ext.form.ComboBox({
                    width:80,
                    name: 'metaop'+i,
                    fieldLabel: 'Operator',
                    emptyText: 'Op',
                    store: this.metaop_store,
                    displayField: 'opname',
                    valueFiled: 'opname',
                    triggerAction: 'all'
                });  
        this.adv_search_flds['metaval'][i]=new Ext.form.TextField({
                    width:150,
                    name: 'metaval'+i,
                    fieldLabel: 'Value',
                    emptyText: 'Value'
                });              
      }
      
      this.adv_search_form.fieldset({legend:'Metadata', hideLabels:true}); // open filedset container for metadata
      this.adv_search_form.column({width:170, hideLabels:true},
        this.adv_search_flds['metaname'][0],
        this.adv_search_flds['metaname'][1],
        this.adv_search_flds['metaname'][2],
        this.adv_search_flds['metaname'][3],
        this.adv_search_flds['metaname'][4]
      ); 
      this.adv_search_form.column({width:100, hideLabels:true},
        this.adv_search_flds['metaop'][0],
        this.adv_search_flds['metaop'][1],
        this.adv_search_flds['metaop'][2],
        this.adv_search_flds['metaop'][3],
        this.adv_search_flds['metaop'][4]
      ); 
      this.adv_search_form.column({width:170, hideLabels:true},
        this.adv_search_flds['metaval'][0],
        this.adv_search_flds['metaval'][1],
        this.adv_search_flds['metaval'][2],
        this.adv_search_flds['metaval'][3],
        this.adv_search_flds['metaval'][4]
      ); 
      this.adv_search_form.end(); // close filedset container for metadata
      
      this.adv_search_dlg= new Ext.BasicDialog(adv_search_dlg_container, { 
              width:600,
              height:500,
              shadow:true,
              minButtonWidth:10,
              minWidth:300,
              minHeight:300,
              proxyDrag: true,
              modal: true,
              autoScroll:true,
    	        closeOnTab: true,
    	        alwaysShowTabs: false,
    	        titlebar: false
    	});
      this.adv_search_dlg.addButton("Search",function(){
        this.shwoAdvSearchResult(null, this.ruri);
      }, this);
      this.adv_search_dlg.addKeyListener(27, this.result_dlg.hide, this.result_dlg); // ESC can also close the dialog
      
      // make sure metadata store get reload when account changes
      this.adv_search_dlg.on('beforeshow',function(){
            var cur_acct=this.ruri.substring(0,this.ruri.indexOf('/'));
            if (!this.ruri_acct)
              this.ruri_acct=cur_acct;
            else
            if (this.ruri_acct!=cur_acct) // if acct has changed
            {
              this.metaname_store.reload();
              this.ruri_acct=cur_acct;
            }
            else
            {
              //do nothing if acct hasn't changed 
            }
            return true;
          }, this);
      
      this.adv_search_form.render(this.adv_search_dlg.body)
      
    }, // end of RODSFileSearchDialog::init()
    
    shwoQuickSearchResult: function (html_elem, _ruri)
    {
      this.ruri=_ruri;
      this.result_dlg.show(html_elem);
      
      this.ds.baseParams = {ruri:_ruri, 'descendantOnly': this.descendantOnly,
            'recursive': this.descendantOnly,
            'name': this.partial_name};
      this.ds.load({params:{start:0, limit:100}});
    },
    
    showAdvSearchDialog: function (html_elem, _ruri, _partial_name, _descendantOnly)
    {
      this.ruri=_ruri;
      this.adv_search_flds['cwd'].setValue(
        _ruri.substr(_ruri.indexOf('/')));
        
      this.adv_search_flds['name'].setValue(_partial_name);
      this.adv_search_flds['descendentOnly'].setValue(_descendantOnly);  
      this.adv_search_dlg.show(html_elem);
    },
    
    shwoAdvSearchResult: function (html_elem, _ruri)
    {
      var has_option=false;
      this.ruri=_ruri;
      
      this.partial_name=Ext.util.Format.trim(this.adv_search_flds['name'].getValue());
      if (this.partial_name.length > 0)
      {
        this.ds.baseParams.name=this.partial_name;
        has_option=true;
      }
      if (this.adv_search_flds['mtime_since_now'].getValue())
      {
        var now_date=new Date();
        var now=now_date.getTime()/1000;
        this.ds.baseParams.smtime= now-this.adv_search_flds['mtime_since_now'].getValue();
        this.ds.baseParams.emtime= now;
        has_option=true;
      }
      
      var owner=Ext.util.Format.trim(this.adv_search_flds['owner'].getValue());
      if (owner.length > 0) 
      {
        this.ds.baseParams.owner=owner; 
        has_option=true;
      }
      
      var rsrc=Ext.util.Format.trim(this.adv_search_flds['rsrc'].getValue());
      if (rsrc.length > 0) 
      {
        this.ds.baseParams.rsrcname=rsrc; 
        has_option=true;
      }
      
      if (this.adv_search_flds['descendentOnly'].getValue()===true)
      {
        this.ds.baseParams.recursive=true;    
        this.ds.baseParams.descendantOnly=true;
        has_option=true;
      }
      
      var metadatas=new Array();
      for (var i=0; i<this.adv_search_flds['metaname'].length; i++)
      {
        var meta={};
        meta['name']=Ext.util.Format.trim(this.adv_search_flds['metaname'][i].getValue());
        meta['op']=Ext.util.Format.trim(this.adv_search_flds['metaop'][i].getValue());
        meta['val']=Ext.util.Format.trim(this.adv_search_flds['metaval'][i].getValue());
        if ( (meta['name'].length>0) && (meta['op'].length>0) && 
             (meta['val'].length>0) )
        {
          metadatas.push(meta);
        }
      }
      if (metadatas.length>0)
      {
        this.ds.baseParams.metadata=escape(Ext.util.JSON.encode(metadatas));
        has_option=true;
      }
      
      if (has_option!=false)
      {
        this.result_dlg.show(html_elem);
        this.ds.baseParams.ruri=_ruri;
        this.ds.load({params:{start:0, limit:100}});
      }
    },
    
    refresh: function()
    {
      this.ds.reload();
    }
  }
} // end of RODSFileSearchDialog

function RODSMetadataGrid()
{
  var gridpanel, grid, grid_view, ds, num_newrows;
  
  return {
    init : function(grid_container) {  
      this.num_newrows=0;
      
      // shorthand alias
      var fm = Ext.form, Ed = Ext.grid.GridEditor;
  
      // the column model has information about grid columns
      // dataIndex maps the column to the specific data field in
      // the data store (created below)
      var cm = new Ext.grid.ColumnModel([{
             header: "Name",
             dataIndex: 'name',
             width: 200,
             editor: new Ed(new fm.TextField({
                 allowBlank: false,
                 emptyText: 'Name',
                 emptyClass: 'x-form-empty-field'
             }))
          },{
             header: "Value",
             id: 'meta_grid_col_value',
             dataIndex: 'value',
             width: 200,
             editor: new Ed(new fm.TextField({
                 allowBlank: false
             }))
          },{
             header: "Unit",
             dataIndex: 'unit',
             width: 100,
             editor: new Ed(new fm.TextField({
                 allowBlank: true
             }))
          }
      ]);
      
      // by default columns are sortable
      cm.defaultSortable = true;
      
      // this could be inline, but we want to define the metadata record
      // type so we can add records dynamically
      var MetadataRecord = Ext.data.Record.create([
             // the "name" below matches the tag name to read
             {name: 'id', type: 'string'},
             {name: 'name', type: 'string'},
             {name: 'value', type: 'string'},
             {name: 'unit', type: 'string'},
             {name: 'isnew', type: 'bool'},
             {name: 'isremoved',type: 'bool'}
      ]);
      
      // create the Data Store
      this.ds = new Ext.data.Store({
          proxy: new Ext.data.HttpProxy({
              url: 'services/metadata.php'
          }),
          
          reader: new Ext.data.JsonReader({
              successProperty: 'success',
              root: 'que_results',
              totalProperty: 'totalCount'
          }, MetadataRecord),
          
          // turn off remote sorting
          remoteSort: false
      });
      this.ds.setDefaultSort('name', 'desc'); 
      
      this.ds.on('loadexception', function(a,conn,resp) {
        if (resp.status != 200)
          Ext.Msg.alert('load exception', resp.status+':'+resp.statusText);
        
      });
      
      this.ds.proxy.getConnection().on('requestcomplete', jsonErrorResponseHandler);
      
      // create the editor grid
      //this.grid = new Ext.grid.EditorGrid('metadata-grid', {
      this.grid = new Ext.grid.EditorGrid(grid_container, {
          ds: this.ds,
          cm: cm,
          autoHeight: true,
          loadMask: true,
          selModel: new Ext.grid.RowSelectionModel(),
          enableColLock:false,
          autoExpandColumn:'meta_grid_col_value'
      });
      // if a cell is edited add the record as 'isedited'
      this.grid.on("beforeedit", function(evnt) {
          if (evnt.record.get('isremoved')==true) // cancel edit if row is removed
            evnt.cancel=true;
        }, 
        this
      );
      // if a cell is edited add the record as 'isedited'
      this.grid.on("afteredit", function(evnt) {
          if (evnt.record.get('isnew')==true)
            return;
          else
          {
            if (evnt.value != evnt.originalValue) 
            {
              evnt.record.set('isedited',true); 
            }
          }  
        }, 
        this
      );
      // render it
      this.grid.render();
      
      this.grid_view=this.grid.getView();
      this.grid_view.on('refresh', function(){ // re-apply line-through style to delted records
        for (var i=0; i<this.ds.getCount(); i++)
        {
          if (this.ds.getAt(i).get('isremoved')==true)
          {
            var rowel=this.grid_view.getRow(i);
            rowel.style.textDecoration='line-through';
          }
        }
      }, this);  
      
      // configure header tool bar
      var gridHead = this.grid_view.getHeaderPanel(true);
      var toolbar = new Ext.Toolbar(gridHead);
      toolbar.add(
        {
          icon: "images/add.png",
          text: "Add",
          tooltip: 'Add a new metadata entry',
          cls: 'x-btn-text-icon',
          scope: this,
          handler : function(){
            var p = new MetadataRecord({
                name: 'New Name '+this.num_newrows,
                value: 'New Value '+this.num_newrows,
                unit: '',
                isnew: true,
                id: 'new'+this.num_newrows
            });
            this.num_newrows++;
            this.grid.stopEditing();
            this.ds.insert(0, p);
            this.grid.startEditing(0, 0);
          }
        },
        '-',
        {
          icon: "images/delete.png",
          text: "Remove",
          tooltip: 'Remove an entry',
          cls: 'x-btn-text-icon',
          scope: this,
          handler : function(){
            var selected_rec=this.grid.getSelectionModel().getSelected();
            if (selected_rec!=null)
            {
              if (selected_rec.get('isnew')!=true) // if it's an existing row
              {
                if (selected_rec.get('isremoved')!=true) // if not removed, remove it
                {
                  selected_rec.set('isremoved',true);
                  var index=this.ds.indexOf(selected_rec);
                  if (index >= 0)
                  {
                    var rowel=this.grid_view.getRow(index);
                    rowel.style.textDecoration='line-through';
                  }
                }
                else  // if already removed, un-remove it
                {
                  selected_rec.set('isremoved',false);
                  var index=this.ds.indexOf(selected_rec);
                  if (index >= 0)
                  {
                    var rowel=this.grid_view.getRow(index);
                    rowel.style.textDecoration='';
                  }
                }
              }
              else  // if it's a new row
              {
                this.ds.remove(selected_rec);
              } 
            }
          }
          
        },
        '-',
        {
          icon: "images/arrow_refresh_small.png",
          text: "Reload",
          tooltip: 'Reload and discard all changes',
          cls: 'x-btn-text-icon',
          scope: this,
          handler: this.refresh
        }, 
        '-',
        {
          icon: "images/disk.png",
          text: "Save",
          tooltip: 'Save all changes',
          cls: 'x-btn-text-icon',
          scope: this,
          handler : function(){
            var rec_report='';
            var row_ops=Array();
            
            var del_queue=Array(); //array of id's (string) to be deleted
            var add_queue=Array(); //array of records (record) to be added
            
            for (var i=0; i<this.ds.getCount(); i++) // build del_queue and add_queue
            {
              var rec=this.ds.getAt(i);
              if (rec.get('isnew')==true)
              {
                add_queue.push(rec);
              }
              else
              {
                if (rec.get('isremoved')==true)
                {
                  del_queue.push(rec.get('id'));
                }
                else
                if (rec.get('isedited')==true)
                {
                  del_queue.push(rec.get('id'));
                  add_queue.push(rec);   
                }
                else
                {
                  // nothing need to be done
                }
              }
              // check if there is duplicated names
              if ( rec.get('isnew')==true || rec.get('isedited')==true || 
                    rec.get('isremoved')==true
                 )
              {
                for (var j=0; j < this.ds.getCount(); j++)
                {
                  if (i==j) continue;
                  
                  var rec2=this.ds.getAt(j);
                  if (rec2.get('isremoved')==true) 
                    continue;
                  if (rec.get('name')==rec2.get('name'))
                  {
                    this.grid.getSelectionModel().selectRecords(rec);
                    Ext.MessageBox.alert("Can not continue", "metadata name: '"
                          +rec.get('name')+
                          "' is not unique!");
                    return;
                  }
                }
              }
            }
            
            for(var i=0; i<del_queue.length; i++)
            {
              row_ops.push({op: 'delbyid', id: del_queue[i] });
            }
            
            for(var i=0; i<add_queue.length; i++)
            {
              row_ops.push({op: 'add', 
                  target: { name:  add_queue[i].get('name'),
                            value: add_queue[i].get('value'),
                            unit:  add_queue[i].get('unit')
                          }
              });
            }
                      
            if (row_ops.length>0)
            {
              Ext.MessageBox.wait('Saving metadata changes. Please wait...');
              var batch=escape(Ext.util.JSON.encode(row_ops));
              var conn=new Ext.data.Connection();
              var myparams={ruri: this.ruri,'type': this.type, action:'mod', batch:batch};
              conn.request({url: 'services/metadata.php', params: myparams, 
                callback: function(options,success,r) {
                            Ext.MessageBox.hide();
                            if (success!=true)
                            {
                              alert("HTTP Request Failed ("+
                                r.statusText+'): '+r.responseText);
                            }
                            else
                            {     
                              try {
                                var response = Ext.util.JSON.decode(r.responseText);
                              	if (response && response.success == false) {
                              		Ext.MessageBox.alert("Error", response.errmsg);
                              	}
                              	else
                              	{
                              	  this.ds.reload();  
                              	}
                              } catch (e) {
                                alert("Invalid server response:"+r.responseText+'<br/>Exception:'+e);
                              }   
                            }
                          }, // end of HTTP conn callback  
                scope: this          
              });
            }
          } //end of metadata save handler
        }
      );
      this.gridpanel = new Ext.GridPanel(this.grid, /*'metadata-grid-center',*/ {autoCreate: true, fitToFrame: true, autoScroll: true, title: 'metadata'} );
      
      
    }, // end of RODSMetadataViewer::init()
    
    load: function (_ruri, _type)
    {
      this.ruri=_ruri;
      this.type=_type;
      this.num_newrows=0;
      this.ds.load({params:{ruri:_ruri, 'type':_type, action:'get'}});
    },
    
    refresh: function()
    {
      this.ds.reload();
    }
  }
} // end of RODSMetadataViewer

function RODSFileViewer()
{
  var file_dlg, layout, panels, tab_main, tab_meta, ruri, record, 
    permalink, isImage, metagrid;
  
  return {
    init : function (){ 
        //Ext.QuickTips.init();
        
        this.metagrid=new RODSMetadataGrid();
        this.metagrid.init('fileviewer-tab-meta-grid');
        
        this.file_dlg = new Ext.LayoutDialog("file-dlg", { 
                            width:600,
                            height:400,
                            shadow:true,
                            minButtonWidth:10,
                            minWidth:300,
                            minHeight:300,
                            autoTabs:true,
                            proxyDrag: true,
                            center: {
    	                        autoScroll:true,
    	                        tabPosition: 'bottom',
    	                        titlebar: true,
    	                        title: 'File Details'
    	                      }
                    });
        this.layout = this.file_dlg.getLayout();
        this.layout.beginUpdate();
        
        var main_tb=new Ext.Toolbar(this.file_dlg.getLayout().getRegion('center').titleEl);
        main_tb.add({
            icon: 'images/application_double.png',
            cls: 'x-btn-text-icon bmenu', // icon and text class
            text:'Open',
            tooltip: 'View file in a new browser window',
            cls: 'x-btn-text-icon',
            scope: this,
            handler: this.openFile
                    }); 
        
        this.createReplGridPanel();
        
        this.tab_main=this.layout.add('center', new Ext.ContentPanel('fileviewer-tab-main', {title: 'overview'}));
        // generate some other tabs
        //this.metagrid.gridpanel.setTitle('metadata');
        
        this.tab_meta=this.layout.add('center', this.metagrid.gridpanel);
        this.tab_meta.on('activate', function() {
            this.metagrid.load(this.ruri,1);  
          }, this);
        
        this.tab_repl=this.layout.add('center', this.replGridpanel);
        this.tab_repl.on('activate', function() {
            this.repl_ds.load({params:{ruri:this.ruri}});  
          }, this);  
          
        this.tab_more=this.layout.add('center', new Ext.ContentPanel('fileviewer-tab-more', {title: 'More'}));  
        this.layout.endUpdate(); 
	      
	       
        //permalink_textarea.setDisabled(true); 
          
    },
    
    view : function(_record, html_el){
      this.record=_record;
      this.ruri=this.record.data['ruri'];
      this.permalink=this.getPermaLink();
      this.isImage=this.isFileImage();
       
      this.file_dlg.setTitle(this.record.data['name']);  
      this.setTabMain();
      this.layout.getRegion("center").showPanel("fileviewer-tab-main");
      this.file_dlg.show(html_el);
      this.file_dlg.toFront();
    },
    
    isFileImage : function ()
    {
      var splitname= (this.record.data['name']).split(".");
      var file_ext=splitname.pop();
      if (file_ext.length<1) return false;
      file_ext=file_ext.toLowerCase();
      return (file_ext=='gif' || file_ext == 'jpg' || file_ext == 'jpeg' 
        || file_ext=='png' || file_ext=='bmp');
    },
    
    setTabMain : function ()
    {
      var html= '';
      if (this.isImage==true)
      {
        html=html+'<div id="file-image-preview"><img src="images/loading.gif" /> &nbsp; generating preview...</div> <hr/>';
      }    
      html=html+'<a href="'+this.permalink+'&force_download=true" target="download-frame">Force Download</a> <br/>'+
                '<span class="system-metadata-title">Size:     </span>'+ 
                    this.record.data['fmtsize'] + ' ('+this.record.data['size'] + ' Bytes)<br/>' +
                '<span class="system-metadata-title">RODS URI: </span>'+ 
                    this.ruri + '<br/>' +
                '<span class="system-metadata-title">Resource: </span>'+ 
                                     this.record.data['rescname'] + '<br/>' +
                '<span class="system-metadata-title">Type: </span>'+ 
                                     this.record.data['typename'] + '<br/>' +                     
                '<span class="system-metadata-title">Date Motified:     </span>'+ 
                                     this.record.data['mtime'] + '<br/>' +        
                '<span class="system-metadata-title">Date Created:     </span>'+ 
                                     this.record.data['ctime'] + '<br/>' +
                '<span class="system-metadata-title">Link to file:     </span>'+
                '<input id="permalink-textfld" />';                           
      this.tab_main.setContent(html);
      if (this.isImage==true)
      {
        var current_image=new Image();
        function previewImage()
        {
          var this_image=current_image;
          var preview_div=Ext.get('file-image-preview');
          
          var new_height=this_image.height > 150 ? 150:this_image.height;
          var new_width= Math.round(this_image.width*(new_height/(this_image.height)));
          
          preview_div.dom.innerHTML='<img id="resized-image" src="'+this.src+'" width="'+new_width+'" height="'+new_height+'"/>';
          
          var wrapped = new Ext.Resizable('resized-image', {
              wrap:true,
              //dynamic:true,
              adjustments: 'auto',
              pinned:true,
              preserveRatio: true
          });
        }
        
        function imageError()
        {
          Ext.get('file-image-preview').dom.innerHTML='Unable to generate preview';
        }
        
        function imageAbort()
        {
          Ext.get('file-image-preview').dom.innerHTML='preview canceled';
        }
        
        current_image.onload=previewImage;
        current_image.onerror=imageError;
        current_image.onabort=imageAbort;
        current_image.src=this.permalink;
      }
      
      
      var permalink_textfld=new Ext.form.TextField({width: 450});
      permalink_textfld.setValue(this.permalink); 
      permalink_textfld.applyTo('permalink-textfld');                   
      
      
    },
    
    openFile : function ()
    {
      window.open(this.permalink,'mywindow','width=800,height=600,resizable = yes, scrollbars = yes');
    }, 
    
    getPermaLink : function ()
    {
      var currenturl=location.protocol+'//'+location.host+location.pathname;
      var currenturlpath=currenturl.substring(0,currenturl.lastIndexOf('/'));
      var permalink=currenturlpath+'/rodsproxy/'+this.ruri;
      return permalink;
    },
      
    createReplGridPanel : function()
    {
      // the column model has information about grid columns
      // dataIndex maps the column to the specific data field in
      // the data store (created below)
      var cm = new Ext.grid.ColumnModel([{
             id: "repl_grid_col_replnum",
             header: "#",
             dataIndex: 'repl_num',
             width: 50,
             css: 'white-space:normal;'
          },{
             header: "Chksum",
             dataIndex: 'chk_sum',
             width: 100,
             hidden: true
          },{
             id: "repl_grid_col_rescname",
             header: "Resource",
             dataIndex: 'resc_name',
             width: 100
          },{
             header: "Resc Group",
             dataIndex: 'resc_grp_name',
             width: 100,
             hidden: true 
          },{
             header: "Type",
             dataIndex: 'resc_type',
             width: 100,
             hidden: true 
          },{  
             header: "Class",
             dataIndex: 'resc_class',
             width: 100,
             hidden: true      
          },{
             header: "Location",
             dataIndex: 'resc_loc',
             width: 100,
             hidden: true      
          },{
             header: "Freespace",
             dataIndex: 'resc_freespace',
             width: 100,
             hidden: true      
          },{
             header: "Size",
             dataIndex: 'size',
             width: 50,
             renderer: renderFmtSize,
             align: 'right'
          },{
             header: "Date Created",
             dataIndex: 'ctime',
             width: 150,
             renderer: renderTime,
             align: 'right',
             hidden: true 
          },{
             header: "Up-to-date",
             dataIndex: 'resc_repl_status',
             width: 100,
             renderer: function (value,p,record){
               if (record.data['resc_repl_status']==0)
                 return '<div style="color:red">No</div>';
               else
               if (record.data['resc_repl_status']==1)
                 return '<div style="color:green">Yes</div>';
               else
                 return ''+record.data['resc_repl_status'];    
             },
             align: 'right'
          },{
             header: "Date Modified",
             dataIndex: 'mtime',
             width: 200,
             renderer: renderTime,
             align: 'right'
          }]);
      
      // by default columns are sortable
      cm.defaultSortable = true;
      
      // create the Data Store
      this.repl_ds = new Ext.data.Store({
          proxy: new Ext.data.HttpProxy({
              url: 'services/fileQuery.php?action=replica'
          }),
          
          // create reader that reads the Topic records
          reader: new Ext.data.JsonReader({
              successProperty: 'success',
              root: 'que_results',
              totalProperty: 'totalCount'
          }, [
              {name: 'repl_num', mapping: 'repl_num', type: 'int'},
              {name: 'chk_sum', mapping: 'chk_sum'},
              {name: 'resc_name', mapping: 'resc_name'},
              {name: 'resc_repl_status', mapping: 'resc_repl_status', type: 'int'},
              {name: 'resc_grp_name', mapping: 'resc_grp_name'},
              {name: 'resc_type', mapping: 'resc_type'},
              {name: 'resc_class', mapping: 'resc_class'},
              {name: 'resc_loc', mapping: 'resc_loc'},
              {name: 'resc_freespace', mapping: 'resc_freespace', type: 'int'},
              {name: 'data_status', mapping: 'data_status'},
              {name: 'size', mapping: 'size', type: 'int'},
              {name: 'ctime', mapping: 'ctime', type: 'date', dateFormat: 'timestamp'},
              {name: 'mtime', mapping: 'mtime', type: 'date', dateFormat: 'timestamp'}
          ]),

          // turn on remote sorting
          remoteSort: false
      });
      
      this.repl_ds.proxy.getConnection().on('requestcomplete', jsonErrorResponseHandler);
      
      // create the replica list grid
      this.repl_grid = new Ext.grid.Grid('fileviewer-tab-repl-grid', {
          ds: this.repl_ds,
          cm: cm,
          loadMask: true,
          selModel: new Ext.grid.RowSelectionModel({singleSelect:true}),
          enableColLock:false,
          autoWidth:true,
          autoExpandColumn:'repl_grid_col_rescname'
      });
      
      // render it
      this.repl_grid.render();
      
      var gridHead = this.repl_grid.getView().getHeaderPanel(true);
      var headerToolbar = new Ext.Toolbar(gridHead);
      headerToolbar.add({ 
         icon: "images/add.png",
         text: 'Replicate',
         tooltip: 'Replicate to Additional Resource',
         cls: 'x-btn-text-icon',
         scope: this,
         handler: this.showReplDialog
        }, '-',
        { 
         icon: "images/delete.png",
         text: 'Remove Selected',
         tooltip: 'Remove selected copy from its resource',
         cls: 'x-btn-text-icon',
         scope: this,
         handler: this.removeReplHandler
        }, '-',
        { 
         icon: "images/arrow_refresh_small.png",
         text: 'Refresh List',
         tooltip: 'Remove list of copies',
         cls: 'x-btn-text-icon',
         scope: this,
         handler: function(){
           this.repl_ds.reload();
         }
        }
        
      );
      
      this.replGridpanel = new Ext.GridPanel(this.repl_grid, 
        {autoCreate: true, fitToFrame: true, autoScroll: true,
         title: 'Copies'} );
    }, // end of method RODSFileViewer::createReplGridPanel
    
    removeReplHandler: function(btn)
    {
      if (this.repl_ds.getCount()<2)
      {
        Ext.MessageBox.alert('Failed to remove backup', 
          'There has to be at least one copy for each file');
        return;
      }
      
      var selMod=this.repl_grid.getSelectionModel();
      if (selMod.getCount()<1)
      {
        Ext.MessageBox.alert('Failed to remove backup', 
          'You must select a copy to remove');
        return;
      }
      var selectedResc=selMod.getSelected().get('resc_name');
      
      Ext.MessageBox.wait('Removing backup in progress', 'Please wait');
      
      var conn=new Ext.data.Connection();
      var myparams={ruri: this.ruri,'resource': selectedResc};
      conn.request({url: 'services/repl.php?action=remove', 
        params: myparams, scope: this,
        callback:generalRODSHttpRequestHandler,
        //this property is used by generalRODSHttpRequestHandler
        //this function is called only if everything goes well
        success_calback: function(){
          this.repl_ds.reload();
        },
        //this property is used by generalRODSHttpRequestHandler
        //this function is called regardless of the state.
        first_calback: function(){
          Ext.MessageBox.hide();
        }
      });
    },
    
    showReplDialog: function(btn)
    {
      if (this.replDialog==null)
      {
     	  this.replForm = new Ext.form.Form({
          labelWidth: 100, // label settings here cascade unless overridden
          url:'services/repl.php?action=add',
          timeout: 10
        });
        
        this.rescBox=new RODSResourceBox();
        this.rescBox.init(this.ruri);
        
        this.replForm.add(
          this.rescBox.box
        );
       
        this.replForm.end();
        this.replForm.render('repl-dlg-bd-form');
        
        this.replForm.on("actionfailed", function(form, action) {
          Ext.MessageBox.hide();
          if (action.result)
          {
            var errcode='';
            if (action.result.errcode!=null)
              errcode=action.result.errcode;
            Ext.MessageBox.alert('Failure:'+errcode, action.result.errmsg);
          }
          else
          {
            Ext.MessageBox.alert('Failure:', 
              'Replication request failed with unknown reasons...');
          }
        }, this);
        
        this.replForm.on("actioncomplete", function(form, action) {
          Ext.MessageBox.hide();
          this.replDialog.hide();
          this.repl_ds.reload();
        }, this);
       
        this.replDialog=new Ext.BasicDialog("repl-dlg", {
          height: 130,
          width: 400,
          minHeight: 100,
          minWidth: 150,
          modal: true,
          proxyDrag: true,
          buttonAlign: "center",
          shadow: true
        });
        
        this.replDialog.on('beforeshow',function(){
          this.rescBox.updateRURI(this.ruri);
          return true;
        }, this);
        
        this.replDialog.addKeyListener(27, this.replDialog.hide, this.replDialog); // ESC can also close the dialog
        this.replDialog.addButton('OK', function(){
          if (true==this.replForm.isValid()) 
          {
            Ext.MessageBox.wait('Replication in progress', 'Please wait');
            this.replForm.baseParams = {'ruri': this.ruri};
            this.replForm.submit();
          }
        }, this);    // Could call a save function instead of hiding
        this.replDialog.addButton('Cancel', this.replDialog.hide, this.replDialog);
      }
      
      this.replDialog.show(btn.getEl());
    }
  }; //end of RODSFileViewer's reuturn  
} //end of RODSFileViewer
    
function RodsBrowser(inipath, _ssid)
{
  var layout, grid, tree, root, coll_list_data, browse_srv_script_path, 
      rpath_grid, rpath_tree, rpath_grid_uriobj, file_viewer, force_delete, 
      resources, resource_box, new_file_dialog, upload_dialog, upload_applet_dialog,
      metadata_dialog, metagrid, move_src_parent_node, move_dest_parent_node,
      ssid, quick_search_fld, file_search_dlg;
  rpath_grid=inipath;
  rpath_tree=inipath;
  ssid=_ssid;
  
  function jsonErrorResponseHandler(conn, r, options) {
  	var response = Ext.util.JSON.decode(r.responseText);
  	if (response && response.success == false) {
  		// your error handling code goes here
  		//Ext.MessageBox.alert("Error", response.errors);
  		 if (response.errcode==-301000)
         window.location = 'index.php?redirect='+rpath_grid+'&&errmsg=The Path you tried to access requires authentication!';
       else
       if (response.errcode==-1000)
         window.location = 'index.php?redirect='+rpath_grid+'&&errmsg=Server is not responding, it may be down!';
       else
        Ext.MessageBox.alert("Error", response.errmsg);
  	}
  }
  
  return {
    
    gotoRURI : function (new_ruri){
      this.allGoTo(new_ruri);
    },
    
    getCurrentRURI : function () {
      return rpath_grid;
    },
    
    init : function (){ 
      
      Ext.QuickTips.init();
      
      this.file_viewer=new RODSFileViewer();
	    this.file_viewer.init();
      this.file_search_dlg=new RODSFileSearchDialog();
	    this.file_search_dlg.init('search-dlg',
	      'file-search-result-dlg','file-search-result-grid',
	      this.file_viewer);
      
      //this.createRodsCollDataStore('browse.php?ruri=rods.tempZone:RODS@rt.sdsc.edu:1247/tempZone/home/rods');    
      this.createRodsCollDataStore();
      this.createRodsCollBrowseGrid(coll_list_data);         
      this.createRodsDirTree();
      
      this.layout = new Ext.BorderLayout(document.body, {
	      north: {
	          split:false,
	          initialSize: 32,
	          titlebar: false
	      },
	      west: {
	          split:true,
	          initialSize: 200,
	          minSize: 175,
	          maxSize: 400,
	          titlebar: true,
	          collapsible: true,
              animate: true
	      },
	      center: {
	          titlebar: false,
	          autoScroll:true,
              closeOnTab: true
        }
	    });

      this.layout.beginUpdate();
	    this.layout.add('north', new Ext.ContentPanel('north', 'North'));
	    //this.layout.add('west', new Ext.ContentPanel('west', {title: 'West', closable: false}));
	    this.layout.add('west', new Ext.ContentPanel('tree-div', {title: 'Collections', closable: false, autoScroll:true, fitToFrame:true}));
	    this.layout.add('center', new Ext.GridPanel(grid, 'center2', {title: 'Collections and Files', closable: false}));
	    this.layout.endUpdate();
	    
	    this.createLoginOutDiv();
	    
	    
	  }, // end of RodsBrowser::init 
	  
	  allGoTo : function (new_ruri)
	  {
	    var currentRURI = YAHOO.util.History.getCurrentState( "ruri" );
        if (currentRURI!=new_ruri)
        {
          try {
            //alert("i am going to naviagte with yui history manager:"+new_ruri);
            YAHOO.util.History.navigate( "ruri", new_ruri );
            this.gridGoTo(new_ruri);
          } catch (e)
          {
            //alert("i am going to naviagte without yui history manager:"+new_ruri);
            this.gridGoTo(new_ruri);
          }
        }
        //else
          //alert("last_ruri: "+currentRURI+" already exists!");
    },
    
    gridGoTo: function (new_ruri)
  	{
  	   if (new_ruri!=rpath_grid)
  	   {
         rpath_grid=new_ruri;
         coll_list_data.load({params:{start:0, limit:50}});
         grid.selModel.clearSelections(); //workaround for IE6's wierd behavior
         this.treeGoTo(rpath_grid);

    	 // set new ruri in applet
    	 if (document.myApplet)
         document.myApplet.setRuri(new_ruri);
       }
    },
    
    treeGoTo: function (new_ruri)
  	{
  	   if (new_ruri!=rpath_tree)
  	   {
         var newacctstr=new_ruri.substring(0,new_ruri.indexOf('/'));	    
    	   var oldacctstr=rpath_tree.substring(0,rpath_tree.indexOf('/'));
    	   // if new account and old account matches, just expand the tree
    	   if (newacctstr==oldacctstr) 
  	     {
    	     var path="/"+new_ruri;
    	     tree.expandPath(path);
           tree.selectPath(path);
           rpath_tree=new_ruri;
         }
         // else then re-render the tree
         else
         {
           rpath_tree=new_ruri;
           this.createRodsDirTree();
         }
       }
    },
    
    // refresh a tree node.
    refreshTreeNodeByRURI: function (new_ruri)
    {
      var path="/"+new_ruri;
      tree.expandPath(path, 'id', function(bSuccess, oLastNode){
        oLastNode.reload();    
      });   
    },
    
    // refresh grid.
    refreshGrid: function ()
    {
      coll_list_data.load({params:{start:0, limit:50}});   
    },
	  
	  createMetadataGrid : function ()
	  {
	    if (this.metagrid==null)
      {
        this.metagrid=new RODSMetadataGrid();
        this.metagrid.init('metadata-grid');
      }
    },
	     
	  createLoginOutDiv : function ()
	  {
	    var acct_str=rpath_grid.substring(0,rpath_grid.indexOf('/'));
	    document.getElementById('login-out').innerHTML = acct_str + ' | <a href="logout.php">Sign Out</a>';
	  },
	  
	  moveCallBackHandler: function (options,success,response)
    {
      Ext.MessageBox.hide();
      var resp;
      var need_refresh_tree_nodes=false;
          
      try {
         resp= Ext.util.JSON.decode( response.responseText );
      } catch (e) {
        alert("Invalid server response:"+response.responseText+'<br/>Exception:'+e);
      }   
      if (!resp['success'])
      {
        var title='Failure: ';
        var msg='';
        
        if (!resp['batch_status'])
        {
          if (resp['errcode']!=null)
            title=title+resp['errcode'];
          if (resp['errmsg']!=null)
            msg=resp['errmsg'];
        }
        else
        {
          msg='<b>Some files/collections could not be moved:</b> <br/> \n';
          for (var i=0; i<resp['batch_status'].length; i++)
          {
            var thisstatus=resp['batch_status'][i];
            if (thisstatus['success']==false)
            {
              var name=thisstatus['src'].substring(thisstatus['src'].lastIndexOf('/')+1);
              if (thisstatus.type==0)
               name=name+'/';
              msg=msg+'&nbsp;&nbsp;-&nbsp;<b>'+name+'</b>:'+thisstatus['errmsg']+'<br/> \n';
            }
          }
        }  
          
        Ext.MessageBox.show({
        	title: title,
        	msg: msg,
        	width:300,
        	buttons: Ext.MessageBox.OK
        });
      }
      
      for (var i=0; i<resp['batch_status'].length; i++)
      {
        var thisstatus=resp['batch_status'][i];
        if (thisstatus['type']==0)
        {
          need_refresh_tree_nodes=true;
          break;
        }
      }
      
      if (need_refresh_tree_nodes==true)
      {
        if (this.move_src_parent_node!=null)
          this.move_src_parent_node.reload();
        if (this.move_dest_parent_node!=null)
          this.move_dest_parent_node.reload();
      }
     
      tree.selectPath(this.move_dest_parent_node.getPath());
    }, 
	   
	  createRodsDirTree : function ()
	  {
	    if (tree) 
	    { 
	      Ext.destroy(tree);
        tree.getEl().update("");
	    }
	    
	    tree = new Ext.tree.TreePanel('tree-div', {
        animate:true, 
        loader: new Ext.tree.RODSTreeLoader({dataUrl:'services/dir_tree.php'}),
        enableDD:true,
        ddGroup : 'GridDD',
        ddAppendOnly: true, // can't drop in-between tree nodes
        rootVisible : false,
        containerScroll: true
      });
      
      // add event listeners
      tree.getSelectionModel().addListener('selectionchange', 
        function(selModel,node){
          if (node)
          {
            var path= node.getPath();
            var newruri=path.substring(1,path.length);
            this.allGoTo(newruri);
          }
      },this);
      
       
      
      // set the root node
      var root_id=rpath_tree.substring(0,rpath_tree.indexOf('/'));
      root = new Ext.tree.AsyncTreeNode({
        text: 'Root',
        draggable:false,
        id: root_id
      });
      tree.setRootNode(root);
      //root.attributes.children=
      //
      
      tree.on('beforenodedrop',function(dropEvent){
        if (dropEvent.data.grid!=null) // if coming from grid
        {
          var selmodel=grid.getSelectionModel();  
          var records=selmodel.getSelections();
          var move_queue=new Array();
          for(var i=0; i<records.length; i++)
          {
            var move_que_item={};
            move_que_item['src']=rpath_grid+'/'+records[i].get('name');
            move_que_item['type']=records[i].get('type');
            move_que_item['dest']=dropEvent.target.getPath().substring(1)+'/'+records[i].get('name');
            if (move_que_item['src']!=move_que_item['dest'])
              move_queue.push(move_que_item);
          }
          if (move_queue.length>0)
          {
            this.move_src_parent_node=tree.getSelectionModel().getSelectedNode();
            this.move_dest_parent_node=dropEvent.target;
            Ext.MessageBox.wait('Moving in progress. Please wait...');
            var conn=new Ext.data.Connection();
            var myparams={batch: escape(Ext.util.JSON.encode(move_queue))};
            conn.request({url: 'services/rename.php', params:myparams, callback: this.moveCallBackHandler, scope: this});
          }
          else
            dropEvent.cancel=true;
        }
        else
        if (dropEvent.data.node!=null)  
        {  
          var move_queue=new Array();
          var move_que_item={};
          move_que_item['src']=dropEvent.data.node.getPath().substring(1);
          move_que_item['type']=0;
          move_que_item['dest']=dropEvent.target.getPath().substring(1)+'/'+dropEvent.data.node.id;
          if (move_que_item['src']!=move_que_item['dest'])
            move_queue.push(move_que_item); 
          if (move_queue.length>0)
          {
            this.move_src_parent_node=tree.getSelectionModel().getSelectedNode().parentNode;
            this.move_dest_parent_node=dropEvent.target;
            Ext.MessageBox.wait('Moving in progress. Please wait...');
            var conn=new Ext.data.Connection();
            var myparams={batch: escape(Ext.util.JSON.encode(move_queue))};
            conn.request({url: 'services/rename.php', params:myparams, callback: this.moveCallBackHandler, scope: this});
          }
          else
            dropEvent.cancel=true;       
        }
        else
        {
          Ext.Msg.alert('drop event error', 'Couldn\'t determin what was droppped!');          
          dropEvent.cancel=true;
        }
      }, this);
  
      // render the tree
      tree.render();
      //root.expand(); 
      tree.expandPath('/'+rpath_tree);
      tree.selectPath('/'+rpath_tree);
    },
	  
	  createRodsCollDataStore : function() 
	  {
	    // create the Data Store
      coll_list_data = new Ext.data.Store({
          proxy: new Ext.data.HttpProxy({
              url: 'services/dir_grid.php'
          }),
          
          // create reader that reads the Topic records
          reader: new Ext.data.JsonReader({
              successProperty: 'success',
              root: 'que_results',
              totalProperty: 'totalCount',
              id: 'id'
          }, [
              {name: 'name', mapping: 'name'},
              {name: 'owner', mapping: 'owner'},
              {name: 'rescname', mapping: 'rescname'},
              {name: 'ruri', mapping: 'ruri'},
              {name: 'size', mapping: 'size', type: 'int'},
              {name: 'fmtsize', mapping: 'fmtsize'},
              {name: 'type', mapping: 'type'},
              {name: 'typename', mapping: 'typename'},
              {name: 'ctime', mapping: 'ctime', type: 'date', dateFormat: 'timestamp'},
              {name: 'mtime', mapping: 'mtime', type: 'date', dateFormat: 'timestamp'}
          ]),

          // turn on remote sorting
          remoteSort: true
      });
      coll_list_data.setDefaultSort('mtime', 'desc'); 
      
      coll_list_data.on('beforeload', function() {
           coll_list_data.baseParams = {'ruri': rpath_grid};
           document.title='rods://'+rpath_grid;
       });
      
      /* 
      coll_list_data.on('loadexception', function() {
        var resp=coll_list_data.reader.read();
        alert("loading failed!"+resp.errors);
      });
      */
      
      coll_list_data.proxy.getConnection().on('requestcomplete', jsonErrorResponseHandler);

	  },
	  createRodsCollBrowseGrid : function(ds) 
	  {
	    // the column model has information about grid columns
      // dataIndex maps the column to the specific data field in
      // the data store
      var cm = new Ext.grid.ColumnModel([{
             id: "dir_grid_col_name",
             header: "Name",
             dataIndex: 'name',
             width: 400,
             renderer: renderName,
             //renderer: renderTopic,
             //locked: true, //lock the column
             css: 'white-space:normal;'
          },{
             header: "Owner",
             dataIndex: 'owner',
             width: 100,
             hidden: true
          },{
             header: "Resource",
             dataIndex: 'rescname',
             width: 100,
             hidden: false 
          },{
             header: "Type",
             dataIndex: 'typename',
             width: 100,
             hidden: true      
          },{
             header: "Size",
             dataIndex: 'size',
             width: 100,
             renderer: renderFmtSize,
             align: 'right'
           },{
             header: "Date Created",
             dataIndex: 'ctime',
             width: 200,
             renderer: renderTime,
             align: 'right',
             hidden: true   
          },{
             header: "Date Modified",
             dataIndex: 'mtime',
             width: 200,
             renderer: renderTime,
             align: 'right'
          }]);

      // by default columns are sortable
      cm.defaultSortable = true;
      

      // create the editor grid
      grid = new Ext.grid.Grid('rods-browser-grid', {
          ds: ds,
          cm: cm,
          selModel: new Ext.grid.RowSelectionModel({singleSelect:false}),
          enableColLock:false,
          autoWidth:true,
          enableDrag: true, 
          ddGroup : 'GridDD',
          autoExpandColumn:'dir_grid_col_name'
      });
      
      // drag and drop support
      grid.enableDragDrop = true;
      grid.getDragDropText = function(){
        var selected_items = this.getSelectionModel().getSelections();
        var numfiles = 0;
        var numdirs = 0;
        for (var i=0; i<selected_items.length; i++)
        {
          if (selected_items[i].get('type')==0)
          {
            numdirs++;
          }
          else
            numfiles++;
        }
        if (numdirs<1)
          return 'Move '+numfiles+' Files';
        else
        if (numfiles<1)
          return 'Move '+numdirs+' Collections';
        else
          return 'Move '+numfiles+' Files and '+numdirs+' Collections';
      }
      
      function rowdblclickHandler(grid, rowIndex, e)
      {
        var sm = grid.getSelectionModel();
        sm.selectRow(rowIndex,false);
        var record = sm.getSelected();
        
        if (record.data['type']==0)
        {
          var newrpath='';
          if (rpath_grid.charAt(rpath_grid.length-1)=='/')
            newrpath=rpath_grid+record.data['name'];
          else
            newrpath=rpath_grid+'/'+record.data['name'];
          this.allGoTo(newrpath);
        }  
        else
        { 
          //alert ("file clicked! filename=" + record.data['name']); 
          //file_dlg.setTitle(record.data['name']);  
          //file_dlg.show(e.getTarget());
          
          this.file_viewer.view(record, e.getTarget());
        }
      }
      grid.addListener("rowdblclick",rowdblclickHandler,this);
      
      // render it
      grid.render();

      // configure header tool bar
      var gridHead = grid.getView().getHeaderPanel(true);
      var headerToolbar = new Ext.Toolbar(gridHead);
      headerToolbar.addButton({
          pressed: false,
          icon: "images/check-mark-16px.png",
          enableToggle:true,
          text: 'Select All',
          tooltip: 'select/deselect files and directories under current view',
          cls: 'x-btn-text-icon',
          toggleHandler: selectAllNone
      });
      // add the browseUp button
      headerToolbar.addSeparator();
      headerToolbar.addButton({ 
         icon: "images/folder_up.gif",
         text: 'Browse Up',
         tooltip: 'Browse to it\'s parent collection',
         cls: 'x-btn-text-icon',
         scope: this,
         handler: browseUp
      });
      
      // add the New and Deletion button
      headerToolbar.add('-',
        {  // "New" botton with menu
          cls: 'x-btn-text-icon bmenu', 
          icon: "images/add.png",
          tooltip: 'Create new file or collection',
          text:'New',
          menu: {
            items:[
              {  // "new collection" menu item
                text: 'Collection',
                icon: 'images/folder_add.png',
                cls: 'x-btn-text-icon blist',
                handler: mkdirDialog
              },
              {  // "new text file" menu item
                text: 'File',
                icon: 'images/page_white_add.png',
                cls: 'x-btn-text-icon blist',
                handler: showNewFileDialog
              }
            ]
          }
        }, 
        '-',
        new Ext.Toolbar.MenuButton ({ // 'Delete' button with options
          id: 'delete-button',
          icon: "images/delete.png",
          text: 'Delete',
          tooltip: 'Delete files or collections',
          cls: 'x-btn-text-icon blist',
          handler: deletionDialog,
          menu: {       
            id: 'delete-option',
            items: [
                // stick any markup in a menu
                new Ext.menu.CheckItem({
                    text: 'Force/Permenant Delete',
                    checked: false,
                    checkHandler: onForceDeleteCheck
                })
            ]
          }
        }),
        '-',
        {  // "Upload" botton with menu
          cls: 'x-btn-text-icon bmenu', 
          icon: "images/upload.png",
          tooltip: 'Upload files or directories from local disk to RODS server',
          text:'Upload',
          menu: {
            items:[
              {  // "new collection" menu item
                text: 'Single File',
                icon: 'images/page_white_get.png',
                cls: 'x-btn-text-icon blist',
                handler: showUploadDialog
              },
              {  // "new text file" menu item
                text: 'Files and Directories',
                icon: 'images/folder-upload.png',
                cls: 'x-btn-text-icon blist',
                handler: showUploadAppletDialog,
                scope: this
              }
            ]
          }
        }, 
        '-',
        {  // "More ..." botton with menu
          cls: 'x-btn-text bmenu', 
          tooltip: 'Advanced options',
          text:'More ...',
          menu: {
            items:[
              {  
                text: 'Metadata',
                icon: 'images/table.png',
                cls: 'x-btn-text-icon blist',
                scope: this,
                handler: showMetadataDialog
              },
              {  
                text: 'Replicate',
                icon: 'images/disk_multiple.png',
                cls: 'x-btn-text-icon blist',
                scope: this,
                handler: showReplBulkDialog
              }
            ]
          }
        } 
        
      );   
      
      headerToolbar.addFill();
      
      this.quick_search_fld = new Ext.form.TwinTriggerField({
            name: 'search_by_name',
            width:250,
            emptyText: 'Search By Name...',
            value: '',
            selectOnFocus: true, 
            allowBlank:true,
            trigger1Class: 'x-form-search-trigger',
            trigger2Class: 'x-form-trigger',
            fileSearchDlg: this.file_search_dlg,
            onTrigger1Click: function (evnt){
              var partial_name=Ext.util.Format.trim(this.getRawValue());
              if (partial_name.length<1)
                return;
              this.fileSearchDlg.partial_name=partial_name;
              this.fileSearchDlg.shwoQuickSearchResult(
                evnt.getTarget(), rpath_grid);
            },
            onTrigger2Click: function (evnt){
              if (!this.menu)
              {
                this.menu=new Ext.menu.Menu();
                this.menu.add(
                  new Ext.menu.CheckItem({
                      text: 'Under Current Collection Only',
                      scope: this, // note the scope here is the TwinTriggerField
                      checked: false,
                      checkHandler: function (item, checked){
                        if (checked==true)
                          this.fileSearchDlg.descendantOnly=true;
                        else 
                          this.fileSearchDlg.descendantOnly=false;    
                      }
                  }),
                  new Ext.menu.Item({ 
                    text: 'Advance Search',
                    scope: this,
                    handler: function (evnt){
                      var partial_name=Ext.util.Format.trim(this.getRawValue());
                      this.fileSearchDlg.showAdvSearchDialog(
                        this.getEl(), rpath_grid, partial_name, 
                        this.fileSearchDlg.descendantOnly);
                    }
                  })
                ) 
              }
              this.menu.show(evnt.getTarget());  
            }
      });
      this.quick_search_fld.addListener('specialkey',function(this_fld,evnt) {
        if (evnt.getKey()==Ext.EventObject.RETURN)  
          this_fld.onTrigger1Click(evnt);
      });
      headerToolbar.addField(this.quick_search_fld);
      
      // configure footer paging bar
      var gridFoot = grid.getView().getFooterPanel(true);
      // add a paging toolbar to the grid's footer
      var paging = new Ext.PagingToolbar(gridFoot, ds, {
          pageSize: 50,
          displayInfo: true,
          displayMsg: 'Displaying objects {0} - {1} of {2}',
          emptyMsg: "This collection is empty"
      });
      
      // trigger the data store load
      coll_list_data.load({params:{start:0, limit:50}});
      // all/none button handler
      function selectAllNone(btn, pressed){
          if (pressed==true) 
          {
            grid.selModel.selectAll();
            btn.setText("Select None");
          }
          else
          { 
            grid.selModel.clearSelections();
            btn.setText("Select All");
          }
          grid.getView().refresh();
      }
      
      function browseUp()
	    {
	      var lastindex=rpath_grid.lastIndexOf('/');
	      
	      if (lastindex>1)
	      {
	        var newrpath=rpath_grid.substr(0,lastindex);
	        this.allGoTo(newrpath);
	      }
      }
      
      function showUploadDialog(btn)
      {
        if (upload_dialog==null)
        {
       	  var uploadForm = new Ext.form.Form({
            labelWidth: 100, // label settings here cascade unless overridden
            fileUpload : true,
            url:'services/upload.php',
            timeout: 10,
            baseParams: {ruri: rpath_grid}
          });
          
          var myRescBox=new RODSResourceBox();
          myRescBox.init(rpath_grid);
          
          uploadForm.add(
            new Ext.form.TextField({
               fieldLabel: 'File',
               name: 'file',
               id: 'fileUploadField',
               width:250,
               inputType: 'file',
               selectOnFocus: true, 
               allowBlank:false
            }),
            myRescBox.box
          );
         
         uploadForm.end();
         uploadForm.render('file-upload-dlg-bd-form');
         
         upload_dialog=new Ext.BasicDialog("file-upload-dlg", {
           height: 150,
           width: 400,
           minHeight: 100,
           minWidth: 150,
           modal: true,
           proxyDrag: true,
           buttonAlign: "center",
           shadow: true
          });
          
          upload_dialog.uploadForm=uploadForm;
          upload_dialog.resourcebox=myRescBox;
          
          upload_dialog.on('beforeshow',function(){
            this.resourcebox.updateRURI(rpath_grid);
            return true;
          });
          
          upload_dialog.addKeyListener(27, upload_dialog.hide, upload_dialog); // ESC can also close the dialog
          upload_dialog.addButton('OK', uploadHandler, upload_dialog);    // Could call a save function instead of hiding
          upload_dialog.addButton('Cancel', upload_dialog.hide, upload_dialog);
        }
        else
        {
          fileUploadField=upload_dialog.uploadForm.findField('fileUploadField'); 
          fileUploadField.setRawValue('');
        }
        
        //upload_dialog.resourcebox.setValue(resources.acct_str.getAt(0).data.name);
        upload_dialog.show(btn.getEl());
      }
      
      function showUploadAppletDialog(btn)
      {
        if (upload_applet_dialog==null)
        {
          upload_applet_dialog=new Ext.BasicDialog("upload-applet-dlg", {
           height: 410,
           width: 710,
           minHeight: 100,
           minWidth: 150,
           modal: true,
           proxyDrag: true,
           buttonAlign: "center",
           shadow: true
          });


          Ext.get('upload-applet-dlg-bd-main').dom.innerHTML=
         '<applet name="myApplet" CODE="edu.sdsc.grid.gui.applet.UploadApplet.class" archive="applets/UploadApplet.jar,applets/jargon.jar,applets/json.jar,applets/hsqldb.jar" WIDTH="650" HEIGHT="300" mayscript>'+
            '   <param name="ruri" value="irods://'+rpath_grid+'" />'+
            '   <param name="ssid" value="'+ssid+'" />'+
            '</applet>';
          upload_applet_dialog.addKeyListener(27, upload_applet_dialog.hide, upload_applet_dialog); // ESC can also close the dialog
        }//if
 
        upload_applet_dialog.show(btn.getEl());
      }
      
      function showMetadataDialog(btn)
      {
        var sm = grid.getSelectionModel();
        if (sm.hasSelection()==false) 
          return;
          
        var record = sm.getSelected();
        // deselect all but the first selected item
        sm.selectRecords([record],false);
        var selected_ruri=rpath_grid+'/'+record.data['name'];
        var selected_type=record.data['type'];
        var title='Metadata: '+record.data['name'];
        if (selected_type==0)
        {
          title=title+'/';
        }
        
        if (metadata_dialog==null)
        {
          if (this.metagrid==null)
          {
            this.createMetadataGrid();
          }
          metadata_dialog= new Ext.LayoutDialog("metadata-dlg", { 
                            width:600,
                            height:400,
                            shadow:true,
                            minButtonWidth:10,
                            minWidth:300,
                            minHeight:300,
                            proxyDrag: true,
                            modal: true,
                            center: {
    	                        autoScroll:true,
    	                        //tabPosition: 'top',
    	                        closeOnTab: true,
    	                        alwaysShowTabs: false,
    	                        titlebar: false
    	                      }
                    });
          var layout = metadata_dialog.getLayout();
          layout.beginUpdate();
          layout.add("center", this.metagrid.gridpanel);
          layout.endUpdate();
        }
        
        this.metagrid.load(selected_ruri,selected_type);
        
        metadata_dialog.setTitle(title); 
        metadata_dialog.show();
      }
      function showNewFileDialog(btn)
      {
        if (new_file_dialog==null)
        {
       	  var newFileForm = new Ext.form.Form({
            labelWidth: 100, // label settings here cascade unless overridden
            url:'new-file.php'
          });
          
          var myRescBox=new RODSResourceBox();
          myRescBox.init(rpath_grid);
          
          newFileForm.add(
            new Ext.form.TextField({
               fieldLabel: 'Name',
               name: 'name',
               width:250,
               value: 'Newfile.txt',
               selectOnFocus: true, 
               allowBlank:false
            }),
            myRescBox.box
          );
         
         newFileForm.end();
         newFileForm.render('new-file-dlg-bd-form');
         
         new_file_dialog=new Ext.BasicDialog("new-file-dlg", {
           height: 150,
           width: 400,
           minHeight: 100,
           minWidth: 150,
           modal: true,
           proxyDrag: true,
           buttonAlign: "center",
           shadow: true
          });
          
          new_file_dialog.newFileForm=newFileForm;
          new_file_dialog.resourcebox=myRescBox;
          
          new_file_dialog.on('beforeshow',function(){
            this.resourcebox.updateRURI(rpath_grid);
            return true;
          });
          
          new_file_dialog.addKeyListener(27, new_file_dialog.hide, new_file_dialog); // ESC can also close the dialog
          new_file_dialog.addButton('OK', newFileHandler, new_file_dialog);    // Could call a save function instead of hiding
          new_file_dialog.addButton('Cancel', new_file_dialog.hide, new_file_dialog);
       }
        
         new_file_dialog.show(btn.getEl());
      }
      
      function mkdirDialog (btn)
      {
        Ext.MessageBox.show({
             	title: 'Create New Collection',
             	msg: 'Please enter new collection name',
             	prompt: true,
             	width:400,
             	buttons: Ext.MessageBox.OKCANCEL,
             	animEl: btn.getEl(),
             	fn: mkdirHandler
        });    
      }
      
      function showReplBulkDialog(btn)
      {
        if (this.replDialog==null)
        {
       	  this.replForm = new Ext.form.Form({
            labelWidth: 100, // label settings here cascade unless overridden
            timeout: 10
          });
          
          this.replRescBox=new RODSResourceBox();
          this.replRescBox.init(rpath_grid);
          
          this.replForm.add(
            this.replRescBox.box
          );
         
          this.replForm.end();
          this.replForm.render('repl-bulk-dlg-bd-form');
          
          this.replDialog=new Ext.BasicDialog("repl-bulk-dlg", {
            height: 270,
            width: 400,
            minHeight: 100,
            minWidth: 150,
            modal: true,
            proxyDrag: true,
            buttonAlign: "center",
            shadow: true
          });
          
          this.replDialog.on('beforeshow',function(){
            this.replRescBox.updateRURI(rpath_grid);
            return true;
          }, this);
          
          this.replDialog.addKeyListener(27, this.replDialog.hide, this.replDialog); // ESC can also close the dialog
          this.replDialog.addButton('OK', function(btn){
            var sm = grid.getSelectionModel();
            var records=sm.getSelections();
            var files=new Array();
            var dirs=new Array();
            for (var i=0; i<records.length; i++)
            {
              if (records[i].data['type']==0) 
                dirs.push(records[i].data['ruri']);
              else
                files.push(records[i].data['ruri']);
            }
            
            this.replDialog.hide();
            
            Ext.MessageBox.wait('Schedule replication in progress', 
              'Please wait...');
            
            var conn=new Ext.data.Connection();
            var myparams={'ruri': rpath_grid, 
              'files[]': files, 'dirs[]': dirs, 
              'resc': this.replRescBox.box.getValue()};
            conn.request({url: 'services/replBulk.php', 
              params: myparams, scope: this,
              callback:generalRODSHttpRequestHandler,
              //this property is used by generalRODSHttpRequestHandler
              //this function is called only if everything goes well
              success_calback: function(){
                
              },
              //this property is used by generalRODSHttpRequestHandler
              //this function is called regardless of the state.
              first_calback: function(){
                Ext.MessageBox.hide();
              }});
          }, this);    
          this.replDialog.addButton('Cancel', this.replDialog.hide, this.replDialog);
        }
        
        var sm = grid.getSelectionModel();
        var records=sm.getSelections();
        if (records.length>0)
        {
          var list = '';
          var numfile=0, numcoll=0;
          for (var i=0; i<records.length; i++)
          {
            if (i<5)
            {
              var itemname=records[i].data['name'];
              if (records[i].data['type']==0)
                itemname=itemname+'/';
              list=list+'&nbsp;&nbsp;&nbsp;&nbsp;'+itemname+'<br/>';
            }
            if (records[i].data['type']==0) 
              numcoll++;
            else
              numfile++;
          }
          if (records.length>5)
            list=list+'&nbsp;&nbsp;&nbsp;&nbsp;... <br/>';
          
          var dialog_msg='<b>Schedule replications for the following '+numfile+
               '  files and '+numcoll+' collections?</b> <br/>'+list;  
          
          var el=Ext.get("repl-bulk-dlg-bd-desc");
          el.update(dialog_msg);
          
          this.replDialog.show(btn.getEl());
        }
      }
      
      function mkdirHandler(btn, dirname)
      {
        dirname=dirname.replace(/^\s+|\s+$/g, '');
        if ( (btn!='ok')||(dirname.length<1) ) return; 
        
        Ext.MessageBox.wait('Creation in progress. Please wait...');
        
        var conn=new Ext.data.Connection();
        var myparams={ruri: rpath_grid,'name': dirname};
        conn.request({url: 'services/mkdir.php', params: myparams, callback: generalAjaxCallback});
      }
      
      function uploadHandler()
      {
        if (true==this.uploadForm.isValid()) 
        {
          Ext.MessageBox.wait('uploading in progress', 'Please wait');
          this.uploadForm.baseParams = {'ruri': rpath_grid};
          this.uploadForm.submit({ 
              // callback handler if submit has been successful    
              success: function(uploadForm, action){ 
                  Ext.MessageBox.hide();
                  upload_dialog.hide();
                  coll_list_data.reload();
              },                                
              failure: function(uploadForm, action) {
                  Ext.MessageBox.hide();
                  var errcode='';
                  if (action.result.errcode!=null)
                    errcode=action.result.errcode;
                  Ext.MessageBox.alert('Failure:'+errcode, action.result.errmsg);
              }
          });
        }
      }
      
      function newFileHandler()
      {
        if (true==this.newFileForm.isValid()) 
        {
          var forminputs=this.newFileForm.getValues();
          new_file_dialog.hide();
          Ext.MessageBox.wait('Creation in progress. Please wait...');
          var conn=new Ext.data.Connection();
          var myparams={'ruri': rpath_grid, 'resc': forminputs.resource, 'name':forminputs.name};
          conn.request({url: 'services/newfile.php', params: myparams, callback: generalAjaxCallback});
        }
      }
      
      function onForceDeleteCheck(item, checked)
      {
        if (checked==true)
         force_delete=true 
        else
         force_delete=false;
      }
      
      function deletionDialog(btn)
      {
        var sm = grid.getSelectionModel();
        var records=sm.getSelections();
        if (records.length>0)
        {
          var list = '';
          var numfile=0, numcoll=0;
          for (var i=0; i<records.length; i++)
          {
            if (i<5)
            {
              var itemname=records[i].data['name'];
              if (records[i].data['type']==0)
                itemname=itemname+'/';
              list=list+'&nbsp;&nbsp;&nbsp;&nbsp;'+itemname+'<br/>';
            }
            if (records[i].data['type']==0) 
              numcoll++;
            else
              numfile++;
          }
          if (records.length>5)
            list=list+'&nbsp;&nbsp;&nbsp;&nbsp;... <br/>';
          if (force_delete==true)
            var dialog_msg='<b>Permenantly delete the following '+numfile+
               '  files and '+numcoll+' collections?</b> <br/>'+list;  
          else
            var dialog_msg='<b>Move the following '+numfile+
             '  files and '+numcoll+' collections to trash?</b> <br/>'+list;  
        
          Ext.MessageBox.show({
             	title: 'File/Collection Deletion',
             	msg: dialog_msg,
             	width:400,
             	buttons: Ext.MessageBox.OKCANCEL,
             	animEl: btn.getEl(),
             	fn: deletionHandler
          }); 
        }
      }
      
      function deletionHandler(btn)
      {
        if (btn!='ok') return;
        
        var sm = grid.getSelectionModel();
        var records=sm.getSelections();
        var files=new Array();
        var dirs=new Array();
        for (var i=0; i<records.length; i++)
        {
          if (records[i].data['type']==0) 
            dirs.push(records[i].data['name']);
          else
            files.push(records[i].data['name']);
        }
        
        
        Ext.MessageBox.wait('Deletion in progress. Please wait...');
        
        var conn=new Ext.data.Connection();
        var myparams={ruri: rpath_grid,'files[]': files, 'dirs[]': dirs};
        if (force_delete==true)
          myparams.force=true;
        conn.request({url: 'services/delete.php', params: myparams, callback: generalAjaxCallback});
      }
      
      function generalAjaxCallback(options,success,response)
      {
        Ext.MessageBox.hide();
        var resp;
        try {
           resp= Ext.util.JSON.decode( response.responseText );
        } catch (e) {
          alert("Invalid server response:"+response.responseText+'<br/>Exception:'+e);
        }   
           if (!resp['success'])
           {
             var title='Failure: ';
             var msg='';
             
             if (resp['errcode']!=null)
               title=title+resp['errcode'];
             
             if (resp['errmsg']!=null)
               msg=resp['errmsg'];
             else 
             if (resp['log']!=null)
               msg=resp['log'];
               
             Ext.MessageBox.show({
             	title: title,
             	msg: msg,
             	width:300,
             	buttons: Ext.MessageBox.OK
             });
           }
           else
           {
             coll_list_data.reload();
             var selected_node=tree.getSelectionModel().getSelectedNode();
             if (selected_node)
               selected_node.reload();
           }
        
      } 
    } // end of createRodsCollBrowseGrid
	    
	    
               
	 }; // end of RodsBrowser::return
} // end of RodsBrowser


