clear
close all

%% User settings

path = "/path/to/thin_target_results/folder";
cd(path)

% Convert root file to hdf5 to be read
script = '/path/to/root_to_hdf5.py';
tree = "ParticleScoring";

% Add simulation ROOT files
sim_files = [
    sprintf("%s/output.root", path)
];

% Incident inverse-kinematics beam energy for each ROOT file in MeV.
sim_energies = [
    15
];

sim_labels = string(sim_energies) + " MeV"; %plot labels
theory_colour = "black"; %final plot colour

output_files = "output_" + matlab.lang.makeValidName(sim_labels) + ".h5";

% -------------------------------------------------------------------------
% Reaction used for the calculated theory curve:
%   7Li + p -> n + 7Be
% -------------------------------------------------------------------------
u_to_MeV = 931.49410242;
theory_mass_projectile_u = 7.0160034366;    % 7Li atomic mass
theory_mass_target_u     = 1.00782503223;   % 1H atomic mass
theory_mass_ejectile_u   = 1.00866491595;   % neutron mass
theory_mass_recoil_u     = 7.01692983;      % 7Be atomic mass

%Number of CM emission angles sampled to calculate the relativisitc theory curve
theory_n_angles = 5000;

% Fit / interpolation settings
n = 2000;
line = 3.5;

energy_turn   = []; %[1.4383];  % theory turning-point energy
turn_window   = 0.00;     % MeV. Removes points too close to the turning point when separating branches.

% Agreement metric settings
% This excludes simulation/theory comparison points above the theoretical
% turning angle. Set to 0 to keep everything up to theta_max,theory.
turning_point_exclusion_deg = 0.0;   % deg

% This excludes values close to the turning energy, where the two
% branches merge. Set to 0 to use all valid energies for theta(E).
turning_point_exclusion_MeV = 0.00;  % MeV.

% MAPE for theta is undefined at theta = 0, so exclude very small theory angles.
min_theta_for_theta_mape = 0.1;      % deg

% CSV output for the agreement metrics
metrics_csv_file = "thin_target_agreement_metric.csv";
% for cutting out simulation outliers (e.g. scattered neutrons)
max_E_forward = []; % theory maximum forward-branch energy
max_theta_fit = [15 15 23 30 35]; % theory maximum lab angle 

%plot settings
ylim_energy = [0.5 4];  
xlim_theta  = [0 30];
xlim_ratio_theta = [0.99 1.02];
ylim_ratio_energy = [0.97 1.03];

interp_method = 'pchip';  
smooth_window = 15;       % moving-median window for outlier rejection
outlier_scale = 5;        % larger = less aggressive filtering

plot_individual_figures = false;

%% Check settings
num_sims = numel(sim_files);

if numel(sim_energies) ~= num_sims
    error('sim_energies must contain one incident energy per sim_files entry.')
end

if numel(sim_labels) ~= num_sims
    error('sim_labels must contain one label per sim_files entry.')
end

%checking that there is one value for each simulation
output_files = expandSetting(output_files, num_sims, 'output_files');
turn_window  = expandSetting(turn_window,  num_sims, 'turn_window');
turning_point_exclusion_deg = expandSetting(turning_point_exclusion_deg, num_sims, 'turning_point_exclusion_deg');
turning_point_exclusion_MeV = expandSetting(turning_point_exclusion_MeV, num_sims, 'turning_point_exclusion_MeV');
min_theta_for_theta_mape    = expandSetting(min_theta_for_theta_mape,    num_sims, 'min_theta_for_theta_mape');

%% Calculate theory for each simulation energy

% convert atomic masses to MeV/c2
mass_projectile = theory_mass_projectile_u * u_to_MeV;
mass_target     = theory_mass_target_u     * u_to_MeV;
mass_ejectile   = theory_mass_ejectile_u   * u_to_MeV;
mass_recoil     = theory_mass_recoil_u     * u_to_MeV;

theory = cell(num_sims, 1);

%For each incident energy, calculate relativisitic two body kinematics
for ii = 1:num_sims
    theory{ii} = calculateTwoBodyTheoryRelativistic(sim_energies(ii), ...
        mass_projectile, mass_target, mass_ejectile, mass_recoil, theory_n_angles);
end

%Determine/assign turning point energy value for forward/backward branches
if isempty(energy_turn)
    energy_turn = zeros(num_sims, 1);
    for ii = 1:num_sims
        energy_turn(ii) = theory{ii}.turn_energy;
    end
else
    energy_turn = expandSetting(energy_turn, num_sims, 'energy_turn');
end

%Determine/assign energy fitting limits
if isempty(max_E_forward)
    max_E_forward = zeros(num_sims, 1);
    for ii = 1:num_sims
        max_E_forward(ii) = max(theory{ii}.E_forward);
    end
