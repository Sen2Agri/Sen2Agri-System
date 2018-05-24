<?php
session_start();

include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';

?>
<div id="main">
	<div id="main2">
		<div id="main3">
            
			<div class="panel panel-default">
				<div class="panel-body">
				
				<?php if((isset($_SESSION['isAdmin']) && !$_SESSION['isAdmin']) || !isset($_SESSION['userId'])){?>
				
					<div class="panel-heading row text-danger">Access denied!</div>
				<?php exit;
                      }else{?>
				
					<div class="panel-heading row"><input name="adduser" type="button" class="add-edit-btn" value="Add new user" style="width: 200px"></div>
					<div class="panel-heading row">
						<div class="coll-md-11">
							<table id="users" class="display" style="font-weight:normal"></table>
						</div>
					</div>					
				<?php }?>
				
				<!-- edit user -->
			
				  <div class="add-edit-user" id="div_edituser" style="display: none;">
                  	<form enctype="multipart/form-data" id="user_add_edit" action="getUsers.php" method="post">
                  	<input type="hidden" id="user_id" value="10">
                    	<div class="row">
                        	<div class="col-md-1">
                        		<div class="form-group  form-group-sm">
                            		<label class="control-label" for="login">Login:</label>
                             		<input type="text" class="form-control" id="login" name="login">
                                </div>
                           	</div>
                      	</div>
                        <div class="row">
                        	<div class="col-md-1">
                        		<div class="form-group  form-group-sm">
                        			<label class="control-label" for="email">Email:</label>
                        			<input type="text" class="form-control" id="email" name="email">
                                </div>
                           	</div>
                       	</div>
                            
                       	<div class="row">
                        	<div class="col-md-1">
                            	<div class="form-group  form-group-sm">
                                	<label class="control-label" for="role">Role:</label>
                                	<select class="form-control" name="role" id="role">
                                		<option value="1">admin</option>
                                		<option value="2" selected>user</option>
                                	</select>
                             	</div>
                          	</div>
                      	</div>
                      	
                      	<div class="row">
                        	<div class="col-md-1">
                            	<div class="form-group  form-group-sm">
                                	<label class="control-label" for="site">Sites:</label>
                                	<select class="form-control" multiple="multiple" name="site[]" id="site">
                                	<option value=0></option>
                                	</select>
                                	<input type="checkbox" id="checkbox" >Select All
                             	</div>
                        	</div>
                      	</div>

                      	<div class="submit-buttons">
                       		<input class="add-edit-btn" name="add_edit_user" type="submit" value="Save">
                   			<input class="add-edit-btn" name="abort_add" type="button" value="Abort" >
                       	</div>
                 	</form>
               	</div>
				<!-- end edit user -->
				<!-- dialog delete -->
				<div class="add-edit-user" id="div_deleteuser" style="display: none;">
					<span>Are you sure you wish to remove this user?</span>					
					<hr>
					<div class="submit-buttons">
                    	<input class="add-edit-btn" name="delete_user_yes" type="button" value="Yes" data-user="">
                       	<input class="add-edit-btn" name="delete_user_no" type="button" value="No">
                    </div>
				</div>				
				<!-- end dialog delete -->
				
				</div>
            </div>
				
		</div>
    </div>
</div>

<!-- Dialog message -->
<div id="dialog-message" title="Submit successful">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>

<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<link href="libraries/DataTables-1.10.16/css/jquery.dataTables.min.css" rel="stylesheet" type="text/css">
<script type="text/javascript" src="libraries/DataTables-1.10.16/js/jquery.dataTables.min.js"></script>

<!-- includes for validate form -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

<link href="libraries/select2/select2.min.css" rel="stylesheet" type="text/css">
<script src="libraries/select2/select2.min.js"></script>

<script type="text/javascript">

