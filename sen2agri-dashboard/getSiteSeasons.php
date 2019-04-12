<?php
session_start();
require_once('ConfigParams.php');

function get_active_processors($seasonId) {
    $db = pg_connect(ConfigParams::getConnection()) or die ("Could not connect");
	if ($seasonId == 0) {
		$result = pg_query($db, "SELECT id, short_name FROM sp_get_processors() WHERE name NOT LIKE '%NDVI%'") or die ("Could not execute.");
	} else {
		$result = pg_query_params($db, "SELECT processor_id, processor_short_name FROM sp_get_season_scheduled_processors($1)", array($seasonId)) or die ("Could not execute.");
	}
	if (pg_num_rows($result) > 0) {
		?>
		<div class="proc<?= ($seasonId == 0) ? " hidden" : "" ?>">
			<h2>Hover to reveal</h2>
			<div><?php
			while ( $row = pg_fetch_row ( $result ) ) {
				$id = $row[0];
				$short = substr(strtoupper($row[1]), 0, 7);
				?>
				<label><input name="<?= strtolower($short) ?>" data-id="<?= $id ?>" type="checkbox"<?= ($seasonId > 0) || ($short == "L2A") || ($short == "L2-S1") || ($short == "LPIS") ? " checked disabled"  : "" ?>>&nbsp;<?= $short ?></label>
			<?php } ?>
			</div>
		</div>
		<?php
	}
}
function set_season_line($seasonId, $seasonName, $seasonStart, $seasonMid, $seasonEnd, $seasonEnabled) {
?>
	<tr data-id="<?= $seasonId ?>">
		<td><input class="form-control" type="text" name="season_name"  data-save="<?= $seasonName  ?>" value="<?= $seasonName  ?>" disabled></td>
		<td><input class="form-control" type="text" name="season_start" data-save="<?= $seasonStart ?>" value="<?= $seasonStart ?>" disabled></td>
		<td><input class="form-control" type="text" name="season_mid"   data-save="<?= $seasonMid   ?>" value="<?= $seasonMid   ?>" disabled></td>
		<td><input class="form-control" type="text" name="season_end"   data-save="<?= $seasonEnd   ?>" value="<?= $seasonEnd   ?>" disabled></td>
		<td><input class="form-control" type="checkbox" name="season_enabled" data-save="<?= ($seasonEnabled ? "t" : "f" ) ?>"<?= ($seasonEnabled ? " checked" : "" ) ?> disabled></td>
		<td><?php get_active_processors($seasonId) ?></td>
		<td>
		<?php if ($seasonId > 0) { ?>
			<input type="button" name="edit"   title="Edit season"   class="icon">
			<input type="button" name="save"   title="Save season"   class="icon hidden">
			<input type="button" name="remove" title="Remove season" class="icon">
			<input type="button" name="cancel" title="Cancel"        class="icon hidden">
		<?php } else { ?>
			<input type="button" name="add"    title="Add season"  class="icon">
			<input type="button" name="save"   title="Save season" class="icon hidden">
			<input type="button" name="cancel" title="Cancel"      class="icon hidden">
		<?php } ?>
		</td>
	</tr>
<?php
}

