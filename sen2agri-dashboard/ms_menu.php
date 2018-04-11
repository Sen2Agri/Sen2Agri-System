<?php
if (isset($_SESSION['siteId'])) {
?>
	<script>
		$( document ).ready(function() {
			var str = "<?php echo $_SERVER['PHP_SELF']?>";
			var cld = 1;
			if      (str.indexOf("create_site.php") > 0) cld = 1;
			else if (str.indexOf("main.php")        > 0) cld = 2;
			else if (str.indexOf("system.php")      > 0) cld = 3;
			else if (str.indexOf("dashboard.php")   > 0) cld = 4;
			else if (str.indexOf("config.php")      > 0) cld = 5;
			else if (str.indexOf("monitoring.php")  > 0) cld = 6;
			<?php if( $_SESSION['roleID'] =='1'){?>
				else if (str.indexOf("users.php")   > 0) cld = 7;
			<?php }?>
			else cld = 0;
			$(".activemenu li:nth-child("+cld+")").addClass("selected");
		});
	</script>
    <div id="menu">
        <div id="menu2">
            <ul class="activemenu">
				<li><a href="create_site.php">Sites</a></li>
				<li><a href="main.php">Products</a></li>
				<li><a href="system.php">System Overview</a></li>
				<li><a href="dashboard.php">Dashboard</a></li>
				<li><a href="config.php">Custom Jobs</a></li>
				<li><a href="monitoring.php">Monitoring</a></li>
				<?php if( $_SESSION['roleID'] =='1'){?>
				<li><a href="users.php">Users</a></li>
				<?php }?>
				<li class="logout"><a href="logout.php">Logout</a></li>
            </ul>
        </div>
    </div>
<?php
} else {
?>	
    <div id="menu">
        <div id="menu2">
            <ul class="inactivemenu">
                <li>Sites</li>
                <li>Products</li>
                <li>System Overview</li>
                <li>Dashboard</li>
                <li>Custom Jobs</li>
                <li>Monitoring</li>
            </ul>
        </div>
    </div>
<?php
}
?>
