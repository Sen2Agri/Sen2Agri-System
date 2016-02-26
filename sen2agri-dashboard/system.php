<?php include 'master.php'; ?>
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
                        <table class="table full_width">
                            <tr>
                                <th rowspan="2">Id</th>
                                <th rowspan="2">Processor</th>
                                <th rowspan="2">Site</th>
                                <th rowspan="2">Triggered By</th>
                                <th rowspan="2">Triggered On</th>
                                <th rowspan="2">Status</th>
                                <th rowspan="2">Tasks Completed/Running</th>
                                <th colspan="2">Current Task</th>
                                <th rowspan="2">Actions</th>
                            </tr>
                            <tr>
                                <th>Module</th>
                                <th>Tiles Completed/Running</th>
                            </tr>
                        </table>
                    </div>
                </div>
                <div class="clearing">&nbsp;</div>
            </div>
        </div>
    </div><!-- main --><!-- main2 --><!-- main3 -->
    <script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/3.5.6/d3.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.time.min.js"></script>
    <script src="libraries/flot-0.8.3/jquery.flot.stack.min.js"></script>
    <script src="libraries/nvd3-1.1.11/nv.d3.js"></script>
    <script src="scripts/config.js"></script>
    <script src="scripts/helpers.js"></script>
    <script src="scripts/processing_functions.js"></script>
    <script src="scripts/processing_server.js"></script>
<?php include 'ms_foot.php'; ?>