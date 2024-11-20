let eqchart;

function setPEQ(peq) {
	// send data to microcontroller
	fetch('/cmd.php?mixer:eq:peq' + peq + '=' + document.getElementById('select_peq' + peq + '_type').selectedIndex + ',' + Math.round(slider2freq(document.getElementById('slider_peq' + peq + '_fc').value)) + ',' + document.getElementById('slider_peq' + peq + '_q').value + ',' + document.getElementById('slider_peq' + peq + '_gain').value);
}

function calcIIRCoefficients(type, fc, Q, gain, fs) {
	V = Math.pow(10.0, (Math.abs(gain)/20.0));
	K = Math.tan(Math.PI * fc/fs);
	
	switch(type) {
		case 0:
			norm = 1.0 / (1.0 + K * 1.0/Q + Math.pow(K, 2));
			a0 = (1.0 - K * 1.0/Q + Math.pow(K, 2)) * norm;
			a1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
			a2 = 1.0;
			b1 = a1;
			b2 = a0;
			break;
		case 1:
			if (gain>=0) {
				norm = 1.0 / (1.0 + (1.0/Q) * K + Math.pow(K,2));
			
				a0 = (1.0 + (V/Q) * K + Math.pow(K,2)) * norm;
				a1 = 2.0 * (Math.pow(K,2) - 1.0) * norm;
				a2 = (1.0 - (V/Q) * K + Math.pow(K,2)) * norm;
				//b0 = 1.0;
				b1 = a1;
				b2 = (1.0 - (1.0/Q) * K + Math.pow(K,2)) * norm;
			} else {
				norm = 1.0 / (1.0 + (V/Q) * K + Math.pow(K,2));

				a0 = (1.0 + (1.0/Q) * K + Math.pow(K,2)) * norm;
				a1 = 2.0 * (Math.pow(K,2) - 1.0) * norm;
				a2 = (1.0 - (1.0/Q) * K + Math.pow(K,2)) * norm;
				//b0 = 1.0;
				b1 = a1;
				b2 = (1.0 - (V/Q) * K + Math.pow(K,2)) * norm;
			}
			break;
		case 2:
			if (gain>=0) {    // boost
				norm = 1.0 / (1.0 + Math.sqrt(2.0) * K + Math.pow(K, 2));
				a0 = (1.0 + Math.sqrt(2.0*V) * K + V * Math.pow(K, 2)) * norm;
				a1 = 2.0 * (V * Math.pow(K, 2) - 1.0) * norm;
				a2 = (1.0 - Math.sqrt(2.0*V) * K + V * Math.pow(K, 2)) * norm;
				b1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
				b2 = (1.0 - Math.sqrt(2.0) * K + Math.pow(K, 2)) * norm;
			}
			else {    // cut
				norm = 1.0 / (1.0 + Math.sqrt(2.0*V) * K + V * Math.pow(K, 2));
				a0 = (1.0 + Math.sqrt(2.0) * K + Math.pow(K, 2)) * norm;
				a1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
				a2 = (1.0 - Math.sqrt(2) * K + Math.pow(K, 2)) * norm;
				b1 = 2.0 * (V * Math.pow(K, 2) - 1.0) * norm;
				b2 = (1.0 - Math.sqrt(2.0*V) * K + V * Math.pow(K, 2)) * norm;
			}
			break;
		case 3:
			if (gain>=0) {    // boost
				norm = 1.0 / (1.0 + Math.sqrt(2.0) * K + Math.pow(K, 2));
				a0 = (V + Math.sqrt(2.0*V) * K + Math.pow(K, 2)) * norm;
				a1 = 2.0 * (Math.pow(K, 2) - V) * norm;
				a2 = (V - Math.sqrt(2.0*V) * K + Math.pow(K, 2)) * norm;
				b1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
				b2 = (1.0 - Math.sqrt(2.0) * K + Math.pow(K, 2)) * norm;
			}
			else {    // cut
				norm = 1.0 / (V + Math.sqrt(2.0*V) * K + Math.pow(K, 2));
				a0 = (1.0 + Math.sqrt(2.0) * K + Math.pow(K, 2)) * norm;
				a1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
				a2 = (1.0 - Math.sqrt(2.0) * K + Math.pow(K, 2)) * norm;
				b1 = 2.0 * (Math.pow(K, 2) - V) * norm;
				b2 = (V - Math.sqrt(2.0*V) * K + Math.pow(K, 2)) * norm;
			}
			break;
		case 4:
			norm = 1.0 / (1.0 + K / Q + Math.pow(K, 2));
			a0 = (K / Q) * norm;
			a1 = 0;
			a2 = -a0;
			b1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
			b2 = (1.0 - K / Q + Math.pow(K, 2)) * norm;
			break;
		case 5:
			norm = 1.0 / (1.0 + K / Q + Math.pow(K, 2));
			a0 = (1.0 + Math.pow(K, 2)) * norm;
			a1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
			a2 = a0;
			b1 = a1;
			b2 = (1.0 - K / Q + Math.pow(K, 2)) * norm;
			break;
		case 6:
			norm = 1.0 / (1.0 + K / Q + Math.pow(K, 2));
			a0 = Math.pow(K, 2) * norm;
			a1 = 2.0 * a0;
			a2 = a0;
			b1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
			b2 = (1.0 - K / Q + Math.pow(K, 2)) * norm;
			break;
		case 7:
			norm = 1.0 / (1.0 + K / Q + Math.pow(K, 2));
			a0 = 1.0 * norm;
			a1 = -2.0 * a0;
			a2 = a0;
			b1 = 2.0 * (Math.pow(K, 2) - 1.0) * norm;
			b2 = (1.0 - K / Q + Math.pow(K, 2)) * norm;
			break;
	}

	return [a0, a1, a2, b1, b2];
}

