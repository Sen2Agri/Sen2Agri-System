<?php

require_once ("ConfigParams.php");

// DEFINE our cipher
define('AES_256_CBC', 'aes-256-cbc');
// Generate a 256-bit encryption key
//$encryption_key = openssl_random_pseudo_bytes(32);
$encryption_key = hash('sha256', '', true);
// Generate an initialization vector
// This *MUST* be available for decryption as well
$iv = openssl_random_pseudo_bytes(openssl_cipher_iv_length(AES_256_CBC));

$fetchMode = array('Overwrite'=>1, 'Resume'=>2, 'Copy'=>3, 'Symbolic link'=>4);
$scope = array('Download'=>2,'Query'=>1,'Query and download'=>3);
$sat_scope = array();

function checkscope($sat_id,$scope_id,$source_id){
    $dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
    
    $rows = pg_query($dbconn, "SELECT sum(scope) FROM datasource WHERE satellite_id=".$sat_id." AND enabled=true AND id!=".$source_id." group by satellite_id") or die ("An error occurred.");
    $res =  pg_fetch_all($rows);
    
    if(is_array($res)){
        switch ($res[0]['sum'] ){
            case '3': return 'false';
            case '2': 
            case '1': return (($res[0]['sum']+$scope_id == 3) ? 'true':'false');
        }
     }else{
        return 'true';
     }
   
}
if(isset($_REQUEST['action']) && $_REQUEST['action'] =='checkscope'){
    echo checkscope($_REQUEST['satellite'], $scope[$_REQUEST['scope']],$_REQUEST['source_date']);
    exit;
}

session_start();
include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';

$feedback ='';
if(isset($_REQUEST['btnSave'])){
    $id_source_date    = $_REQUEST['source_date'];   
    $source_scope      = $scope[$_REQUEST['scope']];
    $enable          =  !isset($_REQUEST ['edit_enabled']) ? "0" : "1";
    $fetch_mode      = $fetchMode[$_REQUEST['fetch_mode']];
    $max_retries     = $_REQUEST['max_retries'];
    $retry           = $_REQUEST['retry'];
    $max_connections = $_REQUEST['max_connections'];
    $download_path   = $_REQUEST['download_path'];
    $local_root      = $_REQUEST['local_root'];
    $username        = $_REQUEST['user'];   
    $password        = $_REQUEST['pwd'];
       
    // Encrypt $password using aes-256-cbc cipher with the given encryption key and
    // our initialization vector. The 0 gives us the default options, but can
    // be changed to OPENSSL_RAW_DATA or OPENSSL_ZERO_PADDING
    $encrypted = openssl_encrypt($password, AES_256_CBC, $encryption_key, 0, $iv);  
    // If we lose the $iv variable, we can't decrypt this, so:
    // - $encrypted is already base64-encoded from openssl_encrypt
    // - Append a separator that we know won't exist in base64, ":"
    // - And then append a base64-encoded $iv
    $encrypted = $encrypted . ':' . base64_encode($iv);
    
    $error = false;
    if($enable){

        if (checkscope($_REQUEST['satellite'], $source_scope,$id_source_date)=='false'){
            $feedback = "open_dialog_error('Data source was not updated. The same scope for this satellite already exist.')"; ;
            $error = true;  
        }
        
    }
    if($error==false){
        $dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
        $sql = "SELECT sp_update_datasource($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11)";
        $res = pg_prepare ( $dbconn, "my_query", $sql );
    
        try{
            $res = pg_execute ( $dbconn, "my_query", array (
                $id_source_date,
                $source_scope,
                $enable,
                $fetch_mode,
                $max_retries,
                $retry,
                $max_connections,
                $download_path,
                $local_root,
                $username,
                $encrypted
            ) )or die ("An error occurred.");
            
            $feedback = "open_dialog('Successfully updated')";
        }catch (Exception $e){
            $feedback = "open_dialog_error('. $e->getMessage().')";
        }
    }
    
}

