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
%   Prepares selected ParticleHP channel (e.g. \F01) data for the custom 
%   inverse-kinematics implementation by scaling incident energies by the 
%   specified mass ratio, reversing angular cosines, and updating selected 
%   target atomic-weight-ratio (AWR) strings.
%
%   Angular tables are interpreted as alternating mu and probability values,
%   where mu = cos(theta). This script performs mu -> -mu conversions as 
%   the reference axis reverses when swapping projectile and target.
%
% This script produces data intended for use with the Geant4 toolkit.
% Geant4 is developed by the Geant4 Collaboration:
% https://geant4.web.cern.ch/
%
% This file is distributed under the terms of the LICENSE file included
% with this repository.
% -------------------------------------------------------------------------


clear
close all

% Directory containing the ParticleHP channel file.
path = "/path/to/folder";

% Name of the input ParticleHP channel file.
inFile = "3_7_Lithium_F01.txt";

% Name of the converted output file.
outFile = 'test.txt';

% Geant4 nuclear masses
Li7 = 6533.83; 
proton = 938.272;
massRatio = Li7./proton;

cd(path);

% Identify numbers written in scientific notation, e.g. 1.234567e+06.
lines = splitlines(fileread(inFile));
pattern = '([-+]?\d*\.?\d+(?:[eE][+-]?\d+)?)';


normalProcessing = false; %flag for normal block processing (not angular dist blocks)
energyProbabilityCounter = 0;
insideAngularSliceBlock = false;
expectedAngleTokens = NaN;

N = numel(lines);
processedAngleLines = false(N,1);  % used so anglular distribution blocks are only processed once

