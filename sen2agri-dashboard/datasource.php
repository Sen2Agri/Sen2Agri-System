<?php

require_once ("ConfigParams.php");

$fetchMode = array('Overwrite'=>1, 'Resume'=>2, 'Copy'=>3, 'Symbolic link'=>4);
$scope = array('Download'=>2,'Query'=>1,'Query and download'=>3);
$sat_scope = array();


$dbconn = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
$rows = pg_query($dbconn, "SELECT id, INITCAP(satellite_name) as satellite_name FROM satellite") or die ("An error occurred.");
$res =  pg_fetch_all($rows);
$satelliteArr = [];
foreach ($res as $sat){
    $satelliteArr[$sat['satellite_name']] = $sat['id'];
}

function cmp($a, $b)
{
    if($a->satellite == $b->satellite){
        return strcmp($a->dataSourceName,$b->dataSourceName);        
    }
    return strcmp($a->satellite, $b->satellite);
}

function checkscope($satellite,$scope_id,$source_id){
    $dbconn = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
    
    $rows = pg_query($dbconn, "SELECT sum(scope) FROM datasource WHERE satellite_id=".$satellite." AND enabled=true AND id!=".$source_id." group by satellite_id") or die ("An error occurred.");
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

    $curl = curl_init();
    $url =  ConfigParams::$REST_SERVICES_URL . "/downloader/sources/".$satelliteArr[$_REQUEST['satellite']]."/".rawurlencode($_REQUEST['source_name']);

    $post = [
        'id'    =>  $_REQUEST['source_date'],
        'satellite' => $_REQUEST['satellite'],
        'dataSourceName' => $_REQUEST['source_name'],
        'scope' => $scope[$_REQUEST['scope']],
        'user'  =>  $_REQUEST['user'],
        'password' =>  $_REQUEST['pwd'],
        'fetchMode' => $fetchMode[$_REQUEST['fetch_mode']],
        'maxRetries' => $_REQUEST['max_retries'],
        'retryInterval' => $_REQUEST['retry'],
        'maxConnections' => $_REQUEST['max_connections'],
        'downloadPath' => $_REQUEST['download_path'],
        'localArchivePath' => empty($_REQUEST['local_root'])? null:$_REQUEST['local_root'],
        'enabled' => !isset($_REQUEST ['edit_enabled']) ? false : true
    ];

    curl_setopt($curl, CURLOPT_HTTPHEADER, array('Content-Type: application/json','Content-Length: ' . strlen(json_encode( $post))));
    curl_setopt($curl, CURLOPT_POST, 1);
    curl_setopt($curl, CURLOPT_POSTFIELDS,json_encode( $post));
    curl_setopt($curl, CURLOPT_URL, $url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
    $result = curl_exec($curl);
    
    // Check if any error occurred
    if (!curl_errno($curl)) {
        $httpcode = curl_getinfo($curl,CURLINFO_HTTP_CODE);
        $msg = ($httpcode>=200 && $httpcode<300) ? 'Successfully updated':'Something went wrong.Data source was not updated.';
        $feedback = "open_dialog('".$msg."')";
    }
    
    curl_close($curl);
    
}


$curl = curl_init();
$url =  ConfigParams::$REST_SERVICES_URL . "/downloader/sources/";
curl_setopt($curl, CURLOPT_URL,  $url );
curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);

