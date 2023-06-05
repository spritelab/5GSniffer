function complex_values = gr_to_matlab(path, samp_rate, outpath) 
    narginchk(1,3);
    
    f = fopen(path, "rb");
    values = fread(f, [2, Inf], 'float');
    complex_values = values(1,:) + values(2,:) * 1i;
    complex_values = complex_values.';
    
    if nargin > 1
        rx = struct;
        rx.L_max = 8;
        rx.fPhaseComp = 0;
        rx.minChannelBW = 5;
        rx.sampleRate = samp_rate;
        rx.ssbBlockPattern = 'Case A';
        rx.waveform = complex_values;
        save(outpath, "-struct", "rx");
    end
end