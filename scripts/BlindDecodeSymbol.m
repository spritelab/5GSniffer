function dci = BlindDecodeSymbol(rxSlotGrid, carrier, pdcch, varargin) 
    narginchk(3,4);

    % Determine PDCCH indices
    %[pdcchIndices,~,~] = nrPDCCHResources(carrier,pdcch);

    % Extract PDCCH symbols
    %pdcchSymbols = symbol(pdcchIndices);

    % We need to manually specify a search space for now
    searchspace = nrSearchSpaceConfig;
    searchspace.NumCandidates = [8 4 2 1 0];
    pdcch.SearchSpace = searchspace;

    % Get PDCCH candidates according to TS 38.213 Section 10.1
    [pdcchInd,pdcchDmrsSym,pdcchDmrsInd] = nrPDCCHSpace(carrier, pdcch);
    rxSlotGrid = rxSlotGrid/max(abs(rxSlotGrid(:))); % Normalization of received RE magnitude
    
    dciFormat = DCIFormat1_0_CRNTI(carrier.NSizeGrid); % 52 also works for some reason for symbol 0, TODO find out why
    %dciFormat = DCIFormat1_0_RARNTI(carrier.NSizeGrid);
    %dciFormat = DCIFormat1_0_SIRNTI(carrier.NSizeGrid);
    % Loop over all possible RNTIs
    dci = [];
    for guess = 0:1:65535  
    %for guess = [1625,]
        disp(['Trying C-RNTI ' num2str(guess)]);
        % Loop over all supported aggregation levels
        aLev = 1;
        dciCRC = true;
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
                dcicw = nrPDCCHDecode(pdcchEqSym,pdcch.DMRSScramblingID, guess, nVar);
                %dcicw = nrPDCCHDecode(pdcchEqSym,pdcch.DMRSScramblingID, 0, nVar); %SI
    
                % DCI message decoding
                polarListLength = 8;
                %for size = 12:1:83
                    %[dcibits,dciCRC] = nrDCIDecode(dcicw, size, polarListLength, guess);
                    [dcibits,dciCRC] = nrDCIDecode(dcicw, dciFormat.Width, polarListLength, guess);
                    %[dcibits,dciCRC] = nrDCIDecode(dcicw, dciFormat.Width, polarListLength, 65535); %SI
        
                    if dciCRC == 0
                        disp([' Decoded PDCCH candidate #' num2str(cIdx) ' at aggregation level ' num2str(2^(aLev-1))])
                        disp([dec2hex(bin2dec(num2str(dcibits).'))])
                        dci = fromBits(dciFormat,dcibits);
                        dciCRC = true;
                    end
                %end
                cIdx = cIdx + 1;
            end
            aLev = aLev+1;
        end
    end
end