$result = curl_exec($curl);
$datasources = json_decode($result);
curl_close($curl);
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
    			if ($datasources == "") {
    			        print_r ("Cannot get data sources! Please check that the sen2agri-services are started!");
    			}else{
    			    //sort datasources by satellite name and data source name
    			    usort($datasources, "cmp");
    			    
    			    foreach ($datasources as $data){
    			        if(ConfigParams::isSen2Agri() &&  $data->satellite =='Sentinel1') continue;
    			        $sourcedataId = $data->id;
    			    ?>
        			<div class="panel datasource panel-success">
        			<div class="panel-heading">
        				<h4 class="panel-title">
        					<a data-toggle="collapse" data-parent="#accordion_datasource" href="#source_<?=$sourcedataId?>"><?php echo $data->satellite.' - '.$data->dataSourceName?></a>
        				</h4>
        			</div>	
        			
    				<div id="source_<?=$sourcedataId?>" class="panel-collapse collapse" >
    					<div class="panel-body ">
    						<form  id="form_<?=$sourcedataId?>" enctype="multipart/form-data" role="form" action="" class="" method="post">
								<input type="hidden" name="source_date" value="<?=$sourcedataId?>">
								<input type="hidden" name="satellite" value="<?=$data->satellite?>">
								<input type="hidden" name="source_name" value="<?=$data->dataSourceName?>">
								
								<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="scope_<?=$sourcedataId?>">Scope</label>
                						<select class="form-control" id="scope_<?=$sourcedataId?>" name="scope" data-satellite="<?=$satelliteArr[$data->satellite]?>">
                						<?php 
                						                						
                						foreach ($scope as $k=>$v){
                						    $selected = $data->scope==$v ?'selected':'';
                						        ?>
                							<option id="<?=$v?>" <?=$selected?>><?=$k?></option>
                						<?php }?>                							
                						</select>        				
   									</div>
   									
   									<div class="col-md-6">            											
										<label class="control-label" for="edit_enabled_<?=$sourcedataId?>">Enable</label>
           								<input type="checkbox" name="edit_enabled" id="edit_enabled_<?=$sourcedataId?>" class="form-control " <?php echo $data->enabled=='true' ? "checked" : "" ?>>
        							</div>
            					</div>
								
                				<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="fetch_mode_<?=$sourcedataId?>">Fetch mode</label>
                						<select class="form-control" id="fetch_mode_<?=$sourcedataId?>" name="fetch_mode">
                						<?php 
                						    foreach ($fetchMode as $key=>$value){
                						        $selected = $data->fetchMode==$value ?'selected':'';
                						        ?>
                							<option id="<?=$value?>" <?=$selected?>><?=$key?></option>
                						<?php }?>                							
                						</select>        				
   									</div>
   									
   									<div class="col-md-6">	        			
            							<label class="control-label" for="local_root_<?=$sourcedataId?>">Local root</label>
            							<input type="text" id="local_root_<?=$sourcedataId?>" class="form-control" name="local_root" value="<?= $data->localArchivePath?>"/>
            						</div>
            					</div>
            					
                				<div class="row form-group form-group-sm ">  
    								<div class="col-md-6">	
                						<label class="control-label" for="max_connections_<?=$sourcedataId?>">Max connections</label>
                						<input type="number" id="max_connections_<?=$sourcedataId?>" class="form-control" name="max_connections"  min="1" max="8" value="<?= $data->maxConnections?>"/>
                						<span class="help-block">Connections between 1 and 8.</span>        				
            						</div>
            					
            						<div class="col-md-6">        			
            							<label class="control-label" for="download_path_<?=$sourcedataId?>">Download path</label>
            							<input type="text" id="download_path_<?=$sourcedataId?>" class="form-control" name="download_path" value="<?= $data->downloadPath?>"/>        				
            						</div>        						   
        						</div>
        						
                				<div class="row form-group form-group-sm">    									
   									<div class="col-md-6">						
										<label class="control-label" for="max_retries_<?=$sourcedataId?>">Max retries</label>
        								<input type="number" id="max_retries_<?=$sourcedataId?>" class="form-control" name="max_retries" min="0" step="1" value="<?= $data->maxRetries?>"/>    				
   									</div>
   									
   									<div class="col-md-6">				
            							<label class="control-label" for="retry_<?=$sourcedataId?>">Retry interval minutes</label>
            							<input type="number" id="retry_<?=$sourcedataId?>" class="form-control" name="retry" min="0" step="1" value="<?= $data->retryInterval?>"/>        				
        							</div>
            					</div>
        						       						
        						<div class="row form-group form-group-sm ">
        							<div class="col-md-6">             						  			
            							<label class="control-label" for="user_<?=$sourcedataId?>">User</label>
            							<input type="text" id="user_<?=$sourcedataId?>" class="form-control" name="user" value="<?= $data->user?>" autocomplete="new-user"/>
            						</div> 
        							
            						<div class="col-md-6 form-group has-feedback">	        			
            							<label class="control-label" for="pwd_<?=$sourcedataId?>">Password</label>
            							<input  type="password" id="pwd_<?=$sourcedataId?>" class="form-control" name="pwd" value="<?= $data->password?>" autocomplete="new-password"/>
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
    			<?php }
    			    
    			}?>
    			
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

