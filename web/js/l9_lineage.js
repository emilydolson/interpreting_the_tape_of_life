var vis_configs = {
  data_file_path: "data/lineage_sequences.csv",

  focal_treatment: "l9",
  div_id: "l9_lineage_vis",

  margin: {top: 20, right: 40, bottom: 20, left: 100},
  tick_height: 0.1,
  max_task_width: 10,
  max_seq_hspacer: 5,
  
  slice_vspacer: 50,
  
  full_range: {min: null, max: null},
  slice_ranges: [{min: 0, max: 500}, {min: 50000, max: 55000}, {min: 95000, max: 100000}, {min: 300000, max:305000}],
  
  time_tick_interval: 500, 

  tasks_set: ["NOT", "NAND", "AND", "ORNOT", "OR", "ANDNOT", "NOR", "XOR", "EQUALS"]
};

var TextSize = function(text) {
  if (!d3) return;
  var container = d3.select('body').append('svg');
  container.append('text').attr( "x", -99999).attr("y", -99999).text(text);
  var size = container.node().getBBox();
  container.remove();
  return { width: size.width, height: size.height };
}

var LineageStateSeqDataAccessor = function(row) {
  // Header info: phen_seq_by_geno_state, phen_seq_by_geno_start, phen_seq_by_geno_duration, phen_seq_by_phen_state, phen_seq_by_phen_start, phen_seq_by_phen_duration
  var treatment = row.treatment;
  var run_id = row.run_id;
  var indiv_seq_states = row.phen_seq_by_geno_state.split(",");
  var indiv_seq_starts = row.phen_seq_by_geno_start.split(",");
  var indiv_seq_durations = row.phen_seq_by_geno_duration.split(",");
  var phen_seq_states = row.phen_seq_by_phen_state.split(",");
  var phen_seq_starts = row.phen_seq_by_phen_start.split(",");
  var phen_seq_durations = row.phen_seq_by_phen_duration.split(",");
  // Build a phenotype state sequence
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

  var display_seq = GetCompressionMode();

  var DrawVisualization = function() {
    console.log("==Draw==");

    canvas.selectAll("g").remove();
    
    // Slice data?
    var data_ranges = null;
    if (IsSliced()) {
      data_ranges = vis_configs.slice_ranges;
    } else {
      data_ranges = [{min: vis_configs.full_range.min, 
                      max: vis_configs.full_range.max}];
    }

    // Calculate total size of display range.
    var total_range = 0;
    for (var i = 0; i < data_ranges.length; i++) {
      total_range += data_ranges[i].max - data_ranges[i].min;
    }

    // Setup frame/canvas
    var frame_width = GetVisParentWidth() - 20; // Magic number!
    //                  How tall will lineage sequences be?     + How much space will vertical spaces between data slices take?
    var canvas_height = (total_range * vis_configs.tick_height) + ((data_ranges.length-1) * vis_configs.slice_vspacer);
    // Calculate frame height from desired canvas height + relevant margins
    var frame_height = canvas_height + vis_configs.margin.top + vis_configs.margin.bottom;
    var canvas_width = Math.min(frame_width - vis_configs.margin.left - vis_configs.margin.right,
                                (data.length) * ((vis_configs.tasks_set.length*vis_configs.max_task_width)+vis_configs.max_seq_hspacer));
    // frame.attrs({"width": frame_width, "height": frame_height});
    frame.attr("width", frame_width);
    frame.attr("height", frame_height);
    canvas.attr("transform", "translate(" + vis_configs.margin.left + "," + vis_configs.margin.top + ")");

    // Compute x domain for visualization.
    var x_domain = [0, data.length];
    var x_range = [0, canvas_width];
    var x_scale = d3.scaleLinear().domain(x_domain)
                                  .range(x_range);
    // Clear old axes
    canvas.selectAll(".x-axis").remove();
    // Append new x-axis
    var x_axis = d3.axisTop().scale(x_scale).tickValues([]);
    canvas.append("g").attr("class", "axis x-axis")
                      .attr("id", "seq-vis_x-axis")
                      .call(x_axis);
    
    // Append canvas to display data on.
    var data_canvas = canvas.append("g").attr("class", "data-canvas");
    // Make individual groups for each data slice.
    var slices = data_canvas.selectAll("g").data(data_ranges);
    slices.enter().append("g").attr("class", "data-slice")
                              .attr("transform", function(d, i) {
                                  // Calculate how far down to push this slice.
                                  var down = 0;
                                  for (var ri = 0; ri < i; ri++) {
                                    down += ((data_ranges[ri].max - data_ranges[ri].min)*vis_configs.tick_height)+vis_configs.slice_vspacer;
                                  }
                                  return "translate(0,"+down+")";
                                });
    console.log("Hi?");                            
    data_canvas.selectAll(".data-slice").each(function(slice_range, slice_id) {
      console.log("======================= Slice "+slice_id+"("+slice_range.min+","+slice_range.max+") =======================");
      var y_domain = [slice_range.min, slice_range.max];
      var y_range = [0, (slice_range.max - slice_range.min) * vis_configs.tick_height];
      var y_scale = d3.scaleLinear().domain(y_domain)
                                    .range(y_range)
                                    .clamp(true);
      var y_axis = d3.axisLeft().scale(y_scale).ticks((slice_range.max - slice_range.min)/vis_configs.time_tick_interval);
      d3.select(this).append("g").attr("class", "axis y-axis")
                     .attr("id", "seq-vis_y-axis_r"+slice_id)
                     .call(y_axis);
      console.log("Added that y axis you asked for");
      // Slap down the data.
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
                          
                          console.log("---- Sequence: " + seq_id + " ----");
                          
                          // Filter seq data down to only that which falls in appropriate range.
                          var seq_data = seq[display_seq].filter(function(d) {
                            var begin_t = d.start;
                            var end_t = d.start + d.duration;
                            var begins_in = begin_t >= slice_range.min && begin_t <= slice_range.max;
                            var ends_in = end_t >= slice_range.min && end_t <= slice_range.max;
                            var over = (end_t > slice_range.max) && (begin_t < slice_range.min);
                            return ends_in || begins_in || over;
                          });

                          console.log("All sequence data: "); console.log(seq);
                          console.log("Filtered sequence data: "); console.log(seq_data);

                          var states = d3.select(this).selectAll("g").data(seq_data);
                          console.log("  Selected states");
                          states.enter().append("g")
                                        .attr("class", function(state, state_id) { return "slice-"+slice_id+"_seq-"+seq_id+"_state PHEN-STATE__"+state.state; })
                                        .attr("state", function(state) { return state.state; })
                                        .attr("start", function(state) { return state.start; })
                                        .attr("end", function(state) { return state.start + state.duration; })
                                        .attr("duration", function(state) { return state.duration; })
                                        .attr("transform", function(state, state_id) {
                                          var down = y_scale(state.start);
                                          var over = 0;
                                          return "translate("+over+"," + down + ")";
                                        }).each(function(state, state_id) {
                                          console.log ("Task profile here?");
                                          console.log(state);
                                          var tasks = d3.select(this).selectAll("rect").data(vis_configs.tasks_set);
                                          tasks.enter().append("rect")
                                                       .attr("class", function(task, task_id) {
                                                         // Does this state do this task?
                                                         class_str = "task-indicator";
                                                         if (state.task_set.has(task)) {
                                                          class_str += " PHEN-TASK__" + task;
                                                         }
                                                         return class_str;
                                                       })
                                                       .attr("transform", function(task, task_id) {
                                                        // Move over by task_id
                                                        over = x_scale(1/(vis_configs.tasks_set.length+1))*task_id;
                                                        return "translate(" + over + ",0)";
                                                       })
                                                       .attr("width", x_scale(1/(vis_configs.tasks_set.length+1)) - 0.1)
                                                       .attr("height", y_scale(slice_range.min + Math.min(slice_range.max-state.start,state.duration)) - 0.5)
                                                      //  .style("fill", "grey");
                                        });
                       })

    });

    // Add some default styling to the axes
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

  };

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


}

var main = function() {
  console.log("=> Enter main");
  d3.csv(vis_configs.data_file_path, LineageStateSeqDataAccessor)
    .then(BuildVisualization);
  console.log("=> Done main");
}

main();