else
    max_E_forward = expandSetting(max_E_forward, num_sims, 'max_E_forward');
end

%Determine/assign theta fitting limits
if isempty(max_theta_fit)
    max_theta_fit = zeros(num_sims, 1);
    for ii = 1:num_sims
        max_theta_fit(ii) = min(xlim_theta(2), theory{ii}.theta_max);
    end
else
    max_theta_fit = expandSetting(max_theta_fit, num_sims, 'max_theta_fit');
end

%% Analyse each simulation file
results = cell(num_sims, 1);

% Compare simulation results with the theoretical curves
for ii = 1:num_sims
    results{ii} = analyseOneSimulation( ...
        sim_files(ii), output_files(ii), script, tree, sim_labels(ii), ...
        theory{ii}, n, line, energy_turn(ii), turn_window(ii), ...
        max_E_forward(ii), max_theta_fit(ii), interp_method, ...
        smooth_window, outlier_scale, turning_point_exclusion_deg(ii), ...
        turning_point_exclusion_MeV(ii), min_theta_for_theta_mape(ii), ...
        plot_individual_figures, xlim_theta);
end

%% Save MAPE / agreement metrics
metric_tables = cellfun(@(r) r.metrics_table, results, 'UniformOutput', false);
metrics_table = vertcat(metric_tables{:});
metrics_table = metrics_table(metrics_table.Branch == "combined", :);
writetable(metrics_table, metrics_csv_file);
disp(metrics_table)
fprintf('Agreement metrics written to: %s\n', metrics_csv_file);

%% Combined plot for all simulation energies
figure
t = tiledlayout(6,6,'TileSpacing','compact');

ax_main = nexttile([4 5]);
hold on
colour_order = colororder(ax_main);
legend_handles = gobjects(2*num_sims, 1);
legend_entries = strings(2*num_sims, 1);

for ii = 1:num_sims
    this_colour = colour_order(mod(ii-1, size(colour_order,1)) + 1, :);

    % Theory forward branch - show in legend
    h_theory = plot(results{ii}.theta_th_forward, results{ii}.E_th_forward, '-', ...
        'Color', theory_colour, 'LineWidth', line);
    hold on

    % Theory backward branch - hide from legend
    plot(results{ii}.theta_th_backward, results{ii}.E_th_backward, '-', ...
        'Color', theory_colour, 'LineWidth', line, ...
        'HandleVisibility','off');

    % Geant4 forward branch - show in legend
    h_sim = plot(results{ii}.theta_x_forward, results{ii}.E_fit_forward, '-', ...
        'Color', this_colour, 'LineWidth', line);

    % Geant4 backward branch - hide from legend
    plot(results{ii}.theta_x_backward, results{ii}.E_fit_backward, '--', ...
        'Color', this_colour, 'LineWidth', line, ...
        'HandleVisibility','off');

    legend_handles(2*ii-1) = h_theory;
    legend_handles(2*ii)   = h_sim;

    legend_entries(2*ii-1) = results{ii}.label + " theory";
    legend_entries(2*ii)   = results{ii}.label;
end

legend(legend_handles, legend_entries, 'Location', 'best');

xlim(xlim_theta);
ylim(ylim_energy)
set(gca,'XTick',[])
ylabel('Neutron Energy (MeV)');
legend(legend_entries, 'Location', 'best');

ax_ratio_E = nexttile([2 5]);
hold on
for ii = 1:num_sims
    this_colour = colour_order(mod(ii-1, size(colour_order,1)) + 1, :);
    plot(results{ii}.theta_x_forward,  results{ii}.ratio_energy_forward, '-', ...
        'Color', this_colour, 'LineWidth', line);
    plot(results{ii}.theta_x_backward, results{ii}.ratio_energy_backward, '--', ...
        'Color', this_colour, 'LineWidth', line);
end
yline(1,'k--');
ylim(ylim_ratio_energy)
xlabel('Angle from beam axis \theta_L (deg)')
ylabel('sim_E/the_E')
xlim(xlim_theta);

ax_ratio_theta = nexttile([4 1]);
hold on
for ii = 1:num_sims
    this_colour = colour_order(mod(ii-1, size(colour_order,1)) + 1, :);
    plot(results{ii}.ratio_theta_forward,  results{ii}.E_y_forward, '-', ...
        'Color', this_colour, 'LineWidth', line);
    plot(results{ii}.ratio_theta_backward, results{ii}.E_y_backward, '--', ...
        'Color', this_colour, 'LineWidth', line);