/*
function square(x) {
	return x*x;
}
*/

function calcFrequencyResponse(a0, a1, a2,  b1, b2, f, fs) {
	w = (2.0 * Math.PI * f)/fs;
	phi = Math.pow(Math.sin(w/2.0),2);
	b0 = 1.0;
	
	return ( 10.0 * Math.log10( Math.pow(a0+a1+a2, 2) - 4.0*(a0*a1 + 4.0*a0*a2 + a1*a2) * phi + 16.0*a0*a2*Math.pow(phi,2) ) - 10.0 * Math.log10( Math.pow(b0+b1+b2, 2) - 4.0*(b0*b1 + 4.0*b0*b2 + b1*b2)*phi + 16.0*b0*b2*Math.pow(phi, 2) ) );
	//return 20*Math.log((Math.sqrt(square(a0*square(Math.cos(w))-a0*square(Math.sin(w))+a1*Math.cos(w)+a2)+square(2*a0*Math.cos(w)*Math.sin(w)+a1*(Math.sin(w)))) / Math.sqrt(square(square(Math.cos(w))-   square(Math.sin(w))+b1*Math.cos(w)+b2)+square(2*   Math.cos(w)*Math.sin(w)+b1*(Math.sin(w))))))
}

function slider2freq(position) {
  // position will be between 0 and 10000
  var minp = 0;
  var maxp = 10000;

  // The result should be between 20 an 20000
  var minv = Math.log(20);
  var maxv = Math.log(20000);

  // calculate adjustment factor
  var scale = (maxv-minv) / (maxp-minp);

  return Math.exp(minv + scale*(position-minp));
}

function freq2slider(position) {
  // position will be between 0 and 10000
  var minp = 0;
  var maxp = 10000;

  // The result should be between 20 an 20000
  var minv = Math.log(20);
  var maxv = Math.log(20000);

  // calculate adjustment factor
  var scale = (maxv-minv) / (maxp-minp);

  return ((Math.log(position)-minv)/scale)+minp;
}

function initEQ() {
	eqchart = Highcharts.chart('equalizer', {
		chart: {
			backgroundColor: '#282828',
			type: 'line',
			animation: false,
		},
		title: {
			text: '',
			align: 'left',
			style: {
				color: '#fff',
				//fontWeight: 'bold',
				//fontSize: '12px',
				//fontFamily: 'Trebuchet MS, Verdana, sans-serif'
			}   
		},
		/*
		subtitle: {
			text: 'f/bape Audioplayer, Source: <a href="https://www.github.com/xn--nding-jua/Audioplayer" target="_blank">Github-Projectpage</a>.',
			align: 'left',
			style: {
				color: '#fff',
				//fontWeight: 'bold',
				//fontSize: '12px',
				//fontFamily: 'Trebuchet MS, Verdana, sans-serif'

			}   
		},
		*/

		yAxis: {
			accessibility: {
				rangeDescription: 'Range: -20 to 20'
			},
			min: -20,
			max: 20,
			title: {
				text: 'Magnitude [dB]',
				style: {
					color: '#fff',
					//fontWeight: 'bold',
					//fontSize: '12px',
					//fontFamily: 'Trebuchet MS, Verdana, sans-serif'

				}   
			},
			lineColor: '#fff',
			labels: {
				style: {
					color: '#FFF',
					//font: '11px Trebuchet MS, Verdana, sans-serif'
				}
			}
		},

		xAxis: {
			accessibility: {
				rangeDescription: 'Range: 1 to 20000'
			},
			type: 'logarithmic',
			tickInterval: 1,
			min: 20,
			max: 20000,
			title: {
				text: 'Frequency [Hz]',
				style: {
					color: '#fff',
					//fontWeight: 'bold',
					//fontSize: '12px',
					//fontFamily: 'Trebuchet MS, Verdana, sans-serif'

				}   
			},
			lineColor: '#fff',
			tickColor: '#fff',
			labels: {
				style: {
					color: '#FFF',
					//font: '11px Trebuchet MS, Verdana, sans-serif'
				}
			}
	 },

		legend: {
			enabled:false,
			layout: 'vertical',
			align: 'right',
			verticalAlign: 'middle',
			labels: {
				style: {
					color: '#FFF',
					//font: '11px Trebuchet MS, Verdana, sans-serif'
				}
			}
		},

		plotOptions: {
			line: {
				marker: {
					enabled: false
				}
			},
			series: {
				label: {
					connectorAllowed: false
				},
				pointStart: 1
			}
		},

		series: [{
			name: 'Magnitude [dB]',
			lineWidth: 5,
			lineColor: '#FFFF00',
			data: []
		}],

		responsive: {
			rules: [{
				condition: {
					maxWidth: 200
				},
				chartOptions: {
					legend: {
						layout: 'horizontal',
						align: 'center',
						verticalAlign: 'bottom'
					}
				}
			}]
		}
	});
}

