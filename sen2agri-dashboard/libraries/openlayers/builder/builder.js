/* global $ */

var RELEASE = '3.4.0';

var API = {};

API.baseUrl = 'http://104.131.76.251';
API.listReleases = function() {
  return $.get(API.baseUrl + '/releases/');
};
API.getRelease = function(release) {
  return $.get(API.baseUrl + '/releases/' + release + '/');
};
API.listJobs = function() {
  return $.get(API.baseUrl + '/jobs/');
};
API.getJob = function(release, id) {
  return $.get(API.baseUrl + '/jobs/' + release + '/' + id );
};
API.createJob = function(release, payload) {
  return $.ajax(API.baseUrl + '/jobs/' + release + '/', {
    data: JSON.stringify(payload),
    contentType: 'application/json',
    type: 'POST'
  })
};
API.urlForBuild = function(release, job) {
  return API.baseUrl + '/builds/' + release + '/' + job + '/ol.min.js';
};

/** Defaults toggling **/

var symbolsToggled = true, definesToggled = true, definesDefaultToggled = true;
function setDefaults() {
  if (!definesToggled) {
    $('#btn-toggleDefines').trigger('click'); // clear if toggleAll'd
    definesDefaultToggled = true;
  }
  $('#defines')
    .find('input[default="true"]')
    .prop('checked', definesDefaultToggled);

  definesDefaultToggled = !definesDefaultToggled;
}

function toggleAllSymbols() {
  $('#symbols').find('input.toggle-symbol').prop('checked', symbolsToggled);
  symbolsToggled = !symbolsToggled;
}

function toggleAllDefines() {
  $('#defines').find('input.toggle-defines').prop('checked', definesToggled);
  definesToggled = !definesToggled;
}


/** Checkbox propagation **/


function updateChildState() {
  if(!$(this).prop('indeterminate')) {
    $(this)
      .siblings('ul')
      .find('input.toggle-symbol')
      .prop('checked', $(this).prop('checked'));
  }
}
/**
 * When a checkbox has been updated, update state of parent
 */
function updateParentState() {
  var parentList = $(this).closest('ul');
  var siblings = parentList.find('li > input.toggle-symbol');
  var parentCheckbox = parentList.siblings('input.toggle-symbol');

  if (parentCheckbox && parentCheckbox.length > 0) {
    var num = siblings.length;
    var checked = siblings.filter(':checked').length;

    parentCheckbox
      .prop('checked', checked === num)
      .prop('indeterminate', checked < num)
      .trigger('updateParent');
  }
}

function loadState(opts) {

  // Set symbols
  $('input.toggle-symbol').prop('checked', false);
  opts.symbols.forEach(function(name) {
    if (name.length) {
      $('input.toggle-symbol[value="'+name+'"]')
        .prop('checked', true);
    }
  });

  $('input.toggle-symbol.has-children').trigger('updateParent');

  // Set defines
  Object.keys(opts.defines)
    .forEach(function(name) {
      $('input.toggle-symbol[value="'+name+'"]')
        .prop('checked', opts.defines[name])
        .trigger('change');
    });
}

/**
 * Returns array of selected symbols
 * @return {Array} Selected symbols
 */
function getSymbols() {
  var items = $('input.toggle-symbol:checked');
  var arr = [];
  items.each(function() {
    if (this.value && this.value !== 'on') { //ignore 'on' checkbox value
      arr.push(this.value);
    }
  });
  return arr;
}


/**
 * Returns array of selected Definitions
 * @return {Array} Selected defines
 */
function getDefines() {
  var items = $('input.toggle-defines');
  var obj = {};
  items.each(function() {
    obj[this.value] = this.checked;
  });
  return obj;
}

function pollJob(id, cb) {
  API.getJob(RELEASE, id)
    .done(function(res) {
      if (res.status === 'pending') {
        setTimeout(function() {
          pollJob(id, cb);
        }, 2000);
      } else if (res.status === 'complete') {
        cb(null, res);
      } else if (res.status === 'error') {
        cb('Build failed', res);
      }
    })
    .fail(cb);
}


function noFollow() {
  return false;
};

var buildStatus = {};
buildStatus.state = "";
buildStatus.reset = function() {
  buildStatus.state = "";
  $('#btn-compile')
    .on('click', noFollow)
    .removeClass('download error loading')
    .button('reset')
    .find('.status')
      .html('Compile &amp; Download Selected');
  $('.build-error, .build-success').hide();
};

buildStatus.loading = function() {
  buildStatus.state = "loading";
  $('#btn-compile')
    .removeClass('download error')
    .addClass('loading')
    .button('loading');
};
buildStatus.finished = function(url) {
  buildStatus.state = "finished";
  $('#btn-compile')
    .off('click', noFollow)
    .removeClass('error loading')
    .addClass('download')
    .button('reset')
    .attr('target', '_blank')
    .attr('href', url)
    .find('.status')
      .html('Download Source');
};
buildStatus.error = function(err) {
  buildStatus.state = "error";
  $('#btn-compile')
    .removeClass('download loading')
    .addClass('error')
    .button('reset')
    .find('.status')
    .html('An error occured. Please try again');
  $('#build-error').html('Error').show(150);
  console.log(err);
};

function build() {
  buildStatus.reset();
  buildStatus.loading();
  var obj = {
    symbols: getSymbols(),
    defines: getDefines()
  };
  API.createJob(RELEASE, obj)
    .done(function(res) {
      var job = res.id;
      pollJob(job, function(err, res) {
        if(err) {
          buildStatus.error();
          return;
        }
        buildStatus.finished(API.urlForBuild(RELEASE, job));
      });
    })
    .fail(function(res) {
      buildStatus.error(res);
    })
}



$(document).ready(function () {
   $('#btn-compile')
    .on('click', noFollow);
  //$('[data-toggle="tooltip"]').tooltip()
  $('[data-toggle="popover"]').popover();

  $('label.tree-toggler').click(function toggleTree(e) {
    e.preventDefault();
    $(this).toggleClass('toggleIcon');
    $(this).parent().children('ul.tree').toggle(150);
  });

  //$('label.tree-toggler').trigger('click');
  $('li:has(ul) > input.toggle-symbol').on('change', updateChildState);
  
  $('input.toggle-symbol')
    .on('change', updateParentState)
    .on('updateParent', updateParentState);

  $('input').on('change', function() {
    if (buildStatus.state.length) {
      buildStatus.reset();
    }
  })
  $('#btn-toggleSymbols').on('click', toggleAllSymbols);
  $('#btn-toggleDefines').on('click', toggleAllDefines);

  $('#btn-toggleDefault').on('click', setDefaults);
  $('#btn-toggleDefault').trigger('click');

  $('#btn-compile').on('click', build);
});
