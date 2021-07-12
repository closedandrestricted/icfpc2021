

function clone(obj) {
    return JSON.parse(JSON.stringify(obj));
}

function sub(p1, p2) {
    return [p1[0] - p2[0], p1[1] - p2[1]];
}


function vmul(v1, v2) {
    return v1[0] * v2[1] - v1[1] * v2[0]
}

function smul(v1, v2) {
    return v1[0] * v2[0] + v1[1] * v2[1]
}

function isect2(ua, ub, va, vb) {
    return vmul(sub(ub, ua), sub(va, ua)) * vmul(sub(ub, ua), sub(vb, ua)) < 0 && vmul(sub(vb, va), sub(ua, va)) * vmul(sub(vb, va), sub(ub, va)) < 0;
}

function between(a, mid, b) {
    var v1 = sub(a, mid);
    var v2 = sub(b, mid);
    return vmul(v1, v2) == 0 && smul(v1, v2) <= 0;
}

function isect(ua, ub, poly) {
    for (var i = 0; i < poly.length; ++i) {
        var a = poly[i];
        var b = poly[i + 1 == poly.length ? 0 : i + 1];
        if (between(ua, b, ub)) {
            var c = poly[i + 2 >= poly.length ? i + 2 - poly.length : i + 2];
            var interior = function (p) {
                if (vmul(sub(c, b), sub(a, b)) >= 0) {
                    return vmul(sub(c, b), sub(p, b)) >= 0 && vmul(sub(p, b), sub(a, b)) >= 0;
                } else {
                    return vmul(sub(c, b), sub(p, b)) >= 0 || vmul(sub(p, b), sub(a, b)) >= 0;
                }
            };
            if (!interior(ua) || !interior(ub)) {
                return true;
            }
        } else if (between(ua, a, ub)) {
            continue;
        } else if (between(a, ua, b)) {
            if (vmul(sub(a, b), sub(ub, b)) >= 0) {
                return true;
            }
        } else if (between(a, ub, b)) {
            if (vmul(sub(a, b), sub(ua, b)) >= 0) {
                return true;
            }
        } else if (isect2(ua, ub, a, b)) {
            return true;
        }
    }
    return false;
}

