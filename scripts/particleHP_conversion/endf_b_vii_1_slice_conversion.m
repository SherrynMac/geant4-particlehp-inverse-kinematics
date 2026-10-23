% -------------------------------------------------------------------------
% Copyright (c) 2026 Sherryn MacLeod
%
% This script is part of the Geant4 inverse-kinematics implementation.
%
% Author:
%   Sherryn MacLeod
%   University of Wollongong / ANSTO
%
% Last updated: 2026-10-15
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

Li7 = 6533.83; 
proton = 938.272;
massRatio = Li7./proton;

endf_legendre_list_to_particlehp( ...
    'legendre_coefficients_3_7_Lithium_proton_ENDFVII_1.txt', ...
    'particlehp_slices.txt', ...
    41, ...   % 41 pairs per slice
    12, ...   % tag12
    3, ...    % 3 pairs per output line
    false, ... % includeA0. false assumes isotropic term is 1/2
    massRatio);   % Scale factor


function endf_legendre_list_to_particlehp(inFile, outFile, nPairs, tag12, pairsPerLine, includeA0, EinScale)
% ============================================================
% Convert Legendre-coefficient slices to
% ParticleHP-style (mu,p(mu)) energy-angle slice blocks.
%
% Input ENDF pattern:
%   <Nslices> 2
%   0.000000+0  Ein  0 0  Ncoef  Ncoef   MAT MF MT line
%   <Ncoef coefficients>                      MAT MF MT line
%   ...
%
% Output ParticleHP:
%   Nslices  2
%   Ein(eV)  12  nPairs
%   mu1 p1 mu2 p2 ... (pairsPerLine pairs per line)
%   ...
% ============================================================

    lines = readlines(inFile);
    lines = strip(lines);

    slices = struct('Ein', {}, 'a', {});
    i = 1; nLines = numel(lines);

    while i <= nLines
        line = lines(i);

        if isSliceHeaderLine(line) % if the line is an energy slice header
            [Ein_eV, Ncoef] = parseSliceHeader(line);
            Ein_eV = EinScale * Ein_eV; %multiple by mass ratio

            coeffs = [];
            i = i + 1;
            while i <= nLines && numel(coeffs) < Ncoef
                dl = lines(i);
                
                %strip identifiers not relevant for this conversion
                dl = stripEndfTailText(dl);
                % Extract legendre coefficients
                vals = extractEndfFloatsFromLine(dl); 
                coeffs = [coeffs, vals]; %#ok<AGROW>

                i = i + 1;
            end

            if numel(coeffs) < Ncoef
                error('Ein=%.6e: expected %d coefficients, got %d.', Ein_eV, Ncoef, numel(coeffs));
            end

            coeffs = coeffs(1:Ncoef);

            % Flip the sign for odd Legendre coefficients for mu -> -mu
            % Because includeA0 = false, coeffs(1) corresponds to L = 1
            for ii = 1:length(coeffs)
                if mod(ii,2) == 1
                    coeffs(ii) = -coeffs(ii);
                end
            end
            
            %Store incident energy and coefficients
            slices(end+1).Ein = Ein_eV; %#ok<AGROW>
            slices(end).a = coeffs;
        else
            i = i + 1;
        end
    end

    Nslices = numel(slices);
    if Nslices == 0
        error('No slices found. Check the input format / header detection.');
    end

    %% ---- Write output ----
    fid = fopen(outFile, 'w');
    if fid < 0
        error('Could not open "%s" for writing.', outFile);
    end

    % ParticleHP header: number of slices, and 2 columns (E, probability)
    fprintf(fid, '\t%d\t%d\n\n', Nslices, 2);

    % mu grid (uniform)
    % generate evenly spaced mu values from -1 to +1 depending on nPairs
    mu = linspace(-1, 1, nPairs);

    % For each energy slice, write the header and pairs for particlehp
    for s = 1:Nslices
        Ein_eV = slices(s).Ein;
        a = slices(s).a;

        % Compute p(mu) from Legendre coefficients
        p = endf_pi_from_legendre(a, mu, includeA0);

        % Normalise so integral dmu = 1
        area = trapz(mu, p);
        if area == 0 || ~isfinite(area)
            error('Bad normalisation area for Ein=%.6e (area=%g).', Ein_eV, area);
        end
        p = p ./ area;

        % Write slice block
        fprintf(fid, '  %.6e\t%.0f\t%.0f\n', Ein_eV, tag12, nPairs);
        writePairs(fid, mu, p, pairsPerLine);
        %fprintf(fid, '\n');
    end

    fclose(fid);
    fprintf('Wrote %d slices to: %s\n', Nslices, outFile);
end



%% ======== Functions =========

