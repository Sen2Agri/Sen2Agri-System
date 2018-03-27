<?php include "master.php"; ?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<div class="container-fluid">
				<div class="panel-group config" id="accordion">

					<!-- start L3A -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l3a">L3A processor</a>
							</h4>
						</div>
						<div id="l3a" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l3a_advanced"><input type="checkbox" id="l3a_advanced">Show advanced parameters</label>
								<form role="form" id="l3aform" name="l3aform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"> </select>
											</div>
											<!-- 
											<div class="form-group form-group-sm sensor">
												<label  style="">Sensor:</label>
												<input class="form-control chkS2" id="l3a_chkS2" type="checkbox" name="sensor" value="S2" checked="checked" disabled>
												<label class="control-label" for="l3a_chkS2">S2</label>
												<input class="form-control chkL8" id="l3a_chkL8" type="checkbox" name="sensor" value="L8" checked="checked">
												<label class="control-label" for="l3a_chkL8">L8</label>
											</div>-->
											
										
											<!-- Filter criteria for input files -->

											<?php $prefix = "l3a";
											include "filterInputFiles.php"?>
											<!-- End Filter criteria for input files -->

											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm required" data-provide="datepicker">
												<label class="control-label" for="synthDate">Synthesis date:</label>
												<input type="text" class="form-control" id="synthDate" name="synthDate">
												<span class="help-block">The synthesis date [YYYYMMDD].</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="halfSynthesis">Half synthesis:</label>
												<input type="number" min="0" step="1" class="form-control" id="halfSynthesis" name="halfSynthesis" value="25">
												<span class="help-block">The half synthesis interval [days]. A value of 25 days is recommended in the case of very cloudy regions.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="maxaot">Maximum value of the linear range for weights w.r.t. AOT:</label>
												<input type="text" class="form-control" id="maxaot" name="maxaot" value="0.8">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="minweight">Minimum weight depending on AOT:</label>
												<input type="text" class="form-control" id="minweight" name="minweight" value="0.33">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="maxweight">Maximum weight depending on AOT:</label>
												<input type="text" class="form-control" id="maxweight" name="maxweight" value="1">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="sigmasmall">Standard deviation of gaussian filter for distance to small clouds :</label>
												<input type="text" class="form-control" id="sigmasmall" name="sigmasmall" value="2">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="sigmalarge">Standard deviation of gaussian filter for distance to large clouds :</label>
												<input type="text" class="form-control" id="sigmalarge" name="sigmalarge" value="10">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="coarseresolution">Coarse resolution for quicker convolution:</label>
												<input type="text" class="form-control" id="coarseresolution" name="coarseresolution" value="240">
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="weightdatemin">Minimum weight at edge of the synthesis time window:</label>
												<input type="number" min="0" class="form-control" id="weightdatemin" name="weightdatemin" value="0.5">
											</div>
											<input name="proc_div" type="hidden" value="l3a">
											<input name="l3a" type="submit" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L3A -->

					<!-- start L3B LAI -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l3b">L3B LAI processor</a>
							</h4>
						</div>
						<div id="l3b" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l3b_advanced"><input type="checkbox" id="l3b_advanced">Show advanced parameters</label>
								<form role="form" id="l3b_laiform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"> </select>
											</div>
											
											<!-- Filter criteria for input files -->
											<?php $prefix = "l3b_lai";
											include "filterInputFiles.php"?>
											 <!-- End Filter criteria for input files -->
                          					
											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"> </select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>

											<div class="subgroup lai">
												<label class="control-label">Generate LAI mono-dates:</label>
												<div class="form-group form-group-sm">
													<input class="form-control" id="monolai" type="radio" name="lai" value="monolai" checked="checked">
													<label class="control-label" for="monolai">Generate LAI mono-dates</label>
													<span class="help-block">(Generate LAI mono-dates)</span>
												</div>
												<div class="form-group form-group-sm">
													<input class="form-control" id="reproc" type="radio" name="lai" value="reproc">
													<label class="control-label" for="reproc">Reprocessing with the last N-Days</label>
													<span class="help-block">(Performe reprocessing with the last N-Days)</span>
												</div>
												<div class="form-group form-group-sm">
													<input class="form-control" id="fitted" type="radio" name="lai" value="fitted">
													<label class="control-label" for="fitted">LAI time series fitting at the end of the season</label>
													<span class="help-block">(Performe reprocessing at the end of the season)</span>
												</div>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="bwr">Backward window:</label>
												<input type="number" class="form-control" id="bwr" name="bwr" value="2">
												<span class="help-block">Backward window for LAI N-Day reprocessing</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="fwr">Forward window:</label>
												<input type="number" class="form-control" id="bwr" name="fwr" value="0">
												<span class="help-block">Forward window for LAI N-Day Reprocessing</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="genmodel">Generate models:</label>
												<input type="number" min="0" max="1" step="1" class="form-control" id="genmodel" name="genmodel" value="1">
												<span class="help-block">Specifies if the models should be generated or not [0/1]</span>
											</div>
											<input name="proc_div" type="hidden" value="l3b">
											<input type="submit" name="l3b_lai" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L3B LAI -->

					<!-- start L3B NDVI -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l3b_nvdi">L3E PHENO processor</a>
							</h4>
						</div>
						<div id="l3b_nvdi" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l3b_nvdi_advanced"><input type="checkbox" id="l3b_nvdi_advanced">Show advanced parameters</label>
								<form role="form" id="l3b_nvdiform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"> </select>
											</div>
											<!-- Filter criteria for input files -->
											<?php $prefix = "l3b_nvdi";
											include "filterInputFiles.php"?>
											<!-- End Filter criteria for input files -->
                          					
											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<input name="proc_div" type="hidden" value="l3b_nvdi">
											<input type="submit" name="l3e_pheno" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L3B NDVI -->

					<!-- start L4A with in-situ -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l4a">L4A IN-SITU processor</a>
							</h4>
						</div>
						<div id="l4a" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l4a_advanced"><input type="checkbox" id="l4a_advanced">Show advanced parameters</label>
								<form enctype="multipart/form-data" role="form" id="l4aform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"></select>
											</div>
											<!-- Filter criteria for input files -->
											<?php $prefix = "l4a";
											include "filterInputFiles.php"?>
											<!-- End Filter criteria for input files -->
                          					
											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm required">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp" name="refp" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference polygons. A .zip file containing the shapefile is expected. See Software User Manual for the format of the shapefile. Take care of the header of the columns.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="refr">Reference raster:</label>
												<input type="file" class="form-control" id="refr" name="refr" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference raster when in situ data is not available.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="field">Field:</label>
												<input type="text" class="form-control" id="field" name="field" value="CROP">
												<span class="help-block">Header of the column containing the class label.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ratio">Ratio:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="ratio" name="ratio" value="0.75">
												<span class="help-block">The ratio between the validation and training polygons.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="radius">Radius:</label>
												<input type="number" min="0" step="1" class="form-control" id="radius" name="radius" value="15">
												<span class="help-block">The radius used for gap filling, in days.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="nbtrsample">Training set samples:</label>
												<input type="number" min="0" step="1" class="form-control" id="nbtrsample" name="nbtrsample" value="40000">
												<span class="help-block">The number of samples included in the training set.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="rseed">Random seed:</label>
												<input type="number" min="0" step="1" class="form-control" id="rseed" name="rseed" value="0">
												<span class="help-block">The random seed used for training.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for=window>Window records:</label>
												<input type="number" min="0" step="1" class="form-control" id="window" name="window" value="6">
												<span class="help-block">The number of records used for the temporal features extraction</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="lmbd">Lambda:</label>
												<input type="number" min="0" step="1" class="form-control" id="lmbd" name="lmbd" value="2">
												<span class="help-block">The lambda parameter used in data smoothing.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="weight">Weight:</label>
												<input type="number" min="0" step="1" class="form-control" id="weight" name="weight" value="1">
												<span class="help-block">The weight factor for data smoothing.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="nbcomp">Number of components:</label>
												<input type="number" min="0" step="1" class="form-control" id="nbcomp" name="nbcomp" value="6">
												<span class="help-block">The number of components used by dimensionality reduction.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="ranger">Range radius:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="ranger" name="ranger" value="0.65">
												<span class="help-block">The range radius (expressed in radiometry unit).</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="eroderad">Erosion radius:</label>
												<input type="number" min="0" step="1" class="form-control" id="eroderad" name="eroderad" value="1">
												<span class="help-block">The radius used for erosion.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="alpha">Alpha:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="alpha" name="alpha" value="0.01">
												<span class="help-block">The parameter alpha used by the mahalanobis function.</span>
											</div>

											<div class="subgroup advanced">
												<label class="control-label">Segmentation parameters:</label>
												<div class="form-group form-group-sm">
													<label class="control-label" for="spatialr">Spatial radius:</label>
													<input type="number" min="0" step="1" class="form-control" id="spatialr" name="spatialr" value="10">
													<span class="help-block">The spatial radius of the neighbourhood used for segmentation.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="minsize">Minimum size of a region:</label>
													<input type="number" min="0" step="1" class="form-control" id="minsize" name="minsize" value="10">
													<span class="help-block">Minimum size of a region (in pixel unit) in segmentation.</span>
												</div>
											</div>

											<div class="subgroup advanced">
												<label class="control-label">Clasifier parameters:</label>
												<div class="form-group form-group-sm hidden">
													<label for="classifier">Classifier:</label>
													<select class="form-control" id="classifier" name="classifier">
														<option value="rf" selected="selected">RF</option>
														<option value="svm">SVM</option>
													</select>
													<span class="help-block">Random forest clasifier / SVM classifier</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfnbtrees">Training trees:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfnbtrees" name="rfnbtrees" value="100">
													<span class="help-block" value="100">The number of trees used for training.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmax">Max depth:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmax" name="rfmax" value="25">
													<span class="help-block">Maximum depth for the trees used for Random Forest classifier.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmin">Minimum number of samples:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmin" name="rfmin" value="25">
													<span class="help-block">Minimum number of samples in each node used by the classifier.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="minarea">The minium number of pixels:</label>
													<input type="number" min="0" step="1" class="form-control" id="minarea" name="minarea" value="20">
													<span class="help-block">The minimum number of pixels in an area where for an equal number of crop and nocrop samples the crop decision is taken.</span>
												</div>
											</div>
											<input name="proc_div" type="hidden" value="l4a">
											<input type="submit" name="l4a" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L4A with in-situ -->

					<!-- start L4A w/o in-situ -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l4a_wo">L4A w/o IN-SITU processor</a>
							</h4>
						</div>
						<div id="l4a_wo" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l4a_wo_advanced"><input type="checkbox" id="l4a_wo_advanced">Show advanced parameters</label>
								<form enctype="multipart/form-data" role="form" id="l4a_woform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"></select>
											</div>
											
											<!-- Filter criteria for input files -->
											<?php $prefix = "l4a_wo";
											include "filterInputFiles.php"?> 
											<!-- End Filter criteria for input files -->
										
											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp" name="refp" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference polygons. A .zip file containing the shapefile is expected. See Software User Manual for the format of the shapefile. Take care of the header of the columns.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="refr">Reference raster:</label>
												<input type="file" class="form-control" id="refr" name="refr" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference raster when in situ data is not available.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="field">Field:</label>
												<input type="text" class="form-control" id="field" name="field" value="CROP">
												<span class="help-block">Header of the column containing the class label.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ratio">Ratio:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="ratio" name="ratio" value="0.75">
												<span class="help-block">The ratio between the validation and training polygons.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="radius">Radius:</label>
												<input type="number" min="0" step="1" class="form-control" id="radius" name="radius" value="15">
												<span class="help-block">The radius used for gap filling, in days.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="nbtrsample">Training set samples:</label>
												<input type="number" min="0" step="1" class="form-control" id="nbtrsample" name="nbtrsample" value="40000">
												<span class="help-block">The number of samples included in the training set.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="rseed">Random seed:</label>
												<input type="number" min="0" step="1" class="form-control" id="rseed" name="rseed" value="0">
												<span class="help-block">The random seed used for training.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for=window>Window records:</label>
												<input type="number" min="0" step="1" class="form-control" id="window" name="window" value="6">
												<span class="help-block">The number of records used for the temporal features extraction</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="lmbd">Lambda:</label>
												<input type="number" min="0" step="1" class="form-control" id="lmbd" name="lmbd" value="2">
												<span class="help-block">The lambda parameter used in data smoothing.</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="weight">Weight:</label>
												<input type="number" min="0" step="1" class="form-control" id="weight" name="weight" value="1">
												<span class="help-block">The weight factor for data smoothing.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="nbcomp">Number of components:</label>
												<input type="number" min="0" step="1" class="form-control" id="nbcomp" name="nbcomp" value="6">
												<span class="help-block">The number of components used by dimensionality reduction.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="ranger">Range radius:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="ranger" name="ranger" value="0.65">
												<span class="help-block">The range radius (expressed in radiometry unit).</span>
											</div>

											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="eroderad">Erosion radius:</label>
												<input type="number" min="0" step="1" class="form-control" id="eroderad" name="eroderad" value="1">
												<span class="help-block">The radius used for erosion.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="alpha">Alpha:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="alpha" name="alpha" value="0.01">
												<span class="help-block">The parameter alpha used by the mahalanobis function.</span>
											</div>

											<div class="subgroup advanced">
												<label class="control-label">Segmentation parameters:</label>
												<div class="form-group form-group-sm">
													<label class="control-label" for="spatialr">Spatial radius:</label>
													<input type="number" min="0" step="1" class="form-control" id="spatialr" name="spatialr" value="10">
													<span class="help-block">The spatial radius of the neighbourhood used for segmentation.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="minsize">Minimum size of a region:</label>
													<input type="number" min="0" step="1" class="form-control" id="minsize" name="minsize" value="10">
													<span class="help-block">Minimum size of a region (in pixel unit) in segmentation.</span>
												</div>
											</div>

											<div class="subgroup advanced">
												<label class="control-label">Clasifier parameters:</label>
												<div class="form-group form-group-sm hidden">
													<label for="classifier">Classifier:</label>
													<select class="form-control" id="classifier" name="classifier">
														<option value="rf" selected="selected">RF</option>
														<option value="svm">SVM</option>
													</select>
													<span class="help-block">Random forest clasifier / SVM classifier</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfnbtrees">Training trees:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfnbtrees" name="rfnbtrees" value="100">
													<span class="help-block" value="100">The number of trees used for training.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmax">Max depth:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmax" name="rfmax" value="25">
													<span class="help-block">Maximum depth for the trees used for Random Forest classifier.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmin">Minimum number of samples:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmin" name="rfmin" value="25">
													<span class="help-block">Minimum number of samples in each node used by the classifier.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="minarea">The minium number of pixels:</label>
													<input type="number" min="0" step="1" class="form-control" id="minarea" name="minarea" value="20">
													<span class="help-block">The minimum number of pixels in an area where for an equal number of crop and nocrop samples the crop decision is taken.</span>
												</div>
											</div>
											<input name="proc_div" type="hidden" value="l4a_wo">
											<input type="submit" name="l4a_wo" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L4A w/o in-situ -->

					<!-- start L4B -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l4b">L4B processor</a>
							</h4>
						</div>
						<div id="l4b" class="panel-collapse collapse">
							<div class="panel-body">
								<label class="control_advanced" for="l4b_advanced"><input type="checkbox" id="l4b_advanced">Show advanced parameters</label>
								<form enctype="multipart/form-data" role="form" id="l4bform" method="post" action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-11">
											<div class="form-group form-group-sm required">
												<label class="control-label" for="siteId">Site:</label>
												<select class="form-control" id="siteId" name="siteId"> </select>
											</div>
											<!-- Filter criteria for input files -->
											<?php $prefix = "l4b";
											include "filterInputFiles.php"?>
											<!-- End Filter criteria for input files -->
										
											<div class="form-group form-group-sm required">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]" size="7"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="cropMask">Crop masks:</label>
												<select class="form-control" id="cropMask" name="cropMask"></select>
												<span class="help-block">The list of crop mask products.</span>
											</div>
											<div class="form-group form-group-sm required">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp" name="refp" onchange="$(this).trigger('blur');">
												<span class="help-block">The reference polygons. A .zip file containing the shapefile is expected. See Software User Manual for the format of the shapefile. Take care of the header of the columns</span>
											</div>
											<div class="form-group form-group-sm hidden">
												<label class="control-label" for="field">Field:</label>
												<input type="text" class="form-control" id="field" name="field" value="CODE">
												<span class="help-block">Header of the column containing the class label.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution" name="resolution">
													<option value="">Select a resolution</option>
													<option value="10" selected="selected">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<span class="help-block">Resolution of the output image (in meters).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ratio">Ratio:</label>
												<input type="number" min="0" step="0.01" class="form-control" id="ratio" name="ratio" value="0.75">
												<span class="help-block">The ratio between the validation and training polygons.</span>
											</div>
											<div class="form-group form-group-sm advanced">
												<label class="control-label" for="rseed">Random seed:</label>
												<input type="number" min="0" step="1" class="form-control" id="rseed" name="rseed" value="0">
												<span class="help-block">The random seed used for training.</span>
											</div>

											<div class="subgroup advanced">
												<label class="control-label">Clasifier parameters:</label>
												<div class="form-group form-group-sm hidden">
													<label for="classifier">Classifier:</label>
													<select class="form-control" id="classifier" name="classifier">
														<option value="rf" selected="selected">RF</option>
														<option value="svm">SVM</option>
													</select>
													<span class="help-block">Random forest clasifier / SVM classifier</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfnbtrees">Training trees:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfnbtrees" name="rfnbtrees" value="100">
													<span class="help-block">The number of trees used for training.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmax">Random Forest classifier max depth:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmax" name="rfmax" value="25">
													<span class="help-block">Maximum depth of the trees used for Random Forest classifier.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rfmin">Minimum number of samples:</label>
													<input type="number" min="0" step="1" class="form-control" id="rfmin" name="rfmin" value="25" />
													<span class="help-block">Minimum number of samples in each node used by the classifier.</span>
												</div>
											</div>
											<input name="proc_div" type="hidden" value="l4b">
											<input type="submit" name="l4b" value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!-- end L4B -->

				</div>

			</div>
			<div class="clearing">&nbsp;</div>
		</div>
	</div>