d3.selection.prototype.moveToFront = function () {
    return this.each(function () {
        this.parentNode.appendChild(this);
    });
};

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

    var globalist_problem_id = undefined;

    const svg = d3.select("#svg");

    function dist2(v1, v2) {
        return (v1[0] - v2[0]) ** 2 + (v1[1] - v2[1]) ** 2
    }

    function dist_ok(newV1, newV2, oldV1, oldV2) {
        return Math.abs(1.0 * dist2(newV1, newV2) / dist2(oldV1, oldV2) - 1.0) <= (problem.epsilon + 1e-12) / 1000000.0
    }

    svg.append('path')
        .datum(figdata)
        .attr("stroke", "black")
        .attr("fill", "lightgray")
        .attr('d', line)


    if (problem.bonuses) {
        problem.bonuses.forEach((bonus) => {
            var color = "lightblue";
            if (bonus.bonus == "GLOBALIST") {
                color = "yellow";
            } else if (bonus.bonus == "WALLHACK") {
                color = "orange";
            } else if (bonus.bonus == "BREAK_A_LEG") {
                color = "blue";
            }
            var x = bonus.position[0];
            var y = bonus.position[1];
            svg.append("circle")
                .style("stroke", "black")
                .style("fill", color)
                .attr("r", d => xScale(1) - xScale(0))
                .attr("cx", d => xScale(x))
                .attr("cy", d => yScale(y));
        })
    }



    function plot_figure(vertices, cls, color, mark_bad) {
        svg.selectAll("." + cls + "-e").remove()
        function get_edge(uv) {
            return [vertices[uv[0]], vertices[uv[1]]]
        }
        figure.edges.forEach(uv => {
            var edge_as_array = get_edge(uv);
            var edge = edge_as_array.map(to_obj);
            var actualColor = color;
            if (mark_bad) {
                const newV1 = vertices[uv[0]];
                const newV2 = vertices[uv[1]];
                const oldV1 = figure.vertices[uv[0]];
                const oldV2 = figure.vertices[uv[1]];
                if (!dist_ok(newV1, newV2, oldV1, oldV2)) {
                    actualColor = "red";
                } else if (isect(newV1, newV2, problem.hole)) {
                    actualColor = "orange";
                }
            }
            svg.append('path')
                .datum(edge)
                .attr("class", cls + "-e")
                .attr("stroke", actualColor)
                .attr("fill", "none")
                .attr("stroke-opacity", "0.8")
                .attr('d', line);
        });
        svg.selectAll("." + cls + "-v").moveToFront();
        recompute_globalist();
    }


    plot_figure(figure.vertices, "initial", "blue");
    var solution = undefined;

    function set_solution(sol) {
        solution = clone(sol);
        solution.forEach((d, i) => d.idx = i);
    }
    set_solution(figure.vertices);

    function snap_to_corners_set() {
        return d3.select("#check_snap").property("checked");
    }


    function recompute_globalist() {
        if (!globalist_problem_id) {
            return;
        }
        var n_edges = 0;
        var penalty = 0;
        figure.edges.forEach(uv => {
            const oldV1 = figure.vertices[uv[0]];
            const oldV2 = figure.vertices[uv[1]];
            const newV1 = solution[uv[0]];
            const newV2 = solution[uv[1]];
            penalty += Math.abs(1.0 * dist2(newV1, newV2) / dist2(oldV1, oldV2) - 1.0);
            n_edges += 1;
        })
        var frac = penalty / ((problem.epsilon + 1e-12) * n_edges / 1000000.0)
        d3.select("#globalist_id").property("value", frac.toFixed(4));
        console.log(frac);
        if (frac <= 1) {
            d3.select("#globalist_id").style("background-color", "lightgreen");
        } else {
            d3.select("#globalist_id").style("background-color", "red");
        }
    }


    function draw_solution() {

        plot_figure(solution, "solution", "green", true);

        var drag = d3.drag().subject(this)
            .on('start', function (evt, d) {
                var circles = [];
                figure.edges.forEach(uv => {
                    var v = 0;
                    if (uv[0] == d.idx) {
                        v = uv[1];
                    } else if (uv[1] == d.idx) {
                        v = uv[0];
                    } else {
                        return;
                    }
                    const oldV1 = figure.vertices[uv[0]];
                    const oldV2 = figure.vertices[uv[1]];
                    const r = Math.sqrt(dist2(oldV1, oldV2));
                    circles.push({ "x": solution[v][0], "y": solution[v][1], "r": r });
                })
                // console.log(circles);
                svg.selectAll(".circle-hint")
                    .data(circles)
                    .enter()
                    .append("circle")
                    .attr("class", "circle-hint")
                    .style("stroke", "black")
                    .style("fill", "none")
                    .attr("r", d => xScale(d.x + d.r) - xScale(d.x))
                    .attr("cx", d => xScale(d.x))
                    .attr("cy", d => yScale(d.y));
            })
            .on('end', function (evt, d) {
                svg.selectAll(".circle-hint").remove();
            })
            .on('drag', function (evt, d) {
                var newX = Math.round(xScale.invert(evt.x));
                var newY = Math.round(yScale.invert(evt.y));
                if (newX != d[0] || newY != d[1]) {
                    if (snap_to_corners_set()) {
                        var nearest = undefined;
                        var best_dist = undefined;
                        problem.hole.forEach((d) => {
                            var dist = dist2([newX, newY], d);
                            if (best_dist == undefined || dist < best_dist) {
                                best_dist = dist;
                                nearest = d;
                            }
                        });
                        if (best_dist < 6) {
                            newX = nearest[0];
                            newY = nearest[1];
                        }
                    }
                    if (newY < ymin || newY > ymax || newX < xmin || newX > xmax) {
                        return;
                    }
                    d3.select(this).attr("cx", xScale(newX));
                    d3.select(this).attr("cy", yScale(newY));
                    d[0] = newX;
                    d[1] = newY;
                    plot_figure(solution, "solution", "green", true);
                }
            });

        svg.selectAll(".solution-v")
            .data(solution)
            .join("circle")
            .attr("class", "solution-v")
            .style("stroke", "none")
            .attr("fill-opacity", "0.7")
            .style("fill", "green")
            .attr("r", 3)
            .attr("cx", d => xScale(d[0]))
            .attr("cy", d => yScale(d[1]))
            .call(drag);
    }

    svg.selectAll(".circle-hole")
        .data(problem.hole)
        .enter()
        .append("circle")
        .attr("class", "circle-hole")
        .style("stroke", "none")
        .style("fill", "black")
        .attr("r", 4)
        .attr("cx", d => xScale(d[0]))
        .attr("cy", d => yScale(d[1]))
        .on("mouseout", () => svg.selectAll(".diff-hint").remove())
        .on("mouseover", (ev, d) => {
            const i = problem.hole.indexOf(d);
            const imns1 = i == 0 ? problem.hole.length - 1 : i - 1;
            const ipls1 = i == problem.hole.length - 1 ? 0 : i + 1;
            const oldVm = problem.hole[imns1];
            const oldV0 = problem.hole[i];
            const oldVp = problem.hole[ipls1];
            var okM = {};
            var okP = {};
            function add(di, k, v) {
                if (di[k]) {
                    di[k].push(v)
                } else {
                    di[k] = [v]
                }
            }
            figure.edges.forEach(uv => {
                const newV1 = figure.vertices[uv[0]];
                const newV2 = figure.vertices[uv[1]];
                if (dist_ok(newV1, newV2, oldV0, oldVm)) {
                    add(okM, uv[0], uv[1]);
                    add(okM, uv[1], uv[0]);
                }
                if (dist_ok(newV1, newV2, oldV0, oldVp)) {
                    add(okP, uv[0], uv[1]);
                    add(okP, uv[1], uv[0]);
                }
            });
            for (const [k, vs] of Object.entries(okP)) {
                if (!okM[k]) {
                    continue;
                }
                if (okM[k].length == 1 && okP[k].length == 1 && okM[k][0] == okP[k][0]) {
                    return
                }
                vs.concat(okM[k]).forEach(vidx => {
                    var line_coords = [solution[k], solution[vidx]].map(to_obj);
                    svg.append('path')
                        .datum(line_coords)
                        .attr("class", "diff-hint")
                        .attr("stroke", "red")
                        .attr("fill", "none")
                        .attr("stroke-width", "2")
                        .attr("stroke-opacity", "0.8")
                        .attr('d', line);
                })
                var xy = solution[k];
                svg.append("circle")
                    .attr("class", "diff-hint")
                    .style("stroke", "none")
                    .attr("fill-opacity", "0.7")
                    .style("fill", "red")
                    .attr("r", 5)
                    .attr("cx", d => xScale(xy[0]))
                    .attr("cy", d => yScale(xy[1]));
            }
        });


    draw_solution();

    d3.select('#export').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        var json_sol = { "vertices": solution }
        if (globalist_problem_id) {
            json_sol["bonuses"] = [{ "bonus": "GLOBALIST", "problem": globalist_problem_id }]
        }
        globalist_problem_id
        d3.text("/save_sol?id=" + problem_id, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(json_sol),
        }).then(function (data, error) {
            if (error) {
                d3.select("#export_response").text("Error!")
            } else {
                d3.select("#export_response").text("Saved!")
            }
        });
    });

    d3.select('#solution_load').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        var kind = d3.select("#solution_folder").node().value
        d3.json("/solution?id=" + problem_id + "&kind=" + kind).then(function (data, error) {
            if (error) {
                d3.select("#export_response").text("Error!")
            } else {
                data.vertices.forEach((d, i) => {
                    solution[i][0] = d[0];
                    solution[i][1] = d[1];
                });
                draw_solution();
            }
        });
    });

    d3.select("#check_orig").on("change", function () {
        if (d3.select(this).property("checked")) {
            plot_figure(figure.vertices, "initial", "blue");
            draw_solution();
        } else {
            svg.selectAll(".initial-e").remove()
        }
    })

    d3.select('#v_flip').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        const ymin = d3.min(solution, x => x[1]);
        const ymax = d3.max(solution, x => x[1]);
        solution.forEach((d, i) => { solution[i][1] = ymax - (d[1] - ymin) });
        draw_solution();
    })
    d3.select('#h_flip').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        const xmin = d3.min(solution, x => x[0]);
        const xmax = d3.max(solution, x => x[0]);
        solution.forEach((d, i) => { solution[i][0] = xmax - (d[0] - xmin) });
        draw_solution();
    })

    d3.select('#globalist_set').on('click', function (e) {
        e.stopPropagation();
        e.preventDefault();
        var globalist_id = d3.select("#globalist_id").node().value;
        if (globalist_id) {
            globalist_problem_id = parseInt(globalist_id);
            recompute_globalist();
        } else {
            globalist_problem_id = undefined;
            d3.select("#globalist_id").style("background-color", null);
        }
    })
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
