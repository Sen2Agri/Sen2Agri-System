<?php
require_once("ConfigParams.php");

function getDatabaseVersion() {
	$dbconn = pg_connect(ConfigParams::$CONN_STRING) or die ("Could not connect");
	$rows = pg_query($dbconn, "select version from meta;") or die(pg_last_error());
	return (pg_numrows($rows) > 0 ? pg_fetch_array($rows, 0)[0] : "");
}

$version = getDatabaseVersion();
?>

    <div id="bar">
        <div id="bar2">
            <div id="bar3">
                <div class="clearing" style="height:20px;">&nbsp;</div>
				<div class='message_footer'>
<?php
if (isset($_SESSION['userName'])) {
?>
					Logged in as <?php echo $_SESSION['userName']; ?> |
<?php
}
?>
				DB version <?php echo($version); ?></div>
			</div>
        </div>
    </div>
    <div id="footer" style="height: 20px;">
		<div id="footer2" style="height: 20px; text-align: center; vertical-align: middle; padding: 0 0 0 0;">
			<span>Copyright &copy; 2015&ndash;2017 CS ROMANIA SA</span>
			<div class="debug"></div>
		</div>
    </div>
	<script type="text/javascript">
		var jsonSiteId = <?php echo  $_SESSION['isAdmin'] ? "0" : "".json_encode( $_SESSION['siteId']) ?>;
		var jsonJobsPage = 1;
	</script>
</body>
</html>
