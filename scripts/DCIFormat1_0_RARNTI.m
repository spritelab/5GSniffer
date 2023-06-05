classdef DCIFormat1_0_RARNTI < MessageFormat
    
    properties
        FrequencyDomainResources = BitField();
        TimeDomainResources = BitField(4);
        VRBToPRBMapping = BitField(1);
        ModulationCoding = BitField(5);
        TBScaling = BitField(2);
        LSBsOfSFN = BitField();
        ReservedBits = BitField(16); 
    end
    
    methods
       
        function obj = DCIFormat1_0_RARNTI(nsizebwp,sharedspectrum)
            % Set FrequencyDomainResources
            N = ceil(log2(nsizebwp*(nsizebwp+1)/2));
            obj.FrequencyDomainResources = BitField(N);
            
            % If operation in a Release 16 cell with shared spectrum
            % channel access then adjust the field size
            if nargin>1 && sharedspectrum
                obj.ReservedBits = BitField(18);
                obj.LSBsOfSFN = BitField(2);
            end

        end
    
    end
    
end
