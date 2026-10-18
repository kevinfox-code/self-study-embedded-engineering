% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Sections 3.1 and 5.4
% Description: Numerically linearize the vehicle about a constant speed.
%
%   [a, b, u0] = cruise_linearize(p, v0, u0)
%
%   Near (v0, u0) the vehicle behaves like  d(dv)/dt = a dv + b du,
%   i.e. the first-order transfer function  b / (s - a).
%
%   If u0 is omitted it is solved for as the trim throttle that holds v0 on
%   a level road. Both partial derivatives use central differences.

function [a, b, u0] = cruise_linearize(p, v0, u0)
  if nargin < 3
    u0 = fzero(@(u) cruise_accel(v0, u, p), [0 1]);
  end

  dv = 0.1;
  du = 0.01;
  a = (cruise_accel(v0 + dv, u0, p) - cruise_accel(v0 - dv, u0, p)) / (2*dv);
  b = (cruise_accel(v0, u0 + du, p) - cruise_accel(v0, u0 - du, p)) / (2*du);
end
