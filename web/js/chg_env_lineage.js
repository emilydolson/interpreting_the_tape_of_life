
var vis_configs = {
  data_file_path: "data/lineage_sequences.csv",

  focal_treatment: "change",
  div_id: "chg_env_lineage_vis",

  margin: {top: 20, right: 40, bottom: 20, left: 100},
  tick_height: 0.1,
  max_seq_width: 20,
  max_seq_hspacer: 3,
  slice_vspacer: 50,
  
  env_seq_width: 10,
  env_seq_hspacer: 5,
  
  full_range: {min: null, max: null},
  slice_ranges: [{min: 0, max: 500}, {min: 50000, max: 55000}, {min: 95000, max: 100000}],
  
  time_tick_interval: 500,

  env_seq: {
    states: ["ENV-NAND", "ENV-NOT"],
    interval: 200
  }

};


var TextSize = function(text) {
  if (!d3) return;
  var container = d3.select('body').append('svg');
  container.append('text').attr( "x", -99999).attr("y", -99999).text(text);
  var size = container.node().getBBox();
  container.remove();
  return { width: size.width, height: size.height };
}

var GenEnvSequence = function(env_states, interval, end) {
  seq = [];
  cur_env_id = 0;
  for (var i = 0; i < end; i+=interval) {
    seq.push({
      state: env_states[cur_env_id],
      start: i,
      duration: interval
    });
    cur_env_id += 1;
    cur_env_id %= env_states.length;
  }
  return seq;
}

var LineageStateSeqDataAccessor = function(row) {
  // Header info: phen_seq_by_geno_state, phen_seq_by_geno_start, phen_seq_by_geno_duration, phen_seq_by_phen_state, phen_seq_by_phen_start, phen_seq_by_phen_duration
  console.log("Process line: "); console.log(row);
  var treatment = row.treatment;
  var run_id = row.run_id;
  var indiv_seq_states = row.phen_seq_by_geno_state.split(",");
  var indiv_seq_starts = row.phen_seq_by_geno_start.split(",");
  var indiv_seq_durations = row.phen_seq_by_geno_duration.split(",");
  var phen_seq_states = row.phen_seq_by_phen_state.split(",");
  var phen_seq_starts = row.phen_seq_by_phen_start.split(",");
  var phen_seq_durations = row.phen_seq_by_phen_duration.split(",");
  // Build a phenotype state sequence
  console.log("Build phenotype sequence");
  var phen_seq = [];
  for (i = 0; i < phen_seq_states.length; i++) {
    var state_tasks = new Set(phen_seq_states[i].split("-"));
    var state = phen_seq_states[i];
    if (state == "") { state = "NONE"; }
    phen_seq.push({
      raw_state: phen_seq_states[i],
      state: state,
      task_set: state_tasks,
      duration: +phen_seq_durations[i],
      start: +phen_seq_starts[i]
    });
  }
  // Build an individual state sequence
  console.log("Build individual sequence");
  var indiv_seq = [];
  for (i = 0; i < indiv_seq_states.length; i++) {
    var state_tasks = new Set(indiv_seq_states[i].split("-"));
    var state = indiv_seq_states[i];
    if (state == "") { state = "NONE"; }
    indiv_seq.push({
      raw_state: indiv_seq_states[i],
      state: state,
      task_set: state_tasks,
      duration: +indiv_seq_durations[i],
      start: +indiv_seq_starts[i]
    });
  }

  if (vis_configs.full_range.max == null) { vis_configs.full_range.max = indiv_seq[indiv_seq.length - 1].start + indiv_seq[indiv_seq.length - 1].duration; }
  
  if (vis_configs.full_range.min == null) { vis_configs.full_range.min = indiv_seq[0].start; }

  // Update max time?
  if (indiv_seq[indiv_seq.length - 1].start + indiv_seq[indiv_seq.length - 1].duration > vis_configs.full_range.max) {
    vis_configs.full_range.max = indiv_seq[indiv_seq.length - 1].start + indiv_seq[indiv_seq.length - 1].duration;
  }
  if (phen_seq[phen_seq.length - 1].start + phen_seq[phen_seq.length - 1].duration > vis_configs.full_range.max) {
    vis_configs.full_range.max = phen_seq[phen_seq.length - 1].start + phen_seq[phen_seq.length - 1].duration;
  }
  // Update min time?
  if (indiv_seq[0].start < vis_configs.full_range.min) {
    vis_configs.full_range.min = indiv_seq[0].start;
  }
  if (phen_seq[0].start < vis_configs.full_range.min) {
    vis_configs.full_range.min = phen_seq[0].start;
  }

  // Return data
  return {
    treatment: treatment,
    run_id: run_id,
    phen_seq: phen_seq,
    indiv_seq: indiv_seq,
  };
}

var IsSliced = function() {
  return $("#slice-toggle").prop("checked");
}

