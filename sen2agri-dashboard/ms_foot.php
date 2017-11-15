    <div id="bar">
        <div id="bar2">
            <div id="bar3">
                <div class="clearing" style="height:20px;">&nbsp;</div>
<?php
if (isset($_SESSION['siteId'])) {
?>
				<div class='message_footer'>Logged in as <?php echo(ConfigParams::$USER_NAME); ?></div>
<?php
}
?>
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
		var jsonSiteId = <?php echo empty($_SESSION['siteId']) ? "0" : "".$_SESSION['siteId'] ?>;
		var jsonJobsPage = 1;
	</script>
</body>
</html>