end
xline(1,'k--');
xlim(xlim_ratio_theta)
ylim(ylim_energy)
set(gca,'YTick',[])
xlabel('sim_\theta/the_\theta')
ax_ratio_theta.XAxisLocation = 'top';

ax = findall(t, 'Type', 'axes');
set(ax, 'FontSize', 18);

%% ======== Functions ========
function result = analyseOneSimulation(filename, output, script, tree, sim_label, ...
    theory, n, line, energy_turn, turn_window, max_E_forward, ...
    max_theta_fit, interp_method, smooth_window, outlier_scale, ...
    turning_point_exclusion_deg, turning_point_exclusion_MeV, ...
    min_theta_for_theta_mape, plot_individual_figures, xlim_theta)
    
    %theoretical curve branches
    theta_th_forward  = theory.theta_forward;
    E_th_forward      = theory.E_forward;
    theta_th_backward = theory.theta_backward;
    E_th_backward     = theory.E_backward;

    % Convert ROOT output to HDF5
    command = sprintf('python3 %s %s %s %s', script, filename, tree, output);
    [status, cmdout] = system(command);

    if status ~= 0
        error("ROOT to HDF5 conversion failed for %s:\n%s", filename, cmdout)
    end

    tbl = h5read(output, sprintf('/%s/table',tree));
    energy_raw = tbl.values_block_3(1,:)';
    rad_raw = tbl.values_block_3(5,:)';
    theta_raw = rad_raw * (180/pi);

    theta = theta_raw;
    energy = energy_raw;

    %raw-data check
    if plot_individual_figures
        figure
        plot(theta_th_forward,E_th_forward,'-','LineWidth',2);
        hold on
        plot(theta_th_backward,E_th_backward,'-','LineWidth',2);
        plot(theta,energy,'.','MarkerSize',16);
        xlim(xlim_theta);
        h = gca;
        h.FontSize = 20;
        xlabel('Angle from beam axis \theta_L (deg)')
        ylabel('Neutron Energy (MeV)');
        legend('Theory forward branch', 'Theory backward branch', sim_label + ' Geant4 raw points');
        title(sim_label)
    end

    % Forward branch fit
    % Use energies above the turning point, excluding a small
    % transition window (turn_window) where the two branches can be misclassified.
    idx_forward = energy > energy_turn + turn_window;
    theta_forward_raw = theta(idx_forward);
    E_forward_raw = energy(idx_forward);

    % Clean and interpolate E(theta)
    [theta_forward_clean, E_forward_clean, theta_x_forward, E_fit_forward] = ...
        cleanCurveByTheta(theta_forward_raw, E_forward_raw, n, max_theta_fit, ...
        smooth_window, outlier_scale, interp_method);

    % Interpolate theory on the same theta grid
    E_th_interp_forward = interpTheoryByTheta(theta_th_forward, E_th_forward, theta_x_forward);

    % Clean and interpolate theta(E)
    E_y_forward_min = min(E_forward_clean);
    E_y_forward_max = min(max_E_forward, max(E_forward_clean));
    [E_forward_clean2, theta_forward_clean2, E_y_forward, theta_fit_forward] = ...
        cleanCurveByEnergy(E_forward_raw, theta_forward_raw, n, E_y_forward_min, E_y_forward_max, ...
        smooth_window, outlier_scale, interp_method);
    theta_th_interp_forward = interpTheoryByEnergy(E_th_forward, theta_th_forward, E_y_forward);
    
    % ratio of experiment to theory
    ratio_energy_forward = E_fit_forward ./ E_th_interp_forward;
    ratio_theta_forward  = theta_fit_forward ./ theta_th_interp_forward;

    if plot_individual_figures
        figure
        t = tiledlayout(6,6,'TileSpacing','compact');
        nexttile([4 5])
        plot(theta_th_forward,E_th_forward,'-','LineWidth',2);
        hold on
        plot(theta_forward_raw,E_forward_raw,'.','MarkerSize',10);
        plot(theta_forward_clean,E_forward_clean,'.','MarkerSize',16);
        plot(theta_x_forward,E_fit_forward,'-','LineWidth',line);
        xlim(xlim_theta);
        ylim([min(E_th_forward) max(E_th_forward)])
        ylabel('Neutron Energy (MeV)');
        set(gca,'XTick',[])
        legend('Relativistic theory', sim_label + ' raw', sim_label + ' kept', sim_label + ' robust fit');
        title(sim_label + ' forward branch')

        nexttile([2 5])
        plot(theta_x_forward,ratio_energy_forward,'-','LineWidth',line);
        yline(1,'k--');
        xlim(xlim_theta);
        xlabel('Angle from beam axis \theta_L (deg)')
        ylabel('sim_E/the_E')

        ax = nexttile([4 1]);
        plot(ratio_theta_forward,E_y_forward,'-','LineWidth',line);
        xline(1,'k--');
        ylim([min(E_y_forward) max(E_y_forward)])
        xlabel('sim_\theta/the_\theta')
        set(gca,'YTick',[])
        ax.XAxisLocation = 'top';

        ax = findall(t, 'Type', 'axes');
        set(ax, 'FontSize', 18);
    end

    % Backward branch fit
    % Use energies below the turning point, excluding a small
    % transition window (turn_window) where the two branches can be misclassified.
    idx_backward = energy < energy_turn - turn_window;
    theta_backward_raw = theta(idx_backward);
    E_backward_raw = energy(idx_backward);

    % Clean and interpolate E(theta)
    [theta_backward_clean, E_backward_clean, theta_x_backward, E_fit_backward] = ...
        cleanCurveByTheta(theta_backward_raw, E_backward_raw, n, max_theta_fit, ...
        smooth_window, outlier_scale, interp_method);

    % Interpolate theory on the same theta grid
    E_th_interp_backward = interpTheoryByTheta(theta_th_backward, E_th_backward, theta_x_backward);

    % Clean and interpolate theta(E)
    E_y_backward_min = max(min(E_th_backward), min(E_backward_clean));
    E_y_backward_max = max(E_backward_clean);
    [E_backward_clean2, theta_backward_clean2, E_y_backward, theta_fit_backward] = ...
        cleanCurveByEnergy(E_backward_raw, theta_backward_raw, n, E_y_backward_min, E_y_backward_max, ...
        smooth_window, outlier_scale, interp_method);
    theta_th_interp_backward = interpTheoryByEnergy(E_th_backward, theta_th_backward, E_y_backward);
    
    % ratio of experiment to theory
    ratio_energy_backward = E_fit_backward ./ E_th_interp_backward;
    ratio_theta_backward  = theta_fit_backward ./ theta_th_interp_backward;

    if plot_individual_figures
        figure
        t = tiledlayout(6,6,'TileSpacing','compact');
        nexttile([4 5])
        plot(theta_th_backward,E_th_backward,'-','LineWidth',2);
        hold on
        plot(theta_backward_raw,E_backward_raw,'.','MarkerSize',10);
        plot(theta_backward_clean,E_backward_clean,'.','MarkerSize',16);
        plot(theta_x_backward,E_fit_backward,'-','LineWidth',line);
        xlim(xlim_theta);
        ylim([min(E_th_backward) max(E_th_backward)])
        ylabel('Neutron Energy (MeV)');
        set(gca,'XTick',[])
        legend('Relativistic theory', sim_label + ' raw', sim_label + ' kept', sim_label + ' robust fit');
        title(sim_label + ' backward branch')

        nexttile([2 5])
        plot(theta_x_backward,ratio_energy_backward,'-','LineWidth',line);
        yline(1,'k--');
        xlabel('Angle from beam axis \theta_L (deg)')
        ylabel('sim_E/the_E')
        xlim(xlim_theta);

        ax = nexttile([4 1]);
        plot(ratio_theta_backward,E_y_backward,'-','LineWidth',line);
        xline(1,'k--');
        ylim([min(E_y_backward) max(E_y_backward)])
        xlabel('sim_\theta/the_\theta')
        set(gca,'YTick',[])
        ax.XAxisLocation = 'top';

        ax = findall(t, 'Type', 'axes');
        set(ax, 'FontSize', 18);
    end

    % Endpoint differences
    delta_E_upper = abs(max(E_fit_forward, [], 'omitnan') - max(E_th_forward, [], 'omitnan') );
    delta_E_lower = abs(min(E_fit_backward, [], 'omitnan') - min(E_th_backward, [], 'omitnan') );
    
    % Agreement metrics
    % Energy metrics: compare E_sim(theta) with E_theory(theta).
    % Only points up to the theoretical turning angle are used.
    energy_metrics_forward = agreementMetricsByTheta(E_fit_forward, E_th_interp_forward, ...
        theta_x_forward, theory.theta_max, turning_point_exclusion_deg, 0);
    energy_metrics_backward = agreementMetricsByTheta(E_fit_backward, E_th_interp_backward, ...
        theta_x_backward, theory.theta_max, turning_point_exclusion_deg, 0);
    energy_metrics_combined = agreementMetricsByTheta( ...
        [E_fit_forward(:); E_fit_backward(:)], ...
        [E_th_interp_forward(:); E_th_interp_backward(:)], ...
        [theta_x_forward(:); theta_x_backward(:)], ...
        theory.theta_max, turning_point_exclusion_deg, 0);

    % Theta metrics: compare theta_sim(E) with theta_theory(E).
    % These are filtered using energy, not angle. This avoids using the
    % angular turning-point cut for theta(E). The optional MeV exclusion only
    % removes values close to the turning energy where the branches merge.
    theta_metrics_forward = agreementMetricsByEnergy(theta_fit_forward, theta_th_interp_forward, ...
        E_y_forward, theory.turn_energy, turning_point_exclusion_MeV, min_theta_for_theta_mape);
    theta_metrics_backward = agreementMetricsByEnergy(theta_fit_backward, theta_th_interp_backward, ...
        E_y_backward, theory.turn_energy, turning_point_exclusion_MeV, min_theta_for_theta_mape);
    theta_metrics_combined = agreementMetricsByEnergy( ...
        [theta_fit_forward(:); theta_fit_backward(:)], ...
        [theta_th_interp_forward(:); theta_th_interp_backward(:)], ...
        [E_y_forward(:); E_y_backward(:)], ...
        theory.turn_energy, turning_point_exclusion_MeV, min_theta_for_theta_mape);

    % Turning-point angular difference: compare the maximum fitted simulated
    % lab angle with the theoretical kinematic turning angle. This captures
    % cases where the simulated curve extends past theta_max,theory.
    theta_turn_metrics = thetaTurningPointDifference(theta_fit_forward, theta_fit_backward, theory.theta_max);

    metrics_table = makeMetricsTable(sim_label, theory.E_projectile, theory.theta_max, theory.turn_energy, ...
        turning_point_exclusion_deg, turning_point_exclusion_MeV, min_theta_for_theta_mape, ...
        energy_metrics_forward, energy_metrics_backward, energy_metrics_combined, ...
        theta_metrics_forward, theta_metrics_backward, theta_metrics_combined, ...
        theta_turn_metrics, delta_E_upper, delta_E_lower);

    %% Store results for final combined plot
    result.label = sim_label;
    result.filename = filename;

    result.theta_th_forward = theta_th_forward;
    result.E_th_forward = E_th_forward;
    result.theta_th_backward = theta_th_backward;
    result.E_th_backward = E_th_backward;
    result.theory_turn_energy = theory.turn_energy;
    result.theory_theta_max = theory.theta_max;

    result.theta_x_forward = theta_x_forward;
    result.E_fit_forward = E_fit_forward;
    result.ratio_energy_forward = ratio_energy_forward;
    result.E_y_forward = E_y_forward;
    result.ratio_theta_forward = ratio_theta_forward;

    result.theta_x_backward = theta_x_backward;
    result.E_fit_backward = E_fit_backward;
    result.ratio_energy_backward = ratio_energy_backward;
    result.E_y_backward = E_y_backward;
    result.ratio_theta_backward = ratio_theta_backward;

    result.metrics_table = metrics_table;
