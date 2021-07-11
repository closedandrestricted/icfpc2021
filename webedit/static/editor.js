

function clone(obj) {
    return JSON.parse(JSON.stringify(obj));
}

function refresh_svg(d, problem_id) {
    const DIM = 700;
    const AREA = DIM * DIM;

    d3.select("#main_div").select("svg").remove();

    var problem = d;

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

    const padding = 10;

    const xScale = d3.scaleLinear()
        .domain([xmin, xmax])
        .range([padding, X_SIZE - padding]);

    const yScale = d3.scaleLinear()
        .domain([ymin, ymax])
        .range([padding, Y_SIZE - padding]);

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

    function dist2(v1, v2) {
        return (v1[0] - v2[0]) ** 2 + (v1[1] - v2[1]) ** 2
    }

    function plot_figure(vertices, cls, color, mark_bad) {
        svg.selectAll("." + cls + "-e").remove()
        function get_edge(uv) {
            return [vertices[uv[0]], vertices[uv[1]]]
        }
        figure.edges.forEach(uv => {
            var edge = get_edge(uv).map(to_obj);
            var actualColor = color;
            if (mark_bad) {
                const newV1 = vertices[uv[0]];
                const newV2 = vertices[uv[1]];
                const oldV1 = figure.vertices[uv[0]];
                const oldV2 = figure.vertices[uv[1]];
                if (Math.abs(1.0 * dist2(newV1, newV2) / dist2(oldV1, oldV2) - 1.0) > (problem.epsilon + 1e-12) / 1000000.0) {
                    actualColor = "red";
                }
            }
            svg.append('path')
                .datum(edge)
                .attr("class", cls + "-e")
                .attr("stroke", actualColor)
                .attr("fill", "none")
                .attr('d', line);
        });
    }


    plot_figure(figure.vertices, "initial", "blue");

    var solution = clone(figure.vertices);

    plot_figure(solution, "solution", "green", true);


    var drag = d3.drag().subject(this)
        .on('drag', function (evt, d) {
            var newX = Math.round(xScale.invert(evt.x));
            var newY = Math.round(yScale.invert(evt.y));
            if (newX != d[0] || newY != d[1]) {
                d3.select(this).attr("cx", xScale(newX));
                d3.select(this).attr("cy", yScale(newY));
                d[0] = newX;
                d[1] = newY;
                plot_figure(solution, "solution", "green", true);
            }
        });

    svg.selectAll("solution-v")
        .data(solution)
        .join("circle")
        .style("stroke", "none")
        .style("fill", "green")
        .attr("r", 5)
        .attr("cx", d => xScale(d[0]))
        .attr("cy", d => yScale(d[1]))
        .call(drag);



    d3.select('#export').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        d3.text("/save_sol?id=" + problem_id, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ "vertices": solution }),
        }).then(function (data, error) {
            if (error) {
                d3.select("#export_response").text("Error!")
            } else {
                d3.select("#export_response").text("Saved!")
            }
        });
    });
}


function load_problem(id) {
    d3.json("/problem?id=" + id).then(function (data) {
        refresh_svg(data, id);
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