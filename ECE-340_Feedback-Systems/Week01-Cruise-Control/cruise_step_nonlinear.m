% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Sections 3.1 and 10.4
% Description: 25 -> 30 m/s step response on the full nonlinear vehicle.
%
%   [t, v, u] = cruise_step_nonlinear(p, c, tf)
%
%   Same controller and engine lag as cruise_step_linear, but the plant is
%   equation (3.3) and the throttle saturates to [0, 1]. The trim throttle
%   u0 for 25 m/s is applied as a feedforward offset so the run starts in
%   equilibrium. Anti-windup is conditional integration: the controller
%   state is frozen while the throttle is saturated and the error would
%   push it further into saturation.

function [t, v, u] = cruise_step_nonlinear(p, c, tf)
  if nargin < 3, tf = 20; end

  [~, ~, u0] = cruise_linearize(p, 25);
  kc = c.ki - c.kp * c.eps;

  opts = odeset('MaxStep', 0.01);
  [t, x] = ode45(@(t, x) loop_rhs(t, x, p, c, u0, kc), [0 tf], [0; u0; 25], opts);
  v = x(:, 3);
  u = zeros(size(t));
  for k = 1:numel(t)
    [~, u(k)] = loop_rhs(t(k), x(k, :)', p, c, u0, kc);
  end
end

function [dx, us] = loop_rhs(t, x, p, c, u0, kc)
  xc = x(1);  ue = x(2);  v = x(3);
  e  = 25 + 5 * (t >= 2.5) - v;
  uc = u0 + c.kp * e + kc * xc;
  us = min(max(uc, 0), 1);
  if (uc > 1 && e > 0) || (uc < 0 && e < 0)
    dxc = 0;
  else
    dxc = -c.eps * xc + e;
  end
  dx = [ dxc;
         c.ae * (us - ue);
         cruise_accel(v, ue, p) ];
end