end

% Agreement metrics for E(theta).
function m = agreementMetricsByTheta(sim_values, theory_values, theta_reference, theta_turn, turning_point_exclusion_deg, min_theory_value)
    
    sim_values = sim_values(:);
    theory_values = theory_values(:);
    theta_reference = theta_reference(:);

    theta_limit = theta_turn - turning_point_exclusion_deg;

    valid = isfinite(sim_values) & isfinite(theory_values) & isfinite(theta_reference);
    valid = valid & theta_reference <= theta_limit;
    valid_for_mape = valid & abs(theory_values) > min_theory_value;

    m = calculateMetricStruct(sim_values, theory_values, valid, valid_for_mape);
end

function m = agreementMetricsByEnergy(sim_values, theory_values, energy_reference, energy_turn, turning_point_exclusion_MeV, min_theory_value_for_mape)
    % Agreement metrics for theta(E).
    % The independent variable is energy, so filtering is based on the energy
    % grid. The optional exclusion removes energies close to the turning
    % energy, where the two branches merge.
    sim_values = sim_values(:);
    theory_values = theory_values(:);
    energy_reference = energy_reference(:);

    valid = isfinite(sim_values) & isfinite(theory_values) & isfinite(energy_reference);

    if turning_point_exclusion_MeV > 0
        valid = valid & abs(energy_reference - energy_turn) > turning_point_exclusion_MeV;
    end

    % For theta MAPE only, avoid division by very small theory angles. The
    % MAE and RMSE still use the full valid set because they do not divide by theta.
    valid_for_mape = valid & abs(theory_values) > min_theory_value_for_mape;

    m = calculateMetricStruct(sim_values, theory_values, valid, valid_for_mape);
