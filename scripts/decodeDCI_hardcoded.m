pdcch = nrPDCCHConfig;
pdcch.DMRSScramblingID = 1008 + 429;
pdcch.AggregationLevel = 1;
pdcch.SearchSpace.NumCandidates = [8 4 2 1 0];
pdcch.CORESET.Duration = 1;
% pdcch.CORESET.CCEREGMapping = 'noninterleaved';

c0Carrier = nrCarrierConfig;
c0Carrier.NSizeGrid = 48;
c0Carrier.NCellID = 429;
% Get PDCCH candidates according to TS 38.213 Section 10.1
    [pdcchInd,pdcchDmrsSym,pdcchDmrsInd] = nrPDCCHSpace(c0Carrier,pdcch);
%     slot0 = gr_to_matlab("../5gsniffer/test/samples/slot0.fc32");
    slot0 = gr_to_matlab("../5gsniffer/test/samples/PDCCH_SameSlot_as_SSB_FirstOFDMSymbol_Real_data_CID429_goodSNR_2_wholeSlot.fc32");
    rxSlotGrid = reshape(slot0, 624 , 14);
%     RNTI = 1625;
    dci = DCIFormat1_0_CRNTI(pdcch.NSizeBWP);
    dciCRC = true;
    mSlot = 0;

    for dciSize = 39:41
%     rxSlotGrid = rxSlotGrid/max(abs(rxSlotGrid(:))); % Normalization of received RE magnitude
    for RNTI = 65535
    % Loop over all supported aggregation levels
    aLev = 1;
    
%     while (aLev <= 5) && dciCRC ~= 0
        % Loop over all candidates at each aggregation level in SS
        cIdx = 5;
        numCandidatesAL = pdcch.SearchSpace.NumCandidates(aLev);
%         while (cIdx <= numCandidatesAL) && dciCRC ~= 0
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
                disp([' Decoded PDCCH candidate #' num2str(cIdx) ' at aggregation level ' num2str(2^(aLev-1)) ' RNTI ' num2str(RNTI), ' cIdx ', num2str(cIdx), 'dci size', num2str(dciSize)])
                dciCRC = 10;
            end
%             cIdx = cIdx + 1;
%         end
%         aLev = aLev+1;
%     end
%     mSlot = mSlot+1;

    end
    disp([' DCI size ' num2str(dciSize)]);
    end