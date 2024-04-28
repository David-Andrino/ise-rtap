function calcBiquad(type, Fc, Fs, Q, peakGain, plotType) {
	var a0,a1,a2,b1,b2,norm;
	var ymin, ymax, minVal, maxVal;
	
	var V = Math.pow(10, Math.abs(peakGain) / 20);
	var K = Math.tan(Math.PI * Fc / Fs);
	switch (type) {
		case "one-pole lp":
			b1 = Math.exp(-2.0 * Math.PI * (Fc / Fs));
            a0 = 1.0 - b1;
            b1 = -b1;
			a1 = a2 = b2 = 0;
			break;
            
		case "one-pole hp":
			b1 = -Math.exp(-2.0 * Math.PI * (0.5 - Fc / Fs));
            a0 = 1.0 + b1;
            b1 = -b1;
			a1 = a2 = b2 = 0;
			break;
            
		case "lowpass":
			norm = 1 / (1 + K / Q + K * K);
			a0 = K * K * norm;
			a1 = 2 * a0;
			a2 = a0;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case "highpass":
			norm = 1 / (1 + K / Q + K * K);
			a0 = 1 * norm;
			a1 = -2 * a0;
			a2 = a0;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case "bandpass":
			norm = 1 / (1 + K / Q + K * K);
			a0 = K / Q * norm;
			a1 = 0;
			a2 = -a0;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case "notch":
			norm = 1 / (1 + K / Q + K * K);
			a0 = (1 + K * K) * norm;
			a1 = 2 * (K * K - 1) * norm;
			a2 = a0;
			b1 = a1;
			b2 = (1 - K / Q + K * K) * norm;
			break;
		
		case "peak":
			if (peakGain >= 0) {
				norm = 1 / (1 + 1/Q * K + K * K);
				a0 = (1 + V/Q * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - V/Q * K + K * K) * norm;
				b1 = a1;
				b2 = (1 - 1/Q * K + K * K) * norm;
			}
			else {	
				norm = 1 / (1 + V/Q * K + K * K);
				a0 = (1 + 1/Q * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - 1/Q * K + K * K) * norm;
				b1 = a1;
				b2 = (1 - V/Q * K + K * K) * norm;
			}
			break;
		case "lowShelf":
			if (peakGain >= 0) {
				norm = 1 / (1 + Math.SQRT2 * K + K * K);
				a0 = (1 + Math.sqrt(2*V) * K + V * K * K) * norm;
				a1 = 2 * (V * K * K - 1) * norm;
				a2 = (1 - Math.sqrt(2*V) * K + V * K * K) * norm;
				b1 = 2 * (K * K - 1) * norm;
				b2 = (1 - Math.SQRT2 * K + K * K) * norm;
			}
			else {	
				norm = 1 / (1 + Math.sqrt(2*V) * K + V * K * K);
				a0 = (1 + Math.SQRT2 * K + K * K) * norm;
				a1 = 2 * (K * K - 1) * norm;
				a2 = (1 - Math.SQRT2 * K + K * K) * norm;
				b1 = 2 * (V * K * K - 1) * norm;
				b2 = (1 - Math.sqrt(2*V) * K + V * K * K) * norm;
			}
			break;
		case "highShelf":
            if (peakGain >= 0) {
                norm = 1 / (1 + Math.SQRT2 * K + K * K);
                a0 = (V + Math.sqrt(2*V) * K + K * K) * norm;
                a1 = 2 * (K * K - V) * norm;
                a2 = (V - Math.sqrt(2*V) * K + K * K) * norm;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - Math.SQRT2 * K + K * K) * norm;
            }
            else {	
                norm = 1 / (V + Math.sqrt(2*V) * K + K * K);
                a0 = (1 + Math.SQRT2 * K + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = (1 - Math.SQRT2 * K + K * K) * norm;
                b1 = 2 * (K * K - V) * norm;
                b2 = (V - Math.sqrt(2*V) * K + K * K) * norm;
			}
			break;
			
		return [a0, a1, a2, b1, b2];
	}
}

let type = "";
let Fc = "";
let Fs = "";
let Q = "";
let peakGain = "";
let plotType = "";

console.log(calcBiquad(type, Fc, Fs, Q, peakGain, plotType));