function updateEQ() {
	let data_x = [];
	let data_y = [];

	const fs = 48000;
	const f_max = 20000;
	// PEQ1
	var type = document.getElementById('select_peq1_type').selectedIndex;
	var fc = slider2freq(document.getElementById("slider_peq1_fc").value);
	var Q = document.getElementById("slider_peq1_q").value;
	var Gain = document.getElementById("slider_peq1_gain").value;
	[a0, a1, a2,  b1, b2] = calcIIRCoefficients(type, fc, Q, Gain, fs); // type, f_center, Q, Gain, f_sample
	for (let i=0; i<f_max; i++) {
		data_x[i] = i;
		data_y[i] = calcFrequencyResponse(a0, a1, a2, b1, b2, i, fs);
	}
	// PEQ2
	var type = document.getElementById('select_peq2_type').selectedIndex;
	var fc = slider2freq(document.getElementById("slider_peq2_fc").value);
	var Q = document.getElementById("slider_peq2_q").value;
	var Gain = document.getElementById("slider_peq2_gain").value;
	[a0, a1, a2,  b1, b2] = calcIIRCoefficients(type, fc, Q, Gain, fs); // type, f_center, Q, Gain, f_sample
	for (let i=0; i<f_max; i++) {
		data_x[i] = i;
		data_y[i] = data_y[i] + calcFrequencyResponse(a0, a1, a2, b1, b2, i, fs);
	}
	// PEQ3
	var type = document.getElementById('select_peq3_type').selectedIndex;
	var fc = slider2freq(document.getElementById("slider_peq3_fc").value);
	var Q = document.getElementById("slider_peq3_q").value;
	var Gain = document.getElementById("slider_peq3_gain").value;
	[a0, a1, a2,  b1, b2] = calcIIRCoefficients(type, fc, Q, Gain, fs); // type, f_center, Q, Gain, f_sample
	for (let i=0; i<f_max; i++) {
		data_x[i] = i;
		data_y[i] = data_y[i] + calcFrequencyResponse(a0, a1, a2, b1, b2, i, fs);
	}
	// PEQ4
	var type = document.getElementById('select_peq4_type').selectedIndex;
	var fc = slider2freq(document.getElementById("slider_peq4_fc").value);
	var Q = document.getElementById("slider_peq4_q").value;
	var Gain = document.getElementById("slider_peq4_gain").value;
	[a0, a1, a2,  b1, b2] = calcIIRCoefficients(type, fc, Q, Gain, fs); // type, f_center, Q, Gain, f_sample
	for (let i=0; i<f_max; i++) {
		data_x[i] = i;
		data_y[i] = data_y[i] + calcFrequencyResponse(a0, a1, a2, b1, b2, i, fs);
	}
	// PEQ5
	var type = document.getElementById('select_peq5_type').selectedIndex;
	var fc = slider2freq(document.getElementById("slider_peq5_fc").value);
	var Q = document.getElementById("slider_peq5_q").value;
	var Gain = document.getElementById("slider_peq5_gain").value;
	[a0, a1, a2,  b1, b2] = calcIIRCoefficients(type, fc, Q, Gain, fs); // type, f_center, Q, Gain, f_sample
	for (let i=0; i<f_max; i++) {
		data_x[i] = i;
		data_y[i] = data_y[i] + calcFrequencyResponse(a0, a1, a2, b1, b2, i, fs);
	}

	eqchart.series[0].setData(data_y);
}
