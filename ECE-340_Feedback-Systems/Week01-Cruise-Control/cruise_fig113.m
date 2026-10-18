% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Figure 1.13 (p. 18)
% Description: Reproduce Figure 1.13 and compare it against a corrected
%              linearization and the full nonlinear vehicle model.
%
% The figure in the book comes from cruise_response.m on the companion site.
% That script linearizes the vehicle in gear 3 about 25 m/s with a fixed
% trim throttle of 0.135, adds a 2/(s+2) engine lag, and closes the loop
% with PI gains kp = 1, ki = 0.02. Its throttle-gain finite difference
% spans +/-0.01 but divides by 0.01, so the plant gain it plots is 2x the
% true derivative. Panel (a) keeps that factor to match the printed figure.
%
% Run from this directory:  octave --no-gui --quiet cruise_fig113.m

clear; clc;

masses = [1000 2000 3000];
c = cruise_pi(1, 0.02);        % gains from the companion-site script

models = { 'a) Book (gain x2, u0 = 0.135)', ...
           'b) Corrected linearization', ...
           'c) Nonlinear, throttle saturation' };

figure(1); clf;
for k = 1:3
  subplot(1, 3, k); hold on;
  printf('%s\n', models{k});
  for m = masses
    p = cruise_params(m, 3);
    switch k
      case 1
        [a, b] = cruise_linearize(p, 25, 0.135);
        [t, v] = cruise_step_linear(a, 2*b, c);
      case 2
        [a, b] = cruise_linearize(p, 25);
        [t, v] = cruise_step_linear(a, b, c);
      case 3
        [t, v] = cruise_step_nonlinear(p, c);
    end
    plot(t, v, 'LineWidth', 1.5);
    printf('  m = %4d kg: peak = %.2f m/s, overshoot = %.2f m/s\n', ...
           m, max(v), max(v) - 30);
  end
  plot([0 2.5 2.5 20], [25 25 30 30], 'k-');
  axis([0 10 24 33]); grid on;
  title(models{k});
  xlabel('Time [s]'); ylabel('Speed [m/s]');
  if k == 1, legend('1000 kg', '2000 kg', '3000 kg', 'Location', 'southeast'); end
end

print('-dpng', '-S1500,450', 'cruise_fig113.png');
printf('Saved cruise_fig113.png\n');