% Determine if the line is an energy slice header
function tf = isSliceHeaderLine(line)
    if strlength(line) == 0
        tf = false; return;
    end

    toks = split(line);
    if numel(toks) < 6
        tf = false; return;
    end

    % Slice header begins with 0.0 and has "... 0 0 Ncoef Ncoef"
    try
        c1 = endf_str2double(char(toks(1)));
    catch
        tf = false; return; %return if not a number
    end
    if c1 ~= 0
        tf = false; return;
    end

    L1 = str2double(toks(3));
    L2 = str2double(toks(4));
    NR = str2double(toks(5));
    NP = str2double(toks(6));

    tf = isfinite(L1) && isfinite(L2) && isfinite(NR) && isfinite(NP) && (L1==0) && (L2==0) && (NR==NP) && (NP>0);
end

%read incident energy and number of legendre coefficients of header
function [Ein_eV, Ncoef] = parseSliceHeader(line)
    toks = split(line);
    Ein_eV = endf_str2double(char(toks(2)));
    Ncoef  = str2double(toks(6));
    if isnan(Ncoef) || Ncoef <= 0
        error('Could not parse Ncoef from header: %s', line);
    end
end

% If line ends with 4 pure-integer fields, strip them (MAT MF MT lineNo).
function lineOut = stripEndfTailText(lineIn)
    
    toks = split(lineIn);
    if numel(toks) < 5
        lineOut = lineIn; return;
    end

    tail = toks(end-3:end);
    if all(isPureIntegerToken(tail))
        toks = toks(1:end-4);
        lineOut = strjoin(toks, ' ');
    else
        lineOut = lineIn;
    end
end

function tf = isPureIntegerToken(tok)
    s = char(tok); s = strtrim(s);
    v = str2double(s);
    if isnan(v), tf = false; return; end
    if contains(s, ".") || contains(lower(s), "e"), tf = false; return; end
    if numel(s) > 1
        if any(s(2:end) == '+') || any(s(2:end) == '-')
            tf = false; return;
        end
    end
    tf = abs(v - round(v)) < 1e-12;
end

% Extract ENDF floats like "1.10420-13"
% Sometimes numbers like "1.10420-13-7.92540-14" are actually two numbers:
% 1.10420-13 
% -7.92540-14
function nums = extractEndfFloatsFromLine(line)
    pat = '[-+]?\d*\.\d+[-+]\d+';
    m = regexp(char(line), pat, 'match');
    nums = zeros(1, numel(m));
    for k = 1:numel(m)
        nums(k) = endf_str2double(m{k});
    end
end

function x = endf_str2double(s)
    s = strtrim(s);
    if contains(s, 'e') || contains(s, 'E')
        x = str2double(s);
        if isnan(x), error('Bad number: %s', s); end
        return;
    end

    plusPos  = find(s == '+');
    minusPos = find(s == '-');
    cand = [plusPos(plusPos > 1), minusPos(minusPos > 1)];

    if isempty(cand)
        x = str2double(s);
        if isnan(x), error('Bad ENDF float: %s', s); end
        return;
    end

    k = max(cand);
    mant = s(1:k-1);
    expo = s(k:end);

    x = str2double([mant 'e' expo]);
    if isnan(x)
        error('Bad ENDF float: %s', s);
    end
end

% Convert legendre coefficients to angular probability distribution
function p = endf_pi_from_legendre(a, mu, includeA0)
    if any(mu < -1-1e-12) || any(mu > 1+1e-12)
        error('mu must lie in [-1,1].');
    end
    
    % Determine whether the input coefficients include a0.
    %l0 = starting legendre order
    %NL = highest order
    if includeA0
        l0 = 0;
        NL = numel(a) - 1;
        idx = @(l) l+1;
        p = zeros(size(mu));
    else % include isotropic contribution
        l0 = 1;
        NL = numel(a);
        idx = @(l) l;
        p = 0.5 * ones(size(mu));
    end
    
    %initialise P0(mu) = 1 and P1(mu) = mu (first two legendre polynomials)
    %P values are the legendre polynomials
    P0 = ones(size(mu));
    if NL >= 1
        P1 = mu;
    else
        P1 = zeros(size(mu));
    end
    
    %reconstruct p(mu) by summing the legendre polynomials.
    for l = l0:NL
        if l == 0
            Pl = P0;
        elseif l == 1
            Pl = P1;
        else %recurrence relation for calculating polynomials
            Pl = ((2*l - 1) .* mu .* P1 - (l - 1) .* P0) ./ l;
            P0 = P1;
            P1 = Pl;
        end
       
        % legendre coefficients as in the file for each incident energy
        coeff = a(idx(l));
        %angular probability distribution calculation
        p = p + 0.5*(2*l + 1) * coeff .* Pl;
    end
end

% writes each individual energy probability slice
function writePairs(fid, mu, p, pairsPerLine)
    nPairs = numel(mu);
    k = 1;
    while k <= nPairs
        thisLine = min(pairsPerLine, nPairs - k + 1);
        for j = 1:thisLine
            fprintf(fid, '% .6e  % .6e', mu(k), p(k));
            if j < thisLine
                fprintf(fid, '  ');
            end
            k = k + 1;
        end
        fprintf(fid, '\n');
    end
end
