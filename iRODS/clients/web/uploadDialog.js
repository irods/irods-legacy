var UploadForm=function(){
    
    var upload, upload_button;
    return {
        init : function(){
          var tabs = new Ext.TabPanel('upload_form_tabs');  
          tabs.resizeTabs=true;
          var tab1= tabs.addTab('upload_form_tabs-1', "Basic");
          
          var tab2= tabs.addTab('upload_form_tabs-2', "Advanced (applet)");
          var updater = tab2.getUpdateManager();
          updater.setDefaultUrl('uploadApplet.php?ruri='+ruri);
          tab2.on('activate', updater.refresh, updater, true);
          
          tabs.activate('upload_form_tabs-1');
          this.makeBasicUploadTab();
          tabs.autoSizeTabs();
        },
          
        makeBasicUploadTab : function(){  
            Ext.QuickTips.init();

            // turn on validation errors beside the field globally
            Ext.form.Field.prototype.msgTarget = 'side';
            
            var upload = new Ext.form.Form({
                id: 'upload_form_data',
                labelAlign: 'right'
            });
            
            upload.column(
                {width:530, style:'margin-left:10px'}
            );
            
            upload.fieldset(
                {id:'dest_disp', legend:'Destination RODS URI', hideLabels: true},
                new Ext.form.TextField({
                    width:450,
                    name: 'ruri',
                    value: ruri,
                    allowBlank:false
                })
            );
            //upload.end();
            
            //var resource_arr=Ext.util.JSON.decode(resources_json);
            var resource;
            if ((resource_arr!=null)&&(resource_arr['success']===true))
            {
              
              var resource_store = new Ext.data.SimpleStore({
                  fields: ['id', 'name', 'type', 'zone', 
                    'class', 'loc', 'info', 'comment', 'ctime', 'mtime',
                    'vault_path',  'free_space'],
                  data : resource_arr['que_results'] 
              });
              
              resource = new Ext.form.ComboBox({
                  store: resource_store,
                  displayField:'name',
                  typeAhead: true,
                  mode: 'local',
                  triggerAction: 'all',
                  emptyText:'Select a Resource...',
                  selectOnFocus:true,
                  allowBlank:false,
                  hiddenName: 'resource',
                  forceSelection:true
              });
              
              resource.on("render",function() {
                resource.setValue(resource_arr['que_results'][0][1]);
              });
              
              function disp_resource_info(combo, record, index) {
                var ctime= record.data['ctime'];
                var ctime_fmt;
                try {
                  var ctime_date=new Date(ctime*1000);
                  ctime_fmt=ctime_date.dateFormat('F j, Y, g:i a T');
                } catch (e) {
                  ctime_fmt = record.data['ctime'];
                }
                
                document.getElementById('resource_info_disp').innerHTML=
                         "Location:  "+record.data['loc']+"<br/>"
                        +"Class:     "+record.data['class']+"<br/>"
                        +"Created On:"+ctime_fmt+"<br/>"
                        +"Zone:      "+record.data['zone']+"<br/>"
                        +"Free Space:"+record.data['free_space'];
              };
              resource.on("select",disp_resource_info);
            }
            
            else
            {
              resource = new Ext.form.TextField({ 
                width:175,
                name: 'resource',
                allowBlank:false
              });
            }
            
            upload.fieldset(
                {id:'dest_resource_disp', legend:'Destination Resource', hideLabels: true},
                resource
            );
            //upload.end();
            
            // =================
            upload.fieldset(
                {id:'filechooser', legend:'Choose files'}
            );
            upload.end();
            
            
            // =====================
            this.upload_button=upload.addButton({text:'Upload', handler:this.submitHandler});
            
            //upload.render('form-upload');
            upload.render('upload_form_tabs-1');
            
            // =====================
            /*
            var dest_disp_div=document.createElement('p');
            dest_disp_div.innerHTML=document.getElementById('upload_dest_ruri').innerHTML;
            var dest_disp = Ext.get('dest_disp');
            dest_disp.appendChild(dest_disp_div);
            */
            
            // =====================
            var resource_info_disp_div=document.createElement('div');
            resource_info_disp_div.id="resource_info_disp";
            var dest_disp = Ext.get('dest_resource_disp');
            dest_disp.appendChild(resource_info_disp_div);
            
            // =====================
            var img_upload = document.createElement('img');
            img_upload.id="img_upload";
            img_upload.src="images/upload.png";
            img_upload.alt="upload";
            
            var a_attachFile = document.createElement('a'); 
            a_attachFile.id="a_attachFile";
            a_attachFile.href="#";
            a_attachFile.innerHTML="Upload a file";
            
            // =====================
            var attach = Ext.get('filechooser');
            attach.appendChild(img_upload);
            attach.appendChild(a_attachFile);
            
            
            this.createFirstBrowseUploadFile();
            
            Ext.get('a_attachFile').on('click', this.createBrowseUploadFile, this);
            
        },
        
        // ================================
        submitHandler : function (this_button, this_event){
            
            this_button.setDisabled(true);
            this_button.setText("Uploading...");
          
            // The FIRST argument is form.
            // The SECOND argument is true to indicate file upload.
            // The THIRD argument to true to prevent IE from throwing domain security errors,
            //     when uploading files in applications over SSL and using IE.
            YAHOO.util.Connect.setForm(document.body.getElementsByTagName('FORM')['upload_form_data'], 
                                       true, true);
            
            /*
            var postvars= 'ruri='
                          + document.getElementById('upload_dest_ruri').innerHTML
                          + '&'
                          + "jobid="
                          + UploadForm.jobid;                           
            */              
            YAHOO.util.Connect.asyncRequest('POST', 'services/upload.php', 
                       {upload: UploadForm.uploadCBHandler} 
                                           /*,postvars*/); 
                                           
        },
    
        uploadCBHandler : function (oResponse) {
          UploadForm.upload_button.setDisabled(false);
          UploadForm.upload_button.setText("Upload");
          
          var resp;
          try {
             resp= Ext.util.JSON.decode( oResponse.responseText );
          } catch (e) {
            alert("Invalid server response:"+oResponse.responseText);
          }
          
          //alert(resp['log']);
          
          if (!resp['success'])
          {
            Ext.MessageBox.show({
            	title: 'Failure',
            	msg: resp['log'],
            	width:300,
            	buttons: Ext.MessageBox.OK
            });
          }
          else
          {
            Ext.MessageBox.show({
            	title: 'Success',
            	msg: resp['log'],
            	width:300,
            	buttons: Ext.MessageBox.OK
            });
          }
          
          //UploadForm.resetBrowseUploadFileDivs();
        },
        
        // ================================
        createBrowseUploadFile : function (){
            
            // Create browse file template
            var html_tpl = '<div id="{div_id}">'
                         + '    <input type="file" id="{input_id}" size="52" name="{input_name}"/>'
                         + '    &nbsp;'
                         + '    <a id="{a_id}" href="#" onclick="{a_onclick}">remove</a>'
                         + '</div>';
            var tpl = Ext.DomHelper.createTemplate(html_tpl);
            
            // Insert before image upload
            var oInsert = document.getElementById('img_upload');
            
            // Create new id
            var new_id = this.findInputTag_MaxID() + 1;
            
            // Assign to template
            tpl.insertBefore(oInsert, { div_id: ('div_browser_file_' + new_id), 
                                        input_id: ('file_' + new_id), 
                                        input_name: ('file_' + new_id),
                                        a_id: ('remove_' + new_id),
                                        a_onclick: "javascript:UploadForm.removeBrowseUploadFile('" + new_id + "'); return false"
                                      });
            
            // Change attach file text
            document.getElementById('a_attachFile').innerHTML = "Add another file";
        },
        
        createFirstBrowseUploadFile : function (){
            
            // Create browse file template
            var html_tpl = '<div id="{div_id}">'
                         + '    <input type="file" id="{input_id}" size="52" name="{input_name}"/>'
                         + '    &nbsp;'
                         + '</div>';
            var tpl = Ext.DomHelper.createTemplate(html_tpl);
            
            // Insert before image upload
            var oInsert = document.getElementById('img_upload');
            
            // Create new id
            //var new_id = this.findInputTag_MaxID() + 1;
            var new_id = 0;
            
            // Assign to template
            tpl.insertBefore(oInsert, { div_id: ('div_browser_file_' + new_id), 
                                        input_id: ('file_' + new_id), 
                                        input_name: ('file_' + new_id)
                                      });
            
            // Change attach file text
            document.getElementById('a_attachFile').innerHTML = "Add another file";
        },
        
        resetBrowseUploadFileDivs : function (){
          var max_id=this.findInputTag_MaxID();
          for(var idx=0; idx < max_id; idx++)
          {
            this.removeBrowseUploadFile(idx);
          }
          this.createFirstBrowseUploadFile();  
        },
        
        // ================================
        removeBrowseUploadFile : function (idx){
            var upload_div=Ext.get('div_browser_file_' + idx);
            if (upload_div!=null)
              upload_div.remove();
            if (this.countInputTag_TypeFile() == 0) {
                // Change attach file text
                document.getElementById('a_attachFile').innerHTML="Add a file";
            }
        },
        
        // ================================
        countInputTag_TypeFile : function (){
            var count = 0;
            var inputTags = document.body.getElementsByTagName('INPUT');
            for (idx = 0; idx < inputTags.length; idx++) {
                if (inputTags[idx].type.toLowerCase() != 'file') {
                    continue; 
                }
                count++;
            }
            return count;
        },
        
        // ================================
        findInputTag_MaxID : function (){
            var nameArray;
            var max = 0;
            var inputTags = document.body.getElementsByTagName('INPUT');
            for (idx = 0; idx < inputTags.length; idx++) {
                //if (inputTags[idx].type.toLowerCase() != 'file') {
                var typestr=inputTags[idx].type;
                if (typestr.toLowerCase() != 'file') {
                    continue; 
                }
                nameArray = inputTags[idx].id.split('_');
                max = parseInt(nameArray[1]);
            }
            return (max > 0) ? max : 0;
        }
        
    };
}();