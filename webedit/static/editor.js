

function refresh_svg(d) {
    const DIM = 500
    const AREA = DIM * DIM;

    d3.select("#main_div").select("svg").remove()
    
    if (!d) {
        return;
    }

    const figure = d.figure;

    const xmin = d3.min(d.hole.concat(figure.vertices), x => x[0]);
    const xmax = d3.max(d.hole.concat(figure.vertices), x => x[0]);

    const ymin = d3.min(d.hole.concat(figure.vertices), x => x[1]);
    const ymax = d3.max(d.hole.concat(figure.vertices), x => x[1]);

    const ratio = (xmax - xmin) / (ymax - ymin);
    const X_SIZE = DIM * Math.sqrt(ratio);
    const Y_SIZE = DIM / Math.sqrt(ratio);

    d3.select("#main_div")
        .append("svg")
        .attr("id", "svg")
        .attr("width", X_SIZE)
        .attr("height", Y_SIZE);

    const xScale = d3.scaleLinear()
        .domain([xmin, xmax])
        .range([1, X_SIZE - 1]);

    const yScale = d3.scaleLinear()
        .domain([ymin, ymax])
        .range([1, Y_SIZE - 1]);

    const line = d3.line()
        .x(d => xScale(d.x))
        .y(d => yScale(d.y))

    function to_obj(xy) {
        return { "x": xy[0], "y": xy[1] };
    }

    var figdata = d.hole.map(to_obj);
    figdata.push(figdata[0]);

    const svg = d3.select("#svg");

    svg.append('path')
        .datum(figdata)
        .attr("stroke", "red")
        .attr("fill", "lightgray")
        .attr('d', line)

   

    function get_edge(uv) {
        return [figure.vertices[uv[0]], figure.vertices[uv[1]]]
    }

    figure.edges.forEach(uv => {
        var edge = get_edge(uv).map(to_obj);
        svg.append('path')
            .datum(edge)
            .attr("stroke", "green")
            .attr("fill", "none")
            .attr('d', line)
    });

}


function load_problem(id) {
    d3.json("/problem?id=" + id).then(function (data) {
        refresh_svg(data);
    });
}

function onload() {
    load_problem(1)
    d3.select('#submit').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        load_problem(d3.select("#problem_id").node().value);
    })
}


d3.select(window).on("load", onload);