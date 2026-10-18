% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Section 3.1
% Description: Nonlinear longitudinal vehicle dynamics, AM08 equation (3.3).
%
%   dv = cruise_accel(v, u, p, theta)
%
%   m dv/dt = alpha_n u T(alpha_n v) - m g Cr sgn(v) - 1/2 rho Cd A v^2
%             - m g sin(theta)
%
%   T(w) = Tm (1 - beta (w/wm - 1)^2)                        (3.2)
%
%   v      vehicle speed [m/s]
%   u      throttle command; saturated to [0, 1] here
%   p      parameter struct from cruise_params
%   theta  road slope [rad] (default 0)

function dv = cruise_accel(v, u, p, theta)
  if nargin < 4, theta = 0; end

  u = min(max(u, 0), 1);

  w  = p.an * v;
  T  = p.Tm * (1 - p.beta * (w / p.wm - 1)^2);
  F  = p.an * u * T;

  Fr = p.m * p.g * p.Cr * sign(v);
  Fa = 0.5 * p.rho * p.Cd * p.A * v^2;
  Fg = p.m * p.g * sin(theta);

  dv = (F - Fr - Fa - Fg) / p.m;
end
