<?php include "master.php"; ?>
    <div id="main">
        <div id="main2">
            <div id="main3">
                <div id="content" style="width:100%;">
                    <!-- System Overview ----------------------------------------------------------------------------------------------------------------------- -->
                    <div id="pnl_server_resources_container">
                        <div class="panel panel-default" id="pnl_server_resources">
                            <div class="panel-heading">Server Resources</div>
                        </div>
                    </div>
					
                    <div class="panel panel-default" id="pnl_current_jobs">
                        <div class="panel-heading">Current Jobs</div>
						<div style="float: right; padding: 10px;">
							<button id="page_move_first" type="button" class="btn btn-default" onclick="move_to_first_jobs_page();">&lt;&lt;</button>
							<button id="page_move_prev"  type="button" class="btn btn-default" onclick="move_to_previous_jobs_page();">Previous Page</button>
							<button id="page_current"    type="button" class="btn btn-default" style="font-weight: 700;">1</button>
							<button id="page_move_next"  type="button" class="btn btn-default" onclick="move_to_next_jobs_page()">Next Page</button>
							<button id="page_move_last"  type="button" class="btn btn-default disabled">&gt;&gt;</button>
						</div>
                        <table class="table full_width">
                            <tr>
                                <th rowspan="2">Id</th>
                                <th rowspan="2">Processor</th>
                                <th rowspan="2">Site</th>
                                <th rowspan="2">Triggered By</th>
                                <th rowspan="2">Triggered On</th>
                                <th rowspan="2">Status</th>
                                <th rowspan="2" style="width: 1px;">Tasks Completed / Running</th>
                                <th colspan="2">Current Task</th>
                                <th rowspan="2">Actions</th>
                            </tr>
                            <tr>
                                <th>Module</th>
                                <th>Tiles Completed / Running</th>
                            </tr>
                        </table>
                    </div>
                </div>
                <div class="clearing">&nbsp;</div>
            </div>
        </div>
    </div><!-- main --><!-- main2 --><!-- main3 -->
    <script src="scripts/helpers.js"></script>
    <script src="scripts/processing_functions.js"></script>
    <script src="scripts/processing_server.js"></script>
<?php include "ms_foot.php" ?>
