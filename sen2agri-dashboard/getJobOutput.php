<?php include "master.php";?>
<div id="main">
	<div id="main2">
		<div id="main3">
<?php			
	if (!isset($_REQUEST['jobId'])) {
        http_response_code(400);
        exit();
    }
?>

<style>
    .copy-link {
        cursor: pointer;
    }
    .hidden {
        display: none;
    }
</style>
<?php
$db = pg_connect(ConfigParams::getConnection())
            or die ("Could not connect");

    $jobId = $_REQUEST['jobId'];
    $result = pg_query_params($db, 'SELECT * FROM sp_get_job_output($1)', array($jobId))
                or die ("Could not execute query.");

    $output = '<div class="for-table">' .
			'<table class="table table-striped" style="border:1px solid #dadada">' .
                '<thead>' .
                  '<tr>' .
                    '<th>Step name</th>' .
                    '<th>Command</th>' .
                    '<th>Output</th>' .
                    '<th>Errors</th>' .
                    '<th>Exit code</th>' .
                  '</tr>' .
                '</thead>' .
                '<tbody id="log" style="background-color:white;font-size:1em">';
					$rows = 0;
					while ($row = pg_fetch_row($result)) {
						$rows ++;
						$status = $row[4];
                        $execution_status = 6;  // execution status is FINISHED (6)
                        if (sizeof($row) >= 6) {
                            $execution_status = $row[5];
                        }
                        switch ($execution_status) {
                            case 1:     // "Submitted"
                            case 2:     // "PendingStart"
                                $status = html_entity_decode("&#x231A;");   // WATCH
                                break;
                            case 3:     // "NeedsInput"
                                $status = '⚠️️';   
                                break;
                            case 4:     // "Running"
                                $status = html_entity_decode("&#x26A1;");   // HIGH VOLTAGE SIGN
                                break;
                            case 5:     // "Paused"
                                $status = html_entity_decode("&#x270B;");     // RAISED HAND
                                break;
                            case 6:     // "Finished"
                                if ($status !== '0') { 
                                    // Normally, if execution status exists and is set in the database, this should not happen.
                                    // Nevertheless, for the old version when the execution_status did not exist in database, this is the case 
                                    // when the job did not started yet (execution_status is forced to 6 in one of the lines above)
                                    if (strlen(strval($status)) == 0) {
                                        $status = '⚠️️';
                                    } else {
                                        $status .= html_entity_decode("&#x274C;");  // CROSS MARK
                                    }
                                } else { 
                                    $status .= html_entity_decode("&#x274E;");  // NEGATIVE GREEN SQUARED CROSS MARK
                                }
                                break;
                            case 7:     // "Cancelled"
                                $status = html_entity_decode("&#x26D4;"); // NO ENTRY
                                break;
                            case 8:     // "Error"
                                $status .= html_entity_decode("&#x274C;");      // CROSS MARK
                            default:
                                $status .= html_entity_decode("&#x2753;"); // BLACK QUESTION MARK ORNAMENT
                                break;
                        }
						$status = htmlentities($status);
						$tr =	'<tr>'.
									'<td>' . htmlentities($row[0]) . '</td>'.
									'<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[1]) . '</textarea></td>'.
									'<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[2]) . '</textarea></td>'.
									'<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[3]) . '</textarea></td>'.
									'<td>' . $status  . '</td>'.
								'</tr>';
						$output .= $tr;
					}
					if ($rows == 0) {
						$tr =	'<tr style="background-color:#fffbd3;color:#868686">'.
									'<td colspan="5"><i>No jobs to be displayed</i></td>'.
								'</tr>';
						$output .= $tr;
					}
					$output .= '</tbody>' .
			'</table>'.
		'</div>';
    echo $output;
?>
		</div>
	</div>
</div>
<script>
    function loadHandler() {
        document.getElementById("log").addEventListener("click", function (e) {
            var link = e.target;
            if (!link.classList.contains("copy-link"))
                return;

            link.classList.add("hidden");
            var textArea = link.nextElementSibling;
            textArea.classList.remove("hidden");

            textArea.select();
            document.execCommand("copy");

            textArea.addEventListener("blur", function (e) {
                var textArea = e.target;
                textArea.classList.add("hidden");
                textArea.previousElementSibling.classList.remove("hidden");
            });
        });
    }

    if (document.readyState !== "loading") {
        loadHandler();
    } else {
        document.addEventListener("DOMContentLoaded", loadHandler);
    }
</script>
<?php include "ms_foot.php"; ?>