// NOTE - due to historical contingency, naming of return values is poor.
var GetCompressionMode = function() {
  compression_mode = $('input[name="lineage-state-compression-option"]:checked').val();
  console.log(compression_mode);
  if (compression_mode == "phenotype") {
    return "phen_seq";
  } else if (compression_mode == "genotype") {
    return "indiv_seq";
  } else {
    return "UNDEFINED";
  }
}

var GetVisParentWidth = function() {
  return $("#"+vis_configs.div_id).parent().width();
};

var BuildVisualization = function(data) {
  console.log("=> Building lineage visualization!");
  // Filter data
  data = data.filter(function(d) {
    return d.treatment == vis_configs.focal_treatment;
  });
  // Setup the canvas
  var chart_area = d3.select("#"+vis_configs.div_id);
  var frame = chart_area.append("svg");
  var canvas = frame.append("g").attr("class", "vis-canvas");

  // Collect some user input
  var display_seq = GetCompressionMode();

  // Call this function to redraw the visualization on screen.
  var DrawVisualization = function() {
    console.log("===DRAW===");
    // canvas.remove("*");
    var display_full = !IsSliced();
    canvas.selectAll("g").remove();

    var env_data = GenEnvSequence(vis_configs.env_seq.states, vis_configs.env_seq.interval, vis_configs.full_range.max);

    // Slice data?
    var data_ranges = null;
    if (display_full) {
      data_ranges = [{min: vis_configs.full_range.min, 
                      max: vis_configs.full_range.max}];
    } else {
      data_ranges = vis_configs.slice_ranges;
    }

    // Calculate total size of display range.
    var total_range = 0;
    for (var i = 0; i < data_ranges.length; i++) {
      total_range += data_ranges[i].max - data_ranges[i].min;
    }

    // Helper function to get range ID of seq object
    var GetRangeID = function(seq_obj) {
      var start = seq_obj.start;
      for (var i = 0; i < data_ranges.length; i++) {
        if (start >= data_ranges[i].min && start <= data_ranges[i].max) { return i; }
      }
      // Failure
      return -1;
    }

    // Setup the frame/canvas.
    var frame_width = GetVisParentWidth() - 20; // Magic number!
    var canvas_height = (total_range * vis_configs.tick_height) + ((data_ranges.length-1) * vis_configs.slice_vspacer) // Here's the canvas height we want
    
    var frame_height = canvas_height + vis_configs.margin.top + vis_configs.margin.bottom;                             // Given desired canvas height, calculate frame height w/margins included.
    
    var canvas_width = Math.min(frame_width - vis_configs.margin.left - vis_configs.margin.right, 
                                (data.length+1) * (vis_configs.max_seq_width+vis_configs.max_seq_hspacer) );


  
    frame.attr("width", frame_width);
    frame.attr("height", frame_height);
    canvas.attr("transform", "translate(" + vis_configs.margin.left + "," + vis_configs.margin.top + ")");

    // var canvas_xscale = d3.scaleLinear().domain([0, data_ranges.length]).range([0, canvas_height]);
    // console.log(canvas_xscale);

    // Compute x/y domains for visualization.
    var x_domain = [0, data.length];
    var x_range = [0, canvas_width];
    var x_scale = d3.scaleLinear().domain(x_domain)
                                  .range(x_range);
    
    // Clear old axes.
    canvas.selectAll(".x-axis").remove();
    // Append x-axis.
    var x_axis = d3.axisTop().scale(x_scale).tickValues([]);
    canvas.append("g").attr("class", "axis x-axis")
                      .attr("id", "seq-vis_x-axis")
                      .call(x_axis);

    var data_canvas = canvas.append("g").attr("class", "data-canvas");
    var slices = data_canvas.selectAll("g").data(data_ranges);
    
    console.log("Data ranges: "); console.log(data_ranges);
    
    slices.enter()
          .append("g").attr("class", "data-slice")
          .attr("rmin", function(d) { return d.min; })
          .attr("rmax", function(d) { return d.max; })
          .attr("transform", function(d, i) {
            // Calculate down
            var down = 0;
            for (var ri = 0; ri < i; ri++) {
              down += ((data_ranges[ri].max - data_ranges[ri].min)*vis_configs.tick_height)+vis_configs.slice_vspacer;
            }
            return "translate(0," + down + ")";
          });
    
    data_canvas.selectAll(".data-slice").each(function(slice_range, slice_id) {
      // Make y axis for data slice
      var y_domain = [slice_range.min, slice_range.max];
      var y_range = [0, (slice_range.max - slice_range.min) * vis_configs.tick_height];
      var y_scale = d3.scaleLinear().domain(y_domain)
                                    .range(y_range)
                                    .clamp(true);
      var y_axis = d3.axisLeft().scale(y_scale).ticks((slice_range.max - slice_range.min)/vis_configs.time_tick_interval);                  
      d3.select(this).append("g").attr("class", "axis y-axis")
                                 .attr("id", "seq-vis_y-axis_r"+slice_id)
                                 .attr("transform", function(d) { return "translate(" + -1*(vis_configs.env_seq_width+vis_configs.env_seq_hspacer) +  ",0)"; } )
                                 .call(y_axis);
                                 
      
      // Here's 'g' canvas to work in for this slice.
      var slice_data_canvas = d3.select(this).append("g").attr("class", "data-slice-canvas");
      
      var sequences = slice_data_canvas.selectAll("g").data(data);
      sequences.enter().append("g")
                       .attr("class", function(d, i) { return "sequence-"+i; })
                       .attr("transform", function(seq, i) {
                         var xtran = x_scale(i);
                         var ytran = y_scale(0);
                         return "translate(" + xtran + "," + ytran + ")";
                       })
                      .each(function(seq, seq_id) {
                        // Filter seq data down to only that which falls in appropriate range.
                        var seq_data = seq[display_seq].filter(function(d) { 
                                                                var begin_t = d.start;
                                                                var end_t = d.start + d.duration;
                                                                var begins_in = begin_t >= slice_range.min && begin_t <= slice_range.max;
                                                                var ends_in = end_t >= slice_range.min && end_t <= slice_range.max;
                                                                var over = (end_t > slice_range.max) && (begin_t < slice_range.min);
                                                                return ends_in || begins_in || over;
                                                              });
                        var states = d3.select(this).selectAll("rect").data(seq_data);
                        states.enter().append("rect")
                                      .attr("class", function(state, state_id) { return "PHEN-STATE__"+state.state; } )
                                      .attr("state", function(state, state_id) { return state.state; })
                                      .attr("start", function(state, state_id) { return state.start; })
                                      .attr("end", function(state, state_id) { return state.start + state.duration; })
                                      .attr("duration", function(state, state_id) { return state.duration; })
                                      .attr("transform", function(state, state_id) {
                                        return "translate(0," + y_scale(state.start) + ")";
                                      })
                                      .attr("height", function(state, state_id) { return y_scale(slice_range.min + Math.min(slice_range.max-state.start,state.duration)) - 0.5; })
                                      .attr("width", x_scale(0.9))
                                      .attr("fill", function(state, state_id) { if (state.state == "0") return "grey";
                                                                                else return "blue"; });
                        
                      });

      // Overlay environment sequence
      var slice_env_canvas = d3.select(this).append("g")
                                            .attr("class", "env-slice-canvas")
                                            .attr("transform", function(seq, i) {
                                              var xtran = -1*(vis_configs.env_seq_width+vis_configs.env_seq_hspacer);
                                              var ytran = y_scale(0);
                                              return "translate(" + xtran + "," + ytran + ")";
                                            });
      var slice_env_data = env_data.filter(function(d) {
        var begin_t = d.start;
        var end_t = d.start + d.duration;
        var begins_in = begin_t >= slice_range.min && begin_t <= slice_range.max;
        var ends_in = end_t >= slice_range.min && end_t <= slice_range.max;
        return ends_in || begins_in;        
      });
      var env_states = slice_env_canvas.selectAll("rect").data(slice_env_data);
      env_states.enter().append("rect")
                        .attr("class", function(state, state_id) { return "ENV-STATE__"+state.state; })
                        .attr("state", function(state, state_id) { return state.state; })
                        .attr("start", function(state, state_id) { return state.start; })
                        .attr("end", function(state, state_id) { return state.start + state.duration; })
                        .attr("duration", function(state, state_id) { return state.duration; })
                        .attr("transform", function(state, state_id) {
                          return "translate(0," + y_scale(state.start) + ")";
                        })
                        .attr("height", function(state, state_id) { return y_scale(slice_range.min + Math.min(slice_range.max-state.start,state.duration)) - 0.5; })
                        .attr("width", vis_configs.env_seq_width);
    });
    
    // Add some default styling to axes
    var axes = canvas.selectAll(".axis");
    axes.selectAll("path").style("fill", "none")
                          .style("stroke", "black")
                          .style("shape-rendering","crispEdges");
    
    // Add y axis label
    canvas.selectAll(".axis-label").remove();
    canvas.append("text").attr("class", "axis-label")
                         .style("text-anchor", "middle")
                         .attr("x", 0 - (canvas_height/2))
                         .attr("y", 0 - vis_configs.margin.left / 1.5)
                         .attr("transform", "rotate(-90)")
                         .text("Time");
  }

  // Hookup slice toggle.
  $("#slice-toggle").change(function() {
    console.log("Hello!");
    DrawVisualization();
  });

  // Hookup compression toggles.
  $("input[type='radio'][name='lineage-state-compression-option']").on("change", function(){
    display_seq = GetCompressionMode();
    DrawVisualization()
  });

  $(window).resize(function() {
    DrawVisualization();
  });

  DrawVisualization();

  console.log("=>Finished building visualization");
  
}

var main = function() {
  console.log("=> Enter main");
  d3.csv(vis_configs.data_file_path, LineageStateSeqDataAccessor)
    .then(BuildVisualization);
  console.log("=> Done main");
}

// Call main!
main();