for L = 1:N

    line = lines{L};

    % If this line was already rewritten as part of a full angle block, skip it
    if processedAngleLines(L)
        continue;
    end

    % ====== START/STOP READ BLOCKS ======
    % THe start of each new block begins with a "1"
    % If the next line starts with a "6", it is an angular distribution block
    % if the block hits a "1" and not "6", proceed with normal processing
    if strcmp(strtrim(line), "1")
        insideAngularSliceBlock = false;
        nextIs6 = false;

        % Check if the next line is a "6"
        if L < N
            nextIs6 = strcmp(strtrim(lines{L+1}), "6");
        end
        
        %enable/disable processing if the next line is a 6
        normalProcessing = ~nextIs6;
        continue;
    end
    

    if ~normalProcessing
        insideAngularSliceBlock = false;
        continue;
    end


    % ====== DETECT ENERGY SLICE HEADER LINE ======
    %     incidentEnergy   interpolationFlag   numberOfMuPoints
    %     Example:
    %        2e+07            12                  41
    % scientific-notation AND next line starts with "-1."
    isEnergySliceHeader = false;
    if L < N
        %Determine if the next line starts with -1.000000e+00
        nextLine = strtrim(lines{L+1});
        nextIsStartAngle = startsWith(nextLine, "-1.000000e+00");
        %Determine if the line contains a number with scientific notation
        thereIsSciNumber = ~isempty(regexp(strtrim(line), pattern, 'once'));
        isEnergySliceHeader = thereIsSciNumber && nextIsStartAngle;

    end


    % ====== ENERGY SLICE HEADER HANDLING ======
    % The third value of the header tells us exactly how many mu-probability pairs follow.
    if isEnergySliceHeader
        headerTokens = regexp(line, pattern, 'match');
        if numel(headerTokens) < 3
            error('Energy header on line %d does not contain enough values to read the angular point count.', L);
        end
        
        %read number of angular values
        numAngles = str2double(headerTokens{3});
        expectedAngleTokens = 2 * numAngles;

        % modify only first number using mass ratio
        [m, sIdx, eIdx] = regexp(line, pattern, 'match', 'start', 'end', 'once');
        if ~isempty(m)
            new = sprintf('%.6e', str2double(m) * massRatio);
            line = [line(1:sIdx-1) new line(eIdx+1:end)]; %form new line
        end
        lines{L} = line;

        % angular distribution block starts here
        insideAngularSliceBlock = true;
        continue;
    end

    % ====== ENERGY/ANGLE SLICE HANDLING ======
    if insideAngularSliceBlock

        sliceStart = L;
        sliceEnd = L;

        tokensPerLine = [];
        allTokens = {};

        % reads all of the tokens (cosines and probabilities) in the slice
        while sliceEnd <= N && numel(allTokens) < expectedAngleTokens
            toks = regexp(lines{sliceEnd}, pattern, 'match');
            tokensPerLine(end+1,1) = numel(toks); 
            allTokens = [allTokens toks]; 
            sliceEnd = sliceEnd + 1;
        end

        sliceEnd = sliceEnd - 1;

        if numel(allTokens) ~= expectedAngleTokens
            error('Expected %d angular values from line %d, but found %d by line %d.', ...
                  expectedAngleTokens, sliceStart, numel(allTokens), sliceEnd);
        end

        vals = str2double(allTokens);

        mu = vals(1:2:end);
        prob = vals(2:2:end);

        mu = -mu;  % inverse kinematics angular conversion

        % Reorder so slice still starts at mu = -1 for ParticleHP format
        [mu, order] = sort(mu, 'ascend');
        prob = prob(order);

        newVals = zeros(size(vals));
        newVals(1:2:end) = mu;
        newVals(2:2:end) = prob;

        % Write the values back using the same line token counts as the
        % original angular block.
        idx = 1;
        for b = sliceStart:sliceEnd
            nTok = tokensPerLine(b-sliceStart+1);

            if nTok == 0
                lines{b} = "";
                continue;
            end

            thisVals = newVals(idx:idx+nTok-1);
            idx = idx + nTok;

            lineOut = "";
            for q = 1:numel(thisVals)
                lineOut = lineOut + sprintf('%.6e \t', thisVals(q));
            end

            lines{b} = char(strtrim(lineOut));
        end

        processedAngleLines(sliceStart:sliceEnd) = true;
        insideAngularSliceBlock = false;
        expectedAngleTokens = NaN;
        continue;
    end

    % ====== Energy Angle Slice Processing: Scale incident energy ======
    [matches, startIdx, endIdx] = regexp(line, pattern, 'match', 'start', 'end');

    %skip metadata/header lines so they do not get scaled
    % Example metadata block:
    %   4.007000e+03   6.956651e+00   0   4
    %   -1.644239e+06  -1.644239e+06
    %   2
    %   1
    %   2  2

    % Skip lines containing only plain integers
    if ~isempty(matches)
        allPlainIntegers = true;
        for t = 1:numel(matches)
            tok = matches{t};
            if contains(lower(tok), 'e') || contains(tok, '.')
                allPlainIntegers = false;
                break;
            end
        end
        if allPlainIntegers
            continue; % leave line unchanged
        end
    end

    % Skip reaction header line: ZA, AWR, LIS, LISO
    looksLikeReactionHeader = numel(matches) == 4 && ...
        contains(lower(matches{1}), 'e') && ...
        contains(lower(matches{2}), 'e') && ...
        ~contains(matches{3}, '.') && ~contains(lower(matches{3}), 'e') && ...
        ~contains(matches{4}, '.') && ~contains(lower(matches{4}), 'e');

    if looksLikeReactionHeader
        continue; %next line
    end

    % Skip Q-value line immediately after the reaction header line.
    if L > 1
        prevTokens = regexp(lines{L-1}, pattern, 'match');
        prevLooksLikeReactionHeader = numel(prevTokens) == 4 && ...
            contains(lower(prevTokens{1}), 'e') && ...
            contains(lower(prevTokens{2}), 'e') && ...
            ~contains(prevTokens{3}, '.') && ~contains(lower(prevTokens{3}), 'e') && ...
            ~contains(prevTokens{4}, '.') && ~contains(lower(prevTokens{4}), 'e');

        looksLikeQValueLine = numel(matches) == 2 && ...
            contains(lower(matches{1}), 'e') && ...
            contains(lower(matches{2}), 'e');

        if prevLooksLikeReactionHeader && looksLikeQValueLine
            continue; %next line
        end
    end

    newLine = line;
    offset = 0;
    
    %count the number of energy-probability values on line
    for m = 1:length(matches)
        energyProbabilityCounter = energyProbabilityCounter + 1;
        old = matches{m};

         % never modify single-digit numbers 
        isSingleDigit = ~isempty(regexp(old, '^[-+]?\d$', 'once'));

        if isSingleDigit
            new = old;
        else
            if mod(energyProbabilityCounter, 2) == 1 % if odd numbered (energy)
                new = sprintf('%.6e', str2double(old) * massRatio); %multiply by mass ratio
            else
                new = old; %leave probabilities unchanges
            end
        end

        s = startIdx(m) + offset;
        e = endIdx(m) + offset;

        newLine = [newLine(1:s-1) new newLine(e+1:end)];
        offset  = offset + (length(new) - (e - s + 1));
    end

    lines{L} = newLine; %save the modified line
