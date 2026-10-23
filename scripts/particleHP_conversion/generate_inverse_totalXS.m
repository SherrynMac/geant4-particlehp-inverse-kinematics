% -------------------------------------------------------------------------
% Copyright (c) 2026 Sherryn MacLeod
%
% This script is part of the Geant4 inverse-kinematics implementation.
%
% Author:
%   Sherryn MacLeod
%   University of Wollongong / ANSTO
%
% Last updated: 2026-09-25
%
% Purpose:
%   Converts a Geant4 ParticleHP total cross-section (\CrossSection) file 
%   for use in inverse-kinematics calculations. Incident-energy values are 
%   scaled by the specified projectile-to-target mass ratio, while the 
%   corresponding cross-section values are preserved.
%
% This software produces data intended for use with the
% Geant4 toolkit. Geant4 is developed by the Geant4 Collaboration:
% https://geant4.web.cern.ch/
%
% This file is distributed under the terms of the LICENSE file included
% with this repository.
% -------------------------------------------------------------------------


clear
close all

% Directory containing the ParticleHP cross-section file.
path = "/path/to/folder";

% Name of the input ParticleHP cross-section file.
inFile = "3_7_Lithium_totalXS.txt";

% Name of the converted output file.
outFile = "name_of_output_file.txt";

% Geant4 nuclear masses
Li7 = 6533.83; 
proton = 938.272; 
massRatio = Li7./proton;

cd(path); % Change directory to the specified path

% Identify numbers written in scientific notation, e.g. 1.234567e+06.
lines = splitlines(fileread(inFile));
pattern = '([-+]?\d+\.\d+e[+-]\d+)';

oddEvenCounter = 0;

N = numel(lines);

for L = 1:N

    line = lines{L};

    % find all sci-notation numbers on this line
    [matches, startIdx, endIdx] = regexp(line, pattern, 'match', 'start', 'end');

    if isempty(matches)
        continue; % header / blank / anything without sci-notation numbers
    end

    newLine = line;
    offset = 0; % Needed if output numbers differ from the '%.6e' format

    for m = 1:length(matches)
        oddEvenCounter = oddEvenCounter + 1;

        old = matches{m};

        if mod(oddEvenCounter, 2) == 1
            % odd numbers are energy values
            % convert energy using mass ratio
            new = sprintf('%.6e', str2double(old) * massRatio);
        else
            % even numbers are cross section values
            new = old;
        end

        s = startIdx(m) + offset;
        e = endIdx(m)   + offset;

        newLine = [newLine(1:s-1) new newLine(e+1:end)];
        offset  = offset + (length(new) - (e - s + 1))
    end

    lines{L} = newLine;
end

% write output
fid = fopen(outFile, "w");
for L = 1:N
    fprintf(fid, "%s\n", lines{L});
end
fclose(fid);

fprintf("Done. Wrote: %s\n", outFile);