end

% Common metric calculation. MAE, RMSE, MAPE
function m = calculateMetricStruct(sim_values, theory_values, valid, valid_for_mape)
    
    diff = sim_values(valid) - theory_values(valid);

    diff_mape = sim_values(valid_for_mape) - theory_values(valid_for_mape);
    abs_percent = 100 * abs(diff_mape ./ theory_values(valid_for_mape));

    m.N_total = numel(sim_values);
    m.N_used = nnz(valid);
    m.N_used_MAPE = nnz(valid_for_mape);

    if isempty(diff)
        m.MAE = NaN;
        m.RMSE = NaN;
        m.max_abs_difference = NaN;
    else
        m.MAE = mean(abs(diff), 'omitnan');
        m.RMSE = sqrt(mean(diff.^2, 'omitnan'));
        m.max_abs_difference = max(abs(diff), [], 'omitnan');
    end

    if isempty(abs_percent)
        m.MAPE_percent = NaN;
        m.max_abs_percent = NaN;
    else
        m.MAPE_percent = mean(abs_percent, 'omitnan');
        m.max_abs_percent = max(abs_percent, [], 'omitnan');
    end
end

function m = thetaTurningPointDifference(theta_fit_forward, theta_fit_backward, theta_turn_theory_deg)
    % Difference between the largest fitted Geant4 lab angle and the
    % theoretical turning angle. This is reported separately from the RMSE
    % because it specifically describes the branch-convergence region.
    theta_all = [theta_fit_forward(:); theta_fit_backward(:)];
    theta_all = theta_all(isfinite(theta_all));

    if isempty(theta_all)
        m.sim_max_theta_deg = NaN;
        m.signed_difference_deg = NaN;
        m.abs_difference_deg = NaN;
    else
        m.sim_max_theta_deg = max(theta_all);
        m.signed_difference_deg = m.sim_max_theta_deg - theta_turn_theory_deg;
        m.abs_difference_deg = abs(m.signed_difference_deg);
    end
