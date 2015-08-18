// This file contains client side javascript helper functions.

// Toggles the target element to display or not by sliding.
function toggleAdditionalContent(target, show)
{
	if(show)
	{
		$(target).slideDown("fast");
	}
	else
	{
		$(target).slideUp("fast");
	}
}

// Toggles the target elements to display or not by sliding.
function toggleMultipleAdditionalContent(targets, shows)
{
	for(i = 0; i < targets.length; i++)
	{
		toggleAdditionalContent(targets[i], shows[i]);
	}
	
}