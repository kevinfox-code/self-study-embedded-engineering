%% svpwm.m — Space Vector PWM
%  Standard seven-segment, center-aligned SVPWM.
%
%  [Ta, Tb, Tc, sector] = svpwm(V_alpha, V_beta, Vdc)
%
%  Inputs:
%    V_alpha, V_beta — stationary αβ voltage reference [V]
%    Vdc             — DC bus voltage [V]
%
%  Outputs:
%    Ta, Tb, Tc      — duty cycles ∈ [0,1] for phases A, B, C
%    sector          — active sector 1..6

function [Ta, Tb, Tc, sector] = svpwm(V_alpha, V_beta, Vdc)

  % Normalisation factor: maps |V| = Vdc/sqrt(3) → dwell sum = 1
  Vn = 2 / (sqrt(3) * Vdc);

  % Project onto three phase axes (60° apart) to identify sector
  Ua =  V_alpha;
  Ub = -0.5*V_alpha + (sqrt(3)/2)*V_beta;
  Uc = -0.5*V_alpha - (sqrt(3)/2)*V_beta;

  % Sector identification (sign pattern of Ua, Ub, Uc)
  if     Ua >= 0 && Ub <  0 && Uc <  0;  sector = 1;
  elseif Ua >= 0 && Ub >= 0 && Uc <  0;  sector = 2;
  elseif Ua <  0 && Ub >= 0 && Uc <  0;  sector = 3;
  elseif Ua <  0 && Ub >= 0 && Uc >= 0;  sector = 4;
  elseif Ua <  0 && Ub <  0 && Uc >= 0;  sector = 5;
  else;                                   sector = 6;
  end

  % Active vector dwell times via adjacent phase projections
  switch sector
    case 1;  T1 = Vn*( Ua - Ub);  T2 = Vn*( Ub - Uc);
    case 2;  T1 = Vn*( Ub - Ua);  T2 = Vn*(-Uc + Ua);
    case 3;  T1 = Vn*( Ub - Uc);  T2 = Vn*(-Ua + Uc);
    case 4;  T1 = Vn*( Uc - Ub);  T2 = Vn*( Ua - Ub);
    case 5;  T1 = Vn*( Uc - Ua);  T2 = Vn*(-Ub + Ua);
    case 6;  T1 = Vn*(-Ua + Uc);  T2 = Vn*( Ua - Ub);
    otherwise; T1 = 0; T2 = 0;
  end

  % Clamp and handle overmodulation by proportional scaling
  T1 = max(0, min(1, T1));
  T2 = max(0, min(1, T2));
  if T1 + T2 > 1
    sc = 1 / (T1 + T2);
    T1 = T1 * sc;
    T2 = T2 * sc;
  end
  T0 = 1 - T1 - T2;

  % Map to per-phase compare values (symmetric / center-aligned placement)
  switch sector
    case 1;  Ta = T0/2+T1+T2;  Tb = T0/2+T2;     Tc = T0/2;
    case 2;  Ta = T0/2+T1;     Tb = T0/2+T1+T2;  Tc = T0/2;
    case 3;  Ta = T0/2;        Tb = T0/2+T1+T2;  Tc = T0/2+T2;
    case 4;  Ta = T0/2;        Tb = T0/2+T1;     Tc = T0/2+T1+T2;
    case 5;  Ta = T0/2+T2;     Tb = T0/2;        Tc = T0/2+T1+T2;
    case 6;  Ta = T0/2+T1+T2;  Tb = T0/2;        Tc = T0/2+T1;
    otherwise; Ta = 0.5; Tb = 0.5; Tc = 0.5;
  end

  % Final clamp
  Ta = max(0, min(1, Ta));
  Tb = max(0, min(1, Tb));
  Tc = max(0, min(1, Tc));

end