end

function metrics_table = makeMetricsTable(sim_label, incident_energy_MeV, theta_turn_deg, turn_energy_MeV, ...
    turning_point_exclusion_deg, turning_point_exclusion_MeV, min_theta_for_theta_mape, ...
    energy_forward, energy_backward, energy_combined, ...
    theta_forward, theta_backward, theta_combined, theta_turn_metrics, delta_E_upper, delta_E_lower)

    branch = ["forward"; "backward"; "combined"];
    energy_metrics = [energy_forward; energy_backward; energy_combined];
    theta_metrics  = [theta_forward;  theta_backward;  theta_combined];

    n_rows = numel(branch);
    metrics_table = table;
    metrics_table.Label = repmat(string(sim_label), n_rows, 1);
    metrics_table.IncidentEnergy_MeV = repmat(incident_energy_MeV, n_rows, 1);
    metrics_table.Branch = branch;
    metrics_table.TheoryTurningAngle_deg = repmat(theta_turn_deg, n_rows, 1);
    metrics_table.TheoryTurningEnergy_MeV = repmat(turn_energy_MeV, n_rows, 1);
    metrics_table.SimMaxThetaAtTurningPoint_deg = repmat(theta_turn_metrics.sim_max_theta_deg, n_rows, 1);
    metrics_table.ThetaTurningPointDiff_deg = repmat(theta_turn_metrics.signed_difference_deg, n_rows, 1);
    metrics_table.ThetaTurningPointAbsDiff_deg = repmat(theta_turn_metrics.abs_difference_deg, n_rows, 1);
    metrics_table.EnergyThetaLimit_deg = repmat(theta_turn_deg - turning_point_exclusion_deg, n_rows, 1);
    metrics_table.TurningPointExclusion_deg = repmat(turning_point_exclusion_deg, n_rows, 1);
    metrics_table.TurningPointExclusion_MeV = repmat(turning_point_exclusion_MeV, n_rows, 1);
    metrics_table.MinThetaForThetaMAPE_deg = repmat(min_theta_for_theta_mape, n_rows, 1);

    metrics_table.EnergyMAPE_percent = [energy_metrics.MAPE_percent]';
    metrics_table.EnergyMAE_MeV = [energy_metrics.MAE]';
    metrics_table.EnergyRMSE_MeV = [energy_metrics.RMSE]';
    metrics_table.EnergyMaxAbsPercent = [energy_metrics.max_abs_percent]';
    metrics_table.EnergyMaxAbsDifference_MeV = [energy_metrics.max_abs_difference]';
    metrics_table.EnergyUpperEndpointDiff_MeV = repmat(delta_E_upper, n_rows, 1);
    metrics_table.EnergyLowerEndpointDiff_MeV = repmat(delta_E_lower, n_rows, 1);
    metrics_table.EnergyNUsed = [energy_metrics.N_used]';
    metrics_table.EnergyNUsedMAPE = [energy_metrics.N_used_MAPE]';
    metrics_table.EnergyNTotal = [energy_metrics.N_total]';

    metrics_table.ThetaMAPE_percent = [theta_metrics.MAPE_percent]';
    metrics_table.ThetaMAE_deg = [theta_metrics.MAE]';
    metrics_table.ThetaRMSE_deg = [theta_metrics.RMSE]';
    metrics_table.ThetaMaxAbsPercent = [theta_metrics.max_abs_percent]';
    metrics_table.ThetaMaxAbsDifference_deg = [theta_metrics.max_abs_difference]';
    metrics_table.ThetaNUsed = [theta_metrics.N_used]';
    metrics_table.ThetaNUsedMAPE = [theta_metrics.N_used_MAPE]';
    metrics_table.ThetaNTotal = [theta_metrics.N_total]';
