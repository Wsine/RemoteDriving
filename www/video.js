var guageChart;

$(function() {
	adjustEleHeight();
	initChart();
	randomData();
});

function adjustEleHeight() {
	var img = $("#camera img");
	img.height(img.width() * 9 / 16);
	$("#chart").height(img.height());
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
                    fontWeight: 'bolder'
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
            axisLine: {            // 坐标轴线
                lineStyle: {       // 属性lineStyle控制线条样式
                    width: 8
                }
            },
            axisTick: {            // 坐标轴小标记
                length:12,        // 属性length控制线长
                lineStyle: {       // 属性lineStyle控制线条样式
                    color: 'auto'
                }
            },
            splitLine: {           // 分隔线
                length:20,         // 属性length控制线长
                lineStyle: {       // 属性lineStyle（详见lineStyle）控制线条样式
                    color: 'auto'
                }
            },
            pointer: {
                width:5
            },
            title: {
                offsetCenter: [0, '-30%'],       // x, y，单位px
            },
            detail: {
                textStyle: {       // 其余属性默认使用全局文本样式，详见TEXTSTYLE
                    fontWeight: 'bolder'
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