</div>
<!-- main -->
<!-- main2 -->
<!-- main3 -->

<!-- Diablog message -->
<div id="dialog-message" title="Submit successful">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>
<!-- Diablog wait -->
<div id="dialog-wait" title="Processing request">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>
<!-- Diablog message -->
<div id="dialog-error" title="Submit failed">
	<p>
		<span class="ui-icon ui-icon-circle-check" style="float:left; margin:0 7px 50px 0;"></span>
		<span id="dialog-content"></span>
	</p>
</div>

<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>
<script src="scripts/processing_functions.js"></script>
<link rel="stylesheet" href="libraries/jquery-ui/jquery-ui.min.css">
<script src="libraries/jquery-ui/jquery-ui.min.js"></script>

<script>
	$('#l3a_advanced').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
	$('#l3b_advanced').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
	$('#l3b_nvdi').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
	$('#l4a_advanced').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
	$('#l4a_wo_advanced').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
	$('#l4b_advanced').click(function() {
		if ($(this).is(':checked')) { $(this).parent().parent().find(".advanced").removeClass("hidden"); }
		else {  $(this).parent().parent().find(".advanced").addClass("hidden"); }
	});
</script>

<script>
	//$("#l3aform").validate();
	var l2a_proc_id;
	var l4a_proc_id;

	function open_dialog(message) {
		$("#dialog-message #dialog-content").text(message);
		$("#dialog-message").dialog("open");
	};
	function open_dialog_wait(message) {
		$("#dialog-wait #dialog-content").text(message);
		$("#dialog-wait").dialog("open");
	};
	function open_dialog_error(message) {
		$("#dialog-error #dialog-content").text(message);
		$("#dialog-error").dialog("open");
	};
	function reset_form(form_name) {
		$("#"+form_name)[0].reset();
		$("#"+form_name+" #inputFiles").find('option').remove().end();
	};

	$(document).ready(function() {
		// hide all advanced parameters
		$('.advanced').addClass("hidden");
		
		initialise_datepickers();
		// add events to elements used to filter input files
		add_events();
		
		// load sites
		get_all_sites();
		get_processor_id('l2a', 'l2a_proc_id');
		get_processor_id('l4a', 'l4a_proc_id');

		// initialize date picker
		$("#synthDate").datepicker({
			dateFormat: "yymmdd",
			onSelect: function() { $(this).keyup(); } // force validation after selection
		});

		// initialize dialogs
		$("#dialog-message").dialog({
			width: '400px',
			autoOpen: false,
			modal: true,
			buttons: { Ok: function() { $(this).dialog("close"); } }
		});
		$("#dialog-wait").dialog({
			width: '400px',
			autoOpen: false,
			modal: true,
			buttons: { }
		});
		$("#dialog-error").dialog({
			width: '400px',
			autoOpen: false,
			modal: true,
			buttons: { Ok: function() { $(this).dialog("close"); } }
		}).parent().children(".ui-dialog-titlebar").addClass('ui-state-error');

		$("input[type='submit']").click(function() {
			var form_valid = false;
			switch ($(this).attr('name')) {
				case "l3a":       form_valid = $("#l3aform").valid();      break;
				case "l3b_lai":   form_valid = $("#l3b_laiform").valid();  break;
				case "l3e_pheno": form_valid = $("#l3b_nvdiform").valid(); break;
				case "l4a":       form_valid = $("#l4aform").valid();      break;
				case "l4a_wo":    form_valid = $("#l4a_woform").valid();   break;
				case "l4b":       form_valid = $("#l4bform").valid();      break;
			}
			if (form_valid) {
				open_dialog_wait("Please wait, your request is being processed!");
			}
		});
		
		<?php
		// Check if this is a redirect from a FORM being submitted
		if (isset($_SESSION['processor'])) {
			echo "$('#" . $_SESSION['proc_div'] . "').collapse('show');";
			echo "$('#" . $_SESSION['proc_div'] . " #siteId').trigger('focus');";
			echo ($_SESSION['status'] === "OK" ? "open_dialog('" : "open_dialog_error('").$_SESSION['message']."')";
			
			unset($_SESSION['processor']);
			unset($_SESSION['proc_div']);
			unset($_SESSION['status']);
			unset($_SESSION['message']);
		}
		?>

		// validate l3aform form on keyup and submit
					$("#l3aform").validate({
						rules: {
							siteId:			{ required: true },
							'inputFiles[]': { required: true },
							synthDate:		{ required: true, pattern: "[0-9]{4}(0[1-9]|1[0-2])(0[1-9]|[1-2][0-9]|3[0-1])" },
						},
						messages: {
							synthDate: { pattern : "Enter a date in YYYYMMDD format" }
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
						},
					});

		// validate l3b LAI form form on keyup and submit
					$("#l3b_laiform").validate({
						rules : {
							siteId:			{ required: true },
							'inputFiles[]': { required: true },
							genmodel:		{ pattern: "[0-1]{1}" },
						},
						messages: {
							genmodel: { pattern : "Accepted values: 0 and 1" },
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
						},
					});

		// validate l3b NVDI(pheno) form form on keyup and submit
					$("#l3b_nvdiform").validate({
						rules: {
							siteId: 		{ required: true },
							'inputFiles[]': { required: true },
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
						},
					});

		// validate l4aform with in-situ form on keyup and submit
					$("#l4aform").validate({
						rules : {
							siteId: 		{ required: true },
							'inputFiles[]': { required: true },
							refp:           { required: true },
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
						success : function(label) {
							label.remove();
						},
					});

		// validate l4aform w/o in-situ form on keyup and submit
					$("#l4a_woform").validate({
						rules : {
							siteId: 		{ required: true },
							'inputFiles[]': { required: true },
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
						success : function(label) {
							label.remove();
						},
					});

		// validate l4bform form on keyup and submit
					$("#l4bform") .validate( {
						rules : {
							siteId: 		{ required: true },
							'inputFiles[]': { required: true },
							refp:			{ required: true },
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
						success : function(label) {
							label.remove();
						},
					});
	});
</script>
<?php include "ms_foot.php"; ?>
