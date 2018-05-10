<?php
session_start();

include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';
require_once ("ConfigParams.php");

$fetchMode = array('Override'=>1, 'Resume'=>2, 'Copy'=>3, 'Symbolic link'=>4);

$feedback ='';
if(isset($_REQUEST['btnSave'])){
    $id_source_date    = $_REQUEST['source_date'];   
    $enable          =  !isset($_REQUEST ['edit_enabled']) ? "0" : "1";
    $fetch_mode      = $fetchMode[$_REQUEST['fetch_mode']];
    $max_retries     = $_REQUEST['max_retries'];
    $retry           = $_REQUEST['retry'];
    $max_connections = $_REQUEST['max_connections'];
    $download_path   = $_REQUEST['download_path'];
    $local_root      = $_REQUEST['local_root'];
    $username        = $_REQUEST['username'];   
    $password        = $_REQUEST['password'];
  
    $dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    $sql = "SELECT sp_update_datasource($1,$2,$3,$4,$5,$6,$7,$8,$9,$10)";
    $res = pg_prepare ( $dbconn, "my_query", $sql );

    try{
        $res = pg_execute ( $dbconn, "my_query", array (
            $id_source_date,
            $enable,
            $fetch_mode,
            $max_retries,
            $retry,
            $max_connections,
            $download_path,
            $local_root,
            $username,
            $password
        ) )or die ("An error occurred.");
        
        $feedback = "open_dialog('Successfully updated')";
    }catch (Exception $e){
        $feedback = "open_dialog_error('. $e->getMessage().')";
    }
}

