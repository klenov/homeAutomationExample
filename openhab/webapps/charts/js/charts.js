
var influxdb_url      = "http://192.168.2.2:8086/db/openhab/series";
var influxdb_username = "openhab";
var influxdb_password = "influxdb_fake_password";

var graph;
var x_axis;
var series = [];

// colors http://www.mulinblog.com/a-color-palette-optimized-for-data-visualization/
var series_names = {
  '10m_Heating_Setpoint': { default: true,  color: '#F15854', name: "setpoint",   downsampling: false },
  Raw_Temp_Room_Low:  { default: true,  color: '#4D4D4D', name: "room low",     downsampling: true },
  Raw_Temp_Room_High: { default: false, color: '#5DA5DA', name: "room high",    downsampling: true },
  Raw_Temp_Komorka:   { default: false, color: '#4D4D4D', name: "utility room", downsampling: true },
  Raw_Temp_Outside_1: { default: true,  color: '#B276B2', name: "outside 1",    downsampling: true },
  Raw_Temp_Outside_2: { default: false, color: '#60BD68', name: "outside 2",    downsampling: true }
};

var time_ranges = {
  //'2h':  { default: false, name: "2 hours", appropriate_time_unit: '15 minute', series_prefix: '',     field_prefix: ''      },
  '4h':  { default: false, name: "4 hours", appropriate_time_unit: '15 minute', series_prefix: '',     field_prefix: ''      },
  '8h':  { default: true,  name: "8 hours", appropriate_time_unit: 'hour',      series_prefix: '',     field_prefix: '' },
  '24h': { default: false, name: "a day",   appropriate_time_unit: '6 hour',    series_prefix: '10m_', field_prefix: 'mean_' },
  '1w':  { default: false, name: "a week",  appropriate_time_unit: 'day',       series_prefix: '10m_', field_prefix: 'mean_' },
  '4w':  { default: false, name: "a month", appropriate_time_unit: 'day',       series_prefix: '10m_', field_prefix: 'mean_' },
  '48w':  { default: false, name: "a year", appropriate_time_unit: 'week',       series_prefix: '10m_', field_prefix: 'mean_' }
};

var y_axis_ranges = {
  a:  { default: true,  min: -30, max: 30, name: "-30°C to 30°C" },
  b:  { default: false,  min: 0,  max: 30, name: "0°C to 30°C" },
  c:  { default: false, min: 15,  max: 30, name: "15°C to 30°C"  },
};

function influxdb_request_data(query) {
  return {
    u: influxdb_username,
    p: influxdb_password,
    q: query,
    time_precision: "s"
  };
}

function influxdb_query(item_name) {
  //var selected_time_range_key = $("input[name=time_ranges]:checked").val();
  var selected_time_range     = time_ranges[selected_time_range_key()];
  var time_range_query_condition = ' where time > now() - ' + selected_time_range_key();

  var should_apply_downsampling = series_names[item_name].downsampling;

  var field_prefix  = should_apply_downsampling ? selected_time_range.field_prefix  : ''
  var series_prefix = should_apply_downsampling ? selected_time_range.series_prefix : ''

  return "select " + field_prefix + "value from " + series_prefix + item_name + time_range_query_condition;
}

function selected_input_key(input_name) {
  return $("input[name=" + input_name + "]:checked").val();
}

function selected_time_range_key() {
  return $("input[name=time_ranges]:checked").val();
}

function selected_y_axis_ranges_key() {
  return $("input[name=y_axis_ranges]:checked").val();
}

function make_query_request(item_name) {
  var data;

  $.ajax({
    url: influxdb_url,
    type: "GET",
    dataType: "json",
    async: false,
    data: influxdb_request_data( influxdb_query(item_name) ),
    success: function(resp) {  data = resp[0]; }
  });

  return data;
}

function extract_points(response_data) {
  //console.log(response_data.points);
  var result = [];

  if (typeof response_data != 'undefined'){
    result = response_data.points.map(function(point) {
      return { x: point[0], y: point[2] };
    }).reverse();
  }
  return result;
}

function create_time_ranges(key) {
  add_control(key, 'radio', 'time_ranges');
}

function create_y_axis_ranges(key) {
  add_control(key, 'radio', 'y_axis_ranges');
}

function create_series_names(key) {
  add_control(key, 'checkbox', 'series_names');
}

function add_control(key, type, collection_name) {
  var element = eval(collection_name)[key];

  console.log(element);
  var str =  '<input type="' + type + '" name="' + collection_name + '"' + (element.default ? ' checked="checked" ' : '') + ' id="' + key + '" value="' + key  + '" />';
  str = str + '<label for="' + key + ' ">' + element.name + '</label><br>';

  $('#' + collection_name).append(str);
}

function add_graph_series(key) {
  var element = series_names[key];
  graph.series.push({ color: element.color, data: extract_points( make_query_request(key) ), name: element.name });
}

function redraw_chart()
{
  var checked_checkboxes = $("input[name=series_names]:checked");
  var items_to_query     = $.map( checked_checkboxes, function( val ) { return val.value; });

  //console.log(items_to_query);
  
  graph.series.splice(0, graph.series.length);
  items_to_query.forEach(add_graph_series);

  //graph.configure();
  configure_x_axis();
  configure_y_min_and_max_values();
  graph.render();
}

function reconfigure_y_axis()
{
  configure_y_min_and_max_values();
  graph.render();
}

function configure_x_axis()
{
  var time  = new Rickshaw.Fixtures.Time.Local();
  var tmr   = time_ranges[ selected_input_key('time_ranges') ];

  var appropriate_units = time.unit( tmr.appropriate_time_unit );
  
  if (x_axis === undefined) {
    x_axis = new Rickshaw.Graph.Axis.Time( { graph: graph, timeUnit: appropriate_units } );
  } else
  {
    x_axis.fixedTimeUnit = appropriate_units;
  }
}

function configure_y_min_and_max_values()
{
  var selected_y_range = selected_input_key('y_axis_ranges');
  var y_min = y_axis_ranges[selected_y_range].min;
  var y_max = y_axis_ranges[selected_y_range].max;

  graph.configure({
    min: y_min,
    max: y_max
  });
}


$(function() {
  Object.keys(time_ranges).forEach(create_time_ranges);
  Object.keys(series_names).forEach(create_series_names);
  Object.keys(y_axis_ranges).forEach(create_y_axis_ranges);

  var w_width  = Math.round(  $(window).width() * 0.7 );
  var w_height = Math.round( w_width * 0.4 );

  graph = new Rickshaw.Graph( {
    element: document.querySelector("#chart"),
    width: w_width,
    height: w_height,
    renderer: 'line',
    series: [],
    min: -30,
    max: 30
  } );

  var y_axis = new Rickshaw.Graph.Axis.Y( {
      graph: graph,
      orientation: 'left',
      pixelsPerTick: 20,
      //tickFormat: Rickshaw.Fixtures.Number.formatKMBT,
      element: document.getElementById('y_axis'),
  } );

  var hoverDetail = new Rickshaw.Graph.HoverDetail( {
  graph: graph,
  xFormatter: function(x) {
      return new Date(x * 1000).toLocaleString('en-US', { hour12: false });
    }
  } );

  redraw_chart();

  $( "input[name=series_names]" ).on( "click", redraw_chart );
  $( "input[name=time_ranges]"  ).on( "click", redraw_chart );
  $( "input[name=y_axis_ranges]").on( "click", reconfigure_y_axis );
});



