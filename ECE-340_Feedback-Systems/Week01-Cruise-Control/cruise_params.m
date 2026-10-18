% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Section 3.1
% Description: Vehicle and engine parameters for the cruise control model.
%
%   p = cruise_params(m, gear)
%
%   m     vehicle mass [kg]          (default 1000)
%   gear  gear number 1..5           (default 3, as used for Figure 1.13)

function p = cruise_params(m, gear)
  if nargin < 1, m = 1000; end
  if nargin < 2, gear = 3; end

  alpha = [40 25 16 12 10];   % alpha_n = n/r for gears 1..5 [1/m]

  p.m    = m;                 % vehicle mass [kg]
  p.gear = gear;
  p.an   = alpha(gear);       % engine speed per unit vehicle speed
  p.Tm   = 190;               % peak engine torque [N m]
  p.wm   = 420;               % engine speed at peak torque [rad/s]
  p.beta = 0.4;               % torque-curve shape coefficient
  p.Cr   = 0.01;              % rolling friction coefficient
  p.rho  = 1.3;               % air density [kg/m^3]
  p.Cd   = 0.32;              % aerodynamic drag coefficient
  p.A    = 2.4;               % frontal area [m^2]
  p.g    = 9.8;               % gravitational acceleration [m/s^2]
end