end


%
% ==== FINAL POST-PROCESSING REPLACEMENTS =====

% ------ User-defined replacements ------
targetZA = "3.007000e+03";      % original target: 7Li
residualZA = "4.007000e+03";    % residual product: 7Be

orig1 = "6.95573";              % decimal original target AWR
new1  = "0.998623";             % decimal replacement target proton AWR

orig2 = "6.956651e+00";         % sci-notation original target AWR
new2  = "9.986230e-01";         % sci-notation replacement target proton AWR

orig3 = "2e+07";                % find this
multFactor3 = massRatio;        % multiply by mass ratio
% ----------------------------------------

fprintf('\n--- POST-PROCESSING REPLACEMENTS ---\n');

% IMPORTANT:
% Only change the target AWR on the target ZA/AWR line:

for L = 1:N
    line = lines{L};
    tokens = regexp(line, pattern, 'match');

    % Replace the sci-notation AWR ONLY on the target ZA line.
    if numel(tokens) >= 2 && strcmp(tokens{1}, targetZA) && strcmp(tokens{2}, orig2)
        fprintf('Line %d target AWR: %s --> %s\n', L, orig2, new2);

        [m, sIdx, eIdx] = regexp(line, regexptranslate('escape', orig2), ...
                                 'match', 'start', 'end', 'once');
        if ~isempty(m)
            line = [line(1:sIdx-1) char(new2) line(eIdx+1:end)];
        end
    end

    % Replace decimal target AWR only on lines that are not residual-product headers.
    tokensAfterSci = regexp(line, pattern, 'match');
    isResidualHeader = numel(tokensAfterSci) >= 2 && strcmp(tokensAfterSci{1}, residualZA);
    if ~isResidualHeader && contains(line, orig1)
        fprintf('Line %d decimal target AWR: %s --> %s\n', L, orig1, new1);
        line = strrep(line, orig1, new1);
    end

    tokensAfter = regexp(line, pattern, 'match');
    if numel(tokensAfter) >= 2 && strcmp(tokensAfter{1}, residualZA) && strcmp(tokensAfter{2}, new2)
        fprintf('Line %d residual 7Be AWR restored: %s --> %s\n', L, new2, orig2);

        [m, sIdx, eIdx] = regexp(line, regexptranslate('escape', new2), ...
                                 'match', 'start', 'end', 'once');
        if ~isempty(m)
            line = [line(1:sIdx-1) char(orig2) line(eIdx+1:end)];
        end
    end

    lines{L} = line;
end



%% ====== WRITE OUTPUT ======
fid = fopen(outFile, "w");
for L = 1:N
    fprintf(fid, "%s\n", lines{L});
end
fclose(fid);
