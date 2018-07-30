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

    $output = '<table>' .
                '<thead>' .
                  '<tr>' .
                    '<td>Step name</td>' .
                    '<td>Command</td>' .
                    '<td>Output</td>' .
                    '<td>Errors</td>' .
                    '<td>Exit code</td>' .
                  '</tr>' .
                '</thead>' .
                '<tbody id="log">';

    while ($row = pg_fetch_row($result)) {
        $status = $row[4];
        if ($status !== '0') {
            $status .= '⚠️️';
        }
        $status = htmlentities($status);
        $tr = '<tr>'.
                '<td>' . htmlentities($row[0]) . '</td>'.
                '<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[1]) . '</textarea></td>'.
                '<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[2]) . '</textarea></td>'.
                '<td><a class="copy-link">[Copy to clipboard]</a><textarea class="hidden">' . htmlentities($row[3]) . '</textarea></td>'.
                '<td>' . $status  . '</td>'.
              '</tr>';
        $output .= $tr;
    }
    $output .= '</tbody>' .
        '</table>';

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