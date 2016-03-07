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
							<button type="button" class="btn btn-default" onclick="move_to_first_jobs_page();">&lt;&lt;</button>
							<button type="button" class="btn btn-default" onclick="move_to_previous_jobs_page();">Previous Page</button>
							<button type="button" class="btn btn-default" onclick="move_to_next_jobs_page()">Next Page</button>
							<button type="button" class="btn btn-default disabled" onclick="move_to_last_jobs_page();">&gt;&gt;</button>
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
                                <!--<th rowspan="2">Actions</th>-->
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
    <script src="scripts/config.js"></script>
    <script src="scripts/helpers.js"></script>
    <script src="scripts/processing_functions.js"></script>
    <script src="scripts/processing_server.js"></script>
<?php include "ms_foot.php" ?>
