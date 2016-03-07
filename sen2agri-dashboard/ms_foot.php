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
			<span>Copyright &copy; 2015 CS ROMANIA SA</span>
<div class="debug">
<?php
/*
echo "USER_ID: ".ConfigParams::$USER_ID."</br>";
echo "SITE_ID: ".ConfigParams::$SITE_ID."</br>";
echo "USER_NAME: ".ConfigParams::$USER_NAME."</br>";
echo "</br>";
echo "WEB_BASE_URL: ".ConfigParams::$WEB_BASE_URL."</br>";
echo "SERVICES_BASE_URL: ".ConfigParams::$SERVICES_BASE_URL."</br>";
echo "SITE_PRODUCT_RELATIVE_FOLDER: ".ConfigParams::$SITE_PRODUCT_RELATIVE_FOLDER."</br>";
echo "SERVICES_DASHBOARD_PRODUCTS_URL: ".ConfigParams::$SERVICES_DASHBOARD_PRODUCTS_URL."</br>";
*/
?>
</div>
		</div>
    </div>
	<script type="text/javascript">
		var jsonSiteId = <?php echo empty($_SESSION['siteId']) ? "0" : "".$_SESSION['siteId'] ?>;
		var jsonJobsPage = 1;
	</script>
</body>
</html>
