
function generateLoginForm(formid, host, port, user, pass, init_path, errmsg)
{    
  var loginform = new Ext.form.Form({
      labelWidth: 70, // label settings here cascade unless overridden
      labelAlign: "right"
      
  });
  loginform.fieldset(
      {legend:'Account Information'},
      new Ext.form.TextField({
          fieldLabel: 'Host/IP ',
          name: 'host',
          width:175,
          value: host,
          allowBlank:false
      }),
  
      new Ext.form.TextField({
          fieldLabel: 'Port    ',
          width:175,
          name: 'port',
          value: port,
          allowBlank:false
      }),
  
      new Ext.form.TextField({
          fieldLabel: 'Username',
          name: 'user',
          value: user,
          allowBlank:false,
          width:175
      }),
      
      new Ext.form.TextField({
          fieldLabel: 'Password',
          name: 'pass',
          width:175,
          value: pass,
          allowBlank:false,
          inputType: 'password'
      })
  );
  
  if ( (errmsg!=null) && (errmsg.length > 1) )
  {
    document.getElementById('form-login-err').innerHTML =errmsg;
  }
  
  if ( (init_path!=null) && (init_path.length > 1) )
  {
    
    loginform.add( 
      new Ext.form.TextField({
          fieldLabel: 'Go to Path',
          name: 'init_path',
          width:175,
          value: init_path
      })
    );  
  }
  loginform.on("actioncomplete",function(form, action){
      window.location = "browse.php#ruri="+action.result.ruri_home;
      //alert ("result: "+ action.result.ruri_home);
    });
  
  loginform.on("actionfailed",function(form, action){
      if ((action.result!=null)&&(action.result.errors!=null))
        document.getElementById('form-login-err').innerHTML = "Attempt Failed: "+action.result.errors;
    });  
  
  loginform.addButton('Sign On', function(){
      //loginform.submit({url:'load-form.php'});
      /*
      var fields=loginform.getValues();
      var str="";
      for( var key in fields )
      {
        str=str+key+":"+fields[key]+" \n";
      }
      alert("you entered: \n"+str);
      */
      var targeturl='services/login.php';
      if ( (init_path!=null) && (init_path.length > 1) )
        targeturl=targeturl+"?init_path="+init_path;
      loginform.submit({url:targeturl});
        
  }, loginform);
  
  loginform.render(formid);
}