$(document).ready(function() {
	
	//initialise dataTable 
    var usersTable = $('#users').DataTable( {
    	ajax: {
			method: "GET",
			url: "getUsers.php",
			data: {action:"getAllUsers"}
		    },
        columns: [
            {data:'user_name', name:'user_name', title: 'Login',width:'25%'} ,
            {data:'email',name:'email', title: 'Email',width:'25%'},
            {data:'role_name',name:'role_name',title: 'Role',width:'10%'},
            {data:'site_name',name:'site_name',title:'Sites',width:'30%'},
            {
                data:null,
                title: 'Actions',
                className: "center",
                width:'10%',
              	render:function(e){
                	return "<span class='glyphicon glyphicon-pencil editor_edit' style='color:#ed8a19;padding-right:15px;cursor: pointer;cursor:hand;' name='edituser' data-user-id='"+e.user_id+"' data-sites-id='"+e.site_id+"' data-role-id='"+e.role_id+"' data-toggle='tooltip' title=''></span>  "+
    				"<span class='glyphicon glyphicon-trash editor_remove' style='color:red;cursor: pointer;cursor:hand;' data-user-id='"+e.user_id+"' data-toggle='tooltip' title=''></span>"
                }              
            }
        ]
    } );

    // initialize dialog message
	$("#dialog-message").dialog({
		width: '400px',
		autoOpen: false,
		modal: true,
		buttons: { Ok: function() { $(this).dialog("close"); } }
	});
	
	function dialogMessage(message) {
		$("#dialog-message #dialog-content").text(message);
		$("#dialog-message").dialog("open");
	};

	function refreshUserPage(msg){
		$('.add-edit-user').dialog('close');
		usersTable.ajax.reload();

		dialogMessage(msg);
	}
	
	// disable sites options when 'Admin'
	$("#role").on('change',function(){
		if(this.value == '1'){
			$("#site").attr('disabled','true');
			$("input[type='checkbox']").attr('disabled','true');
        	
			}else{
				$("#site").removeAttr('disabled');	        	
	        	$("input[type='checkbox']").removeAttr('disabled');
	        	}
		
		});

	//close dialog 
	$('input[name="abort_add"], input[name="delete_user_no"]').on('click',function(event){
		$('.add-edit-user').dialog('close');
		});
	 
 	//get all sites for select2 options
 	$.ajax({
        method: "GET",
      	url: "getUsers.php",
      	data:{action:"getSites"},
        dataType: 'json',
        success: function (data) {
        	$.each(data.results, function(index, value) {
        		  $('#site').append(
        		    '<option value="' + data.results[index].id + '">' + data.results[index].text + '</option>'
        		  );
        		});
        }});
 
 	//initialise sites multiselect
    $('#site').select2({ 
        tags:true,
       	placeholder: 'Select sites',
       	width:'100%'
           }).on("change", function (e) {
        	    $(this).valid(); //jquery validation script validate on change
           }).on("select2:open", function() { //correct validation classes (has=*)
               if ($(this).parents("[class*='has-']").length) { //copies the classes
                   var classNames = $(this).parents("[class*='has-']")[0].className.split(/\s+/);

                   for (var i = 0; i < classNames.length; ++i) {
                       if (classNames[i].match("has-")) {
                           $("body > .select2-container").addClass(classNames[i]);
                       }
                   }
               } else { //removes any existing classes
                   $("body > .select2-container").removeClass (function (index, css) {
                       return (css.match (/(^|\s)has-\S+/g) || []).join(' ');
                   });
               }
           });
    
	//event on checkbox 'Select all sites'
    $("#checkbox").click(function(){
        if($("#checkbox").is(':checked') ){
            $("#site > option").prop("selected","selected");
            $("#site").trigger("change");
        }else{
            $("#site > option").removeAttr("selected");
             $("#site").trigger("change");
         }
    });

	//--------------------  Add/Edit user -----------------------//
	
    // create dialog for add/edit site
    $("#div_edituser").dialog({
        width: '560px',
        overflow: 'none',
        autoOpen: false,
        autoResize:false,
        modal: true,
        resizable: false,
        create: function() {
            $(this).css("maxHeight", 200);        
        },
        open: function(event, ui) {
        },
        close:function(event, ui) {
        	$("#user_add_edit")[0].reset();
        	$("#user_add_edit").validate().resetForm();//remove error class on name elements and clear history
        	$("#user_add_edit").removeAttr('novalidate');
            $('.form-group').removeClass('has-error');
            $('.error').removeClass('error');
			//reset multiselect selected options
			$('#site').val('').trigger('change');
        }
    });
 	
    // Open dialog 'New user'
    $('input[name="adduser"]').on('click', function (e) {
        e.preventDefault();
        
        $("#user_id").val(''); 
		$("#site").removeAttr('disabled');	        	
	    $("input[type='checkbox']").removeAttr('disabled');
    
        $("#div_edituser").dialog({'title':'Add new user'});
        $("#div_edituser").dialog("open");
    } );
 
    // Open dialog 'Edit user'
    $('#users').on('click', 'span.editor_edit', function (e) {
        e.preventDefault();
        //the current user can't edit his profile
        if($(this).attr('data-user-id')!=<?php echo $_SESSION['userId']?>){
            $.each($(this).closest('tr').find('td'),function(index,item){
                switch(index){
                	case 0:  $("#login").val(item.textContent);break;
                 	case 1:  $("#email").val(item.textContent);break;
                 	case 2:  $("#role option:contains("+item.textContent+")").prop('selected',true);break;
                 	}
                });
            $("#user_id").val($(this).attr('data-user-id'));
    
            // if not admin
            if($(this).attr('data-role-id') != '1'){
            	$("#site").removeAttr('disabled');
            	$("input[type='checkbox']").removeAttr('disabled');
            	
                var sites = $(this).attr('data-sites-id');
                sites = sites.replace('{','');
                sites = sites.replace('}','');
                $("#site").val(sites.split(",")).trigger('change');
            }else{//if admin disable sites multiselect
            	$("#site").attr('disabled','true');
            	$("input[type='checkbox']").attr('disabled','true');
                }
          
            
            $("#div_edituser").dialog({'title':'Edit User'});
            $("#div_edituser").dialog("open");
        }
    } );

    $('#users ').on('mouseenter','span.editor_edit', function (e) {
    	 e.preventDefault();
         //the current user can't edit his profile
         if($(this).attr('data-user-id')==<?php echo $_SESSION['userId']?>){      	
 		   $(this).tooltip({content:'You can\'t edit your account'});	
         }
        });
    //--------------------  Add/Edit user -----------------------//

    // ------------------- delete user ------------------------//
    //create delete dialog
	$("#div_deleteuser").dialog({
		 width: '560px',
	        autoOpen: false,
	        modal: true,
	        resizable: false
		});
	
   //Open dialog 'Delete user'
   $('#users').on('click','span.editor_remove',function (e) {
        e.preventDefault();
        //the current user can't edit his profile
         if($(this).attr('data-user-id')!=<?php echo $_SESSION['userId']?>){
            var user = $(this).closest('tr').find('td');
            $("#div_deleteuser").dialog({
            	autoOpen:true,
                title:'Delete User "'+$(user[0]).text() +'"',
                
                });
            $('input[name="delete_user_yes"]').attr('data-user',$(this).attr('data-user-id'));
        }
    } );

   $('#users ').on('mouseenter','span.editor_remove', function (e) {
   	 e.preventDefault();
        //the current user can't edit his profile
        if($(this).attr('data-user-id')==<?php echo $_SESSION['userId']?>){
 			$(this).tooltip({content:'You can\'t delete your account'});	
       }
   });

	//action delete user
	$("input[name='delete_user_yes']").on('click',function(){
		var id = $(this).attr('data-user');
		$.ajax({
			url:'getUsers.php',
			type:'POST',
			data:{action:'delete',
				  user_id:id 
			},
		    success: function(data){			    
			    refreshUserPage('The user was successfully deleted!');				

			    }
	
			});
		});
	// ------------------- end actions for delete user ------------------------//
	
    // validate add site form
    var validator = $("#user_add_edit").validate({
        rules: {
           	login:{ 
               	required: true, 
               	pattern: "[a-z]{1}[\\w ]*",
               	minlength: 3,
               	remote: {
                        url: "getUsers.php",
                        type: "post",
                        data:{action:'checkUser',id:function(){
                               return $('#user_id').val();
                                }
                        }
               	}
                
            },
        	email:{ 
            	required: true,
            	pattern: /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/ 
                	},
            "site[]":{
                required: true,
                }
        },
        messages: {
        	login: { 
            	pattern : "Invalid username.", 
            	minlength: $.validator.format("Please, at least {0} characters are necessary."),
            	remote:"This username already exists."
            	 },
        	email: { pattern : "Please enter a valid email address." }
        },
        highlight: function(element, errorClass) {
            $(element).parent().addClass("has-error");
        },
        unhighlight: function(element, errorClass) {
            $(element).parent().removeClass("has-error");
        },
        errorPlacement: function(error, element) {
            error.appendTo(element.parent());
        },
        submitHandler: function(form) {          
           var user_id = $("#user_id").val(); 
           var myForm = $( form ).serialize();
           var msg = " added";
           if(user_id!=''){
        	   myForm = myForm+'&user_id='+user_id;
        	   msg = " updated";
               }
           $.ajax({
        	    url: $(form).attr('action'),
                type: $(form).attr('method'),
                data: myForm,
                success: function(response) {
                    $(form)[0].reset();
                    msg = response.replace('$',msg);
                    refreshUserPage(msg);
    				
                }
            });
        },
        // set this class to error-labels to indicate valid fields
        success: function(label) {
            label.remove();
        },
    });


    
} );

</script>
<?php include "ms_foot.php" ?>
