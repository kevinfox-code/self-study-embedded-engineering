% SPDX-License-Identifier: MIT
% Author:      Kevin Fox
% Book:        Astrom & Murray, Feedback Systems (AM08), Sections 1.5 and 3.1
% Description: PI controller settings used by the Figure 1.13 simulation.
%
%   c = cruise_pi(kp, ki)
%
%   The companion-site code for Figure 1.13 uses a PI controller with a
%   low-frequency pole instead of a pure integrator,
%
%       C(s) = (kp s + ki) / (s + eps),   eps = 0.01 ki / kp,
%
%   driving a first-order engine lag  ae / (s + ae)  with ae = 2 rad/s.

function c = cruise_pi(kp, ki)
  c.kp  = kp;
  c.ki  = ki;
  c.eps = 0.01 * ki / kp;   % low-frequency pole [rad/s]
  c.ae  = 2;                % engine lag bandwidth [rad/s]
end
