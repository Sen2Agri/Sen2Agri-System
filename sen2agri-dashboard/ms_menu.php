<?php
if (isset($_SESSION['siteId'])) {
?>
    <div id="menu">
        <div id="menu2">
            <ul>
                <li><a href="main.php">Products</a></li>
                <li><a href="system.php">System Overview</a></li>
                <li><a href="dashboard.php#tab_l2a">Dashboard</a></li>
                <li><a href="config.php">Configuration</a></li>
                <li style="float: right;"><a href="logout.php">Logout</a></li>
            </ul>
        </div>
    </div><!-- menu --><!-- menu2 -->
<?php
} else {
	$menu_login = "Login";
?>	
    <div id="menu">
        <div id="menu2">
            <ul class="inactivemenu">
                <li>Products</li>
                <li>System Overview</li>
                <li>Dashboard</li>
                <li>Configuration</li>
                <li style="float: right;"><a href="login.php">Login</a></li>
            </ul>
        </div>
    </div><!-- menu --><!-- menu2 -->	
<?php
}
?>