$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
$rows = pg_query($dbconn, "SELECT  d.*,initcap(s.satellite_name) as satellite
 FROM datasource d LEFT JOIN satellite s ON d.satellite_id = s.id ORDER BY s.satellite_name,name") or die ("An error occurred.");
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
    			
    			<?php  /*   	
    			$curl = curl_init();
    			$url =  ConfigParams::$REST_SERVICES_URL . "/downloader/sources/";
    			curl_setopt($curl, CURLOPT_URL,  $url );
    			curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    			
    			$result = curl_exec($curl);
    			$datasources = json_decode($result);
    			if(!empty($datasources)){
    			    foreach ($datasources as $data){
    			        var_dump($data);echo '<br>'; 
    			    }
    			curl_close($curl);
    			*/
    			
    			foreach ($datasources as $data){
    			    $sourcedataId = $data['id'];
    			    
    			    if(trim( $data['passwrd'])!=""){
        			    // To decrypt, separate the encrypted data from the initialization vector ($iv).
        			    $parts = explode(':', $data['passwrd']);
        			    // $parts[0] = encrypted data
        			    // $parts[1] = base-64 encoded initialization vector
        			    // Don't forget to base64-decode the $iv before feeding it back to openssl_decrypt
        			    if(count($parts)==2)  $decrypted = openssl_decrypt($parts[0], AES_256_CBC, $encryption_key, 0, base64_decode($parts[1]));
        			    else
        			    $decrypted = $parts[0];
    			    }else{
    			        $decrypted = "";
    			    }
        			 
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
								<input type="hidden" name="satellite" value="<?=$data['satellite_id']?>">
								
								<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="scope_<?=$sourcedataId?>">Scope</label>
                						<select class="form-control" id="scope_<?=$sourcedataId?>" name="scope" data-satellite="<?=$data['satellite_id']?>">
                						<?php 
                						                						
                						foreach ($scope as $k=>$v){
                						    $selected = $data['scope']==$v ?'selected':'';
                						        ?>
                							<option id="<?=$v?>" <?=$selected?>><?=$k?></option>
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
            							<label class="control-label" for="local_root_<?=$sourcedataId?>">Local root</label>
            							<input type="text" id="local_root_<?=$sourcedataId?>" class="form-control" name="local_root" value="<?= $data['local_root']?>"/>
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
            							<label class="control-label" for="user_<?=$sourcedataId?>">User</label>
            							<input type="text" id="user_<?=$sourcedataId?>" class="form-control" name="user" value="<?= $data['username']?>"/>
            						</div> 
        							
            						<div class="col-md-6 form-group has-feedback">	        			
            							<label class="control-label" for="pwd_<?=$sourcedataId?>">Password</label>
            							<input  type="password" id="pwd_<?=$sourcedataId?>" class="form-control" name="pwd" value="<?= $decrypted?>"/>
            							<i class="glyphicon glyphicon-eye-open form-control-feedback" title="show password"></i>
            						</div>
            						  				
        						</div>  

        					
        						<div class="row form-group form-group-sm ">
        							<div class="col-md-6">	
        							<input type="submit" class="add-edit-btn" name="btnSave" value="Save">
        							</div>
        						</div>    					
    						</form>
    						
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

	$('input[name="pwd"] + .glyphicon').each(function(){
		$(this).on('click', function(elem) {
			 if( $(this).hasClass('glyphicon-eye-open') ){
				 $("#"+this.previousElementSibling.id ).attr('type','text');
				 $(this).removeClass('glyphicon-eye-open');
				 $(this).addClass('glyphicon-eye-close');
				 $(this).attr('title','hide password');
			 }else{
				 $("#"+this.previousElementSibling.id ).attr('type','password');
				 $(this).removeClass('glyphicon-eye-close');
				 $(this).addClass('glyphicon-eye-open');
				 $(this).attr('title','show password');
				 }
			 
		});
	});
	
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
        var formID = this.id;
        $(this).validate({
            rules: {
                local_root:{
                       required:function(element){ //set required TRUE only when fetch_mode is 3 OR 4                     
                               var fetch_mode = $('#'+element.form.id+' select[name="fetch_mode"]');
                               var selected_index = $('#'+element.form.id+' select[name="fetch_mode"]').prop('selectedIndex');
                              
                               return ( fetch_mode[0].options[selected_index].id==3 ||  fetch_mode[0].options[selected_index].id==4);
                       }
                },
                scope:{
					remote:{
						param:{
							type:'get',
							data:{action:'checkscope',
								 source_date: function(){
									 return $('#' + formID + ' input[name="source_date"]').val();
								 },
								 satellite:	function(){
									 return $('#' + formID+' select[name="scope"]').attr('data-satellite');
									 }
								}
							},
						depends: function(element){
							return $('#' + formID + ' input[name="edit_enabled"]').is(':checked');
							}
						}
                    }
                
            },
            messages: {
            	scope:{
                	remote:"The same scope for this satellite already exist."   
            	}    	
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
            success: function(label) {
                label.remove();
            },
        });

    });
	
});

</script>

<?php include "ms_foot.php" ?>

