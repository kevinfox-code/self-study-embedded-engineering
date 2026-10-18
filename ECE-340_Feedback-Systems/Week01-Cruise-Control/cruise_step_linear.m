% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Figure 1.13
% Description: 25 -> 30 m/s step response of the linearized cruise loop.
%
%   [t, v] = cruise_step_linear(a, b, c, tf)
%
%   The reference steps from 25 to 30 m/s at t = 2.5 s. The loop is linear,
%   so throttle saturation is ignored. Simulated exactly with a zero-order
%   hold on the (piecewise constant) reference, dt = 0.01 s.

function [t, v] = cruise_step_linear(a, b, c, tf)
  if nargin < 4, tf = 20; end

  dt = 0.01;
  t  = (0:dt:tf)';
  [A, B] = cruise_closed_loop(a, b, c);

  % Exact ZOH discretization via the augmented matrix exponential
  n  = size(A, 1);
  M  = expm([A B; zeros(1, n + 1)] * dt);
  Ad = M(1:n, 1:n);
  Bd = M(1:n, n + 1);

  r = 5 * (t >= 2.5);
  x = zeros(n, 1);
  dv = zeros(size(t));
  for k = 1:numel(t)
    dv(k) = x(3);
    x = Ad * x + Bd * r(k);
  end
  v = 25 + dv;
end
