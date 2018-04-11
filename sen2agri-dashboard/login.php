<?php
session_start();
require_once('ConfigParams.php');

include 'ms_doc.php';
include 'ms_head.php';
include 'ms_menu.php';
?>
    <div id="main">
        <div id="main2">
            <div id="main3">
            <div class="vspace"></div>
				<div id="signin">
<?php 

if((isset($_REQUEST['action']) && $_REQUEST['action'] == 'first_login') || isset($_SESSION['saveMessage'])){
   unset($_SESSION['loginMessage']);
?>
					<fieldset>
						<form id="formPass" method="POST" action="verifyLogin.php">
						<div class="form-group">
							<label class="control-label" for="user"><span class="glyphicon glyphicon-user"></span>Username:</label><input type="text" class="form-control" name="username" id="username">
						</div>
						<div class="form-group">
							<label class="control-label" for="email"><span class="glyphicon glyphicon-envelope"></span> Email:</label><input type="email" class="form-control" name="email" id="email">
						</div>
						<div class="form-group">
							<label class="control-label" for="pass"><span class="glyphicon glyphicon-eye-open"></span> Password:</label><input type="password" class="form-control" name="password" id="password">
						</div>
						<div class="form-group">
							<label class="control-label" for="password_confirm"><span class="glyphicon glyphicon-eye-open"></span> Confirm password:</label><input type="password" class="form-control" name="password_confirm" id="password_confirm">
						</div>
							<input id="button" type="submit" name="submit" value="save">
						</form>
					</fieldset>
				</div>
				<div class="error">
				<?php
                    if (isset($_SESSION['saveMessage'])) {
                    	echo $_SESSION['saveMessage'];
                    	unset($_SESSION['saveMessage']);
                    }
                    ?>
				</div>
				
				<div id="firstLogin">
				<fieldset>
					<form method="POST" action="login.php">
					<label><a href="" >Back to login.</a> </label>
					</form>
					</fieldset>
				</div>	

<?php } else {
    unset($_SESSION['saveMessage']);
    
    if(isset($_SESSION['success'])){?>
    				<script>alert('<?php echo $_SESSION['success']?>');</script>
<?php unset($_SESSION['success']);
    }?>
					<fieldset>
						<form method="POST" action="verifyLogin.php">
							<label><span class="glyphicon glyphicon-user"></span>Username:</label><input type="text" name="user">
							<label><span class="glyphicon glyphicon-eye-open"></span> Password:</label><input type="password" name="pass">
							</br>
							<input id="button" type="submit" name="submit" value="login">
						</form>
					</fieldset>
				</div>
				<div class="error">
                <?php
                if (isset($_SESSION['loginMessage'])) {
                    echo $_SESSION['loginMessage'];
                    unset($_SESSION['loginMessage']);
                }
                ?>
				</div>
				<div id="firstLogin">
				<fieldset>
					<form id="setPass" method="POST" action="login.php">
						<label>First login? <a href="#" >Set a password.</a> </label>
					</form>
					</fieldset>
				</div>
		<?php }?>
				<div class="vspace"></div>
			</div>
        </div>
    </div>

<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>
    
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>

	<script type="text/javascript">
		$(document).ready( function() {
			if ($('input[name="user"]').length) {
				$('input[name="user"]').focus();
			}
			
			$("#setPass a").on('click',function(ev){
				ev.preventDefault();
				formSubmit();
				return false;
				});
			
			function formSubmit(){
				   $('#setPass').append('<input type="hidden" name="action" value="first_login" />');
				   $("#setPass").submit(); 
				   }

			$('#formPass').validate({
	            rules : {
	            	username:{ 
	                   	required: true,
	                   	pattern: "[a-z]{1}[\\w ]*",
	                   	minlength: 3
	                   	},
	            	email:{
	                	required: true,
	                	pattern: /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/ 
	                    	},
	            	password: {
		                required: true,
	                    minlength: 5
	                },
	                password_confirm: {
	                	required: true,
	                    minlength: 5,
	                    equalTo: "#password"
	                }
	            },
	            messages: {
	            	username:{ 
	                   	required: "Invalid username.",
	                   	pattern : "Invalid username.",
	                	minlength: $.validator.format("Please, at least {0} characters are necessary.")
	                   	},
		            email: "Please enter a valid email address.",
	            	password: {
	                    required: "What is your password?",
	                    minlength: $.validator.format("Your password must contain at least {0} characters."),
	                },
	            	password_confirm:{
	            		required: "You must confirm your password",
	                    minlength: $.validator.format("Your password must contain at least {0} characters."),
	            		equalTo   : "Doesn't match the password."
		            		}
		            },
		        highlight: function(element, errorClass) {
		                $(element).parent().addClass("has-error");
		            },
		        unhighlight: function(element, errorClass) {
		                $(element).parent().removeClass("has-error");
		            },
		        errorPlacement: function(error, element) {
		                error.appendTo(element.parent());
		            },
		        // set this class to error-labels to indicate valid fields
		        success: function(label) {
		                label.remove();
		            }
			});
		});
	</script>
<?php include "ms_foot.php" ?>
