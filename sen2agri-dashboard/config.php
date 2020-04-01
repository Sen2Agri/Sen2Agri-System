<?php 

require_once ("ConfigParams.php");

$isSen2Agri = ConfigParams::isSen2Agri();

function getParameters($current_processor_name, $adv){
    if ($current_processor_name == "l3b_lai") {
        $current_processor_name ='l3b';
    }
    if($GLOBALS['isSen2Agri']){
        switch ($current_processor_name){
            case "l4a_wo": $current_processor_name ='l4a';break;
        }
    } 
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
    $sql = "SELECT cm.*,reverse(split_part(reverse(cm.key),'.',1)) as param_name, cf.value as param_value, cf.site_id as site_id
            FROM config_metadata cm
            INNER JOIN config_category cfc ON cm.config_category_id = cfc.id
            LEFT JOIN config cf ON lower(cf.key) = lower(cm.key)
            WHERE cfc.id IN (3,4,5,6,18,19,20,21) AND cm.Key ilike 'processor.{$current_processor_name}.%' AND is_site_visible=true 
            AND is_advanced='".$adv."' 
            ORDER BY cfc.display_order";
	
    $res = pg_query($db, $sql) or die();
    $processors_advance_params = pg_fetch_all($res);
	
    return $processors_advance_params;
}

if(isset($_REQUEST['action'])){
    switch ($_REQUEST['action'] ){
        case 'getParameters':      
            echo json_encode(array("advanced"=>getParameters($_REQUEST['current_processor'], 'true'),
                "specific"=>getParameters($_REQUEST['current_processor'], 'false')));
            break;
    }
            
    exit;
}

include "master.php";

$db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
if($isSen2Agri){
    $sql =  "SELECT * FROM (
                SELECT (CASE WHEN short_name ilike '%l4a%' THEN 'L4A IN-SITU processor' ELSE name END)as name, short_name FROM processor WHERE name not ilike '%l2a%'
                UNION 
                SELECT 'L4A w/o IN-SITU processor' as name, 'l4a_wo' as short_name FROM processor WHERE name ilike '%l4a%') as proc 
            ORDER BY name";
}else{
    $sql = "Select name, short_name FROM processor WHERE name not ilike '%l2%' and name not ilike '%lpis%' ORDER BY name";
    }
$res = pg_query($db, $sql) or die();
$processors = pg_fetch_all($res);