end

%Calculate relativisitc two body kinematics for the reaction of interest
function theory = calculateTwoBodyTheoryRelativistic(T_projectile, m1, m2, m3, m4, n_angles)
    % Relativistic two-body kinematics for:
    % projectile + target -> ejectile + recoil
    % Target is initially at rest in the lab frame.
    % Masses are in MeV/c^2 and kinetic energies are in MeV.
    % 1 = incident projectile
    % 2 = Stationary target
    % 3 = emitted particle
    % 4 = recoil nucleus
    
    %Q value:
    Q = (m1 + m2 - m3 - m4);

    % Projectile total energy and momentum in the lab frame.
    E1_lab = m1 + T_projectile;
    p1_lab = sqrt(max(E1_lab.^2 - m1.^2, 0));

    % invariant mass of the two-particle entrance channel
    s = m1.^2 + m2.^2 + 2*m2*E1_lab;
    sqrt_s = sqrt(s);

    if sqrt_s < (m3 + m4)
        error('The reaction is below threshold for T_projectile = %.6g MeV.', T_projectile)
    end

    % Final-state momentum and ejectile total energy in the CM frame.
    lambda = (s - (m3 + m4).^2) .* (s - (m3 - m4).^2);
    p3_cm = sqrt(max(lambda, 0)) ./ (2*sqrt_s);
    E3_cm = (s + m3.^2 - m4.^2) ./ (2*sqrt_s);

    % Lab-frame velocity of the CM frame.
    beta_cm = p1_lab ./ (E1_lab + m2);
    gamma_cm = 1 ./ sqrt(1 - beta_cm.^2);

    theta_cm = linspace(0, pi, n_angles).';

    % Lorentz-transform ejectile momentum from CM to lab.
    pz_lab = gamma_cm .* (p3_cm*cos(theta_cm) + beta_cm*E3_cm);
    px_lab = p3_cm*sin(theta_cm);
    E3_lab_total = gamma_cm .* (E3_cm + beta_cm*p3_cm*cos(theta_cm));
    %neutron lab emission angle
    theta_lab = atan2(px_lab, pz_lab) * 180/pi;
    %neutron lab kinetic energy
    T3_lab = E3_lab_total - m3;
    
    %max labaratory emission angle
    [theta_max, idx_turn] = max(theta_lab);
    
    %define branches (from analysis purposes) based on the turning point
    theta_forward = theta_lab(1:idx_turn);
    E_forward = T3_lab(1:idx_turn);

    theta_backward = flipud(theta_lab(idx_turn:end));
    E_backward = flipud(T3_lab(idx_turn:end));
    
    %prepare curves for interpolation for comparison with simulation data.
    [theta_forward, E_forward] = uniqueCurve(theta_forward, E_forward);
    [theta_backward, E_backward] = uniqueCurve(theta_backward, E_backward);

    theory.E_projectile = T_projectile;
    theory.Q = Q;
    theory.theta_forward = theta_forward;
    theory.E_forward = E_forward;
    theory.theta_backward = theta_backward;
    theory.E_backward = E_backward;
    theory.theta_max = theta_max;
    theory.turn_energy = T3_lab(idx_turn);
end

% prepares theoretical energy-angle curves for interpolation.
% ensures that arrays have unique values
function [x_u, y_u] = uniqueCurve(x, y)
    x = x(:);
    y = y(:);
    valid = isfinite(x) & isfinite(y);
    x = x(valid);
    y = y(valid);

    [x_sorted, order] = sort(x);
    y_sorted = y(order);
    [x_u, ~, ic] = unique(x_sorted);
    y_u = accumarray(ic, y_sorted, [], @median);
end

%checks and expands settings so there is one value for each simulation
function values = expandSetting(values, num_sims, setting_name)
    if isscalar(values)
        values = repmat(values, num_sims, 1);
    elseif numel(values) ~= num_sims
        error('%s must be a scalar or contain one value per sim_files entry.', setting_name)
    else
        values = values(:);
    end
