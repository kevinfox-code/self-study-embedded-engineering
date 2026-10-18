% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Exercise 1.4 (p. 26)
% Description: Retune the Figure 1.13 PI controller so that a 1000 kg car
%              overshoots by no more than 1 m/s, then check the design
%              against heavier cars and the nonlinear model.
%
% Exercise 1.4 asks for trial and error on the companion-site model. This
% script does the trial and error as a grid search on that same model
% (Figure 1.13 panel a), scoring each (kp, ki) pair by overshoot and by the
% 2% settling time (|v - 30| <= 0.1 m/s for good).
%
% Run from this directory:  octave --no-gui --quiet cruise_tune.m

1;  % script file: define the helpers below before use

% ---- helpers ----------------------------------------------------------
function [a, b] = book_plant(m)
  % Linearization exactly as the companion-site script computes it
  [a, b] = cruise_linearize(cruise_params(m, 3), 25, 0.135);
  b = 2 * b;
end

function [os, ts] = step_metrics(t, v, band)
  os = max(v) - 30;
  out = find(abs(v - 30) > band, 1, 'last');
  if out == numel(t), ts = inf; else, ts = t(out + 1) - 2.5; end
end

function s = pole_str(p)
  p = sort(p);
  s = strjoin(arrayfun(@(z) sprintf('%.3f%+.3fj', real(z), imag(z)), p', ...
              'UniformOutput', false), ',  ');
end

% ---- main ------------------------------------------------------------
clc;

spec_os = 1.0;      % allowed overshoot [m/s]
band    = 0.1;      % settling band [m/s] (2% of the 5 m/s step)

% ---- 1. Baseline: the book's gains ------------------------------------
[a, b] = book_plant(1000);
c0 = cruise_pi(1, 0.02);
[t, v] = cruise_step_linear(a, b, c0);
[os0, ts0] = step_metrics(t, v, band);
printf('Baseline kp = 1.00, ki = 0.020: overshoot %.2f m/s, settling %.2f s\n\n', ...
       os0, ts0);

% ---- 2. Grid search on the 1000 kg car --------------------------------
kps = 0.05:0.025:1.5;
kis = 0.0025:0.0025:0.1;
OS = nan(numel(kis), numel(kps));
TS = nan(numel(kis), numel(kps));
for i = 1:numel(kis)
  for j = 1:numel(kps)
    [t, v] = cruise_step_linear(a, b, cruise_pi(kps(j), kis(i)));
    [OS(i, j), TS(i, j)] = step_metrics(t, v, band);
  end
end

ok = OS <= spec_os;
TSok = TS;  TSok(~ok) = inf;
[ts_best, idx] = min(TSok(:));
[i, j] = ind2sub(size(TS), idx);
c = cruise_pi(kps(j), kis(i));
printf('%d of %d gain pairs meet overshoot <= %.1f m/s\n', nnz(ok), numel(ok), spec_os);
printf('Fastest-settling design: kp = %.2f, ki = %.3f\n', c.kp, c.ki);
printf('  overshoot %.2f m/s, settling %.2f s\n', OS(i, j), ts_best);

% Simplest fix: keep ki = 0.02 and only lower kp
row = find(abs(kis - 0.02) < 1e-9);
jj  = find(OS(row, :) <= spec_os, 1, 'last');
printf('With ki = 0.020 fixed, the largest passing kp is %.2f (overshoot %.2f m/s)\n\n', ...
       kps(jj), OS(row, jj));

% ---- 3. Closed-loop poles: baseline vs. tuned -------------------------
printf('Closed-loop poles, 1000 kg (book model):\n');
printf('  baseline: %s\n', pole_str(eig(cruise_closed_loop(a, b, c0))));
printf('  tuned:    %s\n\n', pole_str(eig(cruise_closed_loop(a, b, c))));

% ---- 4. Robustness: same gains on every mass and model ----------------
printf('%-34s %8s %8s %8s\n', 'Tuned gains, overshoot [m/s]', '1000 kg', '2000 kg', '3000 kg');
names = {'Book linear model (gain x2)', 'Corrected linear model', 'Nonlinear + saturation'};
figure(1); clf;
for k = 1:3
  subplot(1, 3, k); hold on;
  row_os = zeros(1, 3);
  masses = [1000 2000 3000];
  for n = 1:3
    p = cruise_params(masses(n), 3);
    switch k
      case 1
        [am, bm] = book_plant(masses(n));
        [t, v] = cruise_step_linear(am, bm, c);
      case 2
        [am, bm] = cruise_linearize(p, 25);
        [t, v] = cruise_step_linear(am, bm, c);
      case 3
        [t, v] = cruise_step_nonlinear(p, c);
    end
    row_os(n) = step_metrics(t, v, band);
    plot(t, v, 'LineWidth', 1.5);
  end
  printf('%-34s %8.2f %8.2f %8.2f\n', names{k}, row_os);
  plot([0 2.5 2.5 20], [25 25 30 30], 'k-');
  plot([0 20], [31 31], 'k:');
  axis([0 10 24 33]); grid on;
  title(names{k}); xlabel('Time [s]'); ylabel('Speed [m/s]');
  if k == 1, legend('1000 kg', '2000 kg', '3000 kg', 'Location', 'southeast'); end
end
print('-dpng', '-S1500,450', 'cruise_tune.png');

figure(2); clf;
contourf(kps, kis, min(OS, 3), 0:0.25:3); colorbar; hold on;
contour(kps, kis, OS, [spec_os spec_os], 'k', 'LineWidth', 2);
plot(c0.kp, c0.ki, 'rx', 'MarkerSize', 12, 'LineWidth', 2);
plot(c.kp, c.ki, 'wo', 'MarkerSize', 10, 'LineWidth', 2);
xlabel('k_p'); ylabel('k_i');
title('Overshoot [m/s], 1000 kg; black = 1 m/s spec, x = book, o = tuned');
print('-dpng', '-S800,600', 'cruise_tune_map.png');
printf('\nSaved cruise_tune.png and cruise_tune_map.png\n');