?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<div class="container-fluid">
				<div class="panel-group config cust_jobs" id="accordion">

				<?php 
    				$processorList = array();
    				if(sizeof($processors)>0){				    
				
    				    foreach ($processors as $processor){
    				        $processorList[] = $processor['short_name'].'_advanced';
				    ?>
				
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#<?=$processor['short_name']?>"><?=$processor['name']?></a>
							</h4>
						</div>
						<div id="<?=$processor['short_name']?>" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="<?=$processor['short_name']?>_advanced"><input type="checkbox" id="<?=$processor['short_name']?>_advanced">Show advanced parameters</label>
								<form enctype="multipart/form-data" role="form" id="<?=$processor['short_name']?>form" name="<?=$processor['short_name']?>form" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"> </select>
											</div>
										
											<!-- Filter criteria for input files -->
											<?php $prefix = $processor['short_name'];
											include "filterInputFiles.php"?>
											<!-- End Filter criteria for input files -->
                                            <?php if($processor['short_name'] != "s4c_l4b" && $processor['short_name'] != "s4c_l4c"){?> 
												<div class="form-group form-group-sm required">
													<label class="control-label" for="inputFiles">Available input files:</label>
													<select multiple class="form-control input-files" id="inputFiles" name="inputFiles[]" size="7"></select>
													<span class="help-block">The list of products descriptors (xml files).</span>
												</div>
												
                                            <?php } ?>
											<span id="specificParam_<?=$processor['short_name']?>"></span>		
											
											<?php if($processor['short_name'] == "l3a" || $isSen2Agri){?> 
											<div class="form-group form-group-sm ">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<?php } ?>
											
											<!-- Specific parameters -->
											<?php if($processor['short_name'] == "l3a" && $isSen2Agri){?> 
											<div class="form-group form-group-sm required" data-provide="datepicker">
												<label class="control-label" for="synthDate">Synthesis date:</label>
												<input type="text" class="form-control" id="synthDate" name="synthDate">
												<span class="help-block">The synthesis date [YYYYMMDD].</span>
											</div>										
											<?php }?>
											
											
											<?php if($processor['short_name'] == "l3b_lai"){?> 
											<div class="subgroup lai">
												<label class="control-label">Generate LAI mono-dates:</label>
												<div class="form-group form-group-sm">
													<input class="form-control" id="monolai" type="radio" name="lai" value="monolai" checked="checked">
													<label class="control-label" for="monolai">Generate LAI mono-dates</label>
													<span class="help-block">(Generate LAI mono-dates)</span>
												</div>
												<div class="form-group form-group-sm">
													<input class="form-control" id="reproc" type="radio" name="lai" value="reproc">
													<label class="control-label" for="reproc">Reprocessing with the last N-Days</label>
													<span class="help-block">(Performe reprocessing with the last N-Days)</span>
												</div>
                                                <?php if($isSen2Agri){?> 
                                                    <div class="form-group form-group-sm">
                                                        <input class="form-control" id="fitted" type="radio" name="lai" value="fitted">
                                                        <label class="control-label" for="fitted">LAI time series fitting at the end of the season</label>
                                                        <span class="help-block">(Performe reprocessing at the end of the season)</span>
                                                    </div>
                                                <?php }?>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="bwr">Backward window:</label>
												<input type="number" class="form-control" id="bwr" name="bwr" value="2">
												<span class="help-block">Backward window for LAI N-Day reprocessing</span>
											</div>
											<?php }?>
											
											<?php if(($processor['short_name'] == "l4a" || $processor['short_name'] == "l4b") && $isSen2Agri){?> 
											<div class="form-group form-group-sm required">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp" name="refp" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference polygons. A .zip file containing the shapefile is expected. See Software User Manual for the format of the shapefile. Take care of the header of the columns.</span>
											</div>
											<?php }?>
											
											<?php if($processor['short_name'] == "l4a_wo" && $isSen2Agri){?> 										
											<div class="form-group form-group-sm ">
												<label class="control-label" for="refr">Reference raster:</label>
												<input type="file" class="form-control" id="refr" name="refr" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference raster when in situ data is not available.</span>
											</div>
											<?php }?>
										   <!-- End Particular parameters -->
										   
											<!--  advance params -->
										    <span id="advParam_<?=$processor['short_name']?>"></span>
											<!-- end advance params -->

											<input name="proc_div" type="hidden" value="<?=$processor['short_name']?>">
											<input name="<?=$processor['short_name']?>" type="submit" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L3A -->
					<?php }
    				}?>

				</div>

			</div>
			<div class="clearing">&nbsp;</div>
		</div>
	</div>
</div>
<!-- main -->
<!-- main2 -->
<!-- main3 -->

<!-- Diablog message -->
<div id="dialog-message" class="dialog" title="Submit successful">
	<p>
		<span class="ui-icon ui-icon-circle-check spanDialogMsg"></span>
		<span id="dialog-content"></span>
	</p>
</div>
<!-- Diablog wait -->
<div id="dialog-wait" class="dialog" title="Processing request">
	<p>
		<span class="ui-icon ui-icon-circle-check spanDialogMsg"></span>
		<span id="dialog-content"></span>
	</p>
</div>
<!-- Diablog message -->
<div id="dialog-error" class="dialog" title="Submit failed">
	<p>
		<span class="ui-icon ui-icon-circle-check spanDialogMsg"></span>
		<span id="dialog-content"></span>
	</p>
</div>

