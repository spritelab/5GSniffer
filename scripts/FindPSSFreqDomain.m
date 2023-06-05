function FindPSSFreqDomain(rxWaveform,rxSampleRate,fshift) 
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

    [stftWaveform, frequencies, times] = stft(rxWaveformDS,syncSR,'Window',hann(syncNfft),'OverlapLength',syncNfft-2,'FFTLength',syncNfft);
    %drawstft(stftWaveform, frequencies, times);

    correlations = zeros([3,(syncNfft + 256) - 1,size(stftWaveform, 2)]);
    best_lags = zeros(3,1);
    best_corrs = zeros(3,1);
    best_freq_shifts = zeros(3,1);
    figure;
    hold on;
    for NID2 = [0 1 2]
        pss_chunk = zeros([256,1]);
        pss_chunk(60:60+126) = nrPSS(NID2);
        pss_chunk = complex(pss_chunk);
        r = xcorr2(stftWaveform, pss_chunk);
        %drawcorr(r, frequencies, times);
        correlations(NID2+1,:,:) = abs(r);
        [best_freq_corrs, best_freq_corrs_argmax] = max(correlations(NID2+1,:,:));
        best_freq_corrs = best_freq_corrs(:);
        plot(best_freq_corrs);
        [best_corr, best_corr_argmax] = max(best_freq_corrs);
        best_lags(NID2+1) = best_corr_argmax;
        best_corrs(NID2+1) = best_corr;
        best_freq_shifts(NID2+1) = best_freq_corrs_argmax(best_corr_argmax); % Get the frequency offset at the location of the best correlation
    end

    
    [max_nid, nid] = max(best_corrs);
    lag = best_lags(nid);
    nid = nid - 1;
    disp([' NID: ' num2str(nid) '']);

    drawstft(stftWaveform(:,lag:end), frequencies, times(lag:end));
end

function drawstft(stftWaveform, frequencies, times) 
    stftWaveformMagnitudes = mag2db(abs(stftWaveform));

    figure;
    pcolor(seconds(times), frequencies, stftWaveformMagnitudes);
    xlabel('Time (s)');
    ylabel('Frequency (Hz)');
    title('Spectrogram');
    shading flat;
    colorbar;
    caxis(max(stftWaveformMagnitudes(:)) + [-60 0]);
end

function drawcorr(corr, frequencies, times) 
    corr = abs(corr);

    figure;
    pcolor(seconds(times), [1:size(corr, 1)], corr);
    xlabel('Time (s)');
    ylabel('Frequency (Hz)');
    title('Correlations');
    shading flat;
    colorbar;
    caxis([0 max(corr(:))]);
end