end

% remove invalid values, sort by angle, remove outliers, interpolate
function [theta_clean, E_clean, theta_grid, E_fit] = cleanCurveByTheta(theta_in, E_in, n, theta_grid_max, smooth_window, outlier_scale, interp_method)
    theta_in = theta_in(:);
    E_in = E_in(:);

    valid = isfinite(theta_in) & isfinite(E_in);
    theta_in = theta_in(valid);
    E_in = E_in(valid);

    [theta_sorted, order] = sort(theta_in);
    E_sorted = E_in(order);

    % Collapse duplicate theta values using the median energy.
    [theta_u, ~, ic] = unique(theta_sorted);
    E_u = accumarray(ic, E_sorted, [], @median);

    % Reject isolated points that do not follow the local trend.
    keep = robustLocalKeep(E_u, smooth_window, outlier_scale);
    theta_clean = theta_u(keep);
    E_clean = E_u(keep);

    if numel(theta_clean) < 2
        error('Not enough points left after cleaning the theta curve. Reduce turn_window or increase outlier_scale.')
    end
    
    %interpolate
    theta_grid_end = min(theta_grid_max, max(theta_clean));
    theta_grid = linspace(min(theta_clean), theta_grid_end, n);
    E_fit = interp1(theta_clean, E_clean, theta_grid, interp_method);
end

% remove invalid values, sort by angle, remove outliers, interpolate
function [E_clean, theta_clean, E_grid, theta_fit] = cleanCurveByEnergy(E_in, theta_in, n, E_grid_min, E_grid_max, smooth_window, outlier_scale, interp_method)
    E_in = E_in(:);
    theta_in = theta_in(:);

    valid = isfinite(E_in) & isfinite(theta_in);
    E_in = E_in(valid);
    theta_in = theta_in(valid);

    [E_sorted, order] = sort(E_in);
    theta_sorted = theta_in(order);

    % Collapse duplicate energy values using the median theta.
    [E_u, ~, ic] = unique(E_sorted);
    theta_u = accumarray(ic, theta_sorted, [], @median);

    % Reject isolated points that do not follow the local trend.
    keep = robustLocalKeep(theta_u, smooth_window, outlier_scale);
    E_clean = E_u(keep);
    theta_clean = theta_u(keep);

    if numel(E_clean) < 2
        error('Not enough points left after cleaning the energy curve. Reduce turn_window or increase outlier_scale.')
    end
    
    %interpolate
    E_grid_min = max(E_grid_min, min(E_clean));
    E_grid_max = min(E_grid_max, max(E_clean));
    E_grid = linspace(E_grid_min, E_grid_max, n);
    theta_fit = interp1(E_clean, theta_clean, E_grid, interp_method);
end

function keep = robustLocalKeep(y, smooth_window, outlier_scale)
    y = y(:);

    if numel(y) < smooth_window
        smooth_window = max(3, 2*floor(numel(y)/4) + 1);
    end

    y_med = movmedian(y, smooth_window);
    residual = y - y_med;

    residual_med = median(residual);
    robust_sigma = 1.4826 * median(abs(residual - residual_med));

    if robust_sigma == 0 || ~isfinite(robust_sigma)
        keep = true(size(y));
    else
        keep = abs(residual - residual_med) < outlier_scale * robust_sigma;
    end

    % Always keep the first and last points so the interpolation range is stable.
    keep(1) = true;
    keep(end) = true;
end

function E_interp = interpTheoryByTheta(theta_theory, E_theory, theta_query)
    theta_theory = theta_theory(:);
    E_theory = E_theory(:);

    valid = isfinite(theta_theory) & isfinite(E_theory);
    theta_theory = theta_theory(valid);
    E_theory = E_theory(valid);

    [theta_sorted, order] = sort(theta_theory);
    E_sorted = E_theory(order);
    [theta_u, ~, ic] = unique(theta_sorted);
    E_u = accumarray(ic, E_sorted, [], @median);

    E_interp = interp1(theta_u, E_u, theta_query, 'linear', NaN);
end

function theta_interp = interpTheoryByEnergy(E_theory, theta_theory, E_query)
    E_theory = E_theory(:);
    theta_theory = theta_theory(:);

    valid = isfinite(E_theory) & isfinite(theta_theory);
    E_theory = E_theory(valid);
    theta_theory = theta_theory(valid);

    [E_sorted, order] = sort(E_theory);
    theta_sorted = theta_theory(order);
    [E_u, ~, ic] = unique(E_sorted);
    theta_u = accumarray(ic, theta_sorted, [], @median);

    theta_interp = interp1(E_u, theta_u, E_query, 'linear', NaN);
end