ZLPF {
	*ar { arg in, fc=440, damping=0.707, mul=1, add=0;

		var b0, b1, b2, a0, a1, a2;
		var c, cc, twodC;
		var out;
		
		out = in;
		
		damping.asArray.do({ arg df;
			
			c = (pi * fc/SampleRate.ir).tan.reciprocal;
			cc = c.squared;
			twodC = 2 * df * c;
	
			b0 = (1 + twodC + cc).reciprocal;
			b1 = 2 * b0;
			b2 = b0;
	
			a0 = 1;
			a1 = 2 * b0 * (1 - cc);
			a2 = b0 * (1 - twodC + cc);
	
			out = SOS.ar( out, b0/a0, b1/a0, b2/a0, a1/a0.neg, a2/a0.neg, mul, add )
		});
		^out
	}
}


ZHPF {
	*ar { arg in, fc=440, damping=0.707, mul=1, add=0;

		var b0, b1, b2, a0, a1, a2;
		var c, cc, twodC;
		var out;
		
		out = in;
		
		damping.asArray.do({ arg df;
			
			c = (pi * fc/SampleRate.ir).tan;
			cc = c.squared;
			twodC = 2 * df * c;
	
			b0 = (1 + twodC + cc).reciprocal;
			b1 = -2 * b0;
			b2 = b0;
	
			a0 = 1;
			a1 = 2 * b0 * (cc - 1);
			a2 = b0 * (1 - twodC + cc);
	
			out = SOS.ar( out, b0/a0, b1/a0, b2/a0, a1/a0.neg, a2/a0.neg, mul, add )
		});
		^out
	}
}


BLPF {
	*ar { arg in, order=4, freq=440, mul=1, add=0;
		var df, out;
		order = order.asInteger;
		df = Dictionary[
			2->2.sqrt.reciprocal,
			4->[0.924, 0.383],
			6->[0.966, 0.707, 0.259],
			8->[0.981, 0.831, 0.556, 0.195],
			10->[0.988, 0.891, 0.707, 0.454, 0.156]
		].at(order);
		df.isNil.if({^(this.asString+"invalid order:" +order).error});
		out = (df.asArray.at(0) > 0).if({ZLPF.ar( in, freq, df)},{in});
		^out * mul + add
	
	}
}

BHPF {
	*ar { arg in, order=4, freq=440, mul=1, add=0;
		var df, out;
		order = order.asInteger;
		df = Dictionary[
			2->2.sqrt.reciprocal,
			4->[0.924, 0.383],
			6->[0.966, 0.707, 0.259],
			8->[0.981, 0.831, 0.556, 0.195],
			10->[0.988, 0.891, 0.707, 0.454, 0.156]
		].at(order);
		df.isNil.if({^(this.asString+"invalid order:" +order).error});
		out = (df.asArray.at(0) > 0).if({ZHPF.ar( in, freq, df)},{in});
		^out * mul + add
	
	}
}


LRLPF {
	*ar { arg in, order=4, freq=440, mul=1, add=0;
		var df, out;
		out = in;
		2.do({out = BLPF.ar( out, order/2, freq)})
		^out * mul + add
	}
}


LRHPF {
	*ar { arg in, order=4, freq=440, mul=1, add=0;
		var df, out;
		out = in;
		2.do({out = BHPF.ar( out, order/2, freq)});
		^out * mul + add
	
	}
}
