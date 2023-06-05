function FindPSSTimeDomain(rxWaveform,rxSampleRate,fshift) 
    scs = 15;    
    syncNfft = 256; % minimum FFT size to cover SS burst
    syncSR = syncNfft*scs*1e3;
    
    % PSS subcarrier locations in a 240 subcarrier (SSB) grid
    subsPSS = nrPSSIndices('IndexStyle','subscript');
    kPSS = subsPSS(:,1);
            
    % Frequency offset and PSS search
    t = (0:size(rxWaveform,1)-1).' / rxSampleRate;
    rxWaveformFreqCorrected = rxWaveform .* exp(-1i*2*pi*fshift*t);
    
    % Downsample to the minumum sampling rate to cover SSB bandwidth
    rxWaveformDS = resample(rxWaveformFreqCorrected,syncSR,rxSampleRate);

    spectrogram(rxWaveformDS(:,1),ones(syncNfft,1),0,syncNfft,'centered',syncSR,'yaxis','MinThreshold',-130);
    title('Spectrogram of the downsampled waveform')

    correlations = zeros([3,2*size(rxWaveformDS, 1) - 1]);
    for NID2 = [0 1 2]
        pss_chunk = zeros([256,1]);
        pss_chunk(65:65+126) = nrPSS(NID2);
        pss_chunk = ifft(ifftshift(pss_chunk));
        r = xcorr(rxWaveformDS, pss_chunk);
        correlations(NID2+1,:) = abs(r);
    end

    [max_corr, lags] = max(correlations.');
    [max_nid, nid] = max(max_corr);
    lag = lags(nid) - size(rxWaveformDS,1);
    nid = nid - 1;
    disp([' NID: ' num2str(nid) '']);

    figure;
    hold on;
    plot(correlations(1,:));
    plot(correlations(2,:));
    plot(correlations(3,:));
    title(['Correlations']);

    figure;
    spectrogram(rxWaveformDS(lag:end,1),ones(syncNfft,1),0,syncNfft,'centered',syncSR,'yaxis','MinThreshold',-130);
end