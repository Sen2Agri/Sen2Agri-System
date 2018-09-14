	<div id="header" style="height:120px;">
	<?php 
	require_once('ConfigParams.php');
	if(ConfigParams::isSen2Agri())
	{?>
       <div id="header2" style="background-image: url(images/logo5.png); background-repeat: no-repeat;">&nbsp;</div>
       <?php }else
       {?>
       <div id="header2" style="background-image: url(images/logo_sen4cap.png); background-repeat: no-repeat;">&nbsp;</div> 
       <?php }?>
    </div>
	