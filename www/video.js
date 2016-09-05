var guageChart;

$(function() {
	adjustEleHeight();
	initChart();
	randomData();
});

function adjustEleHeight() {
    var height = $(window).height() - $("#content").position().top - 20;
    $("#video").height(height);
    $("#chart").height(height * 0.5);
	$("#chart").width($("#chart").height() * 16.0 / 9);
}

function initChart() {
	guageChart = echarts.init(document.getElementById("chart"));
	var option = {
		title : {
			show : false
		},
		legend : {
			show : false
		},
		tooltip : {
			show : false
		},
		toolbox : {
			show : true,
            feature : {
                myTool1: {
                    show: true,
                    title: 'toggle',
                    icon: 'path://M432.45,595.444c0,2.177-4.661,6.82-11.305,6.82c-6.475,0-11.306-4.567-11.306-6.82s4.852-6.812,11.306-6.812C427.841,588.632,432.452,593.191,432.45,595.444L432.45,595.444z M421.155,589.876c-3.009,0-5.448,2.495-5.448,5.572s2.439,5.572,5.448,5.572c3.01,0,5.449-2.495,5.449-5.572C426.604,592.371,424.165,589.876,421.155,589.876L421.155,589.876z M421.146,591.891c-1.916,0-3.47,1.589-3.47,3.549c0,1.959,1.554,3.548,3.47,3.548s3.469-1.589,3.469-3.548C424.614,593.479,423.062,591.891,421.146,591.891L421.146,591.891zM421.146,591.891',
                    onclick: function() {
                        $("#content canvas").toggle();
                    }
                }
            }
		},
		series : [{
			type : 'gauge',
			name : '速度',
			center: ['65%', '55%'],
			radius : '85%',
			splitNumber : 11,
			z: 3,
            min: 0,
            max: 220,
			axisLine : {
				lineStyle : {
					width : 10
				}
			},
			axisTick: {
                length: 15,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length: 20,
                lineStyle: {
                    color: 'auto'
                }
            },
            title : {
                textStyle: {
                    fontWeight: 'bolder',
                    fontSize: 20,
                }
            },
            detail : {
                textStyle: {
                    fontWeight: 'bolder',
                    fontSize: 18
                }
            },
            data:[{value: 40, name: 'km/h'}]
		}, {
            name: '转速',
            type: 'gauge',
            center: ['25%', '55%'],
            radius: '65%',
            min:0,
            max:7,
            endAngle:45,
            splitNumber:7,
            axisLine: {
                lineStyle: {
                    width: 8
                }
            },
            axisTick: {
                length:12,
                lineStyle: {
                    color: 'auto'
                }
            },
            splitLine: {
                length:20,
                lineStyle: {
                    color: 'auto'
                }
            },
            pointer: {
                width:5
            },
            title: {
                offsetCenter: [0, '-30%'],
                textStyle: {
                    fontSize: 11
                }
            },
            detail: {
                textStyle: {
                    fontWeight: 'bolder',
                    fontSize: 18
                }
            },
            data:[{value: 1.5, name: 'x1000\nr/min'}]
        }]
	};
	guageChart.setOption(option); 
}

function randomData() {
	setInterval(function() {
		var option = guageChart.getOption();
	    option.series[0].data[0].value = (Math.random()*100).toFixed(2) - 0;
	    option.series[1].data[0].value = (Math.random()*7).toFixed(2) - 0;
	    guageChart.setOption(option, true);
	}, 2000);
}