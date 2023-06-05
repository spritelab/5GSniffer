% Configure carrier
carrier = nrCarrierConfig;

% Configure a CORESET
coreset = nrCORESETConfig;
coreset.FrequencyResources = ones(1,6);
coreset.Duration = 2;
coreset.CCEREGMapping = 'interleaved';
coreset.REGBundleSize = 6;
coreset.InterleaverSize = 3;
coreset.ShiftIndex = 0;

% Configure a search space
searchspace = nrSearchSpaceConfig;
searchspace.NumCandidates = [0 5 3 8 0];

% PDCCH
pdcch = nrPDCCHConfig;
pdcch.NStartBWP = 0;
pdcch.NSizeBWP = 48;
pdcch.CORESET = coreset;
pdcch.SearchSpace = searchspace;
pdcch.AggregationLevel = 8;
pdcch.AllocatedCandidate = 8;
pdcch.DMRSScramblingID = 1;  % Assigned to device in RRC message

% Get DMRS indices using search space
% [~,~,allDMRSInd] = nrPDCCHSpace(carrier,pdcch);

% Generate a random codeword
dciCW = randi([0 1],864,1);

% Generate PDCCH symbols
pdcchSymbols = nrPDCCH(dciCW, pdcch.DMRSScramblingID, pdcch.RNTI);

% Get all PDCCH indices and DMRS
[pdcchIndices,dmrsSymbols,dmrsIndices] = nrPDCCHResources(carrier,pdcch);

% Generate resource grid
nAnts = 1;
resourceGrid = nrResourceGrid(carrier,nAnts);

% Assign symbols to resource grid
dmrsGain = 1.1;  % Make DMRS stand out more
resourceGrid(pdcchIndices) = pdcchSymbols;
resourceGrid(dmrsIndices) = dmrsSymbols*dmrsGain;

% Plot the grid
figure;
set(gcf,'color','w');

for i = 1:nAnts
    subplot(1,nAnts,i)
    hold on;
    imagesc(abs(resourceGrid(:,:,i)));
    
    % Symbol lines
    for j = 2:14
        line([j-0.5 j-0.5],[0 273*12],'Color','white');
    end

    % Subcarrier lines
    %for k = 1:(carrier.NSizeGrid*12)
        %line([0 15],[k+0.5 k+0.5],'Color','white');
    %end

    hold off;
    axis xy;
    box on;

    xlabel('OFDM Symbols');
    ylabel('Subcarriers');

    xlim([0.5 carrier.SymbolsPerSlot+0.5]);
    ylim([0.5 (carrier.NSizeGrid*12)+0.5]);

    set(gca, 'xtick', [0:14]);
    set(gca, 'xticklabel', {'','0','1','2','3','4','5','6','7','8','9','10','11','12','13'});
end