<?php include 'master.php'; ?>
<div id="main">
	<div id="main2">
		<div id="main3">
			<div class="container-fluid">
				<div class="panel-group" id="accordion">

					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l3a">L3A
									processor</a>
							</h4>
						</div>
						<div id="l3a" class="panel-collapse collapse">
							<div class="panel-body">
								<form role="form" id="l3aform" name="l3aform" method="post"
									action="getProcessorNewConfig.php">
									<div class="row">

										<div class="col-md-8">
											<div class="form-group form-group-sm">
												<label class="control-label" for="siteId">Site:</label> <select
													class="form-control" id="siteId" name="siteId">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="sentinel2Tiles">Sentinel2
													tiles:</label> <select class="form-control"
													id="sentinel2Tiles" name="sentinel2Tiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="landsatTiles">Landsat
													tiles:</label> <select class="form-control"
													id="landsatTiles" name="landsatTiles">
												</select>
											</div>


											<div class="form-group form-group-sm">
												<label class="control-label" for="inputFiles">Available
													input files:</label> <select multiple class="form-control"
													id="inputFiles" name="inputFiles[]">
												</select> <span class="help-block">The list of products
													descriptors (xml files).</span>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<select class="form-control" id="resolution"
													name="resolution">
													<option value="">Select a resolution</option>
													<option value="10">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="maxaot">Maximum value of
													the linear range for weights w.r.t. AOT:</label> <input
													type="text" class="form-control" id="maxaot" name="maxaot">
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="minweight">Minimum weight
													depending on AOT:</label> <input type="text"
													class="form-control" id="minweight" name="minweight">
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="maxweight">Maximum weight
													depending on AOT:</label> <input type="text"
													class="form-control" id="maxweight" name="maxweight">
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="sigmasmall">Standard
													deviation of gaussian filter for distance to small clouds :</label>
												<input type="text" class="form-control" id="sigmasmall"
													name="sigmasmall">
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="sigmalarge">Standard
													deviation of gaussian filter for distance to large clouds :</label>
												<input type="text" class="form-control" id="sigmalarge"
													name="sigmalarge">
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="coarseresolution">Coarse
													resolution for quicker convolution:</label> <input
													type="text" class="form-control" id="coarseresolution"
													name="coarseresolution">
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="synthDate">Synthesis date:</label>
												<input type="text" class="form-control" id="synthDate"
													name="synthDate">
												<!--  <span class="help-block">The
															date of the composite product.</span>-->
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="weightdatemin">Minimum
													weight at edge of the synthesis time window:</label> <input
													type="number" min="0" class="form-control"
													id="weightdatemin" name="weightdatemin">
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="halfSynthesis">Half
													synthesis:</label> <input type="number" min="0" step="1"
													class="form-control" id="halfSynthesis"
													name="halfSynthesis"> <span class="help-block">The half
													synthesis period (days).</span>
											</div>
											<input name="l3a" type="submit" class="btn btn-primary"
												value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l3b">L3B
									LAI processor</a>
							</h4>
						</div>
						<div id="l3b" class="panel-collapse collapse">
							<div class="panel-body">
								<form role="form" id="l3b_laiform" method="post"
									action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-8">
											<p id="submitedform"></p>
										</div>
										<div class="col-md-8">
											<div class="form-group form-group-sm">
												<label class="control-label" for="siteId">Site:</label> <select
													class="form-control" id="siteId" name="siteId">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="sentinel2Tiles">Sentinel2
													tiles:</label> <select class="form-control"
													id="sentinel2Tiles" name="sentinel2Tiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="landsatTiles">Landsat
													tiles:</label> <select class="form-control"
													id="landsatTiles" name="landsatTiles">
												</select>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="inputFiles">Available
													input files:</label> <select multiple class="form-control"
													id="inputFiles" name="inputFiles[]">
												</select> <span class="help-block">The list of products
													descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
													<select class="form-control" id="resolution"
													name="resolution">
													<option value="">Select a resolution</option>
													<option value="10">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<!-- <input type="number" min="0" max="20" step="10"
													class="form-control" id="resolution" name="resolution"> -->
													 <span
													class="help-block">Resolution of the output image.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="genmodel">Generate models:</label>
												<input type="number" min="0" max="1" step="1"
													class="form-control" id="genmodel" name="genmodel"
													value="0"> <span class="help-block">Specifies if the models
													should be also generated or not (0/1) </span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="reproc">Reprocessing the
													last N-Days:</label> <input type="number" min="0" max="1"
													step="1" class="form-control" id="reproc" name="reproc"
													value="0"><span class="help-block">Specifies if the
													reprocessing of last N-Days should be also performed or not
													(0/1)</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="fitted">Reprocessing at
													the end of the season:</label> <input type="number" min="0"
													max="1" step="1" class="form-control" id="fitted"
													name="fitted" value="0"> <span class="help-block">Specifies
													if the reprocessing at the end of the season should be also
													performed or not (0/1)</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="bwr">Backward window:</label>
												<input type="number" class="form-control" id="bwr"
													name="bwr">
												<!-- <input type="number" min="0" max="20" step="10"
														class="form-control" id="bwr" name="bwr">  -->
												<span class="help-block">Backward window for LAI N-Day
													reprocessing</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="fwr">Forward window:</label>
												<input type="number" class="form-control" id="bwr"
													name="fwr"></input>
												<!-- <input type="number" min="0" max="20" step="10"
														class="form-control" id="bwr" name="fwr"></input> -->
												<span class="help-block">Forward window for LAI N-Day
													Reprocessing</span>
											</div>
											<!-- not sure if should be in interface 
												<div class="form-group form-group-sm">
													<label class="control-label" for="modelsfolder">Models location:</label>
													<input type="text" class="form-control" id="modelsfolder" name="modelsfolder">
														<span class="help-block">Folder where the models are located.</span>
												</div>
												<div class="form-group form-group-sm">
													<label class="control-label" for="rsrcfgfile">The locations of the RSR files:</label>
													<input type="text" 	class="form-control" id="rsrcfgfile" name="rsrcfgfile">
														<span class="help-block">File containing the locations of the RSR files for each mission.</span>
												</div>
												-->
											<input type="submit" name="l3b_lai" class="btn btn-primary"
												value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion"
									href="#l3b_nvdi">L3B NDVI processor</a>
							</h4>
						</div>
						<div id="l3b_nvdi" class="panel-collapse collapse">
							<div class="panel-body">
								<form role="form" id="l3b_nvdiform" method="post"
									action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-8">
											<div class="form-group form-group-sm">
												<label class="control-label" for="siteId">Site:</label> <select
													class="form-control" id="siteId" name="siteId">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="sentinel2Tiles">Sentinel2
													tiles:</label> <select class="form-control"
													id="sentinel2Tiles" name="sentinel2Tiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="landsatTiles">Landsat
													tiles:</label> <select class="form-control"
													id="landsatTiles" name="landsatTiles">
												</select>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="inputFiles">Available
													input files:</label> <select multiple class="form-control"
													id="inputFiles" name="inputFiles[]">
												</select> <span class="help-block">The list of products
													descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
													<select class="form-control" id="resolution"
													name="resolution">
													<option value="">Select a resolution</option>
													<option value="10">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<!--<input type="number" min="0" max="20" step="10"
													class="form-control" id="resolution" name="resolution">--> <span
													class="help-block">Resolution of the output image.</span>
											</div>
											<input type="submit" name="l3b_pheno" class="btn btn-primary"
												value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>


					<!--start L4A  -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l4a">L4A
									processor</a>
							</h4>
						</div>
						<div id="l4a" class="panel-collapse collapse">
							<div class="panel-body">
								<form enctype="multipart/form-data" role="form" id="l4aform" method="post"
									action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-8">
											<div class="form-group form-group-sm">
												<label class="control-label" for="siteId">Site:</label> <select
													class="form-control" id="siteId" name="siteId">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="sentinel2Tiles">Sentinel2
													tiles:</label> <select class="form-control"
													id="sentinel2Tiles" name="sentinel2Tiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="landsatTiles">Landsat
													tiles:</label> <select class="form-control"
													id="landsatTiles" name="landsatTiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="mission">Mission:</label>
												<select class="form-control" id="mission" name="mission">
													<option value="SPOT" selected="selected">SPOT</option>
													<option value="LANDSAT">LANDSAT</option>
													<option value="SENTINEL">SENTINEL</option>
												</select>
												<!--<input type="text" class="form-control" id="mission"
													name="mission" value="SPOT">--> <span class="help-block">The
													main mission for the time series.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp"
													name="refp"> <span class="help-block">The reference
													polygons.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ratio">Ratio:</label> <input
													type="number" min="0" step="0.01" class="form-control"
													id="ratio" name="ratio" value="0.75"> <span
													class="help-block">The ratio between the validation and
													training polygons.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="inputFiles">Available
													input files:</label> <select multiple class="form-control"
													id="inputFiles" name="inputFiles[]">
												</select> <span class="help-block">The list of products
													descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
													<select class="form-control" id="resolution"
													name="resolution">
													<option value="">Select a resolution</option>
													<option value="10">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select>
												<!--<input type="number" min="0" max="20" step="10"
													class="form-control" id="resolution" name="resolution"
													value="10">--> <span class="help-block">Resolution of the
													output image.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="trm">The temporal
													resampling mode:</label> <input type="radio"
													class="radio-inline" id="trm" name="trm">resample <input
													checked="checked" type="radio" class="radio-inline"
													id="trm" name="trm"> gapfill
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="radius">Radius:</label> <input
													type="number" min="0" step="1" class="form-control"
													id="radius" name="radius" value="15"></input> <span
													class="help-block">The radius used for gap filling, in
													days.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="nbtrsample">Training set
													samples:</label> <input type="number" min="0" step="1"
													class="form-control" id="nbtrsample" name="nbtrsample"
													value="4000"> <span class="help-block">The number of
													samples included in the training set.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rseed">Random seed:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="rseed" name="rseed" value="0"> <span class="help-block">The
													random seed used for training.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for=window>Window records:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="window" name="window" value="6"> <span
													class="help-block">The number of records used for the
													temporal features extraction</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="lmbd">Lambda:</label> <input
													type="number" min="0" step="1" class="form-control"
													id="lmbd" name="lmbd" value="2"> <span class="help-block">The
													lambda parameter used in data smoothing.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="weight">Weight:</label> <input
													type="number" min="0" step="1" class="form-control"
													id="weight" name="weight" value="1"> <span
													class="help-block">The weight factor for data smoothing.</span>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="nbcomp">Number of
													components:</label> <input type="number" min="0" step="1"
													class="form-control" id="nbcomp" name="nbcomp" value="6"> <span
													class="help-block">The number of components used by
													dimensionality reduction.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="spatialr">Spatial radius:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="spatialr" name="spatialr" value="10"> <span
													class="help-block">The spatial radius of the neighbourhood
													used for segmentation.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ranger">Range radius:</label>
												<input type="number" min="0" step="0.01"
													class="form-control" id="ranger" name="ranger" value="0.65">
												<span class="help-block">The range radius (expressed in
													radiometry unit).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="minsize">Minimum size of a
													region:</label> <input type="number" min="0" step="1"
													class="form-control" id="minsize" name="minsize"
													value="200"> <span class="help-block">Minimum size of a
													region (in pixel unit) in segmentation.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="refr">Reference raster:</label>
												<input type="text" class="form-control" id="refr"
													name="refr"> <span class="help-block">The reference raster
													when insitu data is not available.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="eroderad">Erosion radius:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="eroderad" name="eroderad" value="1"> <span
													class="help-block">The radius used for erosion.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="alpha">Alpha:</label> <input
													type="number" min="0" step="0.01" class="form-control"
													id="alpha" name="alpha" value="0.01"> <span
													class="help-block">The parameter alpha used by the
													mahalanobis function.</span>
											</div>
											<div class="form-group form-group-sm">
												<label for="classifier">Classifier:</label> <select
													class="form-control" id="classifier" name="classifier">
													<!-- <option value="">Select a classifier</option> -->
													<option value="rf" selected="selected">RF</option>
													<option value="svm">SVM</option>
												</select> <span class="help-block">Random forest clasifier /
													SVM classifier</span>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="field">Field:</label> <input
													type="text" class="form-control" id="field" name="field"
													value="CODE"><span class="help-block"></span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rfnbtrees">Training trees:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="rfnbtrees" name="rfnbtrees"> <span class="help-block"
													value="100">The number of trees used for training.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rfmax">Max depth:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="rfmax" name="rfmax" value="25"> <span
													class="help-block">Maximum depth of the trees used for
													Random Forest classifier.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rfmin">Minimum number of
													samples:</label> <input type="number" min="0" step="1"
													class="form-control" id="rfmin" name="rfmin" value="25"> <span
													class="help-block">Minimum number of samples in each node
													used by the classifier.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="minarea">The min numbers
													of pixel:</label> <input type="number" min="0" step="1"
													class="form-control" id="minarea" name="minarea" value="20">
												<span class="help-block">The minium number of pixel in an
													area where for an equal number of crop and nocrop samples
													the crop decision is taken.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="pixsize">Pixel size:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="pixsize" name="pixsize" value="10"> <span
													class="help-block">The size, in meters, of a pixel.</span>
											</div>
											<input type="submit" name="l4a" class="btn btn-primary"
												value="Submit">
										</div>

									</div>
								</form>
							</div>
						</div>
					</div>
					<!--end L4A  -->

					<!--start L4B  -->
					<div class="panel panel-default">
						<div class="panel-heading">
							<h4 class="panel-title">
								<a data-toggle="collapse" data-parent="#accordion" href="#l4b">L4B
									processor</a>
							</h4>
						</div>
						<div id="l4b" class="panel-collapse collapse">
							<div class="panel-body">
								<form enctype="multipart/form-data" role="form" id="l4bform" method="post"
									action="getProcessorNewConfig.php">
									<div class="row">
										<div class="col-md-8">
											<div class="form-group form-group-sm">
												<label class="control-label" for="siteId">Site:</label> <select
													class="form-control" id="siteId" name="siteId">
												</select>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="sentinel2Tiles">Sentinel2
													tiles:</label> <select class="form-control"
													id="sentinel2Tiles" name="sentinel2Tiles">
												</select>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="landsatTiles">Landsat
													tiles:</label> <select class="form-control"
													id="landsatTiles" name="landsatTiles">
												</select>
											</div>
											
											<div class="form-group form-group-sm">
												<label class="control-label" for="refp">Reference polygons:</label>
												<input type="file" class="form-control" id="refp" name="refp">
												<span class="help-block">The reference polygons.</span>
											</div>
											
											<div class="form-group form-group-sm">
												<label class="control-label" for="mission">Main mission:</label>
												<select class="form-control" id="mission" name="mission">
													<option value="SPOT" selected="selected">SPOT</option>
													<option value="LANDSAT">LANDSAT</option>
													<option value="SENTINEL">SENTINEL</option>
												</select>
												<!--  <input type="text" class="form-control" id="mission"
													name="mission" value="SPOT">-->
												<span class="help-block">The main mission for the time
													series.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="ratio">Ratio:</label> <input
													type="number" min="0" step="0.01" class="form-control"
													id="ratio" name="ratio" value="0.75"> <span
													class="help-block">The ratio between the validation and
													training polygons.</span>
											</div>
											
											
											<div class="form-group form-group-sm">
												<label class="control-label" for="inputFiles">Available input files:</label>
												<select multiple class="form-control" id="inputFiles" name="inputFiles[]"></select>
												<span class="help-block">The list of products descriptors (xml files).</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="cropMasks">Crop masks:</label>
												<select multiple class="form-control" id="cropMasks" name="cropMasks[]"></select>
												<span class="help-block">The list of crop mask products.</span>
											</div>
											
											
											<div class="form-group form-group-sm">
												<label class="control-label" for="resolution">Resolution:</label>
												<!-- <input type="number" min="0" max="20" step="10"
													class="form-control" id="resolution" name="resolution"
													value="10">--> 
												<select class="form-control" id="resolution"
													name="resolution">
													<option value="">Select a resolution</option>
													<option value="10">10</option>
													<option value="20">20</option>
													<option value="30">30</option>
													<option value="60">60</option>
												</select><span class="help-block">Resolution of the
													output image.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="trm">The temporal
													resampling mode:</label> <input type="radio"
													class="radio-inline" id="trm" name="trm"> resample <input
													checked="checked" type="radio" class="radio-inline"
													id="trm" name="trm"> gapfill
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="radius">Radius:</label> <input
													type="number" min="0" step="1" class="form-control"
													id="radius" name="radius" value="5"></input> <span
													class="help-block">The radius used for gap filling, in
													days.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rseed">Random seed:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="rseed" name="rseed" value="0"></input> <span
													class="help-block">The random seed used for training.</span>
											</div>
											<div class="form-group form-group-sm">
												<label for="classifier">Classifier:</label> <select
													class="form-control" id="classifier" name="classifier">
													<!-- <option value="">Select a classifier</option> -->
													<option value="rf" selected="selected">RF</option>
													<option value="svm">SVM</option>
												</select> <span class="help-block">Random forest clasifier /
													SVM classifier</span>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="field">Field:</label> <input
													type="text" class="form-control" id="field" name="field"
													value="CODE"></input><span class="help-block"></span>
											</div>

											<div class="form-group form-group-sm">
												<label class="control-label" for="rfnbtrees">Training trees:</label>
												<input type="number" min="0" step="1" class="form-control"
													id="rfnbtrees" name="rfnbtrees" value="100"> <span
													class="help-block">The number of trees used for training.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rfmax">Random Forest
													classifier max depth:</label> <input type="number" min="0"
													step="1" class="form-control" id="rfmax" name="rfmax"
													value="25"> <span class="help-block">Maximum depth of the
													trees used for Random Forest classifier.</span>
											</div>
											<div class="form-group form-group-sm">
												<label class="control-label" for="rfmin">Minimum number of
													samples:</label> <input type="number" min="0" step="1"
													class="form-control" id="rfmin" name="rfmin" value="25" />
												<span class="help-block">Minimum number of samples in each
													node used by the classifier.</span>
											</div>
											<input type="submit" name="l4b" class="btn btn-primary"
												value="Submit">
										</div>
									</div>
								</form>
							</div>
						</div>
					</div>
					<!--end L4B  -->

				</div>

			</div>
			<div class="clearing">&nbsp;</div>
		</div>
	</div>
