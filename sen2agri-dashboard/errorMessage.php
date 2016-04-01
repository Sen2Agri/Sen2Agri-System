<?php include "master.php"; ?>
    <div id="main">
        <div id="main2">
            <div id="main3">
				<div class="vspace"></div>
				<div id="signin">
					<fieldset>
						<div class="error" style="padding: 25px 5px;">
<?php
if (isset($_SESSION['errorMessage'])) {
	echo $_SESSION['errorMessage'];
	unset($_SESSION['errorMessage']);
}
?>
							</div>
					</fieldset>
				</div>
				<div class="vspace"></div>
			</div>
        </div>
    </div>
<?php include "ms_foot.php" ?>