<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>
<script src="scripts/processing_functions.js"></script>
<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<script>
    var processorList = [];
    processorList = <?php echo json_encode( $processorList)?>;

	var l2a_proc_id;
	var l4a_proc_id;

	function open_dialog(message) {
		$("#dialog-message #dialog-content").text(message);
		$("#dialog-message").dialog("open");
	};
	function open_dialog_wait(message) {
		$("#dialog-wait #dialog-content").text(message);
		$("#dialog-wait").dialog("open");
	};
	function open_dialog_error(message) {
		$("#dialog-error #dialog-content").text(message);
		$("#dialog-error").dialog("open");
	};
	function reset_form(form_name) {
		$("#"+form_name)[0].reset();
		$("#"+form_name+" #inputFiles").find('option').remove().end();
	};

	$(document).ready(function() {

		function generateHTMLElem(type){
			switch(type){
    			case 'float':
    			case 'int':
    				var newElem = $('<input />').attr({  type: "number", class: "form-control"});
            		break;
    			case 'string':
    				var newElem = $('<input />').attr({  type: "text", class: "form-control"});
        			break;
    			case 'file':
    				var newElem = $('<select />').attr({ class: "form-control"});
            		break;
    			case 'bool':
    				var newElem = $('<input />').attr({  type: "radio", class: "form-control"});
    				break;
    			case 'directory':
    				var newElem = $('<input />').attr({  type: "file", class: "form-control"});
					break;
    			case 'select':
    				var newElem = $('<select multiple=""></select>').attr({  size: "7", class: "form-control input-files"});
    				break;
    		}
    		return newElem;
		
			}
		
		function displaySpecificParameter(data, processor){
			var values = JSON.parse(data.values);
			var elemName = (!$.isEmptyObject(values) && values.hasOwnProperty('name')) ? values.name : data.param_name+'_'+processor;
			var newDiv = $('<div/>').attr({class:"form-group form-group-sm specific"});
			var newLabel = $('<label/>').attr({class:"control-label",for: elemName}).text(data.label+":");		
			var newElem = generateHTMLElem(data.type);
			newElem.attr({id: data.param_name+'_'+processor, name: elemName});
			if(newElem.is('input')){
				newElem.attr({value:data.param_value});
			} else if (newElem.is('select') && !$.isEmptyObject(values) && values.hasOwnProperty('product_type_id')) {
				$(newElem).data('product_type_id', values['product_type_id']);
			}
			
			var span = '';
			if(data.friendly_name !=""){
				var span = $('<span/>').attr({class:"help-block"}).text(data.friendly_name);
				}
									
			return 	$("#specificParam_"+processor).after(newDiv.append(newLabel).append(newElem).append(span));
			
			}

		function displayAdvancedParameter(data, processor){
			var values = JSON.parse(data.values);
			var elemName = (!$.isEmptyObject(values) && values.hasOwnProperty('name')) ? values.name : data.param_name+'_'+processor;
			var newDiv = $('<div/>').attr({class:"form-group form-group-sm advanced"});
			var newLabel = $('<label/>').attr({class:"control-label",for:elemName}).text(data.label+":");		
			var newElem = generateHTMLElem(data.type);
			newElem.attr({id: data.param_name+'_'+processor, name: elemName});
			if(newElem.is('input')){
				newElem.attr({value:data.param_value});
			}
			
			var span = '';
			if(data.frendly_name !=""){
				var span = $('<span/>').attr({class:"help-block"}).text(data.friendly_name);
				}
			
			if(!$.isEmptyObject(values) && values.hasOwnProperty('type') && (values.type == "classifier" || values.type == "segmentation")){
				var classElem = values.type;
			
				if($("#"+processor+"form ."+classElem).length > 0){	
					
					//if element with class 'classifier' or 'segmentation' exist then append new element to it
    				return $("#"+processor+"form ."+classElem).append(newDiv.append(newLabel).append(newElem).append(span));    				
				}else{	
					
					//if element with class 'subgroup' doesn't exist then creat it and append element to it
					var subgroup = $('<div/>').attr({class:"subgroup advanced "+classElem});
    				subgroup.append($('<label/>').attr({class:"control-label", style:"text-transform:capitalize"}).text( classElem+" parameters:"));

    				return $("#advParam_"+processor).after(subgroup.append(newDiv.append(newLabel).append(newElem).append(span)));
					}
				
			}else{
									
				return 	$("#advParam_"+processor).after(newDiv.append(newLabel).append(newElem).append(span));
				
				}
			
			}
            
        function filterParameters(params) {
            return params.filter(function f(currentElement, index, array) {
                return currentElement.site_id == null;
            });
        }

	   	//get advanced parameters for each processor
		var advancedParams = $(".control_advanced>input");
	    $.each(advancedParams,function(index,advParam){
	        
	    	$(advParam).on('click',function(event){	
	    		if ($(this).is(':checked')) { 
	        		$(this).parent().parent().find(".advanced").removeClass("hidden"); 
	    		}
	        	else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	        });

	    	var id = this.id;
	    	var current_processor = id.replace('_advanced','');
	    	
	    	$.ajaxSetup({
	    	        async: false
	    	    });
	    	
	    	var ajaxAdvParam = $.ajax({
	    		type:"GET",
	    		data:{action:'getParameters',current_processor: current_processor},
	    		dataType:'json'
	        		});

			ajaxAdvParam.done(function(data){
                // first filter the parameters such that we can have only one instance of each parameter even it belongs to several sites
				if(data.hasOwnProperty('advanced') && !$.isEmptyObject(data.advanced)){
                    var advancedParams = filterParameters(data.advanced);	    			
    				$.each(advancedParams,function(param){ 
    					 displayAdvancedParameter(advancedParams[param], current_processor);
    					});

    				// hide all advanced parameters
    				$('.advanced').addClass("hidden");
	    		}
				if (data.hasOwnProperty('specific') && !$.isEmptyObject(data.specific)) {
                    var specificParams = filterParameters(data.specific);	    			
					$.each(specificParams, function (param) {
						displaySpecificParameter(specificParams[param], current_processor);   				
					});
				}
				}).fail( function (responseData, textStatus, errorThrown) {
	    			console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
	    		});	    

			  $.ajaxSetup({
			        async: true
			    });

			//get specific params for each processor			
	    });

		
		initialise_datepickers();
		// add events to elements used to filter input files
		add_events();
		
		// load sites
		get_all_sites();
		<?php //if(ConfigParams::isSen2Agri()){?>
		get_processor_id('l2a', 'l2a_proc_id');
		get_processor_id('l4a', 'l4a_proc_id');
        <?php //}?>
		// initialize date picker
		$("#synthDate").datepicker({
			dateFormat: "yymmdd",
			onSelect: function() { $(this).keyup(); } // force validation after selection
		});

		// initialize dialogs
		$.each($('.dialog'),function(){
			
			var popUp = $(this).dialog({
				width: '400px',
				autoOpen: false,
				modal: true,
				buttons: { Ok: function() { $(this).dialog("close"); } }
			});

			if($(this)[0].id == 'dialog-wait') popUp.dialog({buttons: { }});
			if($(this)[0].id == 'dialog-error') popUp.parent().children(".ui-dialog-titlebar").addClass('ui-state-error');
			
		});
		
		$("input[type='submit']").click(function() {
			var form_valid = false;
			 form_valid = $("#"+$(this).attr('name')+'form').valid();
			
			if (form_valid) {
				open_dialog_wait("Please wait, your request is being processed!");
			}
		});
		
		<?php
		// Check if this is a redirect from a FORM being submitted
		if (isset($_SESSION['processor'])) {
			echo "$('#" . $_SESSION['proc_div'] . "').collapse('show');";
			echo "$('#" . $_SESSION['proc_div'] . " #siteId').trigger('focus');";
			echo ($_SESSION['status'] === "OK" ? "open_dialog('" : "open_dialog_error('").$_SESSION['message']."')";
			
			unset($_SESSION['processor']);
			unset($_SESSION['proc_div']);
			unset($_SESSION['status']);
			unset($_SESSION['message']);
		}
		?>

		// validate form on keyup and submit
		$("form").each(function(){
			$(this).validate({
				rules: {
					siteId:			{ required: true },
					synthDate:		{ required: true, pattern: "[0-9]{4}(0[1-9]|1[0-2])(0[1-9]|[1-2][0-9]|3[0-1])" },
					genmodel:		{ pattern: "[0-1]{1}" }, //for l3b LAI
					refp:           { required: true },// for l4a
				},
				messages: {
					synthDate: { pattern : "Enter a date in YYYYMMDD format" }
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
				// set this class to error-labels to indicate valid fields
				success: function(label) {
					label.remove();
				},
			});
			if ($("select[name*='inputFiles']", this).length > 0) {
				$.each($("select[name*='inputFiles']", this), function () {
					$(this).rules("add", { required: true });
				});
			}
		});




	});
</script>
<?php include "ms_foot.php"; ?>
