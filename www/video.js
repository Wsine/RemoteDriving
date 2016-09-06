var guageChart;

$(function() {
	adjustEleHeight();
	initChart();
	randomData();
    $("#tools .btn.yellow").click(function() {
        $("#content canvas").toggle();
    });
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
			show : false
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