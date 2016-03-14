<?php
if (isset($_SESSION['siteId'])) {
?>
    <div id="menu">
        <div id="menu2">
            <ul class="activemenu">
				<li><a href="main.php">Products</a></li>
				<li><a href="system.php">System Overview</a></li>
				<li><a href="dashboard.php#tab_l2a">Dashboard</a></li>
				<li><a href="config.php">Custom Jobs</a></li>
				<li><a href="create_site.php">Sites</a></li>
				<li><a href="monitoring.php">Monitoring</a></li>
				<li style="float: right;"><a href="logout.php">Logout</a></li>
            </ul>
        </div>
    </div>
<?php
} else {
?>	
    <div id="menu">
        <div id="menu2">
            <ul class="inactivemenu">
                <li>Products</li>
                <li>System Overview</li>
                <li>Dashboard</li>
                <li>Custom Jobs</li>
                <li>Sites</li>
                <li>Monitoring</li>
            </ul>
        </div>
    </div>
<?php
}
?>
