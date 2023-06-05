classdef DCIFormat1_0_CRNTI < MessageFormat
    
    properties
        DCIFormatIdentifier = BitField(1);
        FrequencyDomainResources = BitField();

        % If FrequencyDomainResources is all ones:
        %RandomAccessPreambleIndex = BitField(6);
        %ULSULIndicator = BitField(1);
        %SSPBCHIndex = BitField(6);
        %PRACHMaskIndex = BitField(6);
        %ReservedBits = BitField(12);

        % Otherwise:
        TimeDomainResources = BitField(4);
        VRBToPRBMapping = BitField(1);
        ModulationCoding = BitField(5);
        NewDataIndicator = BitField(1);
        RedundancyVersion = BitField(2);
        HARQProcessNumber = BitField(4);
        DownlinkAssignmentIndex = BitField(2);
        TPCForPUCCH = BitField(2);
        PUCCHResourceIndicator = BitField(3);
        PDSCHToHARQFeedbackTimingIndicator = BitField(3);
        ChannelAccessCPext = BitField(); 

    end
    
    methods
       
        function obj = DCIFormat1_0_CRNTI(nsizebwp,sharedspectrum)
            % Set FrequencyDomainResources
            N = ceil(log2(nsizebwp*(nsizebwp+1)/2));
            obj.FrequencyDomainResources = BitField(N);
            
            % If operation in a Release 16 cell with shared spectrum
            % channel access then adjust the field size
            if nargin>1 && sharedspectrum
                % 2 bits if ChannelAccessMode-r16 = "semistatic" is provided, for operation in a cell with shared spectrum channel access;
                obj.ChannelAccessCPext = BitField(2);
            end

        end
    
    end
    
end
