function FindSIDMRSInRxGrid(rxGrid, carrier)
    % Specify CORESET and PDCCH
    coreset = nrCORESETConfig;
    coreset.FrequencyResources = ones(1,8);
    coreset.Duration = 1;
    coreset.CCEREGMapping = 'interleaved';
    coreset.REGBundleSize = 6;
    coreset.ShiftIndex = 0;

    pdcch = nrPDCCHConfig;
    pdcch.CORESET = coreset;
    pdcch.NStartBWP = 0;
    pdcch.NSizeBWP = carrier.NSizeGrid;
    pdcch.AggregationLevel = 8;
    pdcch.DMRSScramblingID = carrier.NCellID;

    correlations = zeros(size(rxGrid, 2));
    for symbol_index = 1:1:size(rxGrid,2)
        disp(['Symbol ' num2str(symbol_index)]);
        symbol = rxGrid(:, symbol_index);
        [~,max_corr] = BlindCorrelateDMRSInSymbol(symbol, carrier, pdcch);
        correlations(symbol_index) = max_corr;
    end

    figure; plot(correlations); title(['Slot ' num2str(carrier.NSlot)]);
end