%matlab_to_cf32("../matlab_waveform_tests/", "tmobile_similar");

function matlab_to_gr(path,name)
    wave_struct_path = sprintf("%s%s_signal.mat", path, name);
    json_path = sprintf("%s%s_signal.json", path, name);
    output_path = sprintf("%s%s_signal.fc32", path, name);
    load(wave_struct_path);
    
    f = fopen(json_path, "w");
    waveStruct.config.Fs = waveStruct.Fs;
    encoded_json = jsonencode(waveStruct.config);
    fprintf(f, encoded_json);
    fclose(f);
    
    f = fopen(output_path, "wb");
    cols = [real(waveStruct.waveform), imag(waveStruct.waveform)];
    interleaved = reshape(cols.',[],1);
    fwrite(f, interleaved, 'float');
    fclose(f);
end