</div>
<!-- main -->
<!-- main2 -->
<!-- main3 -->

<script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-2.1.4.min.js"></script>
<script
	src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.5/js/bootstrap.min.js"></script>
<script src="libraries/jquery-validate/jquery.validate.min.js"></script>
<script src="libraries/jquery-validate/additional-methods.min.js"></script>
<script src="scripts/config.js"></script>
<script src="scripts/processing_functions.js"></script>

<script>
		//$("#l3aform").validate();
		var l2a_proc_id;
		var l4a_proc_id;
		$(document)
				.ready(
						function() {
							//Load sites
							get_all_sites();
							get_processor_id('l2a', 'l2a_proc_id');
							get_processor_id('l4a', 'l4a_proc_id');
							// validate l3aform form on keyup and submit
							$("#l3aform")
									.validate(
											{
												rules : {
													siteId : "required",
													sentinel2Tiles : "required",
													landsatTiles : "required",
													'inputFiles[]' : "required",
													synthDate : {
														required : true,
														pattern : "[0-9]{4}(0[1-9]|1[0-2])(0[1-9]|[1-2][0-9]|3[0-1])"
													},
													halfSynthesis : "required"
												},
												messages : {
													synthDate : {
														pattern : "Enter a date in YYYYMMDD format"
													}
												},
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},

												submitHandler :function(form) {
													$.ajax({
											        url: $(form).attr('action'),
											        type: $(form).attr('method'),
											        data: $(form).serialize(),
											        success: function(response) {
									                    alert("Your form was submitted!");
									                   //clear inputs after submit
									                   $("#l3aform")[0].reset();
									              	   $("#inputFiles").val('');				
											                 }     
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});

							// validate l3b LAI form form on keyup and submit
							$("#l3b_laiform")
									.validate(
											{
												rules : {
													siteId : "required",
													//sentinel2Tiles : "required",
													landsatTiles : "required",
													'inputFiles[]' : "required",
												/*	resolution : "required",
													genmodel : "required",
													reproc : "required",												
													fitted : "required",
													bwr : "required",
													fwr : "required",*/
												/*modelsfolder : "required",
												rsrcfgfile : "required"*/
												},

												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},
												// specifying a submitHandler prevents the default submit
												submitHandler :function(form) {
													$.ajax({
											        url: $(form).attr('action'),
											        type: $(form).attr('method'),
											        data: $(form).serialize(),
											        success: function(response) {
											           alert("Your form was submitted!");
									                   //clear inputs after submit
									                   $("#l3b_laiform")[0].reset();
									              	   $("#inputFiles").val('');				
											                 }     
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});

							// validate l3b NVDI(pheno) form form on keyup and submit
							$("#l3b_nvdiform")
									.validate(
											{
												rules : {
													siteId : "required",
												//	sentinel2Tiles : "required",
													landsatTiles : "required",
													'inputFiles[]' : "required",
												//	resolution : "required",
												},
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},
												// specifying a submitHandler prevents the default submit,
												submitHandler :function(form) {
													$.ajax({
											        url: $(form).attr('action'),
											        type: $(form).attr('method'),
											        data: $(form).serialize(),
											        success: function(response) {
											           alert("Your form was submitted!");
									                   //clear inputs after submit
									                   $("#l3b_nvdiform")[0].reset();
									              	   $("#inputFiles").val('');				
											                 }     
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});

							// validate l4aform form on keyup and submit
							$("#l4aform")
									.validate(
											{
												rules : {
													siteId : "required",
													//sentinel2Tiles : "required",
													landsatTiles : "required",
													'inputFiles[]' : "required",
												/* not required
												mission:"mission",
												refp : "required",
												ratio : "required",													
												radius : "required",
												nbtrsample : "required",
												rseed : "required",
												lmbd : "required",
												weight : "required",
												nbcomp : "required",
												spatialr : "required",
												ranger : "required",
												minsize : "required",
												refr : "required",
												eroderad : "required",
												alpha : "required",
												classifier:"required",
												field:"required",
												rfnbtrees : "required",
												rfmax : "required",
												rfmin : "required",
												minarea : "minarea"
												pixsize : "required"*/
												},
												/*
												messages : {
													t0 : {
														pattern : "Enter a date in YYYYMMDD format"
													},
													tend : {
														pattern : "Enter a date in YYYYMMDD format"
													}
												},*/
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},
												// specifying a submitHandler prevents the default submit, good for the demo
												submitHandler :function(form) {
													$.ajax({
											        url: $(form).attr('action'),
											        type: $(form).attr('method'),
											        data: $(form).serialize(),
											        success: function(response) {
											           alert("Your form was submitted!");
									                   //clear inputs after submit
									                   $("#l4aform")[0].reset();
									              	   $("#inputFiles").val('');				
											                 }     
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});

							// validate l4bform form on keyup and submit
							$("#l4bform")
									.validate(
											{
												rules : {
													siteId : "required",
													//sentinel2Tiles : "required",
													landsatTiles : "required",
													//refp : "required",
													'inputFiles[]' : "required",
													'cropMasks[]' : "required",
													//resolution : "required",
												},
												// the errorPlacement has to take the table layout into account
												errorPlacement : function(
														error, element) {
													error.appendTo(element
															.parent());
												},
												// specifying a submitHandler prevents the default submit, good for the demo
												submitHandler :function(form) {
													$.ajax({
														url: $(form).attr('action'),
														type: $(form).attr('method'),
														data: $(form).serialize(),
														success: function(response) {
															alert("Your form was submitted!");
															//clear inputs after submit
															$("#l4bform")[0].reset();
															$("#inputFiles").val('');
															$("#cropMasks").val('');
														 }
											         });
											 },
												// set this class to error-labels to indicate valid fields
												success : function(label) {
													label.remove();
												},
												highlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.addClass(
																	"has-error");
												},
												unhighlight : function(element,
														errorClass) {
													$(element)
															.parent()
															.removeClass(
																	"has-error");
												}
											});
						});
	</script>
<?php include 'ms_foot.php'; ?>