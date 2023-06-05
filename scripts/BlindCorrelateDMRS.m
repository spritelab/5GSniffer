function BlindCorrelateDMRS(rxWaveform, rxSampleRate, carrier, varargin) 
    narginchk(3,4);
    if nargin == 3
        disp(['Brute-forcing PDCCH DM-RS configuration']);
        % Specify CORESET
        coreset = nrCORESETConfig;
        coreset.FrequencyResources = ones(1,4);  % Each "1" represents 6 RBs starting from StartBWP
        coreset.Duration = 2;  % Affects correlation
        coreset.REGBundleSize = 6;  % Does not affect correlation

        pdcch = nrPDCCHConfig;
        pdcch.CORESET = coreset;
        pdcch.NStartBWP = 0;
        pdcch.NSizeBWP = carrier.NSizeGrid;
        pdcch.AggregationLevel = 8;  % Affects correlation (strongly)

        range = 0:1:1024;
        correlations = size(range,1);
        count = 1;
        for guess = range
            disp(['Trying scrambling ID ' num2str(guess)]);
            pdcch.DMRSScramblingID = guess;
            corr = CorrelateDMRS(rxWaveform, rxSampleRate, carrier, pdcch);
            disp(['Corr: ' num2str(corr)]);
            correlations(count) = corr;
            count = count + 1;
        end
        [max_corr, argmax_corr] = max(correlations);
        disp(['Best guess: scrambling ID is ' num2str(range(argmax_corr))]);
        disp(['Correlation for this ID is ' num2str(max_corr) ' while mean correlation is ' num2str(mean(correlations))]);
        figure; plot(range, correlations);
    else
        pdcch = varargin{1};
        disp(['Using given PDCCH and carrier config']);

        corr = CorrelateDMRS(rxWaveform, rxSampleRate, carrier, pdcch);
        disp(['Corr: ' num2str(corr)]);
    end
end


function best_corr = CorrelateDMRS(rxWaveform, rxSampleRate, carrier, pdcch)
    [~,pdcchDmrsSymbols,pdcchDmrsIndices] = nrPDCCHResources(carrier,pdcch);
    dmrsGrid = nrResourceGrid(carrier, 1);
    dmrsGrid(pdcchDmrsIndices) = pdcchDmrsSymbols;

    scs = 15;    
    syncNfft = 512; % minimum FFT size to cover grid is 512 but we can use smaller FFT because DM-RS is unique per 31 bits (because the scrambling sequence is unique per 31 bits)
    syncSR = syncNfft*scs*1e3;
             
    % Frequency offset
    fshift = 0;
    t = (0:size(rxWaveform,1)-1).' / rxSampleRate;
    rxWaveformFreqCorrected = rxWaveform .* exp(-1i*2*pi*fshift*t);
    
    % Downsample to the minimum sampling rate to cover resource grid
    rxWaveformDS = resample(rxWaveformFreqCorrected,syncSR,rxSampleRate);

    [stftWaveform, frequencies, times] = stft(rxWaveformDS,syncSR,'Window',hann(syncNfft),'OverlapLength',syncNfft-1,'FFTLength',syncNfft);
    %drawstft(stftWaveform, frequencies, times);

    best_lag = 0;
    best_corr = 0;
    best_freq_shift = 0;

    r = xcorr2(stftWaveform, dmrsGrid);
    %drawcorr(r(1:size(stftWaveform, 1),1:size(stftWaveform, 2)), frequencies, times);
    correlations = abs(r);
    [best_freq_corrs, best_freq_corrs_argmax] = max(correlations(:,:));
    best_freq_corrs = best_freq_corrs(:);
    % figure; plot(best_freq_corrs);
    [best_corr, best_lag] = max(best_freq_corrs);
    best_freq_shift = best_freq_corrs_argmax(best_lag); % Get the frequency offset at the location of the best correlation

    disp([' best_freq_shift: ' num2str(best_freq_shift) '']);
    disp([' best_lag: ' num2str(best_lag) '']);

    %drawstft(stftWaveform(:,best_lag:end), frequencies, times(best_lag:end));
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