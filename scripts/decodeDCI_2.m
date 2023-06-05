pdcch = nrPDCCHConfig;
pdcch.DMRSScramblingID = 1008 + 429;
pdcch.AggregationLevel = 1;
pdcch.SearchSpace.NumCandidates = [8 4 2 1 0];
pdcch.CORESET.Duration = 1;
pdcch.CORESET.CCEREGMapping = 'noninterleaved';

c0Carrier = nrCarrierConfig;
c0Carrier.NSizeGrid = 48;
c0Carrier.NCellID = 429;
% for scr_id = 0:2000
% pdcch.DMRSScramblingID = scr_id;
% [pdcchInd,pdcchDmrsSym,pdcchDmrsInd] = nrPDCCHSpace(c0Carrier,pdcch);
% refGrid = zeros(52*12,1);
% refGrid(pdcchDmrsInd{4}(:,1)) = pdcchDmrsSym{4}(:,1);
% y1 = rxSlotGrid(:,1);
% y2 = refGrid;
% corr_val(scr_id+1) = real(sum(y1 .* conj(y2)))/(norm(y1) * norm(y2));
% end

% AL = 1;
% corr_val = 0;
% for scr_id = 0:2000
% pdcch.DMRSScramblingID = scr_id;
% [pdcchInd,pdcchDmrsSym,pdcchDmrsInd] = nrPDCCHSpace(c0Carrier,pdcch);
% y1 = rxSlotGrid(:,1);
% y1 = y1(pdcchDmrsInd{AL}(:,1));
% y2 = pdcchDmrsSym{AL}(:,1);
% corr_val(scr_id+1) = real(sum(y1 .* conj(y2)))/(norm(y1) * norm(y2));
% end

% Get PDCCH candidates according to TS 38.213 Section 10.1
    [pdcchInd,pdcchDmrsSym,pdcchDmrsInd] = nrPDCCHSpace(c0Carrier,pdcch);
    slot0 = gr_to_matlab("../5gsniffer/test/samples/PDCCH_4thSlot_as_SSB_FirstOFDMSymbol_Real_data_CID429_goodSNR_2_wholeSlot.fc32");
    rxSlotGrid = reshape(slot0, 624 , 14);
    RNTI = 1625;
    dci = DCIFormat1_0_CRNTI(pdcch.NSizeBWP);
    dciCRC = true;
    mSlot = 0;
%     rxSlotGrid = rxSlotGrid/max(abs(rxSlotGrid(:))); % Normalization of received RE magnitude
    for RNTI = 0:65535
    % Loop over all supported aggregation levels
    aLev = 1;
    
    while (aLev <= 5) && dciCRC ~= 0
        % Loop over all candidates at each aggregation level in SS
        cIdx = 1;
        numCandidatesAL = pdcch.SearchSpace.NumCandidates(aLev);
        while (cIdx <= numCandidatesAL) && dciCRC ~= 0
            % Channel estimation using PDCCH DM-RS
            [hest,nVar,pdcchHestInfo] = nrChannelEstimate(rxSlotGrid,pdcchDmrsInd{aLev}(:,cIdx),pdcchDmrsSym{aLev}(:,cIdx));

            % Equalization and demodulation of PDCCH symbols
            [pdcchRxSym,pdcchHest] = nrExtractResources(pdcchInd{aLev}(:,cIdx),rxSlotGrid,hest);
            pdcchEqSym = nrEqualizeMMSE(pdcchRxSym,pdcchHest,nVar);
            dcicw = nrPDCCHDecode(pdcchEqSym,pdcch.DMRSScramblingID,RNTI,nVar);

            % DCI message decoding
            polarListLength = 8;
            [dcibits,dciCRC] = nrDCIDecode(dcicw,dci.Width,polarListLength,RNTI);

            if dciCRC == 0
                disp([' Decoded PDCCH candidate #' num2str(cIdx) ' at aggregation level ' num2str(2^(aLev-1)) ' RNTI ' num2str(RNTI), ' cIdx ', num2str(cIdx)])
                dciCRC = 10;
            end
            cIdx = cIdx + 1;
        end
        aLev = aLev+1;
    end
%     mSlot = mSlot+1;

    end