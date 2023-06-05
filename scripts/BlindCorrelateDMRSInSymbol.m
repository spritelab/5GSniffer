function [pdcch, max_corr] = BlindCorrelateDMRSInSymbol(symbol, carrier, varargin) 
    narginchk(2,3);
    if nargin == 2
        disp(['Brute-forcing PDCCH DM-RS configuration']);        
        % Specify CORESET
        coreset = nrCORESETConfig;
        coreset.FrequencyResources = ones(1,8);  % Each "1" represents 6 RBs starting from StartBWP
        coreset.Duration = 1;  % Affects correlation
        coreset.CCEREGMapping = 'interleaved'; % Affects correlation
        coreset.REGBundleSize = 6;  % Affects correlation? TODO test
        coreset.ShiftIndex = 0; % Affects correlation

        pdcch = nrPDCCHConfig;
        pdcch.CORESET = coreset;
        pdcch.NStartBWP = 0;
        pdcch.NSizeBWP = carrier.NSizeGrid;
        pdcch.AggregationLevel = 4;  % Affects correlation

        %range = 0:1:65535;
        %range = 0:1:1800;  % Tmobile UE-specific and SI
        range = 1222:1:1222;  % Tmobile UE-specific
        %range = 210:1:218; % Tmobile SI (214)
        correlations = size(range,1);
        count = 1;
        for guess = range
            disp(['Trying scrambling ID ' num2str(guess)]);
            pdcch.DMRSScramblingID = guess;
            corr = CorrelateDMRS(symbol, carrier, pdcch);
            %disp(['Corr: ' num2str(corr)]);
            correlations(count) = corr;
            count = count + 1;
        end
        [max_corr, argmax_corr] = max(correlations);
        scrambling_id = range(argmax_corr);
        pdcch.DMRSScramblingID = scrambling_id;
        disp(['Best guess: scrambling ID is ' num2str(scrambling_id)]);
        disp(['Correlation for this ID is ' num2str(max_corr) ' while mean correlation is ' num2str(mean(correlations))]);
        figure; plot(range, correlations);
    else
        pdcch = varargin{1};
        disp(['Using given PDCCH and carrier config']);

        max_corr = CorrelateDMRS(symbol, carrier, pdcch);
        disp(['Corr: ' num2str(max_corr)]);
    end
end


function best_corr = CorrelateDMRS(symbol, carrier, pdcch)
    [~,pdcchDmrsSymbols,pdcchDmrsIndices] = nrPDCCHResources(carrier,pdcch);
    dmrsGrid = nrResourceGrid(carrier, 1);
    dmrsGrid(pdcchDmrsIndices) = pdcchDmrsSymbols;
    oneSymbolDmrs = dmrsGrid(:,1);

    [r,lags] = xcorr(symbol, oneSymbolDmrs);
    correlations = abs(r);
    % figure; stem(lags,correlations);
    [best_corr, best_lag] = max(correlations);

    %disp([' best_lag: ' num2str(best_lag) '']);
end
