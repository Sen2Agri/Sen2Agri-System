<?php
session_start();
require_once('ConfigParams.php');

if (isset($_REQUEST["action"]) && isset($_REQUEST["siteId"])) {
	$db = pg_connect ( ConfigParams::$CONN_STRING ) or die ( "Could not connect" );
	$result = pg_query_params ( $db, "SELECT * FROM sp_get_site_seasons($1)", array ($_REQUEST['siteId']) ) or die ( "Could not execute." );
	
	// get list of seasons to be displayed on the main page
	if ($_REQUEST["action"] == "get") { ?>
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
					<th class="action">Action</th>
				</tr>
			</thead>
			<tbody>
			<?php
			while ( $row = pg_fetch_row ( $result ) ) {
				$seasonId       = $row[0];
				$seasonName     = $row[2];
				$seasonStart    = $row[3];
				$seasonEnd      = $row[4];
				$seasonMid      = $row[5];
				$season_enabled = $row[6] == "t" ? true : false;
				?>
				<tr data-id="<?= $seasonId ?>">
					<td><input class="form-control" type="text" name="season_name"  data-save="<?= $seasonName  ?>" value="<?= $seasonName  ?>" disabled></td>
					<td><input class="form-control" type="text" name="season_start" data-save="<?= $seasonStart ?>" value="<?= $seasonStart ?>" disabled></td>
					<td><input class="form-control" type="text" name="season_mid"   data-save="<?= $seasonMid   ?>" value="<?= $seasonMid   ?>" disabled></td>
					<td><input class="form-control" type="text" name="season_end"   data-save="<?= $seasonEnd   ?>" value="<?= $seasonEnd   ?>" disabled></td>
					<td><input class="form-control" type="checkbox" name="season_enabled" data-save="<?= ($season_enabled ? "t" : "f" ) ?>"<?= ($season_enabled ? " checked" : "" ) ?> disabled></td>
					<td>
						<input name="edit" class="icon" type="button" title="Edit season">
						<input name="save" class="icon hidden" type="button" title="Save season">
						<input name="remove" class="icon" type="button" title="Remove season">
						<input name="cancel" class="icon hidden" type="button" title="Cancel">
					</td>
				</tr>
			<?php } ?>
				<tr data-id="0">
					<td><input class="form-control" type="text" name="season_name" data-save="" disabled></td>
					<td><input class="form-control" type="text" name="season_start" data-save="" disabled></td>
					<td><input class="form-control" type="text" name="season_mid" data-save="" disabled></td>
					<td><input class="form-control" type="text" name="season_end" data-save="" disabled></td>
					<td><input class="form-control" type="checkbox" name="season_enabled" data-save="f" disabled></td>
					<td>
						<input name="add" class="icon" type="button" title="Add season">
						<input name="save" class="icon hidden" type="button" title="Save season">
						<input name="cancel" class="icon hidden" type="button" title="Cancel">
					</td>
				</tr>
			<tbody>
		</table>
		<div id="server-response"></div>
		<script>
		function refreshDialogPosition() {
			$("#div_editsite").dialog("option", "position", {my: "center", at: "center", of: window});
		}
		function replace_add_table_line() {
			var td_line =
			'<input name="edit" class="icon" type="button" title="Edit season">' +
			'<input name="save" class="icon hidden" type="button" title="Save season">' +
			'<input name="remove" class="icon" type="button" title="Remove season">' +
			'<input name="cancel" class="icon hidden" type="button" title="Cancel">';
			var tr_line =
			'<tr data-id="0">' +
			'	<td><input class="form-control" type="text" name="season_name" data-save="" disabled></td>' +
			'	<td><input class="form-control" type="text" name="season_start" data-save="" disabled></td>' +
			'	<td><input class="form-control" type="text" name="season_mid" data-save="" disabled></td>' +
			'	<td><input class="form-control" type="text" name="season_end" data-save="" disabled></td>' +
			'	<td><input class="form-control" type="checkbox" name="season_enabled" data-save="f" disabled></td>' +
			'	<td>' +
			'		<input name="add" class="icon" type="button" title="Save season">' +
			'		<input name="save" class="icon hidden" type="button" title="Save season">' +
			'		<input name="cancel" class="icon hidden" type="button" title="Cancel">' +
			'	</td>' +
			'</tr>';
			//replace buttons for the newly added season
			$('#seasons>tbody>tr:last-child>td:last-child').html(td_line);
			//insert a new add season line
			$('#seasons>tbody>tr:last-child').after(tr_line);
			set_button_actions();
			set_datepicker();
			refreshDialogPosition();
		}
		function set_button_actions() {
			$("#seasons input[name='edit']").unbind( "click" );
			$("#seasons input[name='edit']").click(function() {
				var parent = $(this).parent().parent();
				$(parent).find("td>input.form-control").prop("disabled", false);
				$(this).addClass("hidden");
				$(parent).find("input[name='remove']").addClass("hidden");
				$(parent).find("input[name='save']").removeClass("hidden");
				$(parent).find("input[name='cancel']").removeClass("hidden");
			});
			$("#seasons input[name='add']").unbind( "click" );
			$("#seasons input[name='add']").click(function() {
				var parent = $(this).parent().parent();
				$(parent).find("td>input.form-control").prop("disabled", false);
				$(this).addClass("hidden");
				$(parent).find("input[name='save']").removeClass("hidden");
				$(parent).find("input[name='cancel']").removeClass("hidden");
			});
			$("#seasons input[name='cancel']").unbind( "click" );
			$("#seasons input[name='cancel']").click(function() {
				var parent = $(this).parent().parent();
				if ($(parent).data("id") == 0) {
					// cancel add new season
					$(parent).find("td>input[type='checkbox']").prop("checked",false);
					$(parent).find("td>input.form-control").prop("disabled", true);
					$(parent).find("input[name='save']").addClass("hidden");
					$(parent).find("input[name='add']").removeClass("hidden");
				} else {
					// cancel update season
					$($(parent).find("td>input.form-control")).each(function() {
						if($(this).attr("type") == "checkbox") {
							$(this).prop("checked", ($(this).data("save") == "t"));
						} else {
							$(this).val($(this).data("save"));
						}
						$(this).prop("disabled", true);
					});
					$(parent).find("input[name='save']").addClass("hidden");
					$(parent).find("input[name='edit']").removeClass("hidden");
					$(parent).find("input[name='remove']").removeClass("hidden");
				}
				$(this).addClass("hidden");
				$("#server-response").html("");
			});
			$("#seasons input[name='save']").unbind( "click" );
			$("#seasons input[name='save']").click(function() {
				var parent         = $(this).parent().parent();
				var site_id        = <?= $_REQUEST['siteId'] ?>;
				var season_id      = $(parent).data("id");
				var season_name    = $(parent).find("td>input[name='season_name']").val();
				var season_start   = $(parent).find("td>input[name='season_start']").val();
				var season_mid     = $(parent).find("td>input[name='season_mid']").val();
				var season_end     = $(parent).find("td>input[name='season_end']").val();
				var season_enabled = $(parent).find("td>input[name='season_enabled']").prop("checked") ? "t" : "f";
				$.ajax({
					url: "manageSiteSeason.php",
					type: "get",
					cache: false,
					crosDomain: true,
					data: { "action":        "save",
							"siteId":        site_id,
							"seasonId":      season_id,
							"seasonName":    season_name,
							"seasonStart":   season_start,
							"seasonMiddle":  season_mid,
							"seasonEnd":     season_end,
							"seasonEnabled": season_enabled },
					dataType: "html",
					success: function(data) {
						if (data.match("^SUCCESS")) {
							$(parent).find("input[name='save']").addClass("hidden");
							$(parent).find("input[name='cancel']").addClass("hidden");
							$(parent).find("input[name='edit']").removeClass("hidden");
							$(parent).find("input[name='remove']").removeClass("hidden");
							$(parent).find("td>input.form-control").each(function() {
								if($(this).attr("type") == "checkbox") {
									var chk = $(this).prop("checked") ? "t" : "f";
									$(this).attr("data-save", chk);
									$(this).data("save", chk);
								} else {
									var sav = $(this).val();
									$(this).attr("data-save", sav);
									$(this).data("save", sav);
								}
								$(this).prop("disabled", true);
							});
							if (data.match("^SUCCESS: added")) {
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
		}
		function set_datepicker() {
			$("input[name='season_start']").datepicker({dateFormat: "yy-mm-dd"});
			$("input[name='season_mid']").datepicker({dateFormat: "yy-mm-dd"});
			$("input[name='season_end']").datepicker({dateFormat: "yy-mm-dd"});
		}
		
		// onChange event for start datepickers for add site form
		$("#startseason_winter").on('change dp.change', function() {
			var date1 = $('#startseason_winter').datepicker('getDate');
			date1.setDate(date1.getDate() + 1);
			if ($('#endseason_winter').datepicker('getDate') === null) {
				$('#endseason_winter').datepicker('setDate', date1);
			}
		});
		
		set_button_actions();
		set_datepicker();
		refreshDialogPosition();
		</script>
<?php
	}
}
?>
