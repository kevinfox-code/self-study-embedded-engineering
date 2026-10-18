% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Sections 1.4 and 3.1
% Description: State-space matrices of the linearized cruise control loop.
%
%   [A, B] = cruise_closed_loop(a, b, c)
%
%   States x = [xc; ue; dv]:
%     xc  controller state      xc' = -eps xc + e
%     ue  engine throttle       ue' = ae (u - ue)
%     dv  speed deviation       dv' = a dv + b ue
%   with e = r - dv and u = kp e + (ki - kp eps) xc, so that the controller
%   transfer function is (kp s + ki)/(s + eps). Input is the reference r.

function [A, B] = cruise_closed_loop(a, b, c)
  kc = c.ki - c.kp * c.eps;

  A = [ -c.eps,    0,      -1;
         c.ae*kc, -c.ae,   -c.ae*c.kp;
         0,        b,       a ];
  B = [ 1; c.ae*c.kp; 0 ];
end