if (isset($_REQUEST["action"]) && isset($_REQUEST["siteId"])) {
    $db = pg_connect ( ConfigParams::getConnection() ) or die ( "Could not connect" );
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_site_seasons($1)", array ($_REQUEST['siteId']) ) or die ( "Could not execute." );
	
	// get list of seasons to be displayed on the main page
	if ($_REQUEST["action"] == "get") {
		if (pg_num_rows($result) > 0) { ?>
			<table class='subtable'>
			<?php
			while ( $row = pg_fetch_row ( $result ) ) {
				$seasonName     = $row[2];
				$seasonStart    = $row[3];
				$seasonEnd      = $row[4];
				$seasonMid      = $row[5];
				$season_enabled = $row[6] == "t" ? true : false;
				?>
				<tr>
					<td><?= $seasonName  ?></td>
					<td><?= $seasonStart ?></td>
					<td><?= $seasonMid   ?></td>
					<td><?= $seasonEnd   ?></td>
					<td><input class="form-control" type="checkbox" name="season_enabled"<?= ($season_enabled ? " checked" : "" ) ?>></td>
				</tr>
			<?php } ?>
			</table>
		<?php } ?>
<?php // get list of seasons for edit purposes
	} elseif ($_REQUEST["action"] == "edit") { ?>
		<table class="edit" id="seasons">
			<thead>
				<tr>
					<th>Season name</th>
					<th>Season start</th>
					<th>Season mid</th>
					<th>Season end</th>
					<th>Enabled</th>
					<th>Active processors</th>
					<th class="action">Action</th>
				</tr>
			</thead>
			<tbody>
			<?php
			while ( $row = pg_fetch_row ( $result ) ) {
				$seasonId      = $row[0];
				$seasonName    = $row[2];
				$seasonStart   = $row[3];
				$seasonEnd     = $row[4];
				$seasonMid     = $row[5];
				$seasonEnabled = $row[6] == "t" ? true : false;
				set_season_line($seasonId, $seasonName, $seasonStart, $seasonMid, $seasonEnd, $seasonEnabled);
			 }
			 ?>
			</tbody>
		</table>
		<table class="add-line hidden">
			<tbody>
				<?php set_season_line(0, "", "", "", "", false); ?>
			</tbody>
		</table>
		<div id="server-response"></div>
		<script>
		function refreshDialogPosition() {
			$("#div_editsite").dialog("option", "position", {my: "center", at: "center", of: window});
		}
		function set_add_table_line() {
			var tr_line = $("table.add-line tbody").html();
			$('#seasons>tbody:last-child').append(tr_line);
		}
		function replace_add_table_line() {
			//replace buttons for the newly added season
			var td_line =
			'<input type="button" name="edit"   title="Edit season"   class="icon">' +
			'<input type="button" name="save"   title="Save season"   class="icon hidden">' +
			'<input type="button" name="remove" title="Remove season" class="icon">' +
			'<input type="button" name="cancel" title="Cancel"        class="icon hidden">';
			$('#seasons>tbody>tr:last-child>td:last-child').html(td_line);
			
			//insert a new add season line
			set_add_table_line();
			set_button_actions();
			set_datepicker();
			refreshDialogPosition();
		}
		function set_button_actions() {
			$("#seasons input[name='edit']").unbind( "click" );
			$("#seasons input[name='edit']").click(function() {
				var parent = $(this).parent().parent();
				
				// enable all season controls
				$(parent).find("td input.form-control").prop("disabled", false);
				
				// enable bootstrapSwitch for season_enabled
				var boots = $(parent).find("input[name='season_enabled']");
				$(boots).bootstrapSwitch('disabled',false);
				
				// show/hide action buttons
				$(this).addClass("hidden");
				$(parent).find("input[name='remove']").addClass("hidden");
				$(parent).find("input[name='save']").removeClass("hidden");
				$(parent).find("input[name='cancel']").removeClass("hidden");
			});
			$("#seasons input[name='add']").unbind( "click" );
			$("#seasons input[name='add']").click(function() {
				var parent = $(this).parent().parent();
				
				// enable all season controls
				$(parent).find("td input.form-control").prop("disabled", false);
				
				// set visibility for active processors
				$(parent).find("div.proc").removeClass("hidden");
				
				// enable bootstrapSwitch for season_enabled
				var boots = $(parent).find("input[name='season_enabled']");
				$(boots).bootstrapSwitch('disabled',false);
				
				// show/hide action buttons
				$(this).addClass("hidden");
				$(parent).find("input[name='save']").removeClass("hidden");
				$(parent).find("input[name='cancel']").removeClass("hidden");
			});
			$("#seasons input[name='cancel']").unbind( "click" );
			$("#seasons input[name='cancel']").click(function() {
				var parent = $(this).parent().parent();
				if ($(parent).data("id") == 0) {
				// *** cancel add new season ***
					// disable all season controls
					$(parent).find("td input.form-control").prop("disabled", true);
					
					// uncheck bootstrapSwitch for season_enabled and then disable it
					var boots = $(parent).find("input[name='season_enabled']");
					$(boots).bootstrapSwitch('state', false);
					$(boots).bootstrapSwitch('disabled', true);
					
					// restore state of all active processors and hide them
					$(parent).find("div.proc input[type='checkbox']:enabled").attr("checked", false);
					$(parent).find("div.proc").addClass("hidden");
					
					// show/hide action buttons
					$(parent).find("input[name='save']").addClass("hidden");
					$(parent).find("input[name='add']").removeClass("hidden");
				} else {
				// *** cancel update season ***
					$($(parent).find("td input.form-control")).each(function() {
						if($(this).attr("type") == "checkbox") {
							// restore previous state of bootstrapSwitch for season_enabled and then disable it
							if ($(this).attr("name") == "season_enabled") {
								$(this).bootstrapSwitch('state', $(this).data("save") == "t");
								$(this).bootstrapSwitch('disabled', true);
							}
						} else {
							// restore previous value for season control
							$(this).val($(this).data("save"));
						}
						// disable season control
						$(this).prop("disabled", true);
					});
					
					// show/hide action buttons
					$(parent).find("input[name='save']").addClass("hidden");
					$(parent).find("input[name='edit']").removeClass("hidden");
					$(parent).find("input[name='remove']").removeClass("hidden");
				}
				// hide cancel action button
				$(this).addClass("hidden");
				
				$("#server-response").html("");
			});
			$("#seasons input[name='save']").unbind( "click" );
			$("#seasons input[name='save']").click(function() {
				// read season "form" data
				var parent         = $(this).parent().parent();
				var site_id        = <?= $_REQUEST['siteId'] ?>;
				var season_id      = $(parent).data("id");
				var season_name    = $(parent).find("td input[name='season_name']").val();
				var season_start   = $(parent).find("td input[name='season_start']").val();
				var season_mid     = $(parent).find("td input[name='season_mid']").val();
				var season_end     = $(parent).find("td input[name='season_end']").val();
				var season_enabled = $(parent).find("td input[name='season_enabled']").bootstrapSwitch("state") ? "t" : "f";
				var processors     = new Array();
				$(parent).find("div.proc input[type='checkbox']:checked:enabled").each(function() {
					processors.push($(this).data("id"));
				});
				
				// save season to database
				$.ajax({
					url: "manageSiteSeason.php",
					type: "get",
					cache: false,
					crosDomain: true,
					data: { "action":           "save",
							"siteId":           site_id,
							"seasonId":         season_id,
							"seasonName":       season_name,
							"seasonStart":      season_start,
							"seasonMiddle":     season_mid,
							"seasonEnd":        season_end,
							"seasonEnabled":    season_enabled,
							"activeProcessors": processors },
					dataType: "html",
					success: function(data) {
						if (data.match("^SUCCESS")) {
							// show/hide action buttons
							$(parent).find("input[name='save']").addClass("hidden");
							$(parent).find("input[name='cancel']").addClass("hidden");
							$(parent).find("input[name='edit']").removeClass("hidden");
							$(parent).find("input[name='remove']").removeClass("hidden");
							
							$(parent).find("td input.form-control").each(function() {
								if($(this).attr("type") == "checkbox") {
									// save new state of bootstrapSwitch for season_enabled and then disable it
									if ($(this).attr("name") == "season_enabled") {
										var chk = $(this).bootstrapSwitch("state") ? "t" : "f";
										$(this).attr("data-save", chk);
										$(this).data("save", chk);
										$(this).bootstrapSwitch('disabled', true);
									}
								} else {
									// save new values for season controls
									var sav = $(this).val();
									$(this).attr("data-save", sav);
									$(this).data("save", sav);
								}
								// disable season control
								$(this).prop("disabled", true);
							});
							if (data.match("^SUCCESS: added")) {
								// disable controls for active processors
								if ($(parent).find("div.proc input[type='checkbox']:checked").length == 1) {
									$(parent).find("div.proc").remove();
								} else {
									$(parent).find("div.proc input[name='l2a']").parent().remove();
									$(parent).find("div.proc input[type='checkbox']:not(:checked)").parent().remove();
									$(parent).find("div.proc input[type='checkbox']").attr("disabled", true);
								}
								
								// create new "add line" after new season was added
								var new_season_id = data.substr(15);
								$(parent).attr("data-id", new_season_id);
								$(parent).data("id", new_season_id);
								replace_add_table_line();
							}
							
							$("#server-response").html("");
						} else {
							$("#server-response").html(data);
						}
					},
					error: function (responseData, textStatus, errorThrown) {
						console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
					}
				});
			});
			$("#seasons input[name='remove']").unbind( "click" );
			$("#seasons input[name='remove']").click(function() {
				if (confirm('Are you sure you want to delete this season?')) {
					var parent = $(this).parent().parent();
					var season_id = $(parent).data("id");
					$.ajax({
						url: "manageSiteSeason.php",
						type: "get",
						cache: false,
						crosDomain: true,
						data: { "action":   "remove",
								"seasonId": season_id },
						dataType: "html",
						success: function(data) {
							if (data.match("^SUCCESS: removed")) {
								$(parent).remove();
								$("#server-response").html("");
								refreshDialogPosition();
							} else {
								$("#server-response").html(data);
							}
						},
						error: function (responseData, textStatus, errorThrown) {
							console.log("Response: " + responseData + "   Status: " + textStatus + "   Error: " + errorThrown);
						}
					});
				}
			});
			// set bootstrapSwitch for all season_enabled controls
			$("#seasons").find("input[name='season_enabled']").bootstrapSwitch({
				size: "mini",
				onColor: "success",
				offColor: "default",
				disabled: true,
				handleWidth: 25
			});
		}
		function set_datepicker() {
			$("table.edit input[name='season_start']").datepicker({dateFormat: "yy-mm-dd"});
			$("table.edit input[name='season_mid']").datepicker({dateFormat: "yy-mm-dd"});
			$("table.edit input[name='season_end']").datepicker({dateFormat: "yy-mm-dd"});
		}

/*
		// onChange event for start datepickers for add site form
		$("#startseason_winter").on('change dp.change', function() {
			var date1 = $('#startseason_winter').datepicker('getDate');
			date1.setDate(date1.getDate() + 1);
			if ($('#endseason_winter').datepicker('getDate') === null) {
				$('#endseason_winter').datepicker('setDate', date1);
			}
		});
*/
		set_add_table_line();
		set_button_actions();
		set_datepicker();
		refreshDialogPosition();
		</script>
<?php
	}
}
?>
