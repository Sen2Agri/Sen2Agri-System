// override this function in your page
// to perform the required actions when page changes
function pagingGo(page_no) {
	return;
}

// pagination: create button
function pagingButton(button, number) {
	var html = "<li></li>";
	switch (button) {
		case "prev_enabled" : html = "<li class='prev' onclick='pagingGo(" + number + ");'><a href='javascript:;'><span>Prev</span></a></li>"; break;
		case "prev_disabled": html = "<li class='prev'><a href='javascript:;' class='disabled'><span>Prev</span></a></li>";                    break;
		case "dotdotdot"    : html = "<li class='dotdotdot' onclick='pagingGoToToggle(this);'><a href='javascript:;'>...</a></li>";            break;
		case "number"       : html = "<li class='number' onclick='pagingGo(" + number + ");'><a href='javascript:;'>" + number + "</a></li>";  break;
		case "current"      : html = "<li class='number current'><a>" + number + "</a href='javascript:;'></li>";                              break;
		case "next_enabled" : html = "<li class='next' onclick='pagingGo(" + number + ");'><a href='javascript:;'><span>Next</span></a></li>"; break;
		case "next_disabled": html = "<li class='next'><a href='javascript:;' class='disabled'><span>Next</span></a></li>";                    break;
		default             : html = "<li></li>"; break;
	}
	return html;
}

// pagination: create pagination
function pagingRefresh(count_total, selected_page) {
	var cnt = count_total < 1 ? 1 : count_total;
	var per_page = $('select[name=rows_per_page] option:selected').val();
	var page_cnt = Math.floor(cnt/per_page) + ((cnt % per_page) == 0 ? 0 : 1);
	
	if (selected_page > page_cnt) {
		selected_page = page_cnt
	} else if (selected_page < 1) {
		selected_page = 1;
	}
	
	var show_f_dot = true;
	var show_l_dot = true;
	var first_page = 2;
	var last_page = page_cnt - 1;
	
	//if ()
	if (selected_page <= first_page + 2) { show_f_dot = false; } else { first_page = selected_page - 1 };
	if (selected_page >= last_page  - 2) { show_l_dot = false; } else { last_page  = selected_page + 1 };
	
	var inner_html="";

	// first button + first <<...>> button
	if (show_f_dot) {
		inner_html += pagingButton("number", 1);
		inner_html += pagingButton("dotdotdot", 0);
	} else if (selected_page > 1) {
		inner_html += pagingButton("number", 1);
	}
	
	// current + neighbour buttons
	for (i = first_page; i < selected_page; i ++) {
		inner_html += pagingButton("number", i);
	}
	inner_html += pagingButton("current", selected_page);
	for (i = selected_page + 1; i <= last_page; i ++) {
		inner_html += pagingButton("number", i);
	}
	
	// last button + last <<...>> button
	if (show_l_dot) {
		inner_html += pagingButton("dotdotdot", 0);
		inner_html += pagingButton("number", page_cnt);
	} else if (selected_page < page_cnt) {
		inner_html += pagingButton("number", page_cnt);
	}
	
	// prev button
	if (selected_page == 1) {
		inner_html += pagingButton("prev_disabled", selected_page - 1);
	} else {
		inner_html += pagingButton("prev_enabled", selected_page - 1);
	}

	// next button
	if (selected_page == page_cnt) {
		inner_html += pagingButton("next_disabled", selected_page + 1);
	} else {
		inner_html += pagingButton("next_enabled", selected_page + 1);
	}
	$(".pages").html(inner_html);
	
	// set style to first and last buttons
	if (page_cnt == 1) {
		$(".pages li.number:first a").css("border-radius", "8px");
	} else {
		$(".pages li.number:first a").css("border-radius", "8px 0 0 8px");
		$(".pages li.number:last a").css("border-radius",  "0 8px 8px 0");
	}
	
	// hide gotobox
	$("#gotobox").addClass("hidden");
}

// gotobox: navigate to selected page
function pagingGoTo() {
	var selected_page = parseInt($('input[name=gotobox-input]').val());
	var last_page = parseInt($($('ul.pages li:nth-last-child(3) a')[0]).text());
	
	selected_page = (selected_page > last_page) ? last_page : selected_page;
	selected_page = (selected_page < 1) ? 1 : selected_page;
	if (selected_page > 0 && selected_page <= last_page) {
		pagingGo(selected_page);
		$("#gotobox").addClass("hidden");
	} else {
		//error: invalid page number
		$("#gotobox input[name=gotobox-input]").val("");
	}
}

// gotobox: toggle visibility (show/hide)
function pagingGoToToggle(dotdotdot) {
	var x0 = $("#gotobox").css("left");
	var x1 = $(dotdotdot).offset().left;
	var x2 = $(".pagination").offset().left;
	var x3 = $("#gotobox").width()/2 - 2;
	
	var crt_x = x0;
	var new_x = (x1 - x2 - x3) + "px";
	
	if ($("#gotobox").hasClass("hidden") || (crt_x != new_x)) {
		// show gotobox
		$("#gotobox").css("left", new_x);
		$("#gotobox").removeClass("hidden");
		$("#gotobox input[name=gotobox-input]").val("");
		$("#gotobox input[name=gotobox-input]").focus();
	} else {
		// hide gotobox
		$("#gotobox").addClass("hidden");
	}
}

// gotobox: hide gotobox when focusout
$(document).ready(function () {
	// hide gotobox when click outside gotobox
	$(document).click(function(event) {
		if (!$("#gotobox").hasClass("hidden")) {
			if ($(event.target).is('#gotobox, #gotobox *, .dotdotdot *')) return;
			$("#gotobox").addClass("hidden");
		}
	});
	// hide gotobox when pressing ENTER/ESC
	$("#gotobox").keyup(function(event) {
		if (event.which == 13) {
			pagingGoTo();
		} else if (event.which == 27) {
			$("#gotobox").addClass("hidden");
		}
	});
});