$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
$rows = pg_query($dbconn, "SELECT  d.*,initcap(s.satellite_name) as satellite
 FROM datasource d LEFT JOIN satellite s ON d.satellite_id = s.id WHERE satellite_id in (1,2) ORDER BY satellite_id,name") or die ("An error occurred.");
$datasources = pg_fetch_all($rows);

?>
<div id="main">
	<div id="main2">
		<div id="main3">
            
		<?php if((isset($_SESSION['isAdmin']) && !$_SESSION['isAdmin']) || !isset($_SESSION['userId'])){?>
    		<div class="panel panel-default">
        		<div class="panel-body">
    				<div class="panel-heading row text-danger">Access denied!</div>
    			</div>
			</div>	
			<?php exit;
               } else {?>
				
            <div class="container-fluid">
			<div class="panel-group"  id="accordion_datasource">    			
    			
    			<?php     			
    			foreach ($datasources as $data){
    			    $sourcedataId = $data['id'];
    			    $fetch_mode = $data['fetch_mode'];
    			    ?>
        			<div class="panel datasource panel-success">
        			<div class="panel-heading">
        				<h4 class="panel-title">
        					<a data-toggle="collapse" data-parent="#accordion_datasource" href="#<?=$sourcedataId?>"><?php echo $data['satellite'].' - '.$data['name']?></a>
        				</h4>
        			</div>	
        			
    				<div id="<?=$sourcedataId?>" class="panel-collapse collapse" >
    					<div class="panel-body ">
    						<form  id="form_<?=$sourcedataId?>" enctype="multipart/form-data" role="form" action="" class="" method="post">
								<input type="hidden" name="source_date" value="<?=$sourcedataId?>">
                				<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="fetch_mode_<?=$sourcedataId?>">Fetch mode</label>
                						<select class="form-control" id="fetch_mode_<?=$sourcedataId?>" name="fetch_mode">
                						<?php 
                						    foreach ($fetchMode as $key=>$value){
                						        $selected = $data['fetch_mode']==$value ?'selected':'';
                						        ?>
                							<option id="<?=$value?>" <?=$selected?>><?=$key?></option>
                						<?php }?>                							
                						</select>        				
   									</div>
   									
   									<div class="col-md-6">            											
										<label class="control-label" for="edit_enabled_<?=$sourcedataId?>">Enable</label>
           								<input type="checkbox" name="edit_enabled" id="edit_enabled_<?=$sourcedataId?>" class="form-control " <?php echo $data['enabled']=='t' ? "checked" : "" ?>>
        							</div>
            					</div>
                				
                				<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="max_retries_<?=$sourcedataId?>">Max retries</label>
        								<input type="number" id="max_retries_<?=$sourcedataId?>" class="form-control" name="max_retries" min="0" step="1" value="<?= $data['max_retries']?>"/>    				
   									</div>
   									
   									<div class="col-md-6">				
            							<label class="control-label" for="retry_<?=$sourcedataId?>">Retry interval minutes</label>
            							<input type="number" id="retry_<?=$sourcedataId?>" class="form-control" name="retry" min="0" step="1" value="<?= $data['retry_interval_minutes']?>"/>        				
        							</div>
            					</div>
            			
								<div class="row form-group form-group-sm ">  
    								<div class="col-md-6">	
                						<label class="control-label" for="max_connections_<?=$sourcedataId?>">Max connections</label>
                						<input type="number" id="max_connections_<?=$sourcedataId?>" class="form-control" name="max_connections"  min="1" max="8" value="<?= $data['max_connections']?>"/>
                						<span class="help-block">Connections between 1 and 8.</span>        				
            						</div>
            					
            						<div class="col-md-6">        			
            							<label class="control-label" for="download_path_<?=$sourcedataId?>">Download path</label>
            							<input type="text" id="download_path_<?=$sourcedataId?>" class="form-control" name="download_path" value="<?= $data['download_path']?>"/>        				
            						</div>        						   
        						</div>
        						       						
        						<div class="row form-group form-group-sm ">
        							<div class="col-md-6">             						  			
            							<label class="control-label" for="username_<?=$sourcedataId?>">User</label>
            							<input type="text" id="username_<?=$sourcedataId?>" class="form-control" name="username" value="<?= $data['username']?>"/>
            						</div> 
        							
            					
            						<div class="col-md-6">	        			
            							<label class="control-label" for="local_root_<?=$sourcedataId?>">Local root</label>
            							<input type="text" id="local_root_<?=$sourcedataId?>" class="form-control" name="local_root" value="<?= $data['local_root']?>"/>
            						</div>
            						  				
        						</div>  
								
								
        						<div class="row form-group form-group-sm ">
        							<div class="col-md-6">	        			
            							<label class="control-label" for="password_<?=$sourcedataId?>">Password</label>
            							<input  type="text" id="password_<?=$sourcedataId?>" class="form-control" name="password" value="<?= $data['passwrd']?>"/>
            						</div>       				
        						</div>  
        					
        						<div class="row form-group form-group-sm ">
        							<div class="col-md-6">	
        							<input type="submit" class="add-edit-btn" name="btnSave" value="Save">
        							</div>
        						</div>    					
    						</form>
    					<!-- 
    					satellite_id
    					scope
    					fetch_mode (1 si 4)
    					user
    					password
    					download_path
    					
    					specific_params
    					
    					max_connections intre 0 si 8
    					local_root  ( obligatoriu pt fetch_mode 3 si 4)  					
    					enable
    					    					
    					
    					max_retries
    					retry_interval_min    					    					
    					 -->  
    					 </div>  				
    				</div>
    				</div>
    			<?php }?>    			
    			
			</div>
			</div>	
			<?php }?>
		</div>
    </div>
</div>

<!-- Diablog message -->
<div id="dialog-message" title="Submit successful">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>

<!-- Diablog message -->
<div id="dialog-error" title="Submit failed">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>

<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<!-- includes for bootstrap-switch -->
<link rel="stylesheet" href="libraries/bootstrap-switch/bootstrap-switch.min.css">
<script src="libraries/bootstrap-switch/bootstrap-switch.min.js"></script>

<!-- includes for validate form -->
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>


<script>
function open_dialog(message) {
	$("#dialog-message #dialog-content").text(message);
	$("#dialog-message").dialog("open");
};

function open_dialog_error(message) {
	$("#dialog-error #dialog-content").text(message);
	$("#dialog-error").dialog("open");
};
$(document).ready(function(){

	// initialize dialogs
	$("#dialog-message").dialog({
		width: '400px',
		autoOpen: false,
		modal: true,
		buttons: { Ok: function() { $(this).dialog("close"); } }
	});
	
	$("#dialog-error").dialog({
		width: '400px',
		autoOpen: false,
		modal: true,
		buttons: { Ok: function() { $(this).dialog("close"); } }
	}).parent().children(".ui-dialog-titlebar").addClass('ui-state-error');

	<?php echo $feedback;?>
	
	// create switch for enabled checkbox
	$("[name='edit_enabled']").bootstrapSwitch({
	    size: "small",
	    onColor: "success",
	    offColor: "default"
	});

    // validate add site form
    $("form").each(function() {
        $(this).validate({
            rules: {
                local_root:{
                       required:function(element){ //set required TRUE only when fetch_mode is 3 OR 4                     
                               var fetch_mode = $('#'+element.form.id+' select[name="fetch_mode"]');
                               var selected_index = $('#'+element.form.id+' select[name="fetch_mode"]').prop('selectedIndex');
                              
                               return ( fetch_mode[0].options[selected_index].id==3 ||  fetch_mode[0].options[selected_index].id==4);
                       }    

                }
                
            },
            messages: {            	
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
            	form.submit();
            },
            // set this class to error-labels to indicate valid fields
            success: function(label) {
                label.remove();
            },
        });

    });
	
});

</script>

<?php include "ms_foot.